#version 330

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization
in vec3 norm;
// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform bool b_solid_color;
uniform vec4 solid_color;
uniform int tc_color_flag;
uniform sampler2D tex;
void main()
{
	//fragColor = b_solid_color ? solid_color : vec4(tc.xy,0,1);
	//fragColor = vec4(normalize(norm), 1.0);
	//fragColor = vec4(tc.xy, 0, 1);
	switch (tc_color_flag) {
	case 0:
		//fragColor = texture(tex, tc);
		fragColor = solid_color;
		break;
	case 1:
		fragColor = vec4(tc.xy, 0, 1);
		break;
	case 2:
		fragColor = vec4(tc.xxx, 1);
		break;
	case 3:
		fragColor = vec4(tc.yyy, 1);
		break;
	}
}