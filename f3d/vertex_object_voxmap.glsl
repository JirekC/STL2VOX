#version 330 core

layout (location = 0) in vec3 model; // vertex
layout (location = 1) in vec3 normal; // normal vector
uniform mat4 view;
uniform mat4 transform;  // vertex: 	 rotate, scale, translate
uniform mat3 normal_mat; // norm-vactor: rotate, scale, translate

out vec3 normal_vec;	// forwarding normalized normal-vector

void main()
{
	gl_Position = view * transform * vec4(model, 1.0f);
	normal_vec = normalize(normal_mat * normal);
}
