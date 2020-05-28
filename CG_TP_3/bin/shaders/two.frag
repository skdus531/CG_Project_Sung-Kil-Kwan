#version 330

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization

// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform bool b_solid_color;
uniform vec4 solid_color;

void main()
{
	fragColor = solid_color;
}