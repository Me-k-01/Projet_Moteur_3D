#include "lab_work_fin.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream> 
// Nombre de lumières
#define NUM_LIGHTS 3

namespace M3D_ISICG
{
	const std::string LabWorkFin::_shaderFolder = "src/lab_works/lab_work_fin/shaders/";

	LabWorkFin::~LabWorkFin() {
		glDeleteProgram( _program );
		_mesh_model.cleanGL();
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_BLEND);
	}

	// Creation du shader
	GLuint LabWorkFin::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWorkFin::_testShader() { 
		GLint compiled;
		glCompileShader( vertShader );
		glGetShaderiv( vertShader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled ) {
			GLchar log[ 1024 ];
			glGetShaderInfoLog( vertShader, sizeof( log ), NULL, log );
			std::cerr << " Error compiling vertex shader : " << log << std ::endl; 
			return false;
		}
		glCompileShader( fragShader );
		glGetShaderiv( fragShader, GL_COMPILE_STATUS, &compiled );
		if (!compiled) {
			GLchar log[ 1024 ];
			glGetShaderInfoLog( fragShader, sizeof( log ), NULL, log );
			std::cerr << " Error compiling fragment shader : " << log << std ::endl; 
			return false;
		}
		return true;
	}
	 
	void LabWorkFin::_initCamera() {    
		_camera.setPosition(Vec3f(0, 1, 0));  // Vec3f(0, 1, 3)
		_updateCameraUniform();
	}
	
	bool LabWorkFin::init() {

		std::cout << "Initializing lab work final..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		////////////////////////// Shaders ////////////////////////////////
		 
		vertShader = _createShader( GL_VERTEX_SHADER , "mesh_texture.vert" );
		fragShader = _createShader( GL_FRAGMENT_SHADER , "mesh_texture.frag" );

		// Test de compillation des shaders
		if ( !_testShader() ) {
			// On arrête le programme si ont arrive pas à les compiler
			glDeleteShader( vertShader );
			glDeleteShader( fragShader );
			return false;
		}
		
		////////////////////////// Création du programe ////////////////////////// 
		_program = glCreateProgram();
		glAttachShader( _program, vertShader );
		glAttachShader( _program, fragShader );

		glLinkProgram( _program );
		// Check if link is ok .
		GLint linked;
		glGetProgramiv( _program, GL_LINK_STATUS, &linked );
		if ( !linked ) {
			GLchar log[ 1024 ];
			glGetProgramInfoLog( _program, sizeof( log ), NULL, log );
			std ::cerr << " Error linking program : " << log << std ::endl;
			return false;
		}
		// Les shaders sont inutiles une fois lié au programme
		glDeleteShader( vertShader );
		glDeleteShader( fragShader ); 		

		///////////////////////////// Variable de contrôle /////////////////////////////////
		_uMVPMatrix = glGetUniformLocation(_program, "uMVPMatrix");
		_uNormalMatrix = glGetUniformLocation(_program, "uNormalMatrix");
		_uMVMatrix = glGetUniformLocation(_program, "uMVMatrix"); 
		_uLightsPos = glGetUniformLocation(_program, "uLightsPos");
		_uLightsTint = glGetUniformLocation(_program, "uLightsTint");
		// Génération de texture procédural
		_uCurrAmbient = glGetUniformLocation(_program, "uAmbientType");
		_uTime = glGetUniformLocation(_program, "uTime");
		_uRes = glGetUniformLocation(_program, "uResolution");
		_uStrength = glGetUniformLocation(_program, "uStrength");
		 
		///////////////////////////// Démarage du programme /////////////////////////////////
		glUseProgram(_program);

		///////////////////////////// Mesh /////////////////////////////////  
		_mesh_model.load("sponza", "data/models/sponza/sponza.obj");
		//_mesh_model.load("conference", "data/models/conference.obj");
		_mesh_model._transformation = glm::scale(_mesh_model._transformation, Vec3f(0.003f)); // Scaling du modèle
		_mesh_model.uniformSetUp(_program); // Préparation des textures du modèle
		
		_lightsPos = {Vec3f(0, 3, 0), Vec3f(1, 3, 0), Vec3f(0, 3, 1)}; // Lumière RGB
		_lightsTint = { Vec3f(0.8, 0.3, 0.3), Vec3f(0.2, 0.7, 0.8), Vec3f(0.2, 0.8, 0.2)};
		_resolutionNoise = 1.f;
		_strength = 1.f;
		glProgramUniform1f(_program, _uStrength, _strength);
		glProgramUniform2fv(_program, _uRes, 1, glm::value_ptr(Vec2f(_resolutionNoise, _resolutionNoise)));
		_updateLightColors();
		_initCamera();

		// Recupération de la skybox
		//skybox.load();
		
		glProgramUniform2fv(_program, _uRes, 1, glm::value_ptr(Vec2f(_resolutionNoise, _resolutionNoise)));

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWorkFin::_updateLightColors() {
		glProgramUniform3fv(_program, _uLightsTint, 2, reinterpret_cast<GLfloat *>(_lightsTint.data()));
	}


	void LabWorkFin::animate( const float p_deltaTime ) {
		_time += p_deltaTime;
		_lightsPos[0].y = 1 + sin(_time);
		_lightsPos[1].y = 1 + cos(_time);

		glProgramUniform1f(_program, _uTime, _time);
		_updateCameraUniform();
	}

	void LabWorkFin::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );    
		//glBindVertexArray(skyboxVAO);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		_mesh_model.render(_program);  
	}
	 
	 
	// Update les variables uniformes qui dépendent de la caméra
	void LabWorkFin::_updateCameraUniform() { 
		Mat4f mvp = _camera.getProjectionMatrix() * _camera.getViewMatrix() * _mesh_model._transformation;
		glProgramUniformMatrix4fv(_program, _uMVPMatrix, 1, false, glm::value_ptr(mvp));
		Mat4f mvMatrix = _camera.getViewMatrix() * _mesh_model._transformation;
		Mat4f normalMatrix = glm::transpose(glm::inverse(mvMatrix));
		glProgramUniformMatrix4fv(_program, _uNormalMatrix, 1, false, glm::value_ptr(normalMatrix)); 
		glProgramUniformMatrix4fv(_program, _uMVMatrix, 1, false, glm::value_ptr(mvMatrix)); 


		std::vector<Vec3f> lightsPos; 
		for (int i = 0; i < NUM_LIGHTS; i++) {
			lightsPos.push_back(
				Vec3f(_camera.getViewMatrix() * Vec4f(_lightsPos[i], 1))
			);
		}
		// Envoie des positions des lumières au shader
		glProgramUniform3fv(_program, _uLightsPos, 2, reinterpret_cast<GLfloat *>(lightsPos.data()));
	} 


	void LabWorkFin::displayUI(){
		ImGui::Begin("Settings du projet");
		if (ImGui::ColorEdit3("Couleur lumiere 1", glm::value_ptr(_lightsTint[0]))) {
			_updateLightColors();
		}
		if (ImGui::ColorEdit3("Couleur lumiere 2", glm::value_ptr(_lightsTint[1]))) {
			_updateLightColors();
		} 
		if (ImGui::ColorEdit3("Couleur lumiere 3", glm::value_ptr(_lightsTint[2]))) {
			_updateLightColors();
		} 
		
		const char* const c[] = {
			"Normal",
			"Dance floor (Voronoi)", 
			"Psychedelique (Voronoi 2)",
			"Perlin static",
			"Perlin & Voronoi"
		};
		if (ImGui::BeginListBox("Type de texture ambient")) {
			ImGui::ListBox("", &_currAmbient, c, 5);
			glProgramUniform1i(_program, _uCurrAmbient, _currAmbient);
			ImGui::EndListBox();
		}  
		
		if (ImGui::SliderFloat("Intensité", &_strength, 0.f, 1.f)) {
			glProgramUniform1f(_program, _uStrength, _strength);
		}
		if (ImGui::SliderFloat("Taille du bruit", &_resolutionNoise, 0.f, 10.f)) {
			glProgramUniform2fv(_program, _uRes, 1, glm::value_ptr(Vec2f(_resolutionNoise, _resolutionNoise)));
		}

		ImGui::End();
	}
	void LabWorkFin::handleEvents( const SDL_Event & p_event ) {
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
