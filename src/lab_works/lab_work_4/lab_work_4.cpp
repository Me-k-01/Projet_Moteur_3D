#include "lab_work_4.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
//#define MODEL_1 // Utilisation du modèle du lapin de la partie 1 du TP

namespace M3D_ISICG
{
	const std::string LabWork4::_shaderFolder = "src/lab_works/lab_work_4/shaders/";

	LabWork4::~LabWork4() {
		glDeleteProgram( _program );
		_mesh_model.cleanGL();
		glDisable(GL_DEPTH_TEST);
	}

	// Creation du shader
	GLuint LabWork4::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWork4::_testShader() { 
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
	 
	void LabWork4::_initCamera() {  
		#ifdef MODEL_1
			_camera.setPosition(Vec3f(0, 1, 3));
		#else
			_camera.setPosition(Vec3f(1, 1, 0));
		#endif
		_updateCameraUniform();
	}

	
	bool LabWork4::init() {
		std::cout << "Initializing lab work 4..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable(GL_DEPTH_TEST);
		
		////////////////////////// Shaders ////////////////////////////////
		vertShader = _createShader( GL_VERTEX_SHADER , "mesh.vert" );
		fragShader = _createShader(GL_FRAGMENT_SHADER, "mesh.frag");
		// Test de compillation des shaders
		if ( !_testShader() ) {
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
		_uLumPos = glGetUniformLocation(_program, "uLumPos");
		 
		 
		// Position de la lumière  
		_lumPos = Vec3f(1, 1, 0); // Lumière au centre de la salle de conférence 

		///////////////////////////// Démarage du programme /////////////////////////////////
		glUseProgram(_program); 

		///////////////////////////// Mesh /////////////////////////////////
		#ifdef MODEL_1
			_mesh_model.load("bunny", "data/models/bunny.obj");
		#else
			_mesh_model.load("conference", "data/models/conference.obj");
			_mesh_model._transformation = glm::scale(_mesh_model._transformation, Vec3f(0.003f));
		#endif			 
		_mesh_model.uniformSetUp(_program); 
		
		_initCamera();

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWork4::animate( const float p_deltaTime ) {
		//_lumPos = Vec3f(_time/2, 1, 0);
		//glProgramUniform3fv(_program, _uLumPos, 1, glm::value_ptr(_lumPos)); 
		//std::cout << _time << std::endl;
		_time += p_deltaTime;
	}

	void LabWork4::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  
		_mesh_model.render(_program);
	}
	 
	// Update les variables uniformes qui dépendent de la caméra
	void LabWork4::_updateCameraUniform() { 
		Mat4f mvp = _camera.getProjectionMatrix() * _camera.getViewMatrix() * _mesh_model._transformation;
		glProgramUniformMatrix4fv(_program, _uMVPMatrix, 1, false, glm::value_ptr(mvp));

		Mat4f mvMatrix = _camera.getViewMatrix() * _mesh_model._transformation;
		glProgramUniformMatrix4fv(_program, _uMVMatrix, 1, false, glm::value_ptr(mvMatrix)); 

		Mat4f normalMatrix = glm::transpose(glm::inverse(mvMatrix));
		glProgramUniformMatrix4fv(_program, _uNormalMatrix, 1, false, glm::value_ptr(normalMatrix));  

		Vec3f lum = Vec3f(_camera.getViewMatrix() * Vec4f(_lumPos, 1.f));
		glProgramUniform3fv(_program, _uLumPos, 1, glm::value_ptr(lum));
	} 


	void LabWork4::displayUI() {
		ImGui::Begin( "Settings lab work 4" );
		if (ImGui::ColorEdit3("Couleur de fond", glm::value_ptr(_bgColor))) {
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		}
		if (ImGui::SliderFloat("fovy", &_fovy, 0.f, 180.f)) {
			_camera.setFovy(_fovy);
			_updateCameraUniform();
		}
		ImGui::End();
	}

	void LabWork4::handleEvents( const SDL_Event & p_event ) {
		if (p_event.type == SDL_KEYDOWN) {
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
				default: 
					break;
			}
			_updateCameraUniform(); // Update de l'emplacement de la camera + lumière
		}

	 	

		// Rotate when left click + motion (if not on Imgui widget).
		if ( p_event.type == SDL_MOUSEMOTION && p_event.motion.state & SDL_BUTTON_LMASK
			 && !ImGui::GetIO().WantCaptureMouse ) {
			_camera.rotate( p_event.motion.xrel * _cameraSensitivity, p_event.motion.yrel * _cameraSensitivity ); 
			_updateCameraUniform();
		} 	
	}


} // namespace M3D_ISICG
