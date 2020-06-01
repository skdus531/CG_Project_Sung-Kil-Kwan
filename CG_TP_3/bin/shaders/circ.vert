// vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec4 epos;
out vec3 norm;
out vec2 tc;	// texture coordinate

// uniforms
uniform mat4	model_matrix;	// 4x4 transformation matrix: explained later in the lectur
uniform mat4	view_matrix;
uniform mat4	projection_matrix;


uniform mat4	p_model_matrix;	// 4x4 transformation matrix
uniform mat4	p_aspect_matrix;	// tricky 4x4 aspect-correction matrix
uniform bool	particle;

void main()
{
	vec4 wpos;
	if (particle) {
		wpos = p_model_matrix * vec4(position, 1);
	}
	else {
		wpos = model_matrix * vec4(position, 1);
	}
	vec4 epos = view_matrix * wpos;
	if (particle) {
		gl_Position = p_aspect_matrix * wpos;
	}
	else {
		gl_Position = projection_matrix * epos;
	}
	norm = normalize(mat3(view_matrix * model_matrix) * normal);
	tc = texcoord;
}
