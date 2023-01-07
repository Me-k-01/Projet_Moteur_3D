#ifndef __LAB_WORK_FIN_HPP__
#define __LAB_WORK_FIN_HPP__

#include "GL/gl3w.h"
#include "common/base_lab_work.hpp"
#include "define.hpp"
#include "common/camera.hpp"
#include "common/models/triangle_mesh_model.hpp"
#include "utils/random.hpp"
#include <vector>

namespace M3D_ISICG
{
	class LabWorkFin : public BaseLabWork
	{
		public:
			LabWorkFin() : BaseLabWork() {}
			~LabWorkFin();

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
			// ================


			// ================ GL data.
			GLuint vertShader = GL_INVALID_INDEX; 
			GLuint fragShader = GL_INVALID_INDEX;
			GLuint _program = GL_INVALID_INDEX;
			GLint _uMVPMatrix = GL_INVALID_INDEX; 
			GLint _uNormalMatrix = GL_INVALID_INDEX; 
			GLint _uMVMatrix = GL_INVALID_INDEX; 
			GLint _uLightsPos = GL_INVALID_INDEX; 
			GLint _uLightsTint = GL_INVALID_INDEX; // Couleur de la lumière
			GLint _uTime = GL_INVALID_INDEX;
			GLint _uCurrAmbient = GL_INVALID_INDEX;
			GLint _uRes = GL_INVALID_INDEX;
			GLint _uStrength = GL_INVALID_INDEX;

			// ================ Time
			float _time = 0.f;
			float _resolutionNoise;  
			float _strength;
			int _currAmbient = 0; // 0 : texture ambient normale, le reste sont des textures ambient procédural
			std::vector<Vec3f> _lightsPos ;
			std::vector<Vec3f> _lightsTint ;

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
			void _updateLightColors();  
	};
} // namespace M3D_ISICG

#endif // __LAB_WORK_FIN_HPP__
