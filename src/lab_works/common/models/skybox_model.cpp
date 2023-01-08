#include "skybox_model.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "utils/image.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>



namespace M3D_ISICG
{
	SkyboxMesh::SkyboxMesh( const std::string &				  p_name,
								const std::vector<Vertex> &		  p_vertices,
								const std::vector<unsigned int> & p_indices) :
		_name( p_name ),
		_vertices( p_vertices ), _indices( p_indices )
	{
		_vertices.shrink_to_fit();
		_indices.shrink_to_fit();
		_setupGL();
	}


	void SkyboxMesh::render( const GLuint p_glProgram ) const
	{
		glBindVertexArray(_vao);
		// passer les valeurs de couleur ambiante et diffus
		//glProgramUniform3fv(p_glProgram, _uAmbient, 1, glm::value_ptr(_material._ambient));
		
		//if (_material._hasAmbientMap) 
		//	glBindTextureUnit(0, _material._ambientMap._id);

		glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindTextureUnit(0, 0);
	}

	void SkyboxMesh::uniformSetUp(const GLuint p_glProgram ) {
		_uAmbient = glGetUniformLocation(p_glProgram, "uAmbient");
	}
	
	void SkyboxMesh::_loadTextures(std::vector<std::string> faces) { 

		if (VERBOSE)
			std::cout << "Loading skybox ";
		 
		GLenum format = GL_INVALID_ENUM;
		GLenum internalFormat = GL_INVALID_ENUM;
		int w; int h; int mipmaplevel;
		for (int i = 0; i < 6; i++) {
			// Load the image and send it to the GPU.
			Image image;

			const std::string fullPath = _dirPath + faces[i].c_str();

			if (image.load(fullPath)) {
				// Create a texture on the GPU.
				glCreateTextures(GL_TEXTURE_2D, 1, &_textureId);  

				format = GL_INVALID_ENUM;
				internalFormat = GL_INVALID_ENUM;
				// Define formats. 
				if (image._nbChannels == 1)
				{
					format = GL_RED;
					internalFormat = GL_R32F;
				}
				else if (image._nbChannels == 2)
				{
					format = GL_RG;
					internalFormat = GL_RG32F;
				}
				else if (image._nbChannels == 3)
				{
					format = GL_RGB;
					internalFormat = GL_RGB32F;
				}
				else
				{
					format = GL_RGBA;
					internalFormat = GL_RGBA32F;
				}
				w = image._width;
				h = image._height;
				mipmaplevel = log2(std::max(w, h));
				// Setup the texture format.
				// GL_TEXTURE_CUBE_MAP_POSITIVE_X  ?
				glTextureStorage2D(_textureId, mipmaplevel, internalFormat, w, h); 
			}
		}
		// Pour les paramètre, je prends simplement la dernière texture qui à été donné
		
		glTextureParameteri(_textureId, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(_textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(_textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(_textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Fill the texture.
		glTextureSubImage3D(_textureId, mipmaplevel, 0, 0, 0, w, h, w, format, GL_UNSIGNED_BYTE, NULL);
	}
	void SkyboxMesh::load() {
		 std::vector<std::string> faces = {
			"right.png",
			"left.png",
			"top.png",
			"bottom.png",
			"front.png",
			"back.png"
		};

		 _loadTextures(faces);
		 _setupGL();
	}
	void SkyboxMesh::cleanGL() 	{
		glDisableVertexArrayAttrib( _vao, 0 );
		glDisableVertexArrayAttrib( _vao, 1 );
		glDisableVertexArrayAttrib( _vao, 2 );
		glDisableVertexArrayAttrib( _vao, 3 );
		glDisableVertexArrayAttrib( _vao, 4 );
		glDeleteVertexArrays( 1, &_vao );
		glDeleteBuffers( 1, &_vbo );
		glDeleteBuffers( 1, &_ebo );
	}

	void SkyboxMesh::_setupGL()
	{				
		glCreateBuffers(1, &_ebo);
		glCreateBuffers(1, &_vbo); 
		glCreateVertexArrays(1, &_vao); 
		// TODO : set up selon la cube map
		/*
		glNamedBufferData(_ebo, _indices.size() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);
		glNamedBufferData(_vbo, _vertices.size() * sizeof(Vertex), _vertices.data(), GL_STATIC_DRAW);

		// activation des attributs vbo de vao
		glEnableVertexArrayAttrib( _vao, 0);
		glEnableVertexArrayAttrib( _vao, 1);
		glEnableVertexArrayAttrib( _vao, 2);
		glEnableVertexArrayAttrib( _vao, 3);
		glEnableVertexArrayAttrib( _vao, 4);
		
		// format de l'attribut d'indice 0, 2 valeur de type float par points, non normalisé, sans offset
		glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, _position));
		glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, _normal));
		glVertexArrayAttribFormat(_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, _texCoords));
		glVertexArrayAttribFormat(_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, _tangent));
		glVertexArrayAttribFormat(_vao, 4, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, _bitangent));
		
		// Indice : 0, offset : 0, stride : distance entre les differents éléments dans le buffer (taille du type des éléments du buffer)
		glVertexArrayVertexBuffer(_vao, 0, _vbo,  0.f, sizeof(Vertex)); 
		
		// Indice d'attribut : i -> addresse de bind : j
		glVertexArrayAttribBinding(_vao, 0, 0);
		glVertexArrayAttribBinding(_vao, 1, 0);
		glVertexArrayAttribBinding(_vao, 2, 0);
		glVertexArrayAttribBinding(_vao, 3, 0);
		glVertexArrayAttribBinding(_vao, 4, 0);

		// lier ebo et vao
		glVertexArrayElementBuffer(_vao, _ebo);
		*/
	}
} // namespace M3D_ISICG
