#include "layer_stack.hpp"

using namespace f3d;
using namespace std;

// 3D objet - full-screen rectangle
static float vertices[] = {
    // first triangle
    // 3D coord                 // texture coords
    1.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,	        // top right
    1.0f,-1.0f, 0.0f, 1.0f,     1.0f, 0.0f,         // bottom right
   -1.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,         // top left
    // second triangle
    1.0f,-1.0f, 0.0f, 1.0f,     1.0f, 0.0f,         // bottom right
   -1.0f,-1.0f, 0.0f, 1.0f,     0.0f, 0.0f,         // bottom left
   -1.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,         // top left
};

void layer_stack::Prepare(glm::vec3 scene_size, std::vector<layer_t>& layers)
{
    _size.x = scene_size.x;
    _size.y = scene_size.y;
    _layers = &layers;

    // OpenGL
    glGenVertexArrays(1, &vao);
	glBindVertexArray(vao); // bind Vertex Array Object
	glGenBuffers(1, &buff);
    // vertices
	glBindBuffer(GL_ARRAY_BUFFER, buff); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // vertices - 3D coordinates
    glVertexAttribPointer(
        0, // attribute "location = 0" in shader
        4, // size of attribute (in floats, vertex coord is vec4)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float), // space between consecutive vertex attributes
        (void*)0 // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(0);
    // vertices - texture coordinates
    glVertexAttribPointer(
        1, // attribute "location = 1" in shader
        2, // size of attribute (in floats, vertex coord is vec4)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float), // space between consecutive vertex attributes
        (void*)(4 * sizeof(float)) // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(1);
}

void layer_stack::Draw()
{
    if(_shader == nullptr || _layers == nullptr)
        return;

    _shader->use();
    // draw layer per layer
    // fragment shader will discard pixels with mat# = 0 (color.red == 0.0), other mat# will replace original value
    for(int i = 0; i < _layers->size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, _layers->at(i).tex_color_buff);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
