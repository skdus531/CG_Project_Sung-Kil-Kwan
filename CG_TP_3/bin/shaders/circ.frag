#version 330

// inputs from vertex shader
in vec4 epos;
in vec2 tc;	// used for texture coordinate visualization
in vec3 norm;
// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform bool b_solid_color;
uniform vec4 solid_color;
uniform int tc_color_flag;
uniform bool game_start;
uniform sampler2D tex;

//shading
uniform mat4	view_matrix;
uniform float	shininess;
uniform vec4	light_position, Ia, Id, Is; //light
uniform vec4	Ka, Kd, Ks;

vec4 phong(vec3 l, vec3 n, vec3 h, vec4 Kd)
{
	vec4 Ira = Ka * Ia;									// ambient reflection
	vec4 Ird = max(Kd * dot(l, n) * Id, 0.0);					// diffuse reflection
	vec4 Irs = max(Ks * pow(dot(h, n), shininess) * Is, 0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	//fragColor = b_solid_color ? solid_color : vec4(tc.xy,0,1);
	//fragColor = vec4(normalize(norm), 1.0);
	//fragColor = vec4(tc.xy, 0, 1);


	vec4 lpos = view_matrix * light_position;

	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz - (lpos.a == 0.0 ? vec3(0) : p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l + v);	// the halfway vector

	if (game_start) {
		switch (tc_color_flag) {
		case 0:
			//fragColor = texture(tex, tc);
			fragColor = phong(l, n, h, solid_color);
			break;
		case 1:
			fragColor = phong(l, n, h, solid_color);
			break;
		case 2:
			fragColor = vec4(tc.xxx, 1);
			break;
		case 3:
			fragColor = vec4(tc.yyy, 1);
			break;
		}
	}
	else {
		fragColor = solid_color;
	}
}