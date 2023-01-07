#ifndef __SKYBOX_MESH_HPP__
#define __SKYBOX_MESH_HPP__

#include "GL/gl3w.h"
#include "define.hpp"
#include <iostream>
#include <vector>

namespace M3D_ISICG
{

	struct Vertex
	{
		Vec3f _position;
		Vec3f _normal;
		Vec2f _texCoords;
		Vec3f _tangent;
		Vec3f _bitangent;
	};
	 

	class SkyboxMesh
	{
		std::string _dirPath = "./data/models/skybox";
	  public:
		SkyboxMesh() = delete;
		SkyboxMesh( const std::string &				p_name,
					  const std::vector<Vertex> &		p_vertices,
					  const std::vector<unsigned int> & p_indices);

		~SkyboxMesh() = default;

		void render( const GLuint p_glProgram ) const;

		void cleanGL();
		void uniformSetUp(const GLuint p_glProgram);

	  private:
		void _setupGL();  
		void _load(std::vector<std::string> faces); // Charger les textures

	  public:
		std::string _name = "Unknown";

		// ================ Geometric data.
		std::vector<Vertex>		  _vertices;
		std::vector<unsigned int> _indices; 
		 

		// ================ GL data.
		unsigned int _textureId;
		GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
		GLuint _vbo = GL_INVALID_INDEX; // Vertex Buffer Object
		GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object
		
		GLuint _uAmbient = GL_INVALID_INDEX; 
	};	
} // namespace M3D_ISICG

#endif // __SKYBOX_MESH_HPP__
