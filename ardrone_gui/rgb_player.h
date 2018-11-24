#pragma once 

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdint.h>
#include <iostream>

#include "shaders.h"


class RGBGL_Player {

public:
	RGBGL_Player()
		: vid_w(0),
		vid_h(0),
		win_w(0),
		win_h(0),
		vao(0),
		vert(0),
		frag(0),
		prog(0),
		u_pos(-1),
		textures_created(false),
		shader_created(false)
	{

	}
	bool setup(int vidW, int vidH,int winW, int winH) {
		resize(winW, winH);
		vid_w = vidW;
		vid_h = vidH;

		if (!vid_w || !vid_h) {
			std::cerr << "Invalid texture size" << std::endl;
			return false;
		}

		pos_x = winW / 2 - vidW / 2;
		pos_y = winH / 2 - vidH / 2;


		if (!setupTextures()) {
			return false;
		}

		if (!setupShader()) {
			return false;
		}

		glGenVertexArrays(1, &vao);

		return true;
	}

	void setPixels(cv::Mat& m) {

		//cv::flip(m, m, 0);
		glBindTexture(GL_TEXTURE_2D, i_tex);
		//use fast 4-byte alignment (default anyway) if possible
		//glPixelStorei(GL_UNPACK_ALIGNMENT, (m.step & 3) ? 1 : 4);

		//set length of one complete row in data (doesn't need to equal image.cols)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m.step / m.elemSize());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w, vid_h, GL_BGR, GL_UNSIGNED_BYTE, m.ptr());

	}


	void draw() {
		assert(textures_created == true);


		glBindVertexArray(vao);
		glUseProgram(prog);

		glUniform4f(u_pos, pos_x, pos_y, vid_w, vid_h);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, i_tex);
		glUniform1i(glGetUniformLocation(prog, "texture"), 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	void resize(int winW, int winH) {
		assert(winW > 0 && winH > 0);

		win_w = winW;
		win_h = winH;

		pm = glm::mat4(1.0);
		pm = glm::ortho(0.0f, (float)win_w, (float)win_h, 0.0f, 0.0f, 100.0f);

		glUseProgram(prog);
		glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, &pm[0][0]);
	}

private:
	bool setupTextures() {
		if (textures_created) {
			printf("Textures already created.\n");
			return false;
		}
		
		glGenTextures(1, &i_tex);
		glBindTexture(GL_TEXTURE_2D, i_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D,     // Type of texture
			0,                 // Pyramid level (for mip-mapping) - 0 is the top level
			GL_RGB,            // Internal colour format to convert to
			vid_w,          // Image width  i.e. 640 for Kinect in standard mode
			vid_h,          // Image height i.e. 480 for Kinect in standard mode
			0,                 // Border width in pixels (can either be 1 or 0)
			GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
			GL_UNSIGNED_BYTE,  // Image data type
			NULL);        // The actual image data itself

		textures_created = true;
		return true;
	}
	bool setupShader() {

		if (shader_created) {
			printf("Already creatd the shader.\n");
			return false;
		}

		MyShader shad;
		if (!shad.OpenMyShader("shaders/vert.glsl", "shaders/frag.glsl")) {
			printf("Cant open the shaders\n");
			return false;
		}
		prog = shad.GetShaderID().getID();


		glUseProgram(prog);
		glUniform1i(glGetUniformLocation(prog, "texture"), 0);

		u_pos = glGetUniformLocation(prog, "draw_pos");

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		resize(viewport[2], viewport[3]);

		return true;
	}

public:
	int pos_x;
	int pos_y;
	int vid_w;
	int vid_h;
	int win_w;
	int win_h;
	GLuint vao;
	GLuint i_tex;
	GLuint i2_tex;
	GLuint vert;
	GLuint frag;
	GLuint prog;
	GLint u_pos;
	bool textures_created;
	bool shader_created;

	glm::mat4 pm;
};