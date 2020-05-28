#version 330

// input attributes of vertices
in vec3 position;
in vec3 normal;
in vec2 texcoord;

// outputs of vertex shader = input to fragment shader
// out vec4 gl_Position: a built-in output variable that should be written in main()
out vec4 epos;
out vec3 norm;	// the second output: not used yet
out vec2 tc;	// the third output: not used yet

// uniform variables
uniform mat4	model_matrix;	// 4x4 transformation matrix: explained later in the lecture
uniform mat4	aspect_matrix;	// tricky 4x4 aspect-correction matrix
uniform mat4	view_matrix;
uniform mat4	projection_matrix;

void main()
{
	vec4 wpos = model_matrix * vec4(position, 1);
	vec4 epos = view_matrix * wpos;
	gl_Position = projection_matrix * epos;
	
	//gl_Position = aspect_matrix * view_projection_matrix*model_matrix*vec4(position,1);
	//gl_Position = aspect_matrix * model_matrix * vec4(position, 1);
	// other outputs to rasterizer/fragment shader
	norm = normalize(mat3(view_matrix * model_matrix) * normal);
	tc = texcoord;
}