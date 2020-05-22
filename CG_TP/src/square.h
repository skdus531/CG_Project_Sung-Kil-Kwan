#pragma once

struct square_t
{
	vec3	center = vec3(0);		// 2D position for translation
	float	width = 5.0f;		// radius
	float	height = 1.0f;
	float	theta = 0.0f;			// rotation angle
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<square_t> create_squares()
{
	std::vector<square_t> squares;
	square_t c;

	c = { vec3(0,0,0.0f),5.0f,1.0f,0.0f,vec4(0.0f,0.5f,0.5f,1.0f) };
	squares.emplace_back(c);
	c = { vec3(-3.0f,0,0.0f),5.0f,1.0f,0.0f,vec4(0.0f,0.5f,0.5f,1.0f) };
	squares.emplace_back(c);
	c = { vec3(-6.0f,0,0.0f),5.0f,1.0f,0.0f,vec4(0.0f,0.5f,0.5f,1.0f) };
	squares.emplace_back(c);
	c = { vec3(-9.0f,0,0.0f),5.0f,1.0f,0.0f,vec4(0.0f,0.5f,0.5f,1.0f) };
	squares.emplace_back(c);
	return squares;
}

inline void square_t::update(float t)
{
	//radius = 0.35f + cos(t) * 0.1f;		// simple animation
	theta = t;
	float c = cos(theta), s = sin(theta);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		1, 0, 0, 0,
		0, width,0, 0,
		0, 0,  height, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
		/*c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1*/
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

inline std::vector<vertex> create_square_vertices()
{
	std::vector<vertex> v; 
	float positions[] = {
	-1.0f,  1.0f, 0.0f, //vertex 1 : Top-left
	1.0f, 1.0f, 0.0f, //vertex 2 : Top-right
	1.0f, -1.0f, 0.0f, //vertex 3 : Bottom-right
	1.0f, -1.0f, 0.0f, //vertex 4 : Bottom-right
	-1.0f, -1.0f, 0.0f, //vertex 5 : Bottom-left
	-1.0f,  1.0f, 0.0f //vertex 6 : Top-left
	};
	for (uint i = 0; i < 6; i++) {
		vec3 _pos = vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
		vec3 _norm = _pos.normalize();
		vec2 _tex = vec2(0.5f, 0.5f);
		v.push_back({ _pos, _norm, _tex });
	}
	return v;
}

inline std::vector<uint> create_square_indices(const std::vector<vertex>& vertices, GLuint& vertex_array)
{
	std::vector<uint> indices;
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); }

	GLuint square_elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	for (size_t i = 0; i < sizeof(square_elements) / sizeof(GLuint); i++) {
		indices.push_back(square_elements[i]);
	}

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);


	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	//if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); }
	return indices;
}
