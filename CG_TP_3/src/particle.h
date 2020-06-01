#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#pragma once

#include "cgmath.h"
#include "cgut.h"


inline float random_range( float min, float max ){ return mix( min, max, rand()/float(RAND_MAX) ); }
struct particle_t
{

	vec3 pos;
	vec4 color;
	vec3 velocity;
	float scale;
	float life;

	//optional
	float elapsed_time;
	float time_interval;

	particle_t() { reset(); }
	void reset();
	void set(vec3 pos, vec4 color);
	void update(vec4 color, float t);
};
inline void particle_t::reset()
{
	//pos = vec3(random_range(0, 0.5f), random_range(0, 0.5f),0);
	pos = vec3(-0.5f,0,0);
	//this->pos = vec3(55,0,10);
	//color = vec4(random_range(0, 1.0f), random_range(0, 1.0f), random_range(0, 1.0f), 1);
	color = vec4(0.8f, 0, 0, 1.0f);

	scale = random_range(0.005f, 0.01f);
	life = 0.0000001f;
	velocity = vec3(random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f), random_range(-1.0f, 1.0f)) * 0.005f;
	elapsed_time = 0.0f;
	time_interval = 100.0f;
}

inline void particle_t::set(vec3 pos, vec4 color) {
	this->pos = pos;
	this->color = color;
}

inline void particle_t::update( vec4 color, float t)
{
	this->color = color;

	const float dwTime = t;
	elapsed_time += dwTime;
	pos += velocity;

	constexpr float life_factor = 10000.0f;
	life -= life_factor * dwTime;

	// disappear
	if (life < 0.0f)
	{
		constexpr float alpha_factor = 0.01f;
		this->color.a -= alpha_factor * dwTime;
	}

	// dead
	//if (this->color.a < 0.0f) reset();
}

#endif