#include "triangle_mesh.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>



namespace M3D_ISICG
{
	TriangleMesh::TriangleMesh( const std::string &				  p_name,
								const std::vector<Vertex> &		  p_vertices,
								const std::vector<unsigned int> & p_indices,
								const Material &				  p_material ) :
		_name( p_name ),
		_vertices( p_vertices ), _indices( p_indices ), _material( p_material )
	{
		_vertices.shrink_to_fit();
		_indices.shrink_to_fit();
		_setupGL();
	}


	void TriangleMesh::render( const GLuint p_glProgram ) const
	{
		glBindVertexArray(_vao);
		// passer les valeurs de couleur ambiante et diffus
		glProgramUniform3fv(p_glProgram, _uAmbient, 1, glm::value_ptr(_material._ambient));
		glProgramUniform3fv(p_glProgram, _uDiffuse, 1, glm::value_ptr(_material._diffuse));
		glProgramUniform3fv(p_glProgram, _uSpecular, 1, glm::value_ptr(_material._specular));
		glProgramUniform1f(p_glProgram, _uShininess, _material._shininess);
		glProgramUniform1i(p_glProgram, _uHasAmbientMap, _material._hasAmbientMap);
		glProgramUniform1i(p_glProgram, _uHasDiffuseMap, _material._hasDiffuseMap);
		glProgramUniform1i(p_glProgram, _uHasSpecularMap, _material._hasSpecularMap);
		glProgramUniform1i(p_glProgram, _uHasShininessMap, _material._hasShininessMap);
		glProgramUniform1i(p_glProgram, _uHasNormalMap, _material._hasNormalMap);
		
		if (_material._hasAmbientMap) 
			glBindTextureUnit(0, _material._ambientMap._id);
		if (_material._hasDiffuseMap) 
			glBindTextureUnit(1, _material._diffuseMap._id);  
		if (_material._hasSpecularMap) 
			glBindTextureUnit(2, _material._specularMap._id);
		if (_material._hasShininessMap) 
			glBindTextureUnit(3, _material._shininessMap._id);
		if (_material._hasNormalMap) 
			glBindTextureUnit(4, _material._normalMap._id);

		glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindTextureUnit(0, 0);
		glBindTextureUnit(1, 0);
		glBindTextureUnit(2, 0);
		glBindTextureUnit(3, 0);
		glBindTextureUnit(4, 0);
		 
	}

	void TriangleMesh::uniformSetUp(const GLuint p_glProgram ) {
		_uAmbient = glGetUniformLocation(p_glProgram, "uAmbient");
		_uDiffuse = glGetUniformLocation(p_glProgram, "uDiffuse");
		_uSpecular = glGetUniformLocation(p_glProgram, "uSpecular");
		_uShininess = glGetUniformLocation(p_glProgram, "uShininess");
		_uNormal = glGetUniformLocation(p_glProgram, "uNormal");

		_uHasAmbientMap = glGetUniformLocation(p_glProgram, "uHasAmbianteMap"); 
		_uHasDiffuseMap = glGetUniformLocation(p_glProgram, "uHasDiffuseMap"); 
		_uHasSpecularMap = glGetUniformLocation(p_glProgram, "uHasSpecularMap"); 
		_uHasShininessMap = glGetUniformLocation(p_glProgram, "uHasShininessMap"); 
		_uHasNormalMap = glGetUniformLocation(p_glProgram, "uHasNormalMap");

	}

	void TriangleMesh::cleanGL()
	{
		glDisableVertexArrayAttrib( _vao, 0 );
		glDisableVertexArrayAttrib( _vao, 1 );
		glDisableVertexArrayAttrib( _vao, 2 );
		glDisableVertexArrayAttrib( _vao, 3 );
		glDisableVertexArrayAttrib( _vao, 4 );
		glDeleteVertexArrays( 1, &_vao );
		glDeleteBuffers( 1, &_vbo );
		glDeleteBuffers( 1, &_ebo );
	}

	void TriangleMesh::_setupGL()
	{				
		glCreateBuffers(1, &_ebo);
		glCreateBuffers(1, &_vbo); 
		glCreateVertexArrays(1, &_vao);

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
	}
} // namespace M3D_ISICG
