/*
 *  EnemyGenerator.cpp
 *  tear
 *
 *  Created by Peter Holzkorn on 17.10.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "EnemyGenerator.h"
#include "cinder/gl/gl.h"
#include "globals.h"

#define VEL 1.0f

EnemyGenerator::EnemyGenerator(float _iv, float _lt)
{
	interval = _iv;
	lifetime = _lt;
	
	last = .0f;
	acc = .0f;
	total = .0f;
	
	rand = new Rand();
}

int EnemyGenerator::collide(Vec2f circle, float radius, bool remove)
{
	int coll = 0;
	
	vector<Enemy*>::iterator it;
	for(it = enemies.begin(); it < enemies.end(); it++)
	{
		if( (*it)->pos.distance(circle) < 4.0f + radius + 20.0f )
		{
			coll++;
			if(remove) enemies.erase(it);
		}
			
	}
	
	return coll;
}

void EnemyGenerator::update(float dt)
{
	last = total;
	total += dt;
	acc += dt;
	
	if(acc >= interval)
	{
		acc = 0;
		Enemy *e = new Enemy(lifetime, rand);
		enemies.push_back(e);
		
	}
	
	vector<Enemy*>::iterator it;
	for(it = enemies.begin(); it < enemies.end(); it++)
	{
		(*it)->update(dt);
		if((*it)->expired > (*it)->lifetime)
			enemies.erase(it);
	}
}

void EnemyGenerator::draw()
{
	vector<Enemy*>::iterator it;
	for(it = enemies.begin(); it < enemies.end(); it++)
	{
		(*it)->draw();
	}
}

Enemy::Enemy(float lt, Rand* rand)
{
	lifetime = lt;
	expired = .0f;
//	int ty = rand->nextInt(0,10);
	
	/*
	if(ty < 6) type = BAD;
	else if(ty < 8) type = UGLY;
	else type = GOOD;
	*/
	
	type = GOOD;
	
	pos = Vec2f(rand->nextFloat(0, G_WIDTH), rand->nextFloat(0, G_HEIGHT));
	
	vel = Vec2f(rand->nextFloat(-VEL, VEL), rand->nextFloat(-VEL, VEL));
	
}

void Enemy::update(float dt)
{
	expired += dt;
	pos += vel;
}

void Enemy::draw()
{
	glPushMatrix();
	
	gl::translate(pos);
	if(type == GOOD)
		gl::color(PLANETCOLOR);
	else if(type == BAD)
		gl::color(Color(1.0f, .1f, 1.0f));
	else
		gl::color(Color(1.0f, 1.0f, .1f));
	gl::drawSolidCircle(Vec2f(.0f, .0f), 4.0f, 32);
	
	glPopMatrix();
}