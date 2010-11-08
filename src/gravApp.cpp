#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include <vector>
#include "cinder/params/Params.h"
#include "EnemyGenerator.h"
#include "globals.h"

#define JUMPINERT .3f

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct {
	float speed;
	float repulse;
	float distFactor;
	int turn;
} paramStruct;

class Planet {
public:
	Vec2f pos;
	float radius;
	
	
	Planet(Vec2f _p, float _r)
	{
		pos = _p;
		radius = _r;
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::drawSolidCircle(Vec2f(0, 0), radius, 32);
		
		glPopMatrix();
	}
};

class Moon {
public:
	Vec2f pos, vel, acc;
	Planet* closestPlanet;
	int player;
	int turn;
	float jumpInert;
	paramStruct pars;
	
	Moon(Vec2f _p, Vec2f _v, Vec2f _a, int _ply)
	{
		pos = _p;
		vel = _v;
		acc = _a;
		closestPlanet = 0;
		player = _ply;
		turn = 0;
		jumpInert = .0f;
	}
	
	void jump()
	{
		if(!closestPlanet) return;
		
		//vel = (pos - closestPlanet->pos);
		
		jumpInert = JUMPINERT;
		
		vel *= 10.0f * pars.speed;
		
		closestPlanet = 0;
		
	}
	
	void update(float dt, vector<Planet*>& planets, paramStruct& params)
	{
		pars = params;
		
		if(jumpInert > .0f)
			jumpInert-=dt;
		if(jumpInert < .0f)
			jumpInert = .0f;
		
		turn = params.turn;
		
		pos += vel * dt;
		
		if(pos.x > getWindowWidth())
		{
			pos.x = pos.x-getWindowWidth();
		}
		
		if(pos.x < 0)
		{
			pos.x = getWindowWidth()+pos.x;
		}
		
		if(pos.y > getWindowHeight())
		{
			pos.y = pos.y-getWindowHeight();
		}
		
		if(pos.y < 0)
		{
			pos.y = getWindowHeight()+pos.y;
		}
		
		Planet* closest = planets[0];
		float dst = 999999999.0f;
		vector<Planet*>::iterator it;
		for(it = planets.begin(); it < planets.end(); it++)
		{
			Vec2f moonToPlanet = ((*it)->pos - pos);
			
			if(moonToPlanet.length() < dst)
			{
				closest = (*it);
				dst = moonToPlanet.length();
			}
				
			
		//	vel += moonToPlanet.normalized() * 1.0f * (*it)->radius * (*it)->radius * 3.14 / (math<float>::max(moonToPlanet.length() * moonToPlanet.length(), 80.0f));

		//	vel += moonToPlanet.normalized() * params.distFactor * moonToPlanet.length();//20000.0f/(math<float>::max(moonToPlanet.length(), 100.0f));
			
		//	vel += (-params.repulse)* moonToPlanet.normalized() / moonToPlanet.length();
		}
		
		Vec2f moonToPlanet = closest->pos - pos;
		
		if(!closestPlanet && moonToPlanet.length() > closest->radius * 2.5f)
		{
			vel += moonToPlanet.normalized() * 36.5f * pars.speed *
					closest->radius * closest->radius * closest->radius * (2.0f/3.0f) * M_PI 
					/ (math<float>::max(moonToPlanet.length() * moonToPlanet.length(), 80.0f));
		}
		if(!jumpInert && !closestPlanet && moonToPlanet.length() < closest->radius * 1.7f)
		{
			closestPlanet = closest;
			
		}
		if(closestPlanet && !jumpInert)
		{
			Vec2f r = pos - closestPlanet->pos;
			r.normalize();
			r.rotate(M_PI/1.97f);
			
			
			vel = r * 300.0f * pars.speed;
		}
		
		//vel /= planets.size();
		
		//
//		Vec2f newpos = (pos + vel * dt);
//		for(it = planets.begin(); it < planets.end(); it++)
//		{
//			if(newpos.distance((*it)->pos) < (*it)->radius * 1.5f)
//				vel -= ((*it)->pos - pos);
//		}
		
		closest = 0;
		delete closest;
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::drawSolidCircle(Vec2f(0, 0), 10.0f, 32);
		
		gl::drawVector(Vec3f(.0f,.0f,.0f), Vec3f(vel.x/5.0f, vel.y/5.0f, .0f), 5.0f, 4.0f);
		
		if(turn == player)
		{
			gl::color(Color(.5f, .5f, 1.0f));
			gl::drawStrokedCircle(Vec2f(0,0), 18.0f, 32);
		}
			
		
		glPopMatrix();
		
		if(closestPlanet)
		{
			glPushMatrix();
			
			gl::translate(closestPlanet->pos);
			
			gl::color(Color(player == 1 ? 1.0f : .0f, player == 2 ? 1.0f : .0f, .0f));
			gl::drawStrokedCircle(Vec2f(0,0), closestPlanet->radius*1.7f, 32);
			
			
			glPopMatrix();
		}
	}
};


class gravApp : public AppBasic {
public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
	void keyDown( KeyEvent event );
	
	vector<Planet*> planets;
	Moon* moon1;
	Moon* moon2;
	float last;
	EnemyGenerator* egen;
	
	params::InterfaceGl		ifParams;
	paramStruct params;
	int turn;
	float turncounter, turntime;
};


void gravApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(G_WIDTH, G_HEIGHT);
	
	
}

void gravApp::setup()
{
	egen = new EnemyGenerator(1.0f, 10.0f);
	
	planets = vector<Planet*>();
	planets.push_back(new Planet(Vec2f(750.0f, 600.0f), 40.0f));
	planets.push_back(new Planet(Vec2f(230.0f, 170.0f), 50.0f));
	planets.push_back(new Planet(Vec2f(650.0f, 400.0f), 30.0f));
	planets.push_back(new Planet(Vec2f(350.0f, 320.0f), 70.0f));
	planets.push_back(new Planet(Vec2f(320.0f, 560.0f), 40.0f));
	
	moon1 = new Moon(Vec2f(50.0f, 50.0f), Vec2f(50.0f, .0f), Vec2f(.0f, .0f), 1);
	moon2 = new Moon(Vec2f(250.0f, 750.0f), Vec2f(-50.0f, .0f), Vec2f(.0f, .0f), 2);
	
	last = getElapsedSeconds();
	
	params.speed = .7f;
	params.repulse = 5000.0f;
	params.distFactor = 1.0f;
	
	// Setup the parameters
	ifParams = params::InterfaceGl( "Parameters", Vec2i( 200, 400 ) );
	ifParams.addParam( "Speed", &(params.speed), "min=0.1 max=1.0 step=0.01 keyIncr=Z keyDecr=z" );
	ifParams.addParam( "x", &(params.repulse), "min=0.1 max=1.0 step=0.01 keyIncr=X keyDecr=x" );
	ifParams.addParam( "y", &(params.distFactor), "min=0.1 max=1.0 step=0.01 keyIncr=C keyDecr=c" );	
	
	turntime = 5.0f;
	turncounter = .0f;
	turn = 1;
	
	
}

void gravApp::mouseDown( MouseEvent event )
{
}

void gravApp::update()
{
	float now = getElapsedSeconds();
	float dt = now - last;
	last = now;
	
	turncounter += dt;
	
	if(turncounter >= turntime)
	{
		turncounter = .0f;
		turn = (turn == 1 ? 2 : 1);
	}
	
	params.turn = turn;
	
	moon1->update(dt, planets, params);
	moon2->update(dt, planets, params);
	
	egen->update(dt);
	
}

void gravApp::draw()
{
	// clear out the window with black
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::color(Color(1.0f, 1.0f, .0f));
	
	vector<Planet*>::iterator it;
	for(it = planets.begin(); it < planets.end(); it++)
	{
		(*it)->draw();
	}
		
	gl::color(Color(1.0f, 0.0f, .0f));
	moon1->draw();
	gl::color(Color(0.0f, 1.0f, .0f));
	moon2->draw();
	
	params::InterfaceGl::draw();
	
	egen->draw();
}

void gravApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ){
		setup();
	}
	
	if( event.getChar() == ' ' ){
		if(turn == 1)
			moon1->jump();
		if(turn == 2)
			moon2->jump();
	}
}


CINDER_APP_BASIC( gravApp, RendererGl )
