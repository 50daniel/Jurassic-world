/************************************************************************
     File:        TrainView.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"
#include "RenderUtilities/Cubemap.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"

#include"Object.H"
class TrainView : public Fl_Gl_Window
{
public:
	// note that we keep the "standard widget" constructor arguments
	TrainView(int x, int y, int w, int h, const char* l = 0);

	// overrides of important window things
	virtual int handle(int);
	virtual void draw();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows = false);

	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick();
	void pick(int, int);
	void addDrop(glm::vec2, float, float);
	//set ubo
	void setUBO();

	void setLight(Shader* s);

	void setWave(Shader* s, float heigh, float length, int num, float dir[2]);
public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube;  // simple - just remember which cube is selected
	glm::vec2 center_coodinate;
	int count=0;


	TrainWindow* tw;				// The parent of this display window
	CTrack* m_pTrack;		// The track of the entire scene

	Shader* shader = nullptr;
	Texture2D* texture = nullptr;
	VAO* plane = nullptr;
	UBO* commom_matrices = nullptr;

	Shader* skyshader = nullptr;
	Cubemap* cubeMap = nullptr; //++
	VAO* skybox = nullptr;

	Shader* waveshader = nullptr;
	Texture2D* wavetexture = nullptr;
	VAO* waveVAO = nullptr;

	Shader* cubeshader = nullptr;
	Texture2D* cubetexture = nullptr;
	VAO* cubeVAO = nullptr;

	Shader* scrshader = nullptr;
	FBO* screenFBO[5] = { nullptr };
	VAO* quadVAO = nullptr;
	Shader* reflectshader = nullptr;
	Shader* refractshader = nullptr;

	Shader* heightMapshader = nullptr;
	Texture2D* heightMaptexture[200] = { nullptr };
	VAO* heightMapVAO = nullptr;
	Texture2D* heightMaptexture1 =  nullptr ;
	Texture2D* normaltexture1 =  nullptr ;


	Shader* watershader = nullptr;
	Texture2D* watertexture =  nullptr ;
	VAO* waterVAO = nullptr;

	Shader* water1shader = nullptr;
	Texture2D* watert1exture = nullptr;
	VAO* water1VAO = nullptr;


	Texture2D* uvtexture = nullptr;
	Texture2D* blacktexture = nullptr;
		//OpenAL
		glm::vec3 source_pos;
		glm::vec3 listener_pos;
		ALCdevice* device	= nullptr;
		ALCcontext* context = nullptr;
		ALuint source;
		ALuint buffer;

		Wave wave;
};