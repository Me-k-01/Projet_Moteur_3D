#ifndef __LAB_WORK_6_HPP__
#define __LAB_WORK_6_HPP__

#include "GL/gl3w.h"
#include "common/base_lab_work.hpp"
#include "define.hpp"
#include "common/camera.hpp"
#include "common/models/triangle_mesh_model.hpp"
#include "utils/random.hpp"
#include <vector>

namespace M3D_ISICG
{
	class LabWork6 : public BaseLabWork
	{
		public:
			LabWork6() : BaseLabWork() {}
			~LabWork6();

			bool init() override;
			void animate( const float p_deltaTime ) override;
			void render() override;

			void handleEvents( const SDL_Event & p_event ) override;
			void displayUI() override;
			
		private:
			// ================ Scene data.  
			//Mesh _mesh;
			TriangleMeshModel _mesh_model; 
			Camera _camera;
			// I/ 1. attribut des textures sous formes d'un tableau 
			GLuint _gBufferTextures[6];
			GLuint _fboId;
			// ================


			// ================ GL data.  
			GLuint _geometryPassProgram = GL_INVALID_INDEX;
			GLuint _shadingPassProgram = GL_INVALID_INDEX;
			GLint _uMVPMatrix = GL_INVALID_INDEX; 
			GLint _uNormalMatrix = GL_INVALID_INDEX; 
			GLint _uMVMatrix = GL_INVALID_INDEX; 
			GLint _uLumPos = GL_INVALID_INDEX; 

			// ================ Time
			float _time = 0.f;
			Vec3f _lumPos = Vec3f(0.f, 0.f, 0.f);

			// ================ Settings.
			Vec4f _bgColor = Vec4f( 0.2f, 0.2f, 0.2f, 1.f ); // Background color
			float _fovy = 60.f;
			float _cameraSpeed		 = 0.1f;
			float _cameraSensitivity = 0.1f; 
			int _currDrawBuffer = 0; // Indice du drawBuffer

			// ================
			static const std::string _shaderFolder;
			static GLuint _createShader( GLenum shader_type, std::string fileName );
			static bool _testShader(GLuint shader); 
			void _initCamera();
			bool _initGeometryPassProgram();
			bool _initShadingPassProgram();
			void _initGBuffer();
			void _geometryPass(); 
			void _shadingPass(); 
			void _updateCameraUniform(); 
	};
} // namespace M3D_ISICG

#endif // __LAB_WORK_6_HPP__
