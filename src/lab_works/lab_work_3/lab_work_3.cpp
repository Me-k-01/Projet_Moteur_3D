#include "lab_work_3.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

namespace M3D_ISICG
{
	const std::string LabWork3::_shaderFolder = "src/lab_works/lab_work_3/shaders/";

	LabWork3::~LabWork3() {
		glDeleteProgram( _program );
		_destroyCube();
		glDisable(GL_DEPTH_TEST);
	}

	// Creation du shader
	GLuint LabWork3::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		//glShaderSource( shader, 1, (GLchar **)shaderSrc.c_str(), NULL );
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWork3::_testShader() { 
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
	 
	
	bool LabWork3::init()
	{
		std::cout << "Initializing lab work 3..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable(GL_DEPTH_TEST);
		
		////////////////////////// Shaders ////////////////////////////////
		 
		vertShader = _createShader( GL_VERTEX_SHADER , "lw3.vert" );
		fragShader = _createShader( GL_FRAGMENT_SHADER , "lw3.frag" );

		// Test compillation vertex shader
		if ( !_testShader() ) {
			glDeleteShader( vertShader );
			glDeleteShader( fragShader );
			return false;
		}
		
		////////////////////////// Création du programe ////////////////////////// 
		_lum = 1; 
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
		// shader inutile une fois lié au programme
		glDeleteShader( vertShader );
		glDeleteShader( fragShader );
		 		

		///////////////////////////// Variable de controle /////////////////////////////////
		_uLum = glGetUniformLocation(_program, "uLum");
		glProgramUniform1f(_program, _uLum, _lum); 
		_uMVPMatrix = glGetUniformLocation(_program, "uMVPMatrix");
			
		///////////////////////////// Démarage du programme /////////////////////////////////
		glUseProgram(_program);
		std::cout << "\nprogram:\n";
		std::cout << _program;
		///////////////////////////// Mesh /////////////////////////////////
		_createCube();
		_initCamera();
		//camera.setLookAt(Vec3f(0, 0, 0));
		

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWork3::_createCube() {
		_mesh.vertPos = { 
			Vec3f(-0.5, 0.5, -0.5), Vec3f(0.5, 0.5, -0.5), Vec3f(0.5, -0.5, -0.5), Vec3f(-0.5, -0.5, -0.5),
			Vec3f(-0.5, 0.5, 0.5),  Vec3f(0.5, 0.5, 0.5),  Vec3f(0.5, -0.5, 0.5),  Vec3f(-0.5, -0.5, 0.5)
		};
		_mesh.vertInd = { 
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			0, 4, 7, 0, 7, 3,
			1, 5, 6, 1, 6, 2,
			7, 2, 3, 7, 6, 2,
			4, 5, 1, 4, 1, 0
		};
				
		for (int i = 0; i < _mesh.vertPos.size(); i++)
			_mesh.vertCols.push_back(getRandomVec3f());
		
		_mesh.matTransform = glm::scale(_mesh.matTransform, Vec3f(0.8f)); 

		_initBuffers();
	}

	void LabWork3::_initBuffers() {
		glCreateBuffers(1, &_mesh.ebo);
		glCreateBuffers(1, &_mesh.vboPos);
		glCreateBuffers(1, &_mesh.vboCols);
		glCreateVertexArrays(1, &_mesh.vao);

		glNamedBufferData(_mesh.ebo,     _mesh.vertInd.size()  * sizeof(int),   _mesh.vertInd.data(),  GL_STATIC_DRAW); 
		glNamedBufferData(_mesh.vboPos,  _mesh.vertPos.size()  * sizeof(Vec3f), _mesh.vertPos.data(),  GL_STATIC_DRAW);
		glNamedBufferData(_mesh.vboCols, _mesh.vertCols.size() * sizeof(Vec3f), _mesh.vertCols.data(), GL_STATIC_DRAW);

		// Activation des attributs vbo de vao
		glEnableVertexArrayAttrib( _mesh.vao, 0);
		glEnableVertexArrayAttrib( _mesh.vao, 1);
		// Format de l'attribut d'indice 0, 2 valeur de type float par points, non normalisé, sans offset
		glVertexArrayAttribFormat(_mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(_mesh.vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
		
		// Indice : 0, offset : 0, stride : distance entre les differents éléments dans le buffer (taille du type des éléments du buffer)
		glVertexArrayVertexBuffer(_mesh.vao, 0, _mesh.vboPos,  0.f, sizeof(Vec3f));
		glVertexArrayVertexBuffer(_mesh.vao, 1, _mesh.vboCols, 0.f, sizeof(Vec3f));
		// Indice d'attribut : 0, addresse de bind : 0
		glVertexArrayAttribBinding(_mesh.vao, 0, 0);
		glVertexArrayAttribBinding(_mesh.vao, 1, 1);

		// Lier le ebo avec le vao
		glVertexArrayElementBuffer(_mesh.vao, _mesh.ebo);
		

	}
	void LabWork3::_initCamera() {    
		_camera.setPosition(Vec3f(0, 1, 3)); 
		_updateMVPMatrix();
	}

	void LabWork3::_destroyCube() {
		glDisableVertexArrayAttrib(_mesh.vao, 0); 
		glDisableVertexArrayAttrib(_mesh.vao, 1); 
		glDeleteBuffers(1, &_mesh.ebo);
		glDeleteBuffers(1, &_mesh.vboPos);
		glDeleteBuffers(1, &_mesh.vboCols);
		glDeleteVertexArrays(1, {&_mesh.vao});
	}

	void LabWork3::animate( const float p_deltaTime ) {
		//glProgramUniform1f(_program, _uTranslationX, glm::sin(_time)/2); 
		//_uTransfoMat = glm::rotate(_uTransfoMat, Vec3f(0, glm::sin(_time), glm::sin(_time)));
		//_mesh.matTransform = glm::rotate(_mesh.matTransform, glm::radians(p_deltaTime), Vec3f(0, 1, 1));
		//_time += p_deltaTime;
	}

	void LabWork3::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
		glBindVertexArray(_mesh.vao);
		glDrawElements(GL_TRIANGLES, _mesh.vertInd.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		_updateMVPMatrix(); 
	}
	 

	void LabWork3::_updateMVPMatrix() {
		Mat4f mvp = _camera.getProjectionMatrix() * _camera.getViewMatrix() * _mesh.matTransform;
		glProgramUniformMatrix4fv(_program, _uMVPMatrix, 1, false, glm::value_ptr(mvp)); 
	} 


	void LabWork3::displayUI()
	{
		ImGui::Begin( "Settings lab work 2" );
		if (ImGui::SliderFloat("Luminosité", &_lum, 0.f, 1.f)) {
			glProgramUniform1f(_program, _uLum, _lum); 
		}
		if (ImGui::ColorEdit3("Couleur de fond", glm::value_ptr(_bgColor))) {
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		}
		
		if (ImGui::SliderFloat("fovy", &_fovy, 0.f, 180.f)) {
			_camera.setFovy(_fovy);
		}
		ImGui::End();
	}
	void LabWork3::handleEvents( const SDL_Event & p_event ) {
		if ( p_event.type == SDL_KEYDOWN ) {
			switch ( p_event.key.keysym.scancode ) {
			case SDL_SCANCODE_W: // Front
				_camera.moveFront( _cameraSpeed );
				_updateMVPMatrix();
				break;
			case SDL_SCANCODE_S: // Back
				_camera.moveFront( -_cameraSpeed );
				_updateMVPMatrix();
				break;
			case SDL_SCANCODE_A: // Left
				_camera.moveRight( -_cameraSpeed );
				_updateMVPMatrix();
				break;
			case SDL_SCANCODE_D: // Right
				_camera.moveRight( _cameraSpeed );
				_updateMVPMatrix();
				break;
			case SDL_SCANCODE_R: // Up
				_camera.moveUp( _cameraSpeed );
				_updateMVPMatrix();
				break;
			case SDL_SCANCODE_F: // Bottom
				_camera.moveUp( -_cameraSpeed );
				_updateMVPMatrix();
				break;
			default: break;
			}
		}

		// Rotate when left click + motion (if not on Imgui widget).
		if ( p_event.type == SDL_MOUSEMOTION && p_event.motion.state & SDL_BUTTON_LMASK
			 && !ImGui::GetIO().WantCaptureMouse )
		{
			_camera.rotate( p_event.motion.xrel * _cameraSensitivity, p_event.motion.yrel * _cameraSensitivity );
			_updateMVPMatrix();
		}
	}


} // namespace M3D_ISICG
