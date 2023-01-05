#include "lab_work_2.hpp"
#include "imgui.h"
#include "utils/read_file.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

namespace M3D_ISICG
{
	const std::string LabWork2::_shaderFolder = "src/lab_works/lab_work_2/shaders/";

	LabWork2::~LabWork2() {
		glDeleteProgram( _program );
		glDisableVertexArrayAttrib(_vao, 0); 
		glDisableVertexArrayAttrib(_vao, 1); 
		glDeleteBuffers(1, &_ebo);
		glDeleteBuffers(1, &_vboPos);
		glDeleteBuffers(1, &_vboCols);
		glDeleteVertexArrays(1, {&_vao});
	}

	// Creation du shader
	GLuint LabWork2::_createShader( GLenum shader_type, std::string fileName ) {
		const std::string shaderSrc	 = readFile( _shaderFolder + fileName ); // lecture du fichier
		GLuint shader = glCreateShader( shader_type );
		const GLchar * src = shaderSrc.c_str();
		//glShaderSource( shader, 1, (GLchar **)shaderSrc.c_str(), NULL );
		glShaderSource( shader, 1, &src, NULL );
		return shader;
	} 
	
	// Test de compillation des shaders
	bool LabWork2::_testShader() { 
		GLint compiled;
		glCompileShader( _vertShader );
		glGetShaderiv( _vertShader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled ) {
			GLchar log[ 1024 ];
			glGetShaderInfoLog( _vertShader, sizeof( log ), NULL, log );
			std::cerr << " Error compiling vertex shader : " << log << std ::endl; 
			return false;
		}
		glCompileShader( _fragShader );
		glGetShaderiv( _fragShader, GL_COMPILE_STATUS, &compiled );
		if (!compiled) {
			GLchar log[ 1024 ];
			glGetShaderInfoLog( _fragShader, sizeof( log ), NULL, log );
			std::cerr << " Error compiling fragment shader : " << log << std ::endl; 
			return false;
		}
		return true;
	}
	 
	
	bool LabWork2::init() {
		std::cout << "Initializing lab work 2..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		 
		////////////////////////// Shaders ////////////////////////////////
		 
		_vertShader = _createShader( GL_VERTEX_SHADER , "lw2.vert" );
		_fragShader = _createShader( GL_FRAGMENT_SHADER , "lw2.frag" );

		// Test compillation vertex shader
		if ( !_testShader() ) {
			glDeleteShader( _vertShader );
			glDeleteShader( _fragShader );
			return false;
		}
		
		////////////////////////// Création du programe //////////////////////////
		_time = 0;
		_lum = 1; 
		_program = glCreateProgram();

		glAttachShader( _program, _vertShader );
		glAttachShader( _program, _fragShader );

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
		glDeleteShader( _vertShader );
		glDeleteShader( _fragShader );
		 
		////////////////////////// EBO ///////////////////////////////////

		_poly = { Vec2f(-0.5, 0.5), Vec2f(0.5, 0.5), Vec2f(0.5, -0.5), Vec2f(-0.5, -0.5)};
		// 1.4. stockage des indices de sommets formant les deux triangles
		_polyTri = { 0, 1, 2, 0, 2, 3 }; 
		// 2.1 stockage des couleurs des sommets
		_cols = { Vec4f(1, 0, 0, 1), Vec4f(0, 1, 0, 1), Vec4f(0, 0, 1, 1), Vec4f(1, 0, 1, 1)};


		glCreateBuffers(1, &_ebo);
		// 1.6. remplir EBO 
		glNamedBufferData(_ebo, _polyTri.size() * sizeof(int), _polyTri.data(), GL_STATIC_DRAW); 

		///////////////////////////// Attribut de VAO /////////////////////////////////

		glCreateBuffers(1, &_vboPos);
		glNamedBufferData(_vboPos, _poly.size() * sizeof(Vec2f), _poly.data(), GL_STATIC_DRAW);
		//glVertexArrayAttribBinding(_vao, 0, 0);
		
		glCreateBuffers(1, &_vboCols);
		glNamedBufferData(_vboCols, _cols.size() * sizeof(Vec4f), _cols.data(), GL_STATIC_DRAW);
		//glVertexArrayAttribBinding(_vboCols, 0, 0);
		
		glCreateVertexArrays(1, &_vao);

		// activation des attributs de vbo
		glEnableVertexArrayAttrib( _vao, 0);
		glEnableVertexArrayAttrib( _vao, 1);
		// format de l'attribut d'indice 0, 2 valeur de type float par points, non normalisé, sans offset
		glVertexArrayAttribFormat(_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, 0);
		
		// Indice : 0, offset : 0, stride : distance entre les differents éléments dans le buffer (taille du type des éléments du buffer)
		glVertexArrayVertexBuffer(_vao, 0, _vboPos, 0.f, sizeof(Vec2f));
		glVertexArrayVertexBuffer(_vao, 1, _vboCols, 0.f, sizeof(Vec4f));
		// Indice d'attribut : 0, addresse de bind : 0
		glVertexArrayAttribBinding(_vao, 0, 0);
		glVertexArrayAttribBinding(_vao, 1, 1);

		// 1.7. lier ebo et vao
		glVertexArrayElementBuffer(_vao, _ebo);
				 
		///////////////////////////// Variable de controle /////////////////////////////////
		_uTranslationX = glGetUniformLocation(_program, "uTranslationX");
		_uLum = glGetUniformLocation(_program, "uLum");
		glProgramUniform1f(_program, _uLum, _lum); 
		
		///////////////////////////// Démarage du programme /////////////////////////////////
		glUseProgram(_program);

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWork2::animate( const float p_deltaTime ) {
		glProgramUniform1f(_program, _uTranslationX, glm::sin(_time)/2); 
		_time += p_deltaTime;
	}

	void LabWork2::render() {
		glClear(GL_COLOR_BUFFER_BIT ); 
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, _polyTri.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void LabWork2::handleEvents( const SDL_Event & p_event ) {}

	void LabWork2::displayUI() {
		ImGui::Begin( "Settings lab work 2" );
		if (ImGui::SliderFloat("Luminosité", &_lum, 0.f, 1.f)) {
			glProgramUniform1f(_program, _uLum, _lum); 
		}
		if (ImGui::ColorEdit3("Couleur de fond", glm::value_ptr(_bgColor))) {
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		}
		ImGui::End();
	}
} // namespace M3D_ISICG
