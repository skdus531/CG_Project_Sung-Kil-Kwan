#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__
struct camera
{
	vec3	eye = vec3(50, 0, 20);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(-1, 0, 0);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f; // must be in radian
	float	aspect = 1.0f;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};


#endif