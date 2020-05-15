#pragma once
float prev_created = 0.0f;
struct obstacle_t {
	vec3 position = vec3(-300.0f, 0.0f, 0.0f);
	vec3 dimension = vec3(5.0f, 8.5f, 8.5f);
	vec3 color = vec3(0.9f, 0.1f, 0.1f);
	mat4 model_matrix;
	void update(float t);
};

void update_obstacles(std::list<obstacle_t>& obstacles, float t)
{
	for (auto p = obstacles.begin(); p != obstacles.end(); p++) {
		if (p->position.x > 300.0f) obstacles.erase(p);
	}
	if (t - prev_created > 0.5f) {
		obstacle_t o;
		int lane = rand() % 3;
		if (lane == 0) o.position.y = -20.0f;
		else if (lane == 2) o.position.y = 20.0f;
		obstacles.push_back(o);
		prev_created = t;
	}
}


inline void obstacle_t::update(float t)
{
	//radius	= 0.35f+cos(t)*0.1f;		// simple animation
	float theta = t*50.0f;
	float c = cos(theta), s = sin(theta);
	position.x += 5.0f;
	
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
			c, 0, s, 0,
			0, 1, 0, 0,
			-s, 0, c, 0,
			0, 0, 0, 1
	};
	/*{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};*/
	mat4 translate_matrix =
	{
		1, 0, 0, position.x,
		0, 1, 0, position.y,
		0, 0, 1, position.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}
