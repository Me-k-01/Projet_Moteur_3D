#include "lab_work_1.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include <iostream>

namespace M3D_ISICG
{
	const std::string LabWork1::_shaderFolder = "src/lab_works/lab_work_1/shaders/";

	LabWork1::~LabWork1() {
		glDeleteProgram( _program );
		glDisableVertexArrayAttrib(_vao, 0);
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, {&_vao});
	}

	// Creation du shader
	GLuint LabWork1::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		//glShaderSource( shader, 1, (GLchar **)shaderSrc.c_str(), NULL );
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWork1::_testShader() { 
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
	
	bool LabWork1::init() {
		std::cout << "Initializing lab work 1..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );

		// Ex 2
		vertShader = _createShader( GL_VERTEX_SHADER , "lw1.vert" );
		fragShader = _createShader( GL_FRAGMENT_SHADER , "lw1.frag" );

		// Test compillation vertex shader
		if ( !_testShader() ) {
			glDeleteShader( vertShader );
			glDeleteShader( fragShader );
			return false;
		}
		
		// Création du programe
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
		// shaders inutiles une fois liés au programme
		glDeleteShader( vertShader );
		glDeleteShader( fragShader );

		// Ex 3 : init des données
		// triangle : fig 1
		_poly = { Vec2f( -0.5, 0.5 ), Vec2f( 0.5, 0.5 ), Vec2f( 0.5, -0.5 ) };		
		 
		glCreateBuffers(1, &_vbo);
		glNamedBufferData(_vbo, _poly.size() * sizeof(Vec2f), _poly.data(), GL_STATIC_DRAW);

		glCreateVertexArrays(1, &_vao);

		// Activation de l'attribut d'indice 0
		glEnableVertexArrayAttrib( _vao, 0);
		// Format de l'attribut d'indice 0, 2 valeur de type float par points, non normalisé, sans offset
		glVertexArrayAttribFormat(_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		// Indice : 0, offset : 0, stride : distance entre les differents éléments dans le buffer (taille du type des éléments du buffer)
		glVertexArrayVertexBuffer(_vao, 0, _vbo, 0.f, sizeof(Vec2f));
		// Indice d'attribut : 0, addresse de bind : 0
		glVertexArrayAttribBinding(_vao, 0, 0);

		glUseProgram(_program);

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWork1::animate( const float p_deltaTime ) {}

	void LabWork1::render() {
		glClear(GL_COLOR_BUFFER_BIT );

		glBindVertexArray(_vao);
		glDrawArrays(GL_TRIANGLES, 0, _poly.size());
		glBindVertexArray(0);
	}

	void LabWork1::handleEvents( const SDL_Event & p_event ) {}

	void LabWork1::displayUI()
	{
		ImGui::Begin( "Settings lab work 1" );
		ImGui::Text( "No setting available!" );
		ImGui::End();
	}


} // namespace M3D_ISICG
