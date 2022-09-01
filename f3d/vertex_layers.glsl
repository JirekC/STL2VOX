#version 330 core

layout (location = 0) in vec4 model; // position of vertices in model (rectangle)
layout (location = 1) in vec2 tex_coord_in; // texture coordinates

out vec2 tex_coord;

void main()
{
	gl_Position = model;
	tex_coord = tex_coord_in; // propagate texture coordinates to fragment shader
}
