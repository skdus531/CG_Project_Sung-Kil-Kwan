#pragma once
struct game_state {
	bool		 game_start = false;
	bool	  	 title = true;
	bool	 	 how_to = false;
	bool		 how_to_next = false;
	bool		 option = false;
	bool		 game_over = false;
	bool		 game_clear = false;
	bool		 hid_game_clear = false;
	int			 no_assignment = 0;
	int			 user_count = 1;
	int			 user_life = 3;
	int			 user_score = 0;
	unsigned int game_mode = 0;
	unsigned int game_level = 0;
	unsigned int game_stage = 1;
};
vec3 subject_color[] = {
	vec3(1.0f,0.0f,0.0f), //R
	vec3(0.0f, 1.0f, 0.0f), //G
	vec3(0.0f, 0.0f, 1.0f), //B
	vec3(0.7f, 0.2f, 0.3f),
	vec3(0.1f, 0.4f, 0.6f)
};
vec3 assignment_color = vec3(1.0f, 1.0f, 0.0f);
vec3 lecture_color = vec3(1.0f, 1.0f, 1.0f);
unsigned int subject_count = 3;
