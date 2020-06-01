#ifdef GL_ES
#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
#define highp mediump
#endif
precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec2 tc;
in vec4 epos;
in vec3 norm;

// the only output variable
out vec4 fragColor;

// texture sampler
uniform int tc_color_flag;
uniform bool game_start;
uniform sampler2D	TEX;
uniform vec4		p_color;
uniform bool		particle;

uniform vec4		solid_color;

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

	vec4 lpos = view_matrix * light_position;

	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz - (lpos.a == 0.0 ? vec3(0) : p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l + v);	// the halfway vector

	if (particle)
	{
		fragColor = texture(TEX, tc);// if(fragColor.a < 0.001) discard;
		fragColor = vec4(fragColor.rgb, fragColor.r) * p_color; // enable alpha blending
	}
	else {
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
}

