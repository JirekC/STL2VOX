#include <iostream>
#include <fstream>
#include <cmath>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <glad/glad.h> // OpenGL loader
#include <GLFW/glfw3.h> // OpenGL window & input
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include "f3d/object_creator.hpp"

#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327950288
#endif /* M_PI */

using namespace std;

int i = 0; // slice number: <0, sc_size.z>

// mouse scroll - frame inc / dec
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	int inc = (int)yoffset;
	
	if (i + inc >= 0)
		i += inc;
}


// main entry :)
int main(int argc, char** argv)
{
	if(argc < 2)
	{
		cerr << "\nYou have to specify at least input-file-name as first parameter:\n";
		cerr << "STL2VOX IN_FILE [-oOUT_FILE] [-mMAT_NUMBER]\n";
		cerr << "MAT_NUMBER must be within <0 .. 255> 1 used otherwise\n";
		cerr << "\n";
		return -1;
	}

	auto sc_size = glm::vec3({512,512,256});

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// we need to create wingow to be able to init OpenGL
	GLFWwindow* window = glfwCreateWindow(sc_size.x, sc_size.y, "STL to Voxel-Map converter", NULL, NULL);
	if (window == NULL)
	{
		cerr << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cerr << "Failed to initialize GLAD" << endl;
        glfwTerminate();
		return -1;
	}
	
	f3d::shader object_shader("f3d/vertex_object_voxmap.glsl", "f3d/fragment_object_voxmap.glsl");
	f3d::object3d object1(object_shader, f3d::loader::LoadSTL(argv[1]), // input file in first argument
							{0, 0, 0}, // trans, rot, scale only for testing now
							{0,0,0},
							{1,1,1},
							{(float)200/256.0, 0, 0, 1}); // material # 200 (in RED channel)

    // create new framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    // create a color atachment texture
    unsigned int tex_color_buff;
    glGenTextures(1, &tex_color_buff);
    glBindTexture(GL_TEXTURE_2D, tex_color_buff);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sc_size.x, sc_size.y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL); // not fill any data, render will :)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not realy needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// create render buffer object for depth and stencil attachment (we won't be sample it)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sc_size.x, sc_size.y);

	// attach tecture and renderbuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buff, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
		cerr << "Failed to initialize framebuffer" << endl;
        glfwTerminate();
		return -1;
	}


//	glViewport(0, 0, sc_size.x, sc_size.y);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // using z-buffer
	//glEnable(GL_CULL_FACE); // commented, because we using both sides of triangles
	glDisable(GL_CULL_FACE); // disable - we need to draw object from inside too
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
	
	ofstream out_file;
	try
	{
		out_file.exceptions(ifstream::failbit | ifstream::badbit);
		out_file.open("../GL_test/scene.ui8", ios_base::binary | ios_base::trunc);
	}
	catch(exception& e)
	{
		cerr << "Failed to open output file" << endl;
        glfwTerminate();
		return -1;
	}
	
	size_t out_buf_size = (size_t)sc_size.x * (size_t)sc_size.y;
	uint8_t* out_buf = new uint8_t[out_buf_size]; // one slice X * Y * 1 Byte
	try
	{	
		for(int i = 0; i < (int)sc_size.z; i++)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			glClear(GL_DEPTH_BUFFER_BIT);

			glm::mat4 view_mat = glm::ortho(0.0f, sc_size.x, 0.0f, sc_size.y, -(float)i, -sc_size.z);
			object1.Draw(view_mat);

			// take texture from GPU & store to file
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_color_buff);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, out_buf);
			out_file.write((char*)out_buf, out_buf_size);

			printf("\r\033[K%d", i);

			glfwPollEvents();
//			glfwSwapBuffers(window);  
//			this_thread::sleep_for(chrono::milliseconds(300));
		}
	}
	catch(exception& e)
	{
		cerr << "Bad thing happend:\n" << e.what() << "\n";
		if(out_file.is_open())
			out_file.close();
		glfwTerminate();
		return -1;
	}

	delete[] out_buf;
	if(out_file.is_open())
		out_file.close();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind framebuffer
	glfwTerminate();
	return 0;
}
