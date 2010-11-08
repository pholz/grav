/*
 *  EnemyGenerator.h
 *  tear
 *
 *  Created by Peter Holzkorn on 17.10.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "cinder/Rand.h"


using namespace ci;
using namespace std;
//using namespace boost;

enum enemy_type {
	GOOD,
	BAD,
	UGLY
};



class Enemy {
private:
public:
	Vec2f pos, vel;
	float lifetime, expired;
	enemy_type type;
	
	Enemy(float lt, Rand* r);
	//~Enemy();
	
	void update(float dt);
	void draw();
};

typedef struct 
{
	int corner;
	Enemy* enemy;
} cornerCollision;

class EnemyGenerator {
private:
	vector<Enemy*> enemies;
	float interval, lifetime;
	float last, acc, total;
	Rand* rand;
	
	
public:
	EnemyGenerator(float interval, float lifetime);
//	~EnemyGenerator();
	
	void update(float dt);
	void draw();
};