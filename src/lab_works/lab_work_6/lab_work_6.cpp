#include "lab_work_6.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

namespace M3D_ISICG
{
	const std::string LabWork6::_shaderFolder = "src/lab_works/lab_work_6/shaders/";

	LabWork6::~LabWork6() {
		glDeleteProgram( _geometryPassProgram );
		_mesh_model.cleanGL();
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_BLEND);
	}

	// Creation du shader
	GLuint LabWork6::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWork6::_testShader(GLuint shader) { 
		GLint compiled;
		glCompileShader( shader );
		glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled ) {
			GLchar log[ 1024 ];
			glGetShaderInfoLog( shader, sizeof( log ), NULL, log );
			std::cerr << " Error compiling vertex shader : " << log << std ::endl; 
			return false;
		}
		return true;
	}
	void LabWork6::_initCamera() {    
		_camera.setPosition(Vec3f(0, 0, 0.2));  // Vec3f(0, 1, 3)
		_updateCameraUniform();
	}	 
	void LabWork6::_initGBuffer() {
		// I/ 2.1 Creation du FBO
		glCreateFramebuffers(1, &_fboId);

		// I/ 2.2 Creation des textures (5 texture RGB et une texture de profondeur)
		glCreateTextures(GL_TEXTURE_2D, 6, _gBufferTextures); 
		for (int i = 0; i < 5; i++) {
			glTextureStorage2D(_gBufferTextures[i], 1, GL_RGBA32F, _windowWidth, _windowHeight);
		}
		glTextureStorage2D(_gBufferTextures[5], 1, GL_DEPTH_COMPONENT32F, _windowWidth, _windowHeight);
		 
		// I/ 2.3 Liez les textures et le FBO  
		/*for (int i = 0; i < 5; i++) {
			glNamedFramebufferTexture(_fboId, drawBuffer[i], _gBufferTextures[i], 0);
		}*/
		glNamedFramebufferTexture(_fboId, GL_COLOR_ATTACHMENT0, _gBufferTextures[0], 0);
		glNamedFramebufferTexture(_fboId, GL_COLOR_ATTACHMENT1, _gBufferTextures[1], 0);
		glNamedFramebufferTexture(_fboId, GL_COLOR_ATTACHMENT2, _gBufferTextures[2], 0);
		glNamedFramebufferTexture(_fboId, GL_COLOR_ATTACHMENT3, _gBufferTextures[3], 0);
		glNamedFramebufferTexture(_fboId, GL_COLOR_ATTACHMENT4, _gBufferTextures[4], 0);
		glNamedFramebufferTexture(_fboId, GL_DEPTH_ATTACHMENT , _gBufferTextures[5], 0);

		// I/ 2.3 Associer les texture à la sortie du fragment shader
		GLenum drawBuffer[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4
		};
		glNamedFramebufferDrawBuffers(_fboId, 5, drawBuffer);
		// I/ 2.4 Vérifier la validité 
		if (glCheckNamedFramebufferStatus(_fboId, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Erreur de FBO" << std::endl;
		}
	}
	bool LabWork6::_initGeometryPassProgram() { // Initialisation de geometryPassProgram
		// III Le programme / 1
		_geometryPassProgram = glCreateProgram();
		
		///////////////////////////// Recupération des shaders ///////////////////////////////// 
		GLuint vertShader = _createShader( GL_VERTEX_SHADER , "geometry_pass.vert" );
		GLuint fragShader = _createShader( GL_FRAGMENT_SHADER , "geometry_pass.frag" );

		// Test compillation vertex shader
		if ( (!_testShader(vertShader)) || !_testShader(fragShader) ) {
			glDeleteShader( vertShader );
			glDeleteShader( fragShader );
			return false;
		}
		glAttachShader( _geometryPassProgram, vertShader );
		glAttachShader( _geometryPassProgram, fragShader );
		glLinkProgram( _geometryPassProgram ); 
		// Check if link is ok .
		GLint linked;
		glGetProgramiv( _geometryPassProgram, GL_LINK_STATUS, &linked );
		if ( !linked ) {
			GLchar log[ 1024 ];
			glGetProgramInfoLog( _geometryPassProgram, sizeof( log ), NULL, log );
			std ::cerr << " Error linking program : " << log << std ::endl;
			return false;
		}
		// shader inutile une fois lié au programme
		glDeleteShader( vertShader );
		glDeleteShader( fragShader );

		///////////////////////////// Variable uniforme /////////////////////////////////
		_uMVPMatrix = glGetUniformLocation(_geometryPassProgram, "uMVPMatrix");
		_uNormalMatrix = glGetUniformLocation(_geometryPassProgram, "uNormalMatrix");
		_uMVMatrix = glGetUniformLocation(_geometryPassProgram, "uMVMatrix"); 
		_uLumPos = glGetUniformLocation(_geometryPassProgram, "uLumPos"); 
		
		return true;
	}

	bool LabWork6::init() {
		std::cout << "Initializing lab work 6..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		 
		// I / 2.0 Creation du G-buffer
		_initGBuffer();
		// III / 2. Méthode permettant d'initialiser geometryPassProgram
		if (!_initGeometryPassProgram())
			return false;   
		 
		// Position de la lumière  
		_lumPos = Vec3f(1, 1, 0); 
		glProgramUniform3fv(_geometryPassProgram, _uLumPos, 1, glm::value_ptr(Vec3f(_camera.getViewMatrix() * Vec4f(_lumPos, 1))));

		// Démarage du programme 
		glUseProgram(_geometryPassProgram);

		///////////////////////////// Mesh /////////////////////////////////
		_mesh_model.load("bunny2", "data/models/bunny_2.obj");
		_mesh_model.uniformSetUp(_geometryPassProgram);

		_initCamera();

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWork6::animate( const float p_deltaTime ) {
		_lumPos = _camera.getPosition();
		Vec3f lumPosView = Vec3f(_camera.getViewMatrix() * Vec4f(_lumPos, 1.f));
		glProgramUniform3fv(_geometryPassProgram, _uLumPos, 1, glm::value_ptr(lumPosView)); 
	}

	void LabWork6::_geometryPass() { 
		glUseProgram(_geometryPassProgram);
		glDisable(GL_DEPTH_TEST);
		// Nettoyer le buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  
		// IV / 2. Indiquer au fragment shader d'écrire dans le fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboId);
		// Nettoyer le buffer 
		glClear(GL_COLOR_BUFFER_BIT);

		// IV / 3. Verifier que cela fonctionne en copiant une des valeurs du buffer dans GL_DRAW_FRAMEBUFFER
		// Selection du buffer à lire
		GLenum drawBuffer[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4
		};
		glNamedFramebufferReadBuffer(_fboId, drawBuffer[_currDrawBuffer]); 
		_mesh_model.render(_geometryPassProgram);  
		// Copie du buffer src (_fboId) vers destination (buffer par defaut = 0)
		glBlitNamedFramebuffer(_fboId, 0, 
			0, 0, _windowWidth, _windowHeight, 
			0, 0, _windowWidth, _windowHeight, 
			GL_COLOR_BUFFER_BIT, // maque soit GL_COLOR_BUFFER_BIT ou GL_DEPTH_BUFFER_BIT 
			GL_LINEAR // Filtre
		);
		// IV / 2. remettre le framebuffer à 0
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  

		glEnable(GL_DEPTH_TEST);
	}

	// Update les differentes matrices model / vue / projection
	void LabWork6::_updateCameraUniform() { 
		Mat4f mvp = _camera.getProjectionMatrix() * _camera.getViewMatrix() * _mesh_model._transformation;
		glProgramUniformMatrix4fv(_geometryPassProgram, _uMVPMatrix, 1, false, glm::value_ptr(mvp));
		Mat4f mvMatrix = _camera.getViewMatrix() * _mesh_model._transformation; 
		Mat4f normalMatrix = glm::transpose(glm::inverse(mvMatrix));
		glProgramUniformMatrix4fv(_geometryPassProgram, _uNormalMatrix, 1, false, glm::value_ptr(normalMatrix));  
		glProgramUniformMatrix4fv(_geometryPassProgram, _uMVMatrix, 1, false, glm::value_ptr(mvMatrix));  
		//std::cout << "testA" << std::endl; 
	} 

	void LabWork6::render() { 
		// IV / 1. Appeler les shaders de la geometry pass
		_geometryPass(); 
	}

	void LabWork6::displayUI()
	{
		ImGui::Begin( "Settings lab work 6" );
		   
		const char* const c[] = {
			"GL_COLOR_ATTACHMENT0",
			"GL_COLOR_ATTACHMENT1",
			"GL_COLOR_ATTACHMENT2",
			"GL_COLOR_ATTACHMENT3",
			"GL_COLOR_ATTACHMENT4"
		};
		if (ImGui::BeginListBox("Afficher texture")) {
			ImGui::ListBox("test", &_currDrawBuffer, c, 5);
			ImGui::EndListBox();
		} 
		/*
		if (ImGui::ColorEdit3("Couleur de fond", glm::value_ptr(_bgColor))) {
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		}
		if (ImGui::SliderFloat("fovy", &_fovy, 0.f, 180.f)) {
			_camera.setFovy(_fovy);
		} 
		*/
		ImGui::End();
	}
	void LabWork6::handleEvents( const SDL_Event & p_event ) {
		if ( p_event.type == SDL_KEYDOWN ) {
			switch ( p_event.key.keysym.scancode ) {
				case SDL_SCANCODE_W: // Front
					_camera.moveFront( _cameraSpeed ); 
					break;
				case SDL_SCANCODE_S: // Back
					_camera.moveFront( -_cameraSpeed ); 
					break;
				case SDL_SCANCODE_A: // Left
					_camera.moveRight( -_cameraSpeed ); 
					break;
				case SDL_SCANCODE_D: // Right
					_camera.moveRight( _cameraSpeed ); 
					break;
				case SDL_SCANCODE_R: // Up
					_camera.moveUp( _cameraSpeed ); 
					break;
				case SDL_SCANCODE_F: // Bottom
					_camera.moveUp( -_cameraSpeed ); 
					break;
				default: break;
			}
			_updateCameraUniform();
		}

		// Rotate when left click + motion (if not on Imgui widget).
		if ( p_event.type == SDL_MOUSEMOTION && p_event.motion.state & SDL_BUTTON_LMASK
			 && !ImGui::GetIO().WantCaptureMouse ) {
			_camera.rotate( p_event.motion.xrel * _cameraSensitivity, p_event.motion.yrel * _cameraSensitivity );
			_updateCameraUniform();
		}
	}


} // namespace M3D_ISICG
