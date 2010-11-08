#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include <vector>
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct {
	float drag;
	float repulse;
	float distFactor;
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
	
	Moon(Vec2f _p, Vec2f _v, Vec2f _a)
	{
		pos = _p;
		vel = _v;
		acc = _a;
	}
	
	void update(float dt, vector<Planet*>& planets, paramStruct& params)
	{
		
		pos += vel * dt;
		//vel += acc * dt * dt * .5;
		
		//vel = Vec2f(.0f, .0f);
	//	vel *= math<float>::pow(params.drag, dt);
		
		vector<Planet*>::iterator it;
		for(it = planets.begin(); it < planets.end(); it++)
		{
			Vec2f moonToPlanet = ((*it)->pos - pos);
			
			vel += moonToPlanet.normalized() * 1.0f * (*it)->radius * (*it)->radius * 3.14 / (math<float>::max(moonToPlanet.length() * moonToPlanet.length(), 80.0f));

		//	vel += moonToPlanet.normalized() * params.distFactor * moonToPlanet.length();//20000.0f/(math<float>::max(moonToPlanet.length(), 100.0f));
			
		//	vel += (-params.repulse)* moonToPlanet.normalized() / moonToPlanet.length();
		}
		
		//vel /= planets.size();
		
		Vec2f newpos = (pos + vel * dt);
		for(it = planets.begin(); it < planets.end(); it++)
		{
			if(newpos.distance((*it)->pos) < (*it)->radius * 1.5f)
				vel -= ((*it)->pos - pos);
		}
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
	void keyDown( KeyEvent event );
	
	vector<Planet*> planets;
	Moon* moon1;
	Moon* moon2;
	float last;
	
	params::InterfaceGl		ifParams;
	paramStruct params;
};

void gravApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1024, 768);
	
	
}

void gravApp::setup()
{
	planets = vector<Planet*>();
//	planets.push_back(new Planet(Vec2f(150.0f, 200.0f), 40.0f));
	planets.push_back(new Planet(Vec2f(230.0f, 170.0f), 50.0f));
	planets.push_back(new Planet(Vec2f(650.0f, 400.0f), 30.0f));
	planets.push_back(new Planet(Vec2f(350.0f, 320.0f), 70.0f));
//	planets.push_back(new Planet(Vec2f(420.0f, 560.0f), 40.0f));
	
	moon1 = new Moon(Vec2f(50.0f, 50.0f), Vec2f(50.0f, .0f), Vec2f(.0f, .0f));
	moon2 = new Moon(Vec2f(250.0f, 750.0f), Vec2f(-50.0f, .0f), Vec2f(.0f, .0f));
	
	last = getElapsedSeconds();
	
	params.drag = .98f;
	params.repulse = 5000.0f;
	params.distFactor = 1.0f;
	
	// Setup the parameters
	ifParams = params::InterfaceGl( "Parameters", Vec2i( 200, 400 ) );
	ifParams.addParam( "Drag", &(params.drag), "min=0.1 max=1.0 step=0.001 keyIncr=Z keyDecr=z" );
	ifParams.addParam( "Repulse", &(params.repulse), "min=1000.0 max=8000.0 step=10 keyIncr=X keyDecr=x" );
	ifParams.addParam( "DistFactor", &(params.distFactor), "min=0.1 max=10.0 step=0.1 keyIncr=C keyDecr=c" );	
	
}

void gravApp::mouseDown( MouseEvent event )
{
}

void gravApp::update()
{
	float now = getElapsedSeconds();
	float dt = now - last;
	last = now;
	
	moon1->update(dt, planets, params);
	moon2->update(dt, planets, params);
	
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
}

void gravApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ){
		setup();
	}
}


CINDER_APP_BASIC( gravApp, RendererGl )
