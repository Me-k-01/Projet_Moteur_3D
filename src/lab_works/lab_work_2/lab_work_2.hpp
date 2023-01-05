#ifndef __LAB_WORK_2_HPP__
#define __LAB_WORK_2_HPP__

#include "GL/gl3w.h"
#include "common/base_lab_work.hpp"
#include "define.hpp"
#include <vector>

namespace M3D_ISICG
{
	class LabWork2 : public BaseLabWork
	{
	  public:
		LabWork2() : BaseLabWork() {}
		~LabWork2();

		bool init() override;
		void animate( const float p_deltaTime ) override;
		void render() override;

		void handleEvents( const SDL_Event & p_event ) override;
		void displayUI() override;

	  private:
		// ================ Scene data.
		std::vector<Vec2f> _poly;
		std::vector<unsigned int> _polyTri;
		std::vector<Vec4f> _cols;
		float _lum;
		  
		// ================


		// ================ GL data.
		GLuint _vertShader; 
		GLuint _fragShader;
		GLuint _program;
		GLint _uTranslationX;
		GLint _uLum;

		GLuint _vao;
		GLuint _ebo;
		GLuint _vboPos;
		GLuint _vboCols;
		// ================ Time
		float _time;

		// ================ Settings.
		Vec4f _bgColor = Vec4f( 0.8f, 0.8f, 0.8f, 1.f ); // Background color

		// ================

		static const std::string _shaderFolder;
		static GLuint _createShader( GLenum shader_type, std::string fileName );
		bool _testShader();
	};
} // namespace M3D_ISICG

#endif // __LAB_WORK_1_HPP__
