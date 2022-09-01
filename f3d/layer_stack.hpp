#ifndef LAYER_STACK_HPP
#define LAYER_STACK_HPP

#include <vector>
#include "glm/glm.hpp"
#include "shader.hpp"
#include "object_creator.hpp"

namespace f3d
{

    struct layer_t
    {
        std::string in_file_name;
        f3d::object3d object;
        unsigned int tex_color_buff;
        uint8_t material_nr = 1;
    };

    struct layer_stack
    {
        // data
		glm::u32vec2 _size = {0, 0}; // always 2D rectangle
        std::vector<layer_t>* _layers = nullptr;

		// OpenGL
		shader* _shader = nullptr;
		unsigned int vao; // Vertex Array Object ID
		unsigned int buff; // Vertex / Element Buffer Object ID

        layer_stack() {}

		layer_stack(shader& layers_shader)
		{
			_shader = &layers_shader;
		}

        layer_stack(shader& layers_shader, glm::vec3 scene_size, std::vector<layer_t>& layers)
        {
            _shader = &layers_shader;
            Prepare(scene_size, layers);
        }

        void Prepare(glm::vec3 scene_size, std::vector<layer_t>& layers);

        // merge all layers together:
        // fragment shader will discard pixels with mat# = 0, other mat# stacked in layer-order
        void Draw();

        ~layer_stack() { /*glDeleteBuffers(1, &buff);*/ }

    };

}
#endif
