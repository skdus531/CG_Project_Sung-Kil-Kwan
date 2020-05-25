struct button_t
{
	vec3	center = vec3(0);		// 2D position for translation
	float	width = 1.0f;		// radius
	float	height = 1.5f;
	float	theta = 0.0f;			// rotation angle
	vec3	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<button_t> create_buttons()
{
	std::vector<button_t> buttons;
	button_t c;

	c = { vec3(10.7f,-20.0f,0.0f),1.0f,0.7f,0.0f,vec3(0.0f,0.5f,0.5f) };
	buttons.emplace_back(c);
	vec3 _pos[] = { vec3(1.0f, -7.0f, 0.0f), vec3(1.0f, -10.0f, 0.0f),vec3(1.0f, -13.0f, 0.0f),vec3(1.0f, -16.0f, 0.0f),vec3(-4.0f, -7.0f, 0.0f),vec3(-9.0f, -7.0f, 0.0f) };
	vec3 _color[] = { vec3(1.0f,0.0f,0.0f),vec3(0.0f,0.0f,1.0f),vec3(0.0f,1.0f,0.0f),vec3(1.0f,1.0f,1.0f),vec3(1.0f,1.0f,0.0f),vec3(0.0f,0.0f,0.0f) };
	for (size_t i = 0; i < 6; i++) {
		button_t c;
		float	diff = 0.05f;
		c.center = _pos[i]; c.color = _color[i]; buttons.push_back(c);
		for (float diff = 0.05f, j = 0; j < 10; diff += 0.02f, j++) {
			c.center.x = _pos[i].x - diff; c.center.y = _pos[i].y + diff * 3 / 2; c.color = _color[i]; buttons.push_back(c);
		}
	}

	return buttons;
}

inline void button_t::update(float t)
{
	//radius = 0.35f + cos(t) * 0.1f;		// simple animation
	theta = t;
	float c = cos(theta), s = sin(theta);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		height, 0, 0, 0,
		0, width,0, 0,
		0, 0,  1, 0,
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
