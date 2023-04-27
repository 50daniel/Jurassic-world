/************************************************************************
     File:        TrainView.cpp

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

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"



#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif
using namespace std;
const unsigned int SCR_WIDTH = 590;
const unsigned int SCR_HEIGHT = 590;

//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event)) 
			return 1;

	// remember what button was used
	static int last_push;

	switch(event) {
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				doPick();
				damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
								static_cast<double>(cp->pos.x), 
								static_cast<double>(cp->pos.y),
								static_cast<double>(cp->pos.z),
								rx, ry, rz,
								(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float) rx;
				cp->pos.y = (float) ry;
				cp->pos.z = (float) rz;
				damage(1);
			}
			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			focus(this);
			break;

		case FL_KEYBOARD:
		 		int k = Fl::event_key();
				int ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				};
				break;
	}

	return Fl_Gl_Window::handle(event);
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	static GLuint* cubemapID;
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		if (!this->cubeshader)
			this->cubeshader = new
			Shader(
				PROJECT_DIR "/src/shaders/cube.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/cube.frag");
		if (!this->waveshader)
			this->waveshader = new
			Shader(
				PROJECT_DIR "/src/shaders/wave.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/wave.frag");
		/*if (!this->shader)
			this->shader = new
			Shader(
				PROJECT_DIR "/src/shaders/cube.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/cube.frag");*/
		if (!this->skyshader)
			this->skyshader = new
			Shader(
				PROJECT_DIR "/src/shaders/skybox.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/skybox.frag");

		if (!this->scrshader)
			this->scrshader = new
			Shader(
				PROJECT_DIR "/src/shaders/scr.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/scr.frag");
		if (!this->heightMapshader)
			this->heightMapshader = new
			Shader(
				PROJECT_DIR "/src/shaders/heightmap.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/heightmap.frag");
		if (!this->watershader)
			this->watershader = new
			Shader(
				PROJECT_DIR "/src/shaders/water.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/water.frag");

		if (!this->water1shader)
			this->water1shader = new
			Shader(
				PROJECT_DIR "/src/shaders/water_update.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/water_update.frag");


		if (!this->commom_matrices)
			this->commom_matrices = new UBO();
		this->commom_matrices->size = 2 * sizeof(glm::mat4);
		glGenBuffers(1, &this->commom_matrices->ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
		glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//VAO
		//if (!this->plane) {
		//	int width = 100, lenth = 100;
		//	GLfloat  vertices[100 * 100 * 3];
		//	GLfloat  normal[100 * 100 * 3] = { 0.0f };
		//	GLfloat  texture_coordinate[100 * 100 * 2] = { 0.0f };
		//	GLuint element[(100 - 1) * (100 - 1) * 6];
		//	for (int i = 0; i < lenth; i++)
		//	{
		//		for (int j = 0; j < width; j++)
		//		{
		//			vertices[(i * lenth + j) * 3] = j;
		//			vertices[(i * lenth + j) * 3 + 1] = 0.0f;
		//			vertices[(i * lenth + j) * 3 + 2] = i;
		//			normal[(i * lenth + j) * 3 + 1] = 1.0f;
		//			texture_coordinate[(i * lenth + j) * 2] = j * 0.01;
		//			texture_coordinate[(i * lenth + j) * 2 + 1] = i * 0.01;
		//		}
		//	}

		//	for (int i = 0; i < lenth - 1; i++)
		//	{
		//		for (int j = 0; j < width - 1; j++)
		//		{
		//			element[(i * (lenth - 1) + j) * 6] = i * lenth + j;
		//			element[(i * (lenth - 1) + j) * 6 + 1] = i * lenth + j + 1;
		//			element[(i * (lenth - 1) + j) * 6 + 2] = (i + 1) * lenth + j;

		//			element[(i * (lenth - 1) + j) * 6 + 3] = i * lenth + j + 1;
		//			element[(i * (lenth - 1) + j) * 6 + 4] = (i + 1) * lenth + j;
		//			element[(i * (lenth - 1) + j) * 6 + 5] = (i + 1) * lenth + j + 1;

		//		}

		//	}

		//	/*GLfloat  vertices[] = {
		//		-0.5f ,0.0f , -0.5f,
		//		-0.5f ,0.0f , 0.5f ,
		//		0.5f ,0.0f ,0.5f ,
		//		0.5f ,0.0f ,-0.5f };
		//	GLfloat  normal[] = {
		//		0.0f, 1.0f, 0.0f,
		//		0.0f, 1.0f, 0.0f,
		//		0.0f, 1.0f, 0.0f,
		//		0.0f, 1.0f, 0.0f };

		//	GLfloat  texture_coordinate[] = {
		//		0.0f, 0.0f,
		//		1.0f, 0.0f,
		//		1.0f, 1.0f,
		//		0.0f, 1.0f };
		//	GLuint element[] = {
		//		0, 1, 2,
		//		0, 2, 3, };*/

		//	this->plane = new VAO;
		//	this->plane->element_amount = sizeof(element) / sizeof(GLuint);
		//	glGenVertexArrays(1, &this->plane->vao);
		//	glGenBuffers(3, this->plane->vbo);
		//	glGenBuffers(1, &this->plane->ebo);

		//	glBindVertexArray(this->plane->vao);

		//	// Position attribute
		//	glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[0]);
		//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		//	glEnableVertexAttribArray(0);

		//	// Normal attribute
		//	glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[1]);
		//	glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
		//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		//	glEnableVertexAttribArray(1);

		//	// Texture Coordinate attribute
		//	glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[2]);
		//	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
		//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		//	glEnableVertexAttribArray(2);

		//	//Element attribute
		//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
		//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

		//	// Unbind VAO
		//	glBindVertexArray(0);
		//}
		if (!this->cubeVAO)
		{
				GLfloat  vertices[] = {
	-0.5f, -0.5f, -0.5f, 
	 0.5f, -0.5f, -0.5f, 
	 0.5f,  0.5f, -0.5f, 
	 0.5f,  0.5f, -0.5f, 
	-0.5f,  0.5f, -0.5f,  
	-0.5f, -0.5f, -0.5f, 

	-0.5f, -0.5f,  0.5f,  
	 0.5f, -0.5f,  0.5f, 
	 0.5f,  0.5f,  0.5f, 
	 0.5f,  0.5f,  0.5f, 
	-0.5f,  0.5f,  0.5f,  
	-0.5f, -0.5f,  0.5f,  

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f, 
	-0.5f, -0.5f, -0.5f, 
	-0.5f, -0.5f, -0.5f, 
	-0.5f, -0.5f,  0.5f, 
	-0.5f,  0.5f,  0.5f, 

	 0.5f,  0.5f,  0.5f,  
	 0.5f,  0.5f, -0.5f, 
	 0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f,  0.5f, 
	 0.5f,  0.5f,  0.5f, 

	-0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f, -0.5f, 
	 0.5f, -0.5f,  0.5f,  
	 0.5f, -0.5f,  0.5f, 
	-0.5f, -0.5f,  0.5f,  
	-0.5f, -0.5f, -0.5f, 

	-0.5f,  0.5f, -0.5f, 
	 0.5f,  0.5f, -0.5f, 
	 0.5f,  0.5f,  0.5f, 
	 0.5f,  0.5f,  0.5f, 
	-0.5f,  0.5f,  0.5f,  
	-0.5f,  0.5f, -0.5f, };
				GLfloat  normal[] = {
	  0.0f,  0.0f, -1.0f,
	  0.0f,  0.0f, -1.0f,
	  0.0f,  0.0f, -1.0f,
	  0.0f,  0.0f, -1.0f,
	  0.0f,  0.0f, -1.0f,
	  0.0f,  0.0f, -1.0f,

	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,

	 -1.0f,  0.0f,  0.0f,
	 -1.0f,  0.0f,  0.0f,
	 -1.0f,  0.0f,  0.0f,
	 -1.0f,  0.0f,  0.0f,
	 -1.0f,  0.0f,  0.0f,
	 -1.0f,  0.0f,  0.0f,

	  1.0f,  0.0f,  0.0f,
	  1.0f,  0.0f,  0.0f,
	  1.0f,  0.0f,  0.0f,
	  1.0f,  0.0f,  0.0f,
	  1.0f,  0.0f,  0.0f,
	  1.0f,  0.0f,  0.0f,

	  0.0f, -1.0f,  0.0f,
	  0.0f, -1.0f,  0.0f,
	  0.0f, -1.0f,  0.0f,
	  0.0f, -1.0f,  0.0f,
	  0.0f, -1.0f,  0.0f,
	  0.0f, -1.0f,  0.0f,

	  0.0f,  1.0f,  0.0f,
	  0.0f,  1.0f,  0.0f,
	  0.0f,  1.0f,  0.0f,
	  0.0f,  1.0f,  0.0f,
	  0.0f,  1.0f,  0.0f,
	  0.0f,  1.0f,  0.0f };

				GLfloat  texture_coordinate[] = {
					0.0f, 0.0f,//1
					1.0f, 0.0f,
					1.0f, 1.0f,
					1.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 0.0f,

					0.0f, 0.0f,//2
					1.0f, 0.0f,
					1.0f, 1.0f,
					1.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 0.0f,

					1.0f, 0.0f,//3
					1.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 0.0f,
					1.0f, 0.0f,

					1.0f, 0.0f,//4
					1.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 0.0f,
					1.0f, 0.0f,

					0.0f, 1.0f,//5
					1.0f, 1.0f,
					1.0f, 0.0f,
					1.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,

					0.0f, 1.0f,//6
					1.0f, 1.0f,
					1.0f, 0.0f,
					1.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,
				};
			/*	GLuint element[] = {
					0, 1, 2,
					0, 2, 3, };*/

				this->cubeVAO = new VAO;
			//	this->plane->element_amount = sizeof(element) / sizeof(GLuint);
				glGenVertexArrays(1, &this->cubeVAO->vao);
				glGenBuffers(3, this->cubeVAO->vbo);
				

				glBindVertexArray(this->cubeVAO->vao);

				// Position attribute
				glBindBuffer(GL_ARRAY_BUFFER, this->cubeVAO->vbo[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);

				// Normal attribute
				glBindBuffer(GL_ARRAY_BUFFER, this->cubeVAO->vbo[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(1);

				// Texture Coordinate attribute
				glBindBuffer(GL_ARRAY_BUFFER, this->cubeVAO->vbo[2]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(2);

				////Element attribute
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
				//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

				// Unbind VAO
				glBindVertexArray(0);
		}
		if (!this->skybox) {


			GLfloat  vertices[] = {
				-0.5f ,0.5f , -0.5f,
				0.5f ,0.5f , -0.5f ,
				0.5f ,-0.5f ,-0.5f ,
				-0.5f ,-0.5f ,-0.5f ,
				-0.5f ,0.5f ,0.5f ,
				0.5f ,0.5f ,0.5f ,
				0.5f ,-0.5f ,0.5f ,
				-0.5f ,-0.0f ,0.5f };
			//GLfloat  normal[] = {
			//	0.0f, 1.0f, 0.0f,
			//	0.0f, 1.0f, 0.0f,
			//	0.0f, 1.0f, 0.0f,
			//	0.0f, 1.0f, 0.0f };

			//GLfloat  texture_coordinate[] = {
			//	0.0f, 0.0f,
			//	1.0f, 0.0f,
			//	1.0f, 1.0f,
			//	0.0f, 1.0f };
			GLuint element[] = {
				0, 1, 2,
				0, 2, 3,
				4, 7, 6,
				4, 6, 5,
				4, 0, 3,
				4, 3, 7,
				5, 6, 2,
				5, 2, 1,
				0, 4, 5,
				0, 5, 1,
				7, 3, 6,
				3, 2, 6, };

			this->skybox = new VAO;
			this->skybox->element_amount = sizeof(element) / sizeof(GLuint);
			glGenVertexArrays(1, &this->skybox->vao);
			glGenBuffers(1, this->skybox->vbo);
			glGenBuffers(1, &this->skybox->ebo);

			glBindVertexArray(this->skybox->vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->skybox->vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

		
			//Element attribute
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->skybox->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

			// Unbind VAO
			glBindVertexArray(0);
		}
		if (!this->waveVAO)
		{
			int width = 100, lenth = 100;
			GLfloat  vertices[100 * 100 * 3];
			GLfloat  normal[100 * 100 * 3] = { 0.0f };
			GLfloat  texture_coordinate[100 * 100 * 2] = { 0.0f };
			GLuint element[(100 - 1) * (100 - 1) * 6];
			for (int i = 0; i < lenth; i++)
			{
				for (int j = 0; j < width; j++)
				{
					vertices[(i * lenth + j) * 3] = j-50;
					vertices[(i * lenth + j) * 3 + 1] = 0.0f;
					vertices[(i * lenth + j) * 3 + 2] = i-50;
					normal[(i * lenth + j) * 3 + 1] = 1.0f;
					texture_coordinate[(i * lenth + j) * 2] = j * 0.01;
					texture_coordinate[(i * lenth + j) * 2 + 1] = i * 0.01;
				}
			}

			for (int i = 0; i < lenth - 1; i++)
			{
				for (int j = 0; j < width - 1; j++)
				{
					element[(i * (lenth - 1) + j) * 6] = i * lenth + j;
					element[(i * (lenth - 1) + j) * 6 + 1] = i * lenth + j + 1;
					element[(i * (lenth - 1) + j) * 6 + 2] = (i + 1) * lenth + j;

					element[(i * (lenth - 1) + j) * 6 + 3] = (i + 1) * lenth + j;
					element[(i * (lenth - 1) + j) * 6 + 4] = i * lenth + j + 1;		
					element[(i * (lenth - 1) + j) * 6 + 5] = (i + 1) * lenth + j + 1;

				}

			}

			this->waveVAO = new VAO;
			this->waveVAO->element_amount = sizeof(element) / sizeof(GLuint);
			glGenVertexArrays(1, &this->waveVAO->vao);
			glGenBuffers(3, this->waveVAO->vbo);
			glGenBuffers(1, &this->waveVAO->ebo);

			glBindVertexArray(this->waveVAO->vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->waveVAO->vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Normal attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->waveVAO->vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);

			// Texture Coordinate attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->waveVAO->vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(2);

			//Element attribute
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->waveVAO->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

			// Unbind VAO
			glBindVertexArray(0);
		}

		if (!this->quadVAO)
		{
			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
			};

			// screen quad VAO
			unsigned int quadVAO, quadVBO;
			this->quadVAO = new VAO;
			glGenVertexArrays(1, &this->quadVAO->vao);
			glGenBuffers(1, this->quadVAO->vbo);

			glBindVertexArray(this->quadVAO->vao);
			glBindBuffer(GL_ARRAY_BUFFER, this->quadVAO->vbo[0]);

			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		}

		if (!this->screenFBO[0])
		{
			this->screenFBO[0] = new FBO;
			unsigned int framebuffer;
			glGenFramebuffers(1, &this->screenFBO[0]->fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[0]->fbo);

			// create a color attachment texture
			unsigned int textureColorbuffer;
			
			glGenTextures(1, &this->screenFBO[0]->textures[0]);
		//	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[0]->textures[0], 0);

	
			unsigned int rbo;
			glGenRenderbuffers(1, &this->screenFBO[0]->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, this->screenFBO[0]->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->screenFBO[0]->rbo); // now actually attach it
			// now that we actually created the framebuffer and 
			
			
			
			//ed all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}
		if (!this->screenFBO[1])
		{
			this->screenFBO[1] = new FBO;
			unsigned int framebuffer;
			glGenFramebuffers(1, &this->screenFBO[1]->fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[1]->fbo);

			// create a color attachment texture
			unsigned int textureColorbuffer;

			glGenTextures(1, &this->screenFBO[1]->textures[0]);
			//	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->screenFBO[1]->textures[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[1]->textures[0], 0);


			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			unsigned int rbo;
			glGenRenderbuffers(1, &this->screenFBO[1]->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, this->screenFBO[1]->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->screenFBO[1]->rbo); // now actually attach it
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}		
		if (!this->screenFBO[2])
		{
			this->screenFBO[2] = new FBO;
			unsigned int framebuffer;
			glGenFramebuffers(1, &this->screenFBO[2]->fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[2]->fbo);

			// create a color attachment texture
			unsigned int textureColorbuffer;

			glGenTextures(1, &this->screenFBO[2]->textures[0]);
			//	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->screenFBO[2]->textures[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[2]->textures[0], 0);


			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			unsigned int rbo;
			glGenRenderbuffers(1, &this->screenFBO[2]->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, this->screenFBO[2]->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->screenFBO[2]->rbo); // now actually attach it
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);





		}
		if (!this->screenFBO[3])
		{
			this->screenFBO[3] = new FBO;
			unsigned int framebuffer;
			glGenFramebuffers(1, &this->screenFBO[3]->fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[3]->fbo);

			// create a color attachment texture
			unsigned int textureColorbuffer;

			glGenTextures(1, &this->screenFBO[3]->textures[0]);
			//	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->screenFBO[3]->textures[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[3]->textures[0], 0);


			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			unsigned int rbo;
			glGenRenderbuffers(1, &this->screenFBO[3]->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, this->screenFBO[3]->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->screenFBO[3]->rbo); // now actually attach it
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


		}
		if (!this->screenFBO[4])
		{
			this->screenFBO[4] = new FBO;
			unsigned int framebuffer;
			glGenFramebuffers(1, &this->screenFBO[4]->fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[4]->fbo);

			// create a color attachment texture
			unsigned int textureColorbuffer;

			glGenTextures(1, &this->screenFBO[4]->textures[0]);
			//	glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->screenFBO[4]->textures[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[4]->textures[0], 0);


			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			unsigned int rbo;
			glGenRenderbuffers(1, &this->screenFBO[4]->rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, this->screenFBO[4]->rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->screenFBO[4]->rbo); // now actually attach it
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


		}


		if (!this->cubetexture)
		{
			this->cubetexture = new Texture2D(PROJECT_DIR "/Images/aa.jpg");
		}
	/*	if (!this->texture) 
		{
			this->texture = new Texture2D(PROJECT_DIR "/Images/white.png");
			
		}*/
		if (!this->wavetexture)
		{
			this->wavetexture = new Texture2D(PROJECT_DIR "/Images/white.png");
		}
		if (!this->cubeMap)
		{
			
			this->cubeMap = new Cubemap(cubemapID);
		}
			

		if (!this->heightMaptexture)
		{
			this->heightMaptexture[0] = new Texture2D(PROJECT_DIR "Images/wave5/000.png");
			this->heightMaptexture[1] = new Texture2D(PROJECT_DIR "Images/wave5/001.png");
			this->heightMaptexture[2] = new Texture2D(PROJECT_DIR "Images/wave5/002.png");
			this->heightMaptexture[3] = new Texture2D(PROJECT_DIR "Images/wave5/003.png");
			this->heightMaptexture[4] = new Texture2D(PROJECT_DIR "Images/wave5/004.png");
			this->heightMaptexture[5] = new Texture2D(PROJECT_DIR "Images/wave5/005.png");
			this->heightMaptexture[6] = new Texture2D(PROJECT_DIR "Images/wave5/006.png");
			this->heightMaptexture[7] = new Texture2D(PROJECT_DIR "Images/wave5/007.png");
			this->heightMaptexture[8] = new Texture2D(PROJECT_DIR "Images/wave5/008.png");
			this->heightMaptexture[9] = new Texture2D(PROJECT_DIR "Images/wave5/009.png");


			for (int i = 10; i < 200; i++)
			{
				string str = "D:/Daniel/Graphics/WaterSurface-master/Images/wave5/";
				if (i<100)
				{
					str += "0";
				}
				  str += to_string(i)+".png";
				const char* path = str.c_str();
				this->heightMaptexture[i] = new Texture2D( path);
			}
		}

		if (!this->heightMaptexture1)
		{
			this->heightMaptexture1 = new Texture2D(PROJECT_DIR "/Images/000.png");


		}
		if (!this->normaltexture1)
		{
			this->normaltexture1 = new Texture2D(PROJECT_DIR "/Images/000normal.png");
		}
		if (!this->blacktexture)
		{
			this->blacktexture = new Texture2D(PROJECT_DIR "/Images/black.png");

			glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[2]->fbo);
			glEnable(GL_DEPTH_TEST);
			//glEnable(GL_CLIP_DISTANCE0);
			// make sure we clear the framebuffer's content
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			this->scrshader->Use();
			glBindVertexArray(this->quadVAO->vao);
			glUniform1i(glGetUniformLocation(this->scrshader->Program, "mode"), 0);
			this->blacktexture->bind(0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
			glClear(GL_COLOR_BUFFER_BIT);



		}
		if (!this->uvtexture)
		{
			this->uvtexture = new Texture2D(PROJECT_DIR "/Images/uvmap.png");

		
		}
		this->source_pos = glm::vec3(0.0f, 0.0f, 0.0f);


		if (!this->device) {
		//	//Tutorial: https://ffainelli.github.io/openal-example/
		//	this->device = alcOpenDevice(NULL);
		//	if (!this->device)
		//		puts("ERROR::NO_AUDIO_DEVICE");

		//	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
		//	if (enumeration == AL_FALSE)
		//		puts("Enumeration not supported");
		//	else
		//		puts("Enumeration supported");

		//	this->context = alcCreateContext(this->device, NULL);
		//	if (!alcMakeContextCurrent(context))
		//		puts("Failed to make context current");

		//	this->source_pos = glm::vec3(0.0f, 10.0f, 0.0f);

		//	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		//	alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
		//	alListener3f(AL_VELOCITY, 0, 0, 0);
		//	alListenerfv(AL_ORIENTATION, listenerOri);

		//	alGenSources((ALuint)1, &this->source);
		//	alSourcef(this->source, AL_PITCH, 1);
		//	alSourcef(this->source, AL_GAIN, 1.0f);
		//	alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
		//	alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
		//	alSourcei(this->source, AL_LOOPING, AL_TRUE);

		//	alGenBuffers((ALuint)1, &this->buffer);

		//	ALsizei size, freq;
		//	ALenum format;
		//	ALvoid* data;
		//	ALboolean loop = AL_TRUE;

		//	//Material from: ThinMatrix
		//	alutLoadWAVFile((ALbyte*)PROJECT_DIR "/Audios/bounce.wav", &format, &data, &size, &freq, &loop);
		//	alBufferData(this->buffer, format, data, size, freq);
		//	alSourcei(this->source, AL_BUFFER, this->buffer);

		//	if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
		//		puts("TYPE::STEREO");
		//	else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
		//		puts("TYPE::MONO");

		//	alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	// set linstener position 
	if (selectedCube >= 0)
		alListener3f(AL_POSITION,
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION,
			this->source_pos.x,
			this->source_pos.y,
			this->source_pos.z);

	

	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);



	//setupFloor();
	glDisable(GL_LIGHTING);
	//drawFloor(200, 10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}



	////bind shader
	//this->shader->Use();
	//
	//setLight(this->shader);
	////light  -viewPos
	//glm::mat4 view;
	//glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
	//glm::mat4 inversion = glm::inverse(view);
	//glm::vec3 viewerPos(inversion[3][0], inversion[3][1], inversion[3][2]);
	//glUniform3f(glGetUniformLocation(this->shader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

	//////wave
	////int numWave = 2;
	////float waterHeight = 3.0;

	////float dir[2][2] = { { 1.0,0.0 }, { 0.0,1.0 } };
	////glUniform1i(glGetUniformLocation(this->shader->Program, "numWaves"), numWave);
	////setWave(waterHeight,10 ,0,dir[1]);
	////setWave(5,17, 1, dir[1]);
	////setWave(waterHeight, 10,2, dir[0]);

	//glm::mat4 model_matrix = glm::mat4();
	//model_matrix = glm::translate(model_matrix, this->source_pos);
	//model_matrix = glm::scale(model_matrix, glm::vec3(1.0f, 1.0f, 1.0f));
	//setUBO();
	//glBindBufferRange(
	//	GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	//glUniformMatrix4fv(
	//	glGetUniformLocation(this->shader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

	//glUniform3fv(
	//	glGetUniformLocation(this->shader->Program, "u_color"), 
	//	1, 
	//	&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
	//this->texture->bind(0);
	//
	//glUniform1i(glGetUniformLocation(this->shader->Program, "u_texture"), 0);
	//
	////bind VAO
	//glBindVertexArray(this->plane->vao);
	//glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);
	////unbind VAO
	//glBindVertexArray(0);
	 
	
	//set uniform
		glm::mat4 view;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);

	
	glm::mat4 inversion = glm::inverse(view);
	glm::vec3 viewerPos(inversion[3][0], inversion[3][1], inversion[3][2]);


	glm::mat4 model_matrix = glm::mat4();
	setUBO();
		
	glm::vec3 sinPos(-50, 0, -50);
	if (tw->waveBrowser->value()==4)
	{
		// bind to framebuffer and draw scene as we normally would to color texture 

		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[0]->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[0]->textures[0], 0);

		glEnable(GL_DEPTH_TEST);
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//else if (tw->waveBrowser->value() == 1)		//reflect視角
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO->fbo);
		//	glEnable(GL_DEPTH_TEST);
		//	glEnable(GL_CLIP_DISTANCE0);
		//	// make sure we clear the framebuffer's content
		//	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//}

		
		
			//bind shader
			this->waveshader->Use();

			setLight(this->waveshader);
			//light  -viewPos

			glUniform3f(glGetUniformLocation(this->waveshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

			//wave
			int numWave = 1;
			float waterHeight = 0.0;

			float dir[2][2] = { { 1.0,0.0 }, { 0.0,1.0 } };
			glUniform1i(glGetUniformLocation(this->waveshader->Program, "numWaves"), numWave);
			setWave(this->waveshader, waterHeight, 30, 0, dir[1]);
			//setWave(this->waveshader,5, 17, 1, dir[1]);
			//setWave(waterHeight, 10,2, dir[0]);

			model_matrix = glm::translate(model_matrix, this->source_pos);
			model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

			glBindBufferRange(
				GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

			glUniformMatrix4fv(
				glGetUniformLocation(this->waveshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

			glUniform3fv(
				glGetUniformLocation(this->waveshader->Program, "u_color"),
				1,
				&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
			this->wavetexture->bind(0);

			glUniform1i(glGetUniformLocation(this->waveshader->Program, "u_texture"), 0);

			//	glEnable(GL_CULL_FACE);
			//	glCullFace(GL_BACK);
			//	glFrontFace(GL_CW);
				//bind VAO
			glBindVertexArray(this->waveVAO->vao);
			glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
			//unbind VAO
			glBindVertexArray(0);

		




		//bind shader
		this->cubeshader->Use();

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 0);//正常
		setLight(this->cubeshader);
		//light  -viewPos

		glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
		glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//setUBO();
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glm::vec3 pos(0.0, 200, 0.0);
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 plane(0, 1, 0, -1);		//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane"), 1, &plane[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->cubeshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->cubetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


		//bind VAO
		glBindVertexArray(this->cubeVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//unbind VAO
		glBindVertexArray(0);


		//skybox
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//正常

		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);



		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);

		this->scrshader->Use();
		glUniform1f(glGetUniformLocation(this->scrshader->Program, "vx_offset"), 0.5);
		glUniform1f(glGetUniformLocation(this->scrshader->Program, "rt_w"), 300);
		glUniform1f(glGetUniformLocation(this->scrshader->Program, "rt_h"), 300);
		glUniform1f(glGetUniformLocation(this->scrshader->Program, "pixel_w"), 15.0);
		glUniform1f(glGetUniformLocation(this->scrshader->Program, "pixel_h"), 10.0);

		
		glBindVertexArray(this->quadVAO->vao);
	//	glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO->textures[1]);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);	// use the color attachment texture as the texture of the quad plane
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//unbind VAO
		glBindVertexArray(0);
	}

	
	if (tw->waveBrowser->value() == 1)
	{
		// bind to framebuffer and draw scene as we normally would to color texture 

		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[0]->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[0]->textures[0], 0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CLIP_DISTANCE0);
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		


		//bind shader
		this->cubeshader->Use();

		setLight(this->cubeshader);
		//light  -viewPos
		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 1);
		//glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
		viewerPos[1] = -viewerPos[1];
		glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//setUBO();
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glm::vec3 pos(0.0, 200, 0.0);
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 plane1(0, 1, 0, -1);		//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane1"), 1, &plane1[0]);
		glm::vec4 plane2(0, -1, 0, 1);		//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane2"), 1, &plane2[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->cubeshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->cubetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


		//bind VAO
		glBindVertexArray(this->cubeVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//unbind VAO
		glBindVertexArray(0);


		//skybox
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 1);//反射
		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);



		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CLIP_DISTANCE0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);
		viewerPos[1] = -viewerPos[1];





		//***************************************折射





		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[1]->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[1]->textures[0], 0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CLIP_DISTANCE1);
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





		//bind shader
		this->cubeshader->Use();

		setLight(this->cubeshader);
		//light  -viewPos
		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 2);
		//glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);

		glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//setUBO();
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

				//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane1"), 1, &plane1[0]);
			//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane2"), 1, &plane2[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->cubeshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->cubetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


		//bind VAO
		glBindVertexArray(this->cubeVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//unbind VAO
		glBindVertexArray(0);


		//skybox
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//折射
		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);



		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		glDisable(GL_CLIP_DISTANCE1);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);


		

		//bind shader
		this->waveshader->Use();

		setLight(this->waveshader);
		//light  -viewPos

		glUniform3f(glGetUniformLocation(this->waveshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//wave
		int numWave = 1;
		float waterHeight = 3.0;

		float dir[2][2] = { { 1.0,0.0 }, { 0.0,1.0 } };
		glUniform1i(glGetUniformLocation(this->waveshader->Program, "numWaves"), numWave);
		setWave(this->waveshader, waterHeight, 30, 0, dir[1]);
		//setWave(this->waveshader,5, 17, 1, dir[1]);
		//setWave(waterHeight, 10,2, dir[0]);
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glUniformMatrix4fv(
			glGetUniformLocation(this->waveshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->waveshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->wavetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->waveshader->Program, "u_texture"), 0);


		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[1]->textures[0]);

		glUniform1i(glGetUniformLocation(this->waveshader->Program, "reflectTexture"), 1);
		glUniform1i(glGetUniformLocation(this->waveshader->Program, "refractTexture"), 2);


		glBindVertexArray(this->waveVAO->vao);
		glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		//unbind VAO
		glBindVertexArray(0);






		//bind shader
		this->cubeshader->Use();

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 0);//正常
		setLight(this->cubeshader);
		//light  -viewPos

		glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
		glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//setUBO();
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 plane(0, 1, 0, -1);		//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane"), 1, &plane[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->cubeshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->cubetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


		//bind VAO
		glBindVertexArray(this->cubeVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//unbind VAO
		glBindVertexArray(0);


		//skybox
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//正常

		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);




		//this->scrshader->Use();


		//glBindVertexArray(this->quadVAO->vao);


		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[1]->textures[0]);
		////glUniform1i(glGetUniformLocation(this->scrshader->Program, "text2"), 1);
		//glUniform1i(glGetUniformLocation(this->scrshader->Program, "screenTexture"), 0);
		////glBindTexture(GL_TEXTURE_2D, this->screenFBO->textures[0]);	// use the color attachment texture as the texture of the quad plane
		//glDrawArrays(GL_TRIANGLES, 0, 6);



		//this->watershader->Use();

		//model_matrix = glm::mat4();
		//model_matrix = glm::translate(model_matrix, this->source_pos);
		//model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		//glBindBufferRange(
		//	GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		//glUniformMatrix4fv(
		//	glGetUniformLocation(this->watershader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);


		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[1]->textures[0]);
		//glUniform1i(glGetUniformLocation(this->watershader->Program, "reflectTexture"), 0);
		//glUniform1i(glGetUniformLocation(this->watershader->Program, "refractTexture"), 1);

		//glBindVertexArray(this->waveVAO->vao);
		//glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);


	}
	

	if (tw->waveBrowser->value() == 2)
	{
		// bind to framebuffer and draw scene as we normally would to color texture 

		//glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[0]->fbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->screenFBO[0]->textures[0], 0);

		//glEnable(GL_DEPTH_TEST);
		//// make sure we clear the framebuffer's content
		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//else if (tw->waveBrowser->value() == 1)		//reflect視角
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO->fbo);
		//	glEnable(GL_DEPTH_TEST);
		//	glEnable(GL_CLIP_DISTANCE0);
		//	// make sure we clear the framebuffer's content
		//	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//}



		//	//bind shader
		//this->waveshader->Use();

		//setLight(this->waveshader);
		////light  -viewPos

		//glUniform3f(glGetUniformLocation(this->waveshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		////wave
		//int numWave = 1;
		//float waterHeight = 0.0;

		//float dir[2][2] = { { 1.0,0.0 }, { 0.0,1.0 } };
		//glUniform1i(glGetUniformLocation(this->waveshader->Program, "numWaves"), numWave);
		//setWave(this->waveshader, waterHeight, 30, 0, dir[1]);
		////setWave(this->waveshader,5, 17, 1, dir[1]);
		////setWave(waterHeight, 10,2, dir[0]);

		//model_matrix = glm::translate(model_matrix, this->source_pos);
		//model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		//glBindBufferRange(
		//	GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		//glUniformMatrix4fv(
		//	glGetUniformLocation(this->waveshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		//glUniform3fv(
		//	glGetUniformLocation(this->waveshader->Program, "u_color"),
		//	1,
		//	&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		//this->wavetexture->bind(0);

		//glUniform1i(glGetUniformLocation(this->waveshader->Program, "u_texture"), 0);

		////	glEnable(GL_CULL_FACE);
		////	glCullFace(GL_BACK);
		////	glFrontFace(GL_CW);
		//	//bind VAO
		//glBindVertexArray(this->waveVAO->vao);
		//glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		////unbind VAO
		//glBindVertexArray(0);




		//bind shader
		this->heightMapshader->Use();

		setLight(this->heightMapshader);
		//light  -viewPos

		glUniform3f(glGetUniformLocation(this->heightMapshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);
		
		//wave
		float speed = Wave::time * 0.01;
		glUniform1f(glGetUniformLocation(heightMapshader->Program, "time"), speed);

		glUniform1i(glGetUniformLocation(heightMapshader->Program, "mode"), 1);//mode
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glUniformMatrix4fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		//this->wavetexture->bind(0);

	//	glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "u_texture"), 0);

		this->heightMaptexture1->bind(0);
		this->normaltexture1->bind(1);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waterHeight"), 0);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waveNormal"), 1);

		glm::vec2 u_delta = { 0.002, 0.002 };
			
		glUniform2fv(glGetUniformLocation(this->heightMapshader->Program, "u_delta"),1 ,&u_delta[0]);
		//	glEnable(GL_CULL_FACE);
		//	glCullFace(GL_BACK);
		//	glFrontFace(GL_CW);
			//bind VAO
		glBindVertexArray(this->waveVAO->vao);
		glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		//unbind VAO
		glBindVertexArray(0);




		//bind shader
		this->cubeshader->Use();

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 0);//正常
		setLight(this->cubeshader);
		//light  -viewPos

		glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
		glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//setUBO();
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glm::vec3 pos(0.0, 200, 0.0);
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 plane(0, 1, 0, -1);		//clip plane
		glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane"), 1, &plane[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->cubeshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		this->cubetexture->bind(0);

		glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


		//bind VAO
		glBindVertexArray(this->cubeVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//unbind VAO
		glBindVertexArray(0);


		//skybox
		//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//正常

		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);



		//// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		//glClear(GL_COLOR_BUFFER_BIT);

		//this->scrshader->Use();
		////glUniform1f(glGetUniformLocation(this->scrshader->Program, "vx_offset"), 0.5);
		////glUniform1f(glGetUniformLocation(this->scrshader->Program, "rt_w"), 300);
		////glUniform1f(glGetUniformLocation(this->scrshader->Program, "rt_h"), 300);
		////glUniform1f(glGetUniformLocation(this->scrshader->Program, "pixel_w"), 15.0);
		////glUniform1f(glGetUniformLocation(this->scrshader->Program, "pixel_h"), 10.0);


		//glBindVertexArray(this->quadVAO->vao);
		////	glActiveTexture(GL_TEXTURE1);
		//	//glBindTexture(GL_TEXTURE_2D, this->screenFBO->textures[1]);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[0]->textures[0]);	// use the color attachment texture as the texture of the quad plane

		//glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	if (tw->waveBrowser->value() == 3)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[4]->fbo);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CLIP_DISTANCE0);
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




		////set uniform
		//glm::mat4 view;
		//glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);


		//glm::mat4 inversion = glm::inverse(view);
		//glm::vec3 viewerPos(inversion[3][0], inversion[3][1], inversion[3][2]);


		//glm::mat4 model_matrix = glm::mat4();
		//setUBO();
		//bind shader
		this->heightMapshader->Use();

		setLight(this->heightMapshader);
		//light  -viewPos

		glUniform3f(glGetUniformLocation(this->heightMapshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//wave
		float speed = Wave::time * 0.01;
		glUniform1f(glGetUniformLocation(heightMapshader->Program, "time"), speed);

		glUniform1i(glGetUniformLocation(heightMapshader->Program, "mode"), 0);//mode
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glUniformMatrix4fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		
		uvtexture->bind(0);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "uvmap"), 0);


		glm::vec2 u_delta = { 0.01,0.01 };
		glUniform2fv(glGetUniformLocation(this->heightMapshader->Program, "u_delta"), 1, &u_delta[0]);

		glBindVertexArray(this->waveVAO->vao);
		glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		//unbind VAO
		glBindVertexArray(0);

		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);
		// bind to framebuffer and draw scene as we normally would to color texture 



		//*************************************update

		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[3]->fbo);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CLIP_DISTANCE0);
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//bind shader
		this->water1shader->Use();
		static glm::vec2 center = { 0.0,0.0 };
		float radius = 0.01;
		float strength = 10.0;
		static int drop = 1;
		static int abc=-1;
		if (drop == 1)
		{
			drop = 2;
		}
		if (abc != this->count)
		{
			abc = count;
			//center = center_coodinate;
			drop = 1;
		}
		glUniform2fv(glGetUniformLocation(this->water1shader->Program, "u_center"), 1, &this->center_coodinate[0]);
		glUniform1fv(glGetUniformLocation(this->water1shader->Program, "u_radius"), 1, &radius);
		glUniform1fv(glGetUniformLocation(this->water1shader->Program, "u_strength"), 1, &strength);
		glUniform1iv(glGetUniformLocation(this->water1shader->Program, "drop"), 1, &drop);



		//glm::vec2 u_delta = { 0.01, 0.01 };
		glUniform2fv(glGetUniformLocation(this->water1shader->Program, "u_delta"), 1, &u_delta[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[2]->fbo);
		glUniform1i(glGetUniformLocation(this->water1shader->Program, "u_water"), 0);
		//bind VAO
		glBindVertexArray(this->quadVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//unbind VAO
		glBindVertexArray(0);


		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);


		glBindFramebuffer(GL_FRAMEBUFFER, this->screenFBO[2]->fbo);
		glEnable(GL_DEPTH_TEST);		
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		this->scrshader->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[3]->fbo);
		glUniform1i(glGetUniformLocation(this->scrshader->Program, "mode"), 1);
		//bind VAO
		glBindVertexArray(this->quadVAO->vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//unbind VAO
		glBindVertexArray(0);

		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT);


		//this->scrshader->Use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->screenFBO[2]->fbo);
		//glUniform1i(glGetUniformLocation(this->scrshader->Program, "mode"), 1);
		////bind VAO
		//glBindVertexArray(this->quadVAO->vao);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		////unbind VAO
		//glBindVertexArray(0);


		//setLight(this->waveshader);
		//light  -viewPos

		//glUniform3f(glGetUniformLocation(this->waveshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);


		//model_matrix = glm::translate(model_matrix, this->source_pos);
	//	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		//glBindBufferRange(
		//	GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

//glUniformMatrix4fv(
		//	glGetUniformLocation(this->waveshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

	//	glUniform3fv(
	//		glGetUniformLocation(this->waveshader->Program, "u_color"),
	//		1,
	//		&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
	






		//bind shader
		this->heightMapshader->Use();

		setLight(this->heightMapshader);
		//light  -viewPos

		glUniform3f(glGetUniformLocation(this->heightMapshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

		//wave
		//float speed = Wave::time * 0.01;
		glUniform1f(glGetUniformLocation(heightMapshader->Program, "time"), speed);

		glUniform1i(glGetUniformLocation(heightMapshader->Program, "mode"), 2);//mode
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

		glUniformMatrix4fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		glUniform3fv(
			glGetUniformLocation(this->heightMapshader->Program, "u_color"),
			1,
			&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
		//this->wavetexture->bind(0);

	//	glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "u_texture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->screenFBO[2]->fbo);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waterHeight"), 0);
		this->heightMaptexture1->bind(1);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waterheight1"), 1);
		/*this->heightMaptexture1->bind(0);
		this->normaltexture1->bind(1);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waterHeight"), 0);
		glUniform1i(glGetUniformLocation(this->heightMapshader->Program, "waveNormal"), 1);*/

		

		glUniform2fv(glGetUniformLocation(this->heightMapshader->Program, "u_delta"), 1, &u_delta[0]);
		//	glEnable(GL_CULL_FACE);
		//	glCullFace(GL_BACK);
		//	glFrontFace(GL_CW);
			//bind VAO
		glBindVertexArray(this->waveVAO->vao);
		glDrawElements(GL_TRIANGLES, this->waveVAO->element_amount, GL_UNSIGNED_INT, 0);
		//unbind VAO
		glBindVertexArray(0);



		//skybox
//glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		this->skyshader->Use();		//sky
		glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//正常

		glUniformMatrix4fv(
			glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		//bind VAO
		glBindVertexArray(this->skybox->vao);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
		this->cubeMap->bind(0);
		glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		//unbind VAO
		glBindVertexArray(0);

	//	//bind shader
	//	this->cubeshader->Use();

	//	glUniform1i(glGetUniformLocation(this->cubeshader->Program, "mode"), 0);//正常
	//	setLight(this->cubeshader);
	//	//light  -viewPos

	//	glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
	//	glUniform3f(glGetUniformLocation(this->cubeshader->Program, "viewPos"), viewerPos[0], viewerPos[1], viewerPos[2]);

	//	//setUBO();
	//	glBindBufferRange(
	//		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	//	glm::vec3 pos(0.0, 200, 0.0);
	//	model_matrix = glm::mat4();
	//	model_matrix = glm::translate(model_matrix, pos);
	//	model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 100.0f, 100.0f));

	//	glm::vec4 plane(0, 1, 0, -1);		//clip plane
	//	glUniform4fv(glGetUniformLocation(this->cubeshader->Program, "plane"), 1, &plane[0]);

	//	glUniformMatrix4fv(
	//		glGetUniformLocation(this->cubeshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

	//	glUniform3fv(
	//		glGetUniformLocation(this->cubeshader->Program, "u_color"),
	//		1,
	//		&glm::vec3(0.0f, 0.0f, 1.0f)[0]);
	//	this->cubetexture->bind(0);

	//	glUniform1i(glGetUniformLocation(this->cubeshader->Program, "u_texture"), 0);


	//	//bind VAO
	//	glBindVertexArray(this->cubeVAO->vao);
	//	glDrawArrays(GL_TRIANGLES, 0, 36);
	//	//unbind VAO
	//	glBindVertexArray(0);


	//	//skybox
	//	//glDepthMask(GL_FALSE);
	//	glDepthFunc(GL_LEQUAL);
	//	this->skyshader->Use();		//sky
	//	glUniform1i(glGetUniformLocation(this->skyshader->Program, "mode"), 0);//正常

	//	glUniformMatrix4fv(
	//		glGetUniformLocation(this->skyshader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
	//	//bind VAO
	//	glBindVertexArray(this->skybox->vao);
	//	//glBindTexture(GL_TEXTURE_CUBE_MAP, *cubemapID);
	//	this->cubeMap->bind(0);
	//	glDrawElements(GL_TRIANGLES, this->skybox->element_amount, GL_UNSIGNED_INT, 0);
	//	glDepthMask(GL_TRUE);
	//	//unbind VAO
	//	glBindVertexArray(0);





	}

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for(size_t i=0; i<m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();
	//cout << mx <<" " << my<<" ";
	
	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;
	this->pick(mx,h()-my-1);
	printf("Selected Cube %d\n",selectedCube);
}
void TrainView::addDrop(glm::vec2 uv, float f1,float f2)
{
	this->center_coodinate = uv;
}
void TrainView::pick(int x, int y)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->screenFBO[4]->fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glm::vec3 uv;
	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &uv[0]);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	if (uv.z != 0.0)
	{
		this->center_coodinate = uv;
		this->count++;
		//this->addDrop(glm::vec2(uv), 0.03f, 0.01f);
	}

}

void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TrainView::setLight(Shader* s)
{
	glUniform3f(glGetUniformLocation(s->Program, "dirLight.direction"), 0.0f, -1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(s->Program, "dirLight.ambient"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(s->Program, "dirLight.diffuse"), 0.0f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(s->Program, "dirLight.specular"), 0.6f, 0.6f, 0.6f);

	GLfloat shiny = 100;
	glUniform1f(glGetUniformLocation(s->Program, "material.shininess"), shiny);

	glUniform3f(glGetUniformLocation(s->Program, "pointLights.position"), 50.0f, 10.0f, 50.0f);
	glUniform3f(glGetUniformLocation(s->Program, "pointLights.ambient"), 1.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(s->Program, "pointLights.diffuse"), 1.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(s->Program, "pointLights.specular"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(s->Program, "pointLights.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(s->Program, "pointLights.linear"), 0.045f);
	glUniform1f(glGetUniformLocation(s->Program, "pointLights.quadratic"), 0.0075f);


	glUniform3f(glGetUniformLocation(s->Program, "spotLight.position"), 50.0f, 30.0f, 50.0f);
	glUniform3f(glGetUniformLocation(s->Program, "spotLight.direction"), 0.0f, -1.0f, 0.0f);
	glUniform3f(glGetUniformLocation(s->Program, "spotLight.ambient"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(s->Program, "spotLight.diffuse"), 0.5f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(s->Program, "spotLight.specular"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(s->Program, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(s->Program, "spotLight.linear"), 0.07f);
	glUniform1f(glGetUniformLocation(s->Program, "spotLight.quadratic"), 0.017f);

	glUniform1f(glGetUniformLocation(s->Program, "spotLight.cutOff"), 0.91f);
	glUniform1f(glGetUniformLocation(s->Program, "spotLight.outerCutOff"), 0.82f);

	


}

void TrainView::setWave(Shader* s,float heigh,float length, int num,float dir[])
{
	string amplitudeStr = "amplitude[";
	string wavelengthStr = "wavelength[";
	string speedStr = "speed[";
	string directionStr = "direction[";
	//string centerStr = "center[";
	amplitudeStr += to_string(num) + "]";
	wavelengthStr += to_string(num) + "]";
	speedStr += to_string(num) + "]";
	directionStr += to_string(num) + "]";
	//centerStr += to_string(num) + "]";
	glUniform1f(glGetUniformLocation(s->Program, "waterHeight"), heigh);
	glUniform1f(glGetUniformLocation(s->Program, "time"), Wave::time);

	glUniform1f(glGetUniformLocation(s->Program, amplitudeStr.c_str()), heigh);
	glUniform1f(glGetUniformLocation(s->Program, wavelengthStr.c_str()), length);
	glUniform1f(glGetUniformLocation(s->Program, speedStr.c_str()), 5.0f);
	glUniform2f(glGetUniformLocation(s->Program, directionStr.c_str()), dir[0],dir[1]);
	//glUniform2f(glGetUniformLocation(this->shader->Program, centerStr.c_str()), center[0], center[1]);
}

