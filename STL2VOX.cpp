#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <glad/glad.h> // OpenGL loader
#include <GLFW/glfw3.h> // OpenGL window & input
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include "f3d/object_creator.hpp"
#include "f3d/layer_stack.hpp"

#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327950288
#endif /* M_PI */

using namespace std;

void PrintHelpExit()
{
	cerr << "\nCorrect usage:\n";
	cerr << "STL2VOX IN_FILE MATERIAL_NUMBER [IN_FILE2 MATERIAL_NUMBER2] ... -sxSIZE_X sySIZEY -szSIZE_Z [-oOUT_FILE]\n";
	cerr << "MAT_NUMBER (following each input file) must be integer <0 .. 255> #1 used otherwise\n";
	cerr << "-sx, -sy, -sz: total scene size in each dimmension (in elements), positive integer value\n";
	cerr << "\n";
	exit(-1);
}

// main entry :)
int main(int argc, char** argv)
{
    vector<f3d::layer_t> layers;
	auto sc_size = glm::vec3({-1.0f,-1.0f,-1.0f});
	auto obj_size = glm::vec3({0.0f, 0.0f, 0.0f});
	string out_file_name = "scene.ui8"; // default output name, if not overriden by -o argument
    string in_file_tmp;
    uint8_t name_or_mat = 0; // input file name or material #

	if(argc < 2)
	{
		PrintHelpExit();
	}

	// parse cmd-line parameters
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            // params beginning with '-' : "setup params"
            if(argv[i][1] == 's')
            {
                try // bacause stol() can throw
                {
                    switch (argv[i][2])
                    {
                    case 'x':
                        sc_size.x = (uint32_t)stol(&argv[i][3]);
                        break;
                    case 'y':
                        sc_size.y = (uint32_t)stol(&argv[i][3]);
                        break;
                    case 'z':
                        sc_size.z = (uint32_t)stol(&argv[i][3]);
                        break;
                    default:
                        throw runtime_error("Unknown argument" + string(argv[i]));
                        break;
                    }
                }
                catch(const std::exception& e)
                {
                    cerr << "\nInvalid scene-size argument(s):\n";
                    cerr << e.what() << '\n';
                    PrintHelpExit();
                }
            }
            else if(argv[i][1] == 'o')
            {
                out_file_name = &argv[i][2];
            }
            else
            {
                cerr << "\nUnknown argument" << string(argv[i]) << "\n";
                PrintHelpExit();
            }
        }
        else
        {
            // input file-names and material numbers
            if(name_or_mat & 0x01)
            {
                // odd: material #
                try
                {
                    f3d::layer_t l;
                    l.material_nr = stoi(argv[i]);
                    l.in_file_name = in_file_tmp;
                    layers.push_back(move(l));
                }
                catch(const std::exception& e)
                {
                    cerr << "\nInvalid material # argument:\n";
                    cerr << e.what() << '\n';
                    PrintHelpExit();
                }
            }
            else
            {
                // even: input file name
                in_file_tmp = argv[i];
            }
            name_or_mat++;
        }
    }

    if(sc_size.x < 1 || sc_size.y < 1 || sc_size.z < 1)
    {
        cerr << "All 3 dimmensions must be specified as positive integer value.\n";
        PrintHelpExit();
    }

	// show some info
	cout << "\nScene size:\nx: " << to_string((uint32_t)sc_size.x) << "\ny: ";
	cout << to_string((uint32_t)sc_size.y) << "\nz: " << to_string((uint32_t)sc_size.z) << "\n";

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
    // for all layers: add ptr to shader above, load STL file & create texture object
    for(int i = 0; i < layers.size(); i++)
    {
        layers[i].object._shader = &object_shader;
        layers[i].object.Prepare(f3d::loader::LoadSTL(layers[i].in_file_name.c_str(), &obj_size),{0,0,0},{0,0,0},{1,1,1},
                                    {(float)layers[i].material_nr / 256.0f, 0,0,1}); // material # in RED channel
        // create a color atachment texture
        glGenTextures(1, &(layers[i].tex_color_buff));
        glBindTexture(GL_TEXTURE_2D, layers[i].tex_color_buff);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sc_size.x, sc_size.y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL); // don't fill any data, render will :)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // final layered stack
	f3d::shader layer_shader("f3d/vertex_layers.glsl", "f3d/fragment_layers.glsl");
    f3d::layer_stack final_stack(layer_shader, sc_size, layers);

    // create new framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    // create a color atachment texture for final data of one slice
    unsigned int tex_color_buff_final;
    glGenTextures(1, &tex_color_buff_final);
    glBindTexture(GL_TEXTURE_2D, tex_color_buff_final);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sc_size.x, sc_size.y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not realy needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// create render buffer object for depth and stencil attachment (we won't be sample it)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sc_size.x, sc_size.y);

	// attach texture and renderbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // textures will be attached in loop below
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buff_final, 0); // TODO: remove, moved to for loop
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
	glDisable(GL_CULL_FACE); // disable - we need to draw object from inside too
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
	
	ofstream out_file;
	try
	{
		out_file.exceptions(ifstream::failbit | ifstream::badbit);
		out_file.open(out_file_name, ios_base::binary | ios_base::trunc);
	}
	catch(exception& e)
	{
		cerr << "Failed to open output file: " << out_file_name << endl;
        glfwTerminate();
		return -1;
	}
	
	size_t out_buf_size = (size_t)sc_size.x * (size_t)sc_size.y;
	uint8_t* out_buf = new uint8_t[out_buf_size]; // one slice X * Y * 1 Byte
	try
	{	
		for(uint32_t slice = 1; slice <= (uint32_t)sc_size.z; slice++)
		{
            glBindFramebuffer(GL_FRAMEBUFFER, fbo); // TODO: remove? binded above
            // calculate view matrix (orthographic) for given slice
			// for Z coord used maximal size of objects, not scene because object must be "closed" on far-end
            glm::mat4 view_mat = glm::ortho(0.0f, sc_size.x, 0.0f, sc_size.y, -(float)slice, -obj_size.z - 1.0f);

            // draw layer-per-layer to individual textures
            for(int layer = 0; layer < layers.size(); layer++)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layers[layer].tex_color_buff, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                layers[layer].object.Draw(view_mat);
            }

            // all layers rendered (current slice) - merge it together:
            // fragment shader will discard pixels with mat# = 0, other mat# stacked in layer-order
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buff_final, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            final_stack.Draw();

			// take final texture from GPU & store to file
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_color_buff_final);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, out_buf);
			out_file.write((char*)out_buf, out_buf_size);

			printf("\r\033[K%d", slice);
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
