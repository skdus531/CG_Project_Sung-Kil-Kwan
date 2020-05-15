#pragma once
#ifndef __SPHERE_H__
#define __SPHERE_H__
uint				LONGITUDE_EDGE = 72;
uint				LATITUDE_EDGE = 36;
uint				NUM_TESS = LONGITUDE_EDGE * LATITUDE_EDGE;
struct sphere_t
{
	vec3	position=vec3(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation
	float	rotation_speed = 50.0f;
	int		rotation_direction = -1;
	bool	is_student = true;
	// public functions
	void	update( float t);	
};

inline std::vector<sphere_t> create_sphere()
{
	std::vector<sphere_t> sphere;
	vec3 _pos[] = { vec3(-300.0f,0.0f, 0.0f), vec3(20.0f, 0.0f, 0.0f) };
	float _rad[] = { 10.3f, 7.2f };
	vec3 _color[] = { vec3(0.1f,0.2f,0.3f), vec3(0.2f, 0.1f, 0.6f) };
	for (size_t i = 0; i < sizeof(_rad) / sizeof(float); i++) {
		sphere_t s;
		if (i == 0) s.is_student = false;
		s.position = _pos[i]; s.radius = _rad[i]; s.color = vec4(_color[i], 1.0f);
		sphere.emplace_back(s);
	}
	return sphere;
}
inline std::vector<vertex> create_sphere_vertices()
{

	std::vector<vertex> v;
	for (uint i = 0; i <= LATITUDE_EDGE; i++) {
		float t = (PI / (float)LATITUDE_EDGE) * float(i);
		for (uint j = 0; j <= LONGITUDE_EDGE; j++) {
			float p = (PI * 2.0f / (float)LONGITUDE_EDGE) * float(j);
			float x = sin(t) * cos(p); float y = sin(t) * sin(p); float z = cos(t);
			v.push_back({ vec3(x, y, z), vec3(x, y, z), vec2(p / (PI * 2.0f), 1.0f - t / PI) });
		}
	}
	return v;
}

inline std::vector<uint> create_sphere_indices(const std::vector<vertex>& vertices, GLuint& vertex_array)
{
	std::vector<uint> indices;
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n");  }


	uint pi_lv[6] = { 0,0,1,0,1,1 };
	uint th_lv[6] = { 0,1,0,1,1,0 };
	for (uint i = 0; i < LATITUDE_EDGE; i++) {
		for (uint j = 0; j < LONGITUDE_EDGE; j++) {
			for (uint k = 0; k < 6; k++) indices.push_back(j + pi_lv[k] + (LONGITUDE_EDGE + 1) * (i + th_lv[k]));
		}
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
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); }
	return indices;
}

inline void sphere_t::update( float t)
{
	//radius	= 0.35f+cos(t)*0.1f;		// simple animation
	float theta	= t* rotation_speed * (float)rotation_direction;
	float c	= cos(theta), s=sin(theta);
	
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};


	/*mat4 rotation_matrix =
	{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};*/
	mat4 rotation_matrix = mat4::rotate(vec3(0, 0, 1), radians(180.0f));
	if(is_student) rotation_matrix=
	{
		c, 0, s, 0,
		0, 1, 0, 0,
		-s, 0, c, 0,
		0, 0, 0, 1
	};
	mat4 translate_matrix =
	{
		1, 0, 0, position.x,
		0, 1, 0, position.y,
		0, 0, 1, position.z,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}



#endif
