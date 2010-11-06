#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

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
	
	Moon(Vec2f _p, Vec2f _v, Vec2f _a)
	{
		pos = _p;
		vel = _v;
		acc = _a;
	}
	
	void update(float dt, vector<Planet*>& planets)
	{
		
		pos += vel * dt;
		//vel += acc * dt * dt * .5;
		
		//vel = Vec2f(.0f, .0f);
		vel *= math<float>::pow(.98f, dt);
		
		vector<Planet*>::iterator it;
		for(it = planets.begin(); it < planets.end(); it++)
		{
			Vec2f moonToPlanet = ((*it)->pos - pos);
			//float fac = 100.0f;
			//moonToPlanet.x = fac / moonToPlanet.x;
			//moonToPlanet.y = fac / moonToPlanet.y;
			vel += moonToPlanet.normalized() * moonToPlanet.length();//20000.0f/(math<float>::max(moonToPlanet.length(), 100.0f));
			
			vel += (-5000.0f)* moonToPlanet.normalized() / moonToPlanet.length();
		}
		
		vel /= planets.size();
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::drawSolidCircle(Vec2f(0, 0), 10.0f, 32);
		
		glPopMatrix();
	}
};

class gravApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
	
	vector<Planet*> planets;
	Moon* moon1;
	Moon* moon2;
	float last;
};

void gravApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1024, 768);
	
	
}

void gravApp::setup()
{
	planets = vector<Planet*>();
//	planets.push_back(new Planet(Vec2f(150.0f, 200.0f), 40.0f));
//	planets.push_back(new Planet(Vec2f(230.0f, 170.0f), 50.0f));
//	planets.push_back(new Planet(Vec2f(650.0f, 400.0f), 30.0f));
	planets.push_back(new Planet(Vec2f(350.0f, 320.0f), 70.0f));
//	planets.push_back(new Planet(Vec2f(420.0f, 560.0f), 40.0f));
	
	moon1 = new Moon(Vec2f(50.0f, 50.0f), Vec2f(500.0f, .0f), Vec2f(.0f, .0f));
	moon2 = new Moon(Vec2f(250.0f, 750.0f), Vec2f(-500.0f, .0f), Vec2f(.0f, .0f));
	
	last = getElapsedSeconds();
}

void gravApp::mouseDown( MouseEvent event )
{
}

void gravApp::update()
{
	float now = getElapsedSeconds();
	float dt = now - last;
	last = now;
	
	moon1->update(dt, planets);
	moon2->update(dt, planets);
	
}

void gravApp::draw()
{
	// clear out the window with black
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
}


CINDER_APP_BASIC( gravApp, RendererGl )
