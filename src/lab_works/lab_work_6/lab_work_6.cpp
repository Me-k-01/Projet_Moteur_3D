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

	bool LabWork6::_initShadingPassProgram() { // Initialisation du shading pass program
		
		_shadingPassProgram = glCreateProgram();

		///////////////////////////// Recupération des shaders ///////////////////////////////// 
		GLuint shadingFragShader = _createShader( GL_FRAGMENT_SHADER , "shading_pass.frag" );
		if (!_testShader(shadingFragShader)) {
			glDeleteShader( shadingFragShader );
			return false;
		}

		glAttachShader( _shadingPassProgram, shadingFragShader );
		glLinkProgram( _shadingPassProgram ); 
		// Check if link is ok .
		GLint linked;
		glGetProgramiv( _shadingPassProgram, GL_LINK_STATUS, &linked );
		if ( !linked ) {
			GLchar log[ 1024 ];
			glGetProgramInfoLog( _shadingPassProgram, sizeof( log ), NULL, log );
			std ::cerr << " Error linking shading program : " << log << std ::endl;
			return false;
		}
		glDeleteShader( shadingFragShader );

		_uMVMatrix = glGetUniformLocation(_shadingPassProgram, "uMVMatrix"); 
		_uLumPos = glGetUniformLocation(_shadingPassProgram, "uLumPos"); 
	}
	bool LabWork6::_initGeometryPassProgram() { // Initialisation de geometryPassProgram
		// III Le programme / 1
		_geometryPassProgram = glCreateProgram(); 
		
		///////////////////////////// Recupération des shaders ///////////////////////////////// 
		GLuint geometryVertShader = _createShader( GL_VERTEX_SHADER , "geometry_pass.vert" );
		GLuint geometryFragShader = _createShader( GL_FRAGMENT_SHADER , "geometry_pass.frag" ); 

		// Test compillation vertex shader
		if ( (!_testShader(geometryVertShader)) || !_testShader(geometryFragShader) ) {
			glDeleteShader( geometryVertShader );
			glDeleteShader( geometryFragShader ); 
			return false;
		}
		glAttachShader( _geometryPassProgram, geometryVertShader );
		glAttachShader( _geometryPassProgram, geometryFragShader );
		glLinkProgram( _geometryPassProgram ); 
		// Check if link is ok .
		GLint linked;
		glGetProgramiv( _geometryPassProgram, GL_LINK_STATUS, &linked );
		if ( !linked ) {
			GLchar log[ 1024 ];
			glGetProgramInfoLog( _geometryPassProgram, sizeof( log ), NULL, log );
			std ::cerr << " Error linking geometry program : " << log << std ::endl;
			return false;
		}
		 

		// shader inutile une fois lié au programme
		glDeleteShader( geometryVertShader );
		glDeleteShader(geometryFragShader);



		///////////////////////////// Variable uniforme /////////////////////////////////
		_uMVPMatrix = glGetUniformLocation(_geometryPassProgram, "uMVPMatrix");
		_uNormalMatrix = glGetUniformLocation(_geometryPassProgram, "uNormalMatrix"); 
		
		return true;
	}

	bool LabWork6::init() {
		std::cout << "Initializing lab work 6..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		 
		// I / 2.0 Creation du G-buffer
		_initGBuffer();
		// III / 2. Méthode permettant d'initialiser les programs
		if (!_initGeometryPassProgram())
			return false;   
		if (!_initShadingPassProgram())
			return false;
		 
		// Position de la lumière  
		_lumPos = Vec3f(1, 1, 0); 
		glProgramUniform3fv(_shadingPassProgram, _uLumPos, 1, glm::value_ptr(Vec3f(_camera.getViewMatrix() * Vec4f(_lumPos, 1))));

		
		///////////////////////////// QUAD /////////////////////////////////
		_poly = {Vec2f(-1, 1), Vec2f(1, 1), Vec2f(1, -1), Vec2f(-1, -1)};
		// 1.4. stockage des indices de sommets formant les deux triangles
		_polyTri = {0, 1, 2, 0, 2, 3};


		glCreateBuffers(1, &_ebo);
		// 1.6. remplir EBO 
		glNamedBufferData(_ebo, _polyTri.size() * sizeof(int), _polyTri.data(), GL_STATIC_DRAW); 

		glCreateBuffers(1, &_vbo);
		glNamedBufferData(_vbo, _poly.size() * sizeof(Vec2f), _poly.data(), GL_STATIC_DRAW);
		//glVertexArrayAttribBinding(_vao, 0, 0);
		 
		glCreateVertexArrays(1, &_vao);

		// activation des attributs de vbo
		glEnableVertexArrayAttrib( _vao, 0);
		glEnableVertexArrayAttrib( _vao, 1);
		// format de l'attribut d'indice 0, 2 valeur de type float par points, non normalisé, sans offset
		glVertexArrayAttribFormat(_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
		
		// Indice : 0, offset : 0, stride : distance entre les differents éléments dans le buffer (taille du type des éléments du buffer)
		glVertexArrayVertexBuffer(_vao, 0, _vbo, 0.f, sizeof(Vec2f)); 
		// Indice d'attribut : 0, addresse de bind : 0
		glVertexArrayAttribBinding(_vao, 0, 0);
		glVertexArrayAttribBinding(_vao, 1, 1);

		// 1.7. lier ebo et vao
		glVertexArrayElementBuffer(_vao, _ebo);




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
		glProgramUniform3fv(_shadingPassProgram, _uLumPos, 1, glm::value_ptr(lumPosView)); 
	}

	// Fonction de rendu
	void LabWork6::_shadingPass() {
		glUseProgram(_shadingPassProgram); 
		// Desactivation du test de profondeur
		glDisable(GL_DEPTH_TEST); 

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
		// Indiquer au fragment shader d'écrire dans le frame buffer par défaut
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  

		// Rendu du carré
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, _polyTri.size(), GL_UNSIGNED_INT, 0);  
		glBindVertexArray(0); 
	}

	void LabWork6::_geometryPass() {  
		glUseProgram(_geometryPassProgram); 
		glEnable(GL_DEPTH_TEST);  
		// IV / 2. Indiquer au fragment shader d'écrire dans le fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboId);
		// Nettoyer le buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );   
		_mesh_model.render(_geometryPassProgram); 

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
		// Copie du buffer src (_fboId) vers destination (buffer par defaut = 0)
		glBlitNamedFramebuffer(_fboId, 0, 
			0, 0, _windowWidth, _windowHeight, 
			0, 0, _windowWidth, _windowHeight, 
			GL_COLOR_BUFFER_BIT, // maque soit GL_COLOR_BUFFER_BIT ou GL_DEPTH_BUFFER_BIT 
			GL_LINEAR // Filtre
		);

		// IV / 2. remettre le framebuffer à 0
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  
		 
	}

	// Update les differentes matrices model / vue / projection
	void LabWork6::_updateCameraUniform() { 
		Mat4f mvp = _camera.getProjectionMatrix() * _camera.getViewMatrix() * _mesh_model._transformation;
		glProgramUniformMatrix4fv(_geometryPassProgram, _uMVPMatrix, 1, false, glm::value_ptr(mvp));
		Mat4f mvMatrix = _camera.getViewMatrix() * _mesh_model._transformation; 
		Mat4f normalMatrix = glm::transpose(glm::inverse(mvMatrix));
		glProgramUniformMatrix4fv(_geometryPassProgram, _uNormalMatrix, 1, false, glm::value_ptr(normalMatrix));  
		glProgramUniformMatrix4fv(_shadingPassProgram, _uMVMatrix, 1, false, glm::value_ptr(mvMatrix));  
	} 

	void LabWork6::render() { 
		// IV / 1. Appeler les shaders de la geometry pass
		_geometryPass(); 
		//_shadingPass(); // L'affichage du lapin sur le quad du shading pass ne fonctionne pas
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
