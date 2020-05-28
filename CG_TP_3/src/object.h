#pragma once
#include "constant.h"
class object_t {
public:
	
	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	vec3 size = vec3(1.0f, 1.0f, 1.0f);
	uint mesh_id = 0;
	uint tex_id = 0;
	int color_flag = 0; // 0: solid color
	int moving_level = 1;
	int rotating_level = 1;
	vec3 color = vec3(0.1f, 0.1f, 0.8f);
	vec3 rotation_axis = vec3(0,0,0);
	float theta = 0.0f; // animation
	unsigned int lane = 1;
	unsigned int subject_id = 0;
	mat4 model_matrix;
	object_t() {

	}
	object_t(vec3 position, vec3 size, uint mesh_id,uint tex_id, int color_flag, int moving_level, int rotating_level, vec3 color) {
		this->position = position;
		this->size = size;
		this->tex_id = tex_id;
		this->mesh_id = mesh_id;
		this->color_flag = color_flag;
		this->moving_level = moving_level;
		this->rotating_level = rotating_level;
		this->color = color;
	}
	void update(float t) {  // temp
		float theta = t;
		float c = (float)rotating_level* cos(theta), s = (float)rotating_level * sin(theta);
		position.x += (float)moving_level*0.1f;
		mat4 scale_matrix = {
			size.x, 0, 0, 0,
			0, size.y, 0, 0,
			0, 0, size.z, 0,
			0, 0, 0, 1
		};
		mat4 rotation_matrix = {
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
		};
		if(rotating_level!=0) rotation_matrix = mat4::rotate(rotation_axis, (float)rotating_level * theta);
		mat4 translate_matrix =
		{
			1, 0, 0, position.x,
			0, 1, 0, position.y,
			0, 0, 1, position.z,
			0, 0, 0, 1
		};
		model_matrix = translate_matrix * rotation_matrix * scale_matrix;
	}
	
	void update_to_random_lane(int f) {
		if ((f % 1000) != 0) return;
		unsigned int next_lane;
		while ((next_lane = rand() % 3) == lane);
		lane = next_lane;
		if (lane == 0) position.y = -20.0f;
		else if (lane == 1) position.y = 0.0f;
		else position.y = 20.0f;
	}
};



class obstacle_t : public object_t {
public:
	unsigned int type=0;
	unsigned int subject_id = 0;
};

std::vector<object_t> create_map(void) {
	std::vector<object_t> ret;
	vec3 _pos[] = { vec3(0.0f, -30.0f, -20.0f), vec3(-600.0f, -30.0f, -20.0f),
		vec3(0.0f, 0.0f, -20.0f), vec3(-600.0f, 0.0f, -20.0f),
		vec3(0.0f, 30.0f, -20.0f), vec3(-600.0f, 30.0f, -20.0f)
	};
	vec3 _size = vec3(300.0f, 15.0f, 8.5f);
	vec3 _col[] = {
		vec3(0.1f, 0.1f, 0.8f), vec3(0.1f, 0.1f, 0.8f),
		vec3(0.1f, 0.8f, 0.1f),vec3(0.1f, 0.8f, 0.1f),
		vec3(1.0f, 1.0f, 0.0f),vec3(1.0f, 1.0f, 0.0f)
	};
	for (int i = 0; i < 6; i++) {
		object_t tmp;
		tmp.position = _pos[i];
		tmp.size = _size;
		tmp.mesh_id = static_cast<uint>(MeshType::cube);
		tmp.moving_level = 0;
		tmp.rotating_level = 0;
		tmp.color = _col[i];
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<object_t> create_title_buttons()
{
	std::vector<object_t> ret;
	float x = 0.0f;
	for (int i = 0; i < 4; i++, x += 3.0f) {
		object_t tmp;
		tmp.position = vec3(0, 0, -x);
	//	printf("%f %f %f\n", tmp.position.x, tmp.position.y, tmp.position.z);
		tmp.size = vec3(0.1f, 5.0f, 1.0f);
		//tmp.mesh_id = static_cast<uint>(meshtype::square);
		tmp.moving_level = 0;
		tmp.rotating_level = 0;
		tmp.color = vec3(0.0f, 0.5f, 0.5f);
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<object_t> create_main_menu_buttons()
{
	std::vector<object_t> ret;


	object_t tmp;
	tmp.position = vec3(0, 0, -4.0f);
	//	printf("%f %f %f\n", tmp.position.x, tmp.position.y, tmp.position.z);
	tmp.size = vec3(0.1f, 4.0f, 1.0f);
	//tmp.mesh_id = static_cast<uint>(meshtype::square);
	tmp.moving_level = 0;
	tmp.rotating_level = 0;
	tmp.color = vec3(0.0f, 0.5f, 0.5f);
	ret.push_back(tmp);

	return ret;
}

inline std::vector<object_t> create_tmp_walls()
{
	std::vector<object_t> ret;
	object_t c;

	c.position = vec3(0, 20.0f, 10.7f);
	c.size = vec3(0.1f, 1.0f, 0.7f);
	c.moving_level = 0;
	c.rotating_level = 0;
	c.color = vec3(0, 0.5f, 0.5f);
	ret.push_back(c);

	vec3 _pos[] = { vec3(0.0f, 7.0f, 1.0f), vec3(0.0f, 10.0f, 1.0f),vec3(0.0f, 13.0f, 1.0f),vec3(0.0f, 16.0f, 1.0f),vec3(0.0f, 7.0f, -4.0f),vec3(0.0f, 7.0f, -9.0f) };
	vec3 _color[] = { vec3(1.0f,0.0f,0.0f),vec3(0.0f,0.0f,1.0f),vec3(0.0f,1.0f,0.0f),vec3(1.0f,1.0f,1.0f),vec3(1.0f,1.0f,0.0f),vec3(0.0f,0.0f,0.0f) };
	for (size_t i = 0; i < 6; i++) {
		object_t c;
		float	diff = 0.05f;
		c.position = _pos[i]; c.size = vec3(0.1f, 1.0f, 1.5f); c.color = _color[i]; c.moving_level = 0; c.rotating_level = 0; ret.push_back(c);
		for (float diff = 0.05f, j = 0; j < 10; diff += 0.02f, j++) {
			c.position.z = _pos[i].z - diff; c.position.y = _pos[i].y - diff * 3 / 2; c.color = _color[i]; ret.push_back(c);
		}
	}

	return ret;
}

inline std::vector<object_t> create_option_buttons() {
	std::vector<object_t> ret;
	object_t tmp;

	tmp.position = vec3(0, 20.0f, 10.7f);
	tmp.size = vec3(0.1f, 1.0f, 0.7f);
	tmp.moving_level = 0;
	tmp.rotating_level = 0;
	tmp.color = vec3(0, 0.5f, 0.5f);
	ret.push_back(tmp);

	vec3 _pos[] = { vec3(0.0f, -10.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f),vec3(0.0f, 14.0f, 0.0f) };
	vec3 _color[] = { vec3(0,0,1.0f),vec3(0,0.5f,0.5f), vec3(1.0f,0,0) };
	for (size_t i = 0; i < 3; i++) {
		object_t tmp;;
		tmp.position = _pos[i];
		tmp.color = _color[i];
		tmp.size = vec3(0.1f, 3.0f, 1.0f);
		tmp.rotating_level = 0;
		tmp.moving_level = 0;
		ret.push_back(tmp);
	}

	return ret;
}

std::vector<object_t> create_student(void) {
	std::vector<object_t> ret;
	vec3 _pos[] = { vec3(60.0f, 0.0f, 0.0f) };
	vec3 _size = vec3(7.0f, 7.0f, 7.0f);
	vec3 _col[] = {
		vec3(0.4f,0.3f,0.8f)
	};
	for (int i = 0; i < 1; i++) {
		object_t tmp;
		tmp.position = _pos[i];
		tmp.size = _size;
		tmp.mesh_id = static_cast<uint>(MeshType::sphere);
		tmp.moving_level = 0;
		tmp.rotating_level = -5;
		tmp.color = subject_color[0];
		tmp.rotation_axis = vec3(0, 1,0);
		ret.push_back(tmp);
	}
	return ret;
}
std::vector<object_t> create_professor(void) {
	std::vector<object_t> ret;
	vec3 _pos[] = { vec3(-300.0f, 0.0f, 0.0f) };
	vec3 _size = vec3(7.0f, 7.0f, 7.0f);
	vec3 _col[] = {
		vec3(0.4f,0.3f,0.8f)
	};
	for (int i = 0; i < 1; i++) {
		object_t tmp;
		tmp.position = _pos[i];
		tmp.size = _size;
		tmp.mesh_id = static_cast<uint>(MeshType::sphere);
		tmp.moving_level = 0;
		tmp.rotating_level = -5;
		tmp.color = _col[i];
		tmp.rotation_axis = vec3(0, 1, 0);
		ret.push_back(tmp);
	}
	return ret;
}
std::vector<object_t> create_pen(void) {
	std::vector<object_t> ret;
	vec3 _pos[] = { vec3(80.0f, 0.0f, 15.0f) };
	vec3 _size = vec3(1.0f,1.0f, 0.5f);
	vec3 _col[] = {
		vec3(0.1f,0.3f,0.8f)
	};
	for (int i = 0; i < 1; i++) {
		object_t tmp;
		tmp.position = _pos[i];
		tmp.size = _size;
		tmp.mesh_id = static_cast<uint>(MeshType::pen);
		tmp.moving_level = 0;
		tmp.rotating_level = 0;
		tmp.color = _col[i];
		tmp.rotation_axis = vec3(0, 1, 0);
		ret.push_back(tmp);
	}
	return ret;
}

void update_obstacles(std::list<obstacle_t>& obstacle_list, unsigned int lane, int frame_count, int stage) {
	for (auto t = obstacle_list.begin(); t != obstacle_list.end(); t++) {
		if (t->position.x > 200.0f) obstacle_list.erase(t);
	}
	if ((frame_count) % (2400 / stage) == 0) {
		obstacle_t o;
		//type 0: 시험, type 1:과제, type 2: 강의
		o.type = rand() % 3;
		o.subject_id = rand() % subject_count;
		if (o.type == 0) o.color = subject_color[o.subject_id];
		else if (o.type == 1) o.color = assignment_color;
		else if (o.type == 2) o.color = lecture_color;
		o.position = vec3(-290.0f, 0.0f, 0.0f);
		if (lane == 0) o.position.y -= 27.5f;
		else if (lane == 2) o.position.y += 27.5f;
		o.lane = lane;
		o.size = vec3(5.0f, 12.0f, 15.0f);
		o.rotating_level = 0;
		o.moving_level = stage;
		obstacle_list.push_back(o);
	}

}
void insert_new_pen(std::list<object_t>& pen_list, unsigned int student_lane) {
	vec3 _pos =  vec3(70.0f, 2.0f, 5.0f);
	vec3 _size = vec3(1.0f, 1.0f, 0.5f);
	vec3 _col = vec3(0.1f, 0.3f, 0.8f);
	object_t tmp;
	tmp.position = _pos;
	tmp.lane = student_lane;
	if (student_lane == 0) tmp.position.y -= 20.5f;
	else if (student_lane == 2) tmp.position.y +=15.0f;
	tmp.size = _size;
	tmp.mesh_id = static_cast<uint>(MeshType::pen);
	tmp.moving_level = -1;
	tmp.rotating_level = 0;
	tmp.color = _col;
	pen_list.push_back(tmp);
}