#pragma once
struct cube_t {
	vec3 position = vec3(0.0f, 0.0f, -20.0f);
	vec3 dimension = vec3(300.0f, 38.5f, 8.5f);
	vec3 color = vec3(0.4f, 0.4f, 0.4f);
	mat4 model_matrix;
	void update(float t);
};

inline std::vector<cube_t> create_cube()
{
	std::vector<cube_t> cube;
	vec3 _pos[] = { vec3(0.0f, 0.0f, -20.0f), vec3(-600.0f, 0.0f, -20.0f) };
	vec3 _color[] = { vec3(0.4f,0.4f,0.4f),vec3(0.1f,0.8f,0.1f) };
	for (size_t i = 0; i < 2; i++) {
		cube_t c;
		c.position = _pos[i];
		c.color = _color[i];
		cube.push_back(c);
	}
	return cube;
}
inline std::vector<vertex> create_cube_vertices()
{

	std::vector<vertex> v;
	float positions[] = {
		// front
	  -1.0, -1.0,  1.0,
	   1.0, -1.0,  1.0,
	   1.0,  1.0,  1.0,
	  -1.0,  1.0,  1.0,
	  // back
	  -1.0, -1.0, -1.0,
	   1.0, -1.0, -1.0,
	   1.0,  1.0, -1.0,
	  -1.0,  1.0, -1.0
	};
	for (uint i = 0; i < 8; i++) {
		vec3 _pos = vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
		vec3 _norm = _pos.normalize();
		vec2 _tex = vec2(0.5f, 0.5f);
		v.push_back({ _pos, _norm, _tex });
	}
	return v;
}

inline std::vector<uint> create_cube_indices(const std::vector<vertex>& vertices, GLuint& vertex_array)
{
	std::vector<uint> indices;
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); }

	GLuint cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	
	for (size_t i = 0; i < sizeof(cube_elements) / sizeof(GLuint); i++) {
		indices.push_back(cube_elements[i]);
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

inline void cube_t::update(float t)
{
	//radius	= 0.35f+cos(t)*0.1f;		// simple animation
	float theta = t;
	float c = cos(theta), s = sin(theta);
	position.x += 10.0f;
	if (position.x > 300.0f) position.x -= 1200.0f;
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		dimension.x, 0, 0, 0,
		0, dimension.y, 0, 0,
		0, 0, dimension.z, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	mat4 translate_matrix =
	{
		1, 0, 0, position.x,
		0, 1, 0, position.y,
		0, 0, 1, position.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}
