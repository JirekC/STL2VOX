#version 330 core

in vec2 tex_coord; // input variable from vertex shader (same name and type)

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
	float value = texture(tex, tex_coord).r;
    if(value == 0.0f)
        discard;
    FragColor = vec4(value, 0.0f, 0.0f, 1.0f);
}
