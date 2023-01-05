#ifndef __LAB_WORK_5_HPP__
#define __LAB_WORK_5_HPP__

#include "GL/gl3w.h"
#include "common/base_lab_work.hpp"
#include "define.hpp"
#include "common/camera.hpp"
#include "common/models/triangle_mesh_model.hpp"
#include "utils/random.hpp"
#include <vector>

namespace M3D_ISICG
{
	class LabWork5 : public BaseLabWork
	{
		public:
			LabWork5() : BaseLabWork() {}
			~LabWork5();

			bool init() override;
			void animate( const float p_deltaTime ) override;
			void render() override;

			void handleEvents( const SDL_Event & p_event ) override;
			void displayUI() override;
			
		private:

			/*struct Mesh {
				std::vector<Vec3f> vertPos;
				std::vector<Vec3f> vertCols;
				std::vector<unsigned int> vertInd; // indices des sommets
				Mat4f matTransform = MAT4F_ID;
				// Obj opengl
				GLuint vao;
				GLuint ebo;
				GLuint vboPos;
				GLuint vboCols;
			};*/

			// ================ Scene data.  
			//Mesh _mesh;
			TriangleMeshModel _mesh_model; 
			Camera _camera;
			// ================


			// ================ GL data.
			GLuint vertShader = GL_INVALID_INDEX; 
			GLuint fragShader = GL_INVALID_INDEX;
			GLuint _program = GL_INVALID_INDEX;
			GLint _uMVPMatrix = GL_INVALID_INDEX; 
			GLint _uNormalMatrix = GL_INVALID_INDEX; 
			GLint _uMVMatrix = GL_INVALID_INDEX; 
			GLint _uLumPos = GL_INVALID_INDEX; 

			// ================ Time
			float _time = 0.f;
			Vec3f _lumPos = Vec3f(0, 0, 0);

			// ================ Settings.
			Vec4f _bgColor = Vec4f( 0.2f, 0.2f, 0.2f, 1.f ); // Background color
			float _fovy = 60.f;
			float _cameraSpeed		 = 0.1f;
			float _cameraSensitivity = 0.1f; 

			// ================
			static const std::string _shaderFolder;
			static GLuint _createShader( GLenum shader_type, std::string fileName );
			bool _testShader();
			void _initCamera();
			void _updateCameraUniform();  
	};
} // namespace M3D_ISICG

#endif // __LAB_WORK_5_HPP__
