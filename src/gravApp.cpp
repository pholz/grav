#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include <vector>
#include "cinder/params/Params.h"
#include "EnemyGenerator.h"
#include "globals.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/Callback.h"
#include "Resources.h"

#define JUMPINERT .3f
#define MAXCHARGE 4.0f
#define MOONSIZE 10.0f

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct {
	float r;
	float theta;
} polarCoords;

Vec2f cart(float r, float theta)
{
	return Vec2f(r * math<float>::cos(theta), r * math<float>::sin(theta));
}

polarCoords polar(Vec2f v)
{
	polarCoords p;
	p.r = math<float>::sqrt(v.x*v.x + v.y*v.y);
	p.theta = math<float>::atan2(v.x, v.y);
	
	return p;
}

typedef struct {
	float speed;
	float repulse;
	float distFactor;
	int turn;
} paramStruct;

class Planet {
public:
	Vec2f pos, vel;
	float radius;
	
	
	Planet(Vec2f _p, float _r)
	{
		pos = _p;
		radius = _r;
		Vec2f middle = Vec2f(getWindowWidth()/2.0f, getWindowHeight()/2.0f);
		vel = pos - middle;
		vel.normalize();
		vel.rotate(M_PI/1.97f);
		vel *= 25.0f;
	}
	
	void update(float dt)
	{
		Vec2f middle = Vec2f(getWindowWidth()/2.0f, getWindowHeight()/2.0f);
		vel = pos - middle;
		vel.normalize();
		vel.rotate(M_PI/1.97f);
		vel *= 25.0f;
		
		pos += vel * dt;
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::color(PLANETCOLOR);
		gl::drawSolidCircle(Vec2f(0, 0), radius, 128);
		
		glPopMatrix();
	}
};

class Particle
{
public:
	Vec2f pos, vel;
	float lt, time;
	Color color;
	
	Particle(Vec2f _pos, Vec2f _vel, float _lt, Color _c)
	{
		pos = _pos;
		vel = _vel;
		lt = _lt;
		time = .0f;
		color = _c;
	}
	
	void update(float dt)
	{
		pos += vel * dt;
		time += dt;
	}
	
	void draw()
	{
		glPushMatrix();
		gl::translate(pos);
		
		gl::color(color);
		
		gl::drawSolidCircle(Vec2f(.0f, .0f), 4.0f, 16);
		
		glPopMatrix();
	}
};

class Moon {
public:
	Vec2f pos, vel, acc;
	Planet* closestPlanet;
	Planet* nearest;
	int player;
	int turn;
	float jumpInert;
	paramStruct pars;
	float charge;
	bool charging;
	Color color;
	int score;
	Rand* rand;
	
	vector<Particle*> losing;
	
	Moon(Vec2f _p, Vec2f _v, Vec2f _a, int _ply)
	{
		pos = _p;
		vel = _v;
		acc = _a;
		closestPlanet = 0;
		player = _ply;
		turn = 0;
		jumpInert = .0f;
		charge = .0f;
		charging = false;
		score = 0;
		
		rand = new Rand();
	}
	
	void startCharge()
	{
		charging = true;
	}
	
	void jump()
	{
		if(!closestPlanet) return;
		
		//vel = (pos - closestPlanet->pos);
		
		jumpInert = JUMPINERT;
		
		vel *= 10.0f * pars.speed * charge;
		
		closestPlanet = 0;
		
		charging = false;
		charge = .0f;
		
	}
	
	void update(float dt, vector<Planet*>& planets, paramStruct& params)
	{
		if(charging && charge < MAXCHARGE) charge += dt * 1.5f;
		
		pars = params;
		
		if(jumpInert > .0f)
			jumpInert-=dt;
		if(jumpInert < .0f)
			jumpInert = .0f;
		
		if(turn != params.turn)
		{
			charging = false;
			charge = .0f;
		}
		
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
		
		nearest = closest;
		
		Vec2f moonToPlanet = closest->pos - pos;
		
		if(!closestPlanet && moonToPlanet.length() > closest->radius * 2.5f)
		{
			vel += moonToPlanet.normalized() * 36.5f * pars.speed *
					closest->radius * closest->radius * closest->radius * (2.0f/3.0f) * M_PI 
					/ (math<float>::max(moonToPlanet.length() * moonToPlanet.length(), 80.0f));
		}
		if(!jumpInert && !closestPlanet && moonToPlanet.length() < closest->radius * 1.7f && losing.size() == 0)
		{
			closestPlanet = closest;
			
		}
		if(closestPlanet && !jumpInert && losing.size() == 0)
		{
			Vec2f m2cp = (closestPlanet->pos - pos);
			
			Vec2f r = pos - closestPlanet->pos;
			r.normalize();
			r.rotate(M_PI/1.97f);
		  
			vel = r * 300.0f * pars.speed;
			
			if(m2cp.length() < closestPlanet->radius)
			{
				pos += -m2cp.normalized() * (5.0f + closestPlanet->radius - m2cp.length());
			}
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
		
		// particles
		
		vector<Particle*>::iterator pit;
		for(pit = losing.begin(); pit < losing.end(); pit++)
		{
			(*pit)->update(dt);
			if((*pit)->time > (*pit)->lt)
				losing.erase(pit);
		}
	}
	
	void draw()
	{
		glPushMatrix();
		
		gl::translate(pos);
		
		gl::color(color);
		gl::drawSolidCircle(Vec2f(0, 0), MOONSIZE, 32);
		
	//	gl::color(Color(.2f, .2f, .2f));
	//	gl::drawVector(Vec3f(.0f,.0f,.0f), Vec3f(vel.x/5.0f, vel.y/5.0f, .0f), 5.0f, 4.0f);
		
		if(charging)
		{
			gl::color(Color(1.0f, 1.0f, 1.0f));
			gl::drawVector(Vec3f(.0f,.0f,.0f), Vec3f(charge * vel.x/5.0f, charge * vel.y/5.0f, .0f), 5.0f, 4.0f);
		}
		
		if(turn == player)
		{
			gl::color(Color(1.0f, 1.0f, 1.0f));
			gl::drawStrokedCircle(Vec2f(0,0), 18.0f, 64);
		}
			
		
		glPopMatrix();
		
		if(closestPlanet)
		{
			glPushMatrix();
			
			gl::translate(closestPlanet->pos);
			
			gl::color(ColorA(color, 0.4f));
			gl::drawStrokedCircle(Vec2f(0,0), closestPlanet->radius*1.7f, 128);
			
			
			glPopMatrix();
		}
		
		
		// particles
		
		vector<Particle*>::iterator it;
		for(it = losing.begin(); it < losing.end(); it++)
		{
			(*it)->draw();
		}
	}
	
	void loseScore(Vec2f impact, int num)
	{
		vel -= impact;
		
		closestPlanet = 0;
		
		for(int i = 0; i < num; i++)
			losing.push_back(new Particle(pos, Vec2f(rand->nextFloat(20.5f), rand->nextFloat(20.5f)), 5.0f, color));
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
	void keyUp( KeyEvent event );
	void sineWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer );
	void squareWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer );
	void drawBG();
	
	vector<Planet*> planets;
	Moon* moon1;
	Moon* moon2;
	Moon* curMoon;
	Moon* winner;
	float last;
	EnemyGenerator* egen;
	
	params::InterfaceGl		ifParams;
	paramStruct params;
	int turn;
	float turncounter, turntime;
	
	float mFreqTarget, mFreqTargetQ;
	float mPhase, mPhaseQ;
	float mPhaseAdjust, mPhaseAdjustQ;
	float mMaxFreq, mMaxFreqQ;
	
	audio::SourceRef s_hit;
	audio::SourceRef s_up;
	shared_ptr< audio::Callback<gravApp, float> > cb1, cb2;
	
	float collInert;
	bool end;
};


void gravApp::prepareSettings(Settings* settings)
{
//	settings->setWindowSize(G_WIDTH, G_HEIGHT);
	
	cb1 = audio::createCallback( this, &gravApp::sineWave ) ;
	cb2 = audio::createCallback( this, &gravApp::squareWave );
	
	audio::Output::play( cb1 );
	audio::Output::play( cb2 );
	
	settings->setFullScreen(true);
}

void gravApp::setup()
{
	egen = new EnemyGenerator(1.0f, 10.0f);
	
	planets = vector<Planet*>();
	planets.push_back(new Planet(Vec2f(750.0f, 600.0f), 40.0f));
	planets.push_back(new Planet(Vec2f(430.0f, 270.0f), 50.0f));
	planets.push_back(new Planet(Vec2f(650.0f, 400.0f), 30.0f));
	planets.push_back(new Planet(Vec2f(450.0f, 320.0f), 70.0f));
	planets.push_back(new Planet(Vec2f(520.0f, 560.0f), 40.0f));
	planets.push_back(new Planet(Vec2f(930.0f, 260.0f), 40.0f));
	planets.push_back(new Planet(Vec2f(1120.0f, 760.0f), 50.0f));
	planets.push_back(new Planet(Vec2f(1200.0f, 800.0f), 30.0f));
	
	moon1 = new Moon(Vec2f(50.0f, 50.0f), Vec2f(50.0f, .0f), Vec2f(.0f, .0f), 1);
	moon2 = new Moon(Vec2f(250.0f, 750.0f), Vec2f(-50.0f, .0f), Vec2f(.0f, .0f), 2);
	
	moon1->color = Color(1.0f,			133.0f/255.0f,	163.0f/255.0f);
	moon2->color = Color(224.0f/255.0f, 1.0f,			133.0f/255.0f);
	
	last = getElapsedSeconds();
	
	params.speed = .7f;
	params.repulse = 5000.0f;
	params.distFactor = 1.0f;
	
	// Setup the parameters
	ifParams = params::InterfaceGl( "Parameters", Vec2i( 200, 400 ) );
	ifParams.addParam( "Speed", &(params.speed), "min=0.1 max=1.0 step=0.01 keyIncr=Z keyDecr=z" );
	ifParams.addParam( "x", &(params.repulse), "min=0.1 max=1.0 step=0.01 keyIncr=X keyDecr=x" );
	ifParams.addParam( "y", &(params.distFactor), "min=0.1 max=1.0 step=0.01 keyIncr=C keyDecr=c" );	
	
	turntime = 8.0f;
	turncounter = .0f;
	turn = 1;
	collInert = .0f;
	
	gl::enableAlphaBlending( false );
	
	curMoon = moon1;
	
	// audio
	
	mMaxFreq = 2000.0f;
	mFreqTarget = 0.0f;
	mPhase = 0.0f;
	mPhaseAdjust = 0.0f;
	
	mMaxFreqQ = 2000.0f;
	mFreqTargetQ = 0.0f;
	mPhaseQ = 0.0f;
	mPhaseAdjustQ = 0.0f;
	
	
	
	s_hit = audio::load( loadResource( RES_HIT ) );
	s_up = audio::load( loadResource( RES_UP ) );
	
	end = false;
	winner = 0;
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
		if(turn == 1)	curMoon = moon1;
		else			curMoon = moon2;
	}
	
	params.turn = turn;
	
	if(collInert > .0f) collInert -= dt;
	if(collInert < .0f) collInert = .0f;
	
	vector<Planet*>::iterator it;
	for(it = planets.begin(); it < planets.end(); it++)
	{
		(*it)->update(dt);
	}
	
	moon1->update(dt, planets, params);
	moon2->update(dt, planets, params);
	
	egen->update(dt);
	
	int num1 = egen->collide(moon1->pos, MOONSIZE, true);
	int num2 = egen->collide(moon2->pos, MOONSIZE, true);
	
	moon1->score += num1;
	moon2->score += num2;
	
	if(num1 > 0 || num2 > 0)
		audio::Output::play( s_up );
	
	if(moon1->pos.distance(moon2->pos) < 3*MOONSIZE)
	{
		Moon* loser = moon1->vel.length() < moon2->vel.length() ? moon1 : moon2;
		Moon* winner = (loser == moon1 ? moon2 : moon1);
		
		if(loser->losing.size() == 0 && loser->score >= 3 && !collInert)
		{
			audio::Output::play( s_hit );
			loser->score -= 3;
			loser->loseScore(winner->vel, 3);
			
			collInert = 5.0f;
		}
		
	}
	
	if(num1 > 0)
		cout << "SCORE P1";
	if(num2 > 0)
		cout << "SCORE P2";
	
	
	mFreqTarget = math<float>::clamp(moon1->nearest->pos.distance(moon1->pos) + 80.0f, 100.0f, 1000.0f);
	mFreqTargetQ = math<float>::clamp(moon2->nearest->pos.distance(moon2->pos) + 80.0f, 100.0f, 1000.0f);
	
	if(moon1->score >= 12)
	{
		end = true;
		winner = moon1;
	} else if(moon2->score >= 12)
	{
		end = true;
		winner = moon2;
	}
}

void gravApp::drawBG()
{
	glPushMatrix();
	
	gl::translate(Vec2f(getWindowWidth()/2.0f, getWindowHeight()/2.0f));
	
	gl::rotate(getElapsedSeconds()*5.0f);
	
	
	
	gl::drawSolidCircle(Vec2f(.0f, .0f), 200.0f, 128);
	
	float sstepsize = M_PI/4.0f;
	int ssteps = 2*M_PI/sstepsize + 1;
	for(int i = 0; i < ssteps; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		
		for(int j = 0; j < 16; j++)
		{
			float offset = i*sstepsize;
			glVertex2f(cart(250.0f, offset + j * sstepsize/24.0f));
			glVertex2f(cart(1200.0f, offset + j * sstepsize/24.0f));
		}
		
		glEnd();
	}
	
	glPopMatrix();
}

void gravApp::draw()
{
	if(end) 
	{
		gl::clear( winner->color ); 
		return;
	}
	// clear out the window with black
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::color(Color(1.0f, 1.0f, .0f));
	
	
	/// BG
	//gl::enableWireframe();
	
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( -2.5f, -2.5f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glLineWidth( 3.0f );
	
	gl::color(Color(1.0f, .5f, .0f));
	drawBG();
	
	//
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	gl::color(ColorA(.0f, .0f, .0f, .8f));
	drawBG();
	
	//gl::color(ColorA(1.0f, .5f, .0f, .3f));
	//drawBG();

	
	glPopAttrib();
	
	
//	gl::disableWireframe();
	
	/// /BG
	
	vector<Planet*>::iterator it;
	for(it = planets.begin(); it < planets.end(); it++)
	{
		(*it)->draw();
	}
		
	gl::color(Color(1.0f, 0.0f, .0f));
	moon1->draw();
	gl::color(Color(0.0f, 1.0f, .0f));
	moon2->draw();
	
	//params::InterfaceGl::draw();
	
	egen->draw();
	
	
	/// turntime display
	
	glPushMatrix();
	
	gl::translate(Vec2f(getWindowWidth() / 2.0f, 80.0f));
	
	gl::color(ColorA((turn == 1 ? moon2 : moon1)->color, .2f));
	
	gl::drawSolidCircle(Vec2f(.0f, .0f), 30.0f, 128);
	
	gl::color(curMoon->color);

	glBegin(GL_TRIANGLE_FAN);
	
	glVertex2f(Vec2f(.0f, .0f));
	
	float stepsize = M_PI/64.0f;
	int steps = 2*M_PI/stepsize + 1;
	for(int i = steps * turncounter/turntime; i < steps; i++)
	{
		glVertex2f(cart(30.0f, i * stepsize));
	}
	
	glEnd();
	
	glPopMatrix();
	
	/// scores
	
	glPushMatrix();
	
	gl::translate(Vec2f(50.0f, 100.0f));
	
	gl::color(ColorA(moon1->color, .7f));
	
	for(int i = 0; i < moon1->score; i++)
	{
		gl::drawSolidCircle(Vec2f(.0f, i * 20.0f), 4.0f, 16.0f);
	}
	
	glPopMatrix();
	
	glPushMatrix();
	
	gl::translate(Vec2f(getWindowWidth() - 50.0f, 100.0f));
	
	gl::color(ColorA(moon2->color, .7f));
	
	for(int i = 0; i < moon2->score; i++)
	{
		gl::drawSolidCircle(Vec2f(.0f, i * 20.0f), 4.0f, 16.0f);
	}
	
	glPopMatrix();
	
}

void gravApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ){
		setup();
	}
	
	if( event.getChar() == ' ' ){
		if(turn == 1 && !moon1->charging)
			moon1->startCharge();
		if(turn == 2 && !moon2->charging)
			moon2->startCharge();
	}
}

void gravApp::keyUp( KeyEvent event)
{
	if( event.getChar() == ' ' ){
		if(turn == 1)
			moon1->jump();
		if(turn == 2)
			moon2->jump();
	}
}

void gravApp::sineWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer ) 
{
	mPhaseAdjust = mPhaseAdjust * 0.95f + ( mFreqTarget / 44100.0f ) * 0.05f;
	for( int  i = 0; i < ioSampleCount; i++ ) {
		mPhase += mPhaseAdjust;
		mPhase = mPhase - math<float>::floor( mPhase );
		float val = math<float>::sin( mPhase * 2.0f * M_PI );
		
		ioBuffer->mData[i*ioBuffer->mNumberChannels] = val;
		ioBuffer->mData[i*ioBuffer->mNumberChannels + 1] = val;
	}
}

void gravApp::squareWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer ) 
{
	mPhaseAdjustQ = mPhaseAdjustQ * 0.95f + ( mFreqTargetQ * 1.5f / 44100.0f ) * 0.05f;
	for( int  i = 0; i < ioSampleCount; i++ ) {
		mPhaseQ += mPhaseAdjustQ;
		mPhaseQ = mPhaseQ - math<float>::floor( mPhaseQ );
		float val = math<float>::sin( mPhaseQ * 2.0f * M_PI );
		
		ioBuffer->mData[i*ioBuffer->mNumberChannels] = val;
		ioBuffer->mData[i*ioBuffer->mNumberChannels + 1] = val;
	}
}


CINDER_APP_BASIC( gravApp, RendererGl )
