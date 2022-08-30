#version 330 core

in vec3 normal_vec;
uniform vec4 color; // object color - used only RED channel to identify material #

out vec4 FragColor;

void main()
{
    /*
    *   Slicing algorithm:
    *   Object is black from outside and white from inside. Slicing "camera" is movig around Z axis and taking
    *   shots. If camera's ray is inside object, white pixel is rendered (solid material). Otherwise black is rendered (no mat).
    */
    float result = normal_vec.z <= 0.0f ? 0.0f : 1.0f; // for slicing algorithm (object must be solid -> bool::OR)
	FragColor = vec4(result * color.r, 0.0, 1.0 - result, 1.0f); // only RED channel matters
}
