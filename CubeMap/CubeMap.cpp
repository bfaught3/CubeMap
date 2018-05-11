
/* cubemap.c - a simple EXT_texture_cube_map example */

/* Copyright NVIDIA Corporation, 1999. */

/* Very simplistic cube map demo.  The cube map faces are each a different
solid color.  Then a sphere is drawn.

When correct, the middle patch should be blue.  The upper patch
should be green.  The lower patch should be yellow.  The left
patch should be magenta.  The right patch should be red.  The back
patch should be cyan and wrapped around the edge of the sphere as
a circle. */

/* Windows command line compile instructions:

cl cubemap.c glut32.lib

Unix command line compile instructions:

cc -o cubemap cubemap.c -lglut -lGLU -lGL -lXext -lX11 -lm

Mac OS X

cc -o cubemap cubemap.c -framework GLUT -framework OpenGL -lXext -lX11 -lm

Note that this program requires the OpenGL Utility Toolkit (GLUT).
You can download GLUT from http://reality.sgi.com/opengl/glut3 */

#include <GL/glew.h>
#include <GL/glut.h> // MUST BE FIRST (not including MACROS. Those are fine first.)
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/freeglut.h>
#include <time.h>       /* time_t, clock, CLOCKS_PER_SEC */
#include <math.h>       /* sqrt */
#include <iostream>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <NIDAQmx.h>
#include <time.h>
#include <windows.h>
#include "wglext.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

/* In case your <GL/gl.h> does not advertise EXT_texture_cube_map... */
#ifndef GL_EXT_texture_cube_map
# define GL_NORMAL_MAP_EXT                   0x8511
# define GL_REFLECTION_MAP_EXT               0x8512
# define GL_TEXTURE_CUBE_MAP_EXT             0x8513
# define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
# define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
# define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
# define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif

/* constants */
#ifndef M_PI
#define M_PI            (3.14159265358979f)
#endif
#define DTOR            (M_PI/180.0)
#define RTOD            (180.0/M_PI)

//#define CUBE_MAP_SIZE 256
#define CUBE_MAP_SIZE 800

#define ZTRANS 4.0F

int exitAfterOneFrame = 0;

const int window_width = 800;
const int window_height = 800;

enum { CUBE_POS_X, CUBE_NEG_X, CUBE_POS_Y, CUBE_NEG_Y, CUBE_POS_Z, CUBE_NEG_Z };
static unsigned char CubeMap[6][CUBE_MAP_SIZE][CUBE_MAP_SIZE][4];
static void InitGraphics(void);
static void light_state(long, long);
static void DrawSphere(float);
static void vert(float, float);
int angle = 0;

// Sphere stuff
float viewingAngle = 10;
int single = 360;
int yp = 0;
bool horizontal = 0;
bool vertical = 1;
bool spinning = 0;
const float R = 584.6207004;
//double *mvmatrix = (double*)malloc(16 * sizeof(GLint));
double mvmatrix[16];
double pjmatrix[16];
double txmatrix[16];

typedef struct
{

	int X;

	int Y;

	int Z;



	double U;

	double V;
}VERTICES;

const double PI = 3.1415926535897;

//const int space = 10;
const int space = 10;


//const int VertexCount = (90 / space) * (360 / space) * 4;
const int VertexCount = (360 / space) * (360 / space) * 4;


VERTICES VERTEX[VertexCount];

// Framebuffer stuff
unsigned int fbo; // The frame buffer object  
unsigned int fbo_depth; // The depth buffer for the frame buffer object  
unsigned int fbo_texture; // The texture object to write our frame buffer object to  


void initFrameBufferDepthBuffer(void) {
	
	glGenRenderbuffersEXT(1, &fbo_depth); // Generate one render buffer and store the ID in fbo_depth  
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth); // Bind the fbo_depth render buffer  

	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, window_width, window_height); // Set the render buffer storage to be a depth component, with a width and height of the window  

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth); // Set the render buffer of this buffer to the depth buffer  

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // Unbind the render buffer  
}

void initFrameBufferTexture(void) {
	glGenTextures(1, &fbo_texture); // Generate one texture  
	glBindTexture(GL_TEXTURE_2D, fbo_texture); // Bind the texture fbo_texture  

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window  

																											  // Setup the basic texture parameters  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Unbind the texture  
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initFrameBuffer(void) {
	initFrameBufferDepthBuffer(); // Initialize our frame buffer depth buffer  

	initFrameBufferTexture(); // Initialize our frame buffer texture  

	glGenFramebuffersEXT(1, &fbo); // Generate one frame buffer and store the ID in fbo  
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); // Bind our frame buffer  

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo_texture, 0); // Attach the texture fbo_texture to the color buffer in our frame buffer  

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth); // Attach the depth buffer fbo_depth to our frame buffer  

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer  

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) // If the frame buffer does not report back as complete  
	{
		std::cout << "Couldn't create frame buffer" << std::endl; // Output an error to the console  
		exit(0); // Exit the application  
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our frame buffer  
}



bool WGLExtensionSupported(const char *extension_name)
{
	// this is pointer to function which returns pointer to string with list of all wgl extensions
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

	// determine pointer to wglGetExtensionsStringEXT function
	_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

	if (_wglGetExtensionsStringEXT == 0 ||
		(_wglGetExtensionsStringEXT == (void*)0x1) || (_wglGetExtensionsStringEXT == (void*)0x2) || (_wglGetExtensionsStringEXT == (void*)0x3) ||
		(_wglGetExtensionsStringEXT == (void*)-1))
	{
		int a = 23;
		HMODULE module = LoadLibraryA("opengl32.dll");
		_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)GetProcAddress(module, "wglGetExtensionsStringEXT");
	}

	if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
	{
		// string was not found
		printf("\nExtension not supported");
		return false;
	}

	// extension is supported
	printf("\nExtension supported");
	return true;
}


void CreateSphere(double R, double H, double K, double Z) {

	int n;

	double a;

	double b;



	n = 0;



	//for (b = 0; b <= 90 - space; b += space) {
	for (b = 0; b <= 360 - space; b += space) {


		for (a = 0; a <= 360 - space; a += space) {

			if (fmod(b, 2 * viewingAngle) < viewingAngle + yp && b <= single) {

				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = R * sin((a) / 180 * PI) * cos((b) / 180 * PI) - H;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) + K;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a) / 180 * PI) - Z;


				VERTEX[n].V = (2 * b) / 360;

				VERTEX[n].U = (a) / 360;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI
				VERTEX[n].X = R * sin((a) / 180 * PI) * cos((b + space) / 180 * PI


				) - H;

				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b + space) / 180 * PI
				VERTEX[n].Y = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI


				) + K;

				//VERTEX[n].Z = R * cos((b + space) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a + space) / 180 * PI) - Z;


				VERTEX[n].V = (2 * (b + space)) / 360;

				VERTEX[n].U = (a) / 360;

				n++;



				//VERTEX[n].X = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI
				VERTEX[n].X = R * sin((a + space) / 180 * PI) * cos((b) / 180 * PI

				) - H;

				//VERTEX[n].Y = R * cos((a + space) / 180 * PI) * sin((b) / 180 * PI
				VERTEX[n].Y = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI

				) + K;

				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a) / 180 * PI) - Z;


				VERTEX[n].V = (2 * b) / 360;

				VERTEX[n].U = (a + space) / 360;

				n++;



				//VERTEX[n].X = R * sin((a + space) / 180 * PI) * sin((b + space) /
				VERTEX[n].X = R * sin((a + space) / 180 * PI) * cos((b + space) /

					180 * PI) - H;

				//VERTEX[n].Y = R * cos((a + space) / 180 * PI) * sin((b + space) /
				VERTEX[n].Y = R * sin((a + space) / 180 * PI) * sin((b + space) /

					180 * PI) + K;

				//VERTEX[n].Z = R * cos((b + space) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a + space) / 180 * PI) - Z;


				VERTEX[n].V = (2 * (b + space)) / 360;

				VERTEX[n].U = (a + space) / 360;

				n++;
			}
			else {
				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;
			}


		}

	}
}


void DisplaySphere(int index) {

	//gluPerspective(90, (GLfloat)window_width / (GLfloat)window_height, 0.1, 1000.0);
	glViewport(0, 0, (GLfloat)window_width, (GLfloat)window_height);
	int b;

	if (horizontal) {
		glRotatef(90, 0, 1, 0);
	}
	else if (vertical) {
		//glPushMatrix();
		//glRotatef(-90, 1, 0, 0);
		//glPopMatrix();
		//glLoadIdentity();
		switch (index) {
		case 1: //right side
			glRotatef(-90, 0, 1, 0);
			//glRotatef(90, 0, 1, 0);
			//glTranslatef(0, 0, R);
			break;
		case 0: //left side
			glRotatef(90, 0, 1, 0);
			//glTranslatef(0, 0, -R);
			break;
		case 2: //top side
			glRotatef(90, 1, 0, 0);
			//glTranslatef(0, 0, -R);
			break;
		case 3: //bottom side
			glRotatef(-90, 1, 0, 0);
			//glTranslatef(0, 0, -R);
			break;
		case 5: //back side
			glRotatef(180, 0, 0, 1);
			glRotatef(180, 1, 0, 0);
			//glRotatef(90, 0, 0, 1);
			//glTranslatef(0, 0, -R);
			break;
		default: //front side
			break;
		}
		glRotatef(-90, 1, 0, 0);
	}

	glBegin(GL_TRIANGLE_STRIP);

	for (b = 0; b <= VertexCount; b++) {

		if (fmod(b, 1) == 0) {
			glTexCoord2f(VERTEX[b].U, VERTEX[b].V);

			glVertex3f(VERTEX[b].X, VERTEX[b].Y, -VERTEX[b].Z);
		}

	}



	for (b = 0; b <= VertexCount; b++) {

		if (fmod(b, 1) == 0) {
			glTexCoord2f(VERTEX[b].U, -VERTEX[b].V);

			glVertex3f(VERTEX[b].X, VERTEX[b].Y, VERTEX[b].Z);
		}

	}

	glEnd();

}

static void
vert(float theta, float phi)
{
	float r = 0.75f;
	float x, y, z, nx, ny, nz;

	nx = sin(DTOR * theta) * cos(DTOR * phi);
	ny = sin(DTOR * phi);
	nz = cos(DTOR * theta) * cos(DTOR * phi);
	glNormal3f(nx, ny, nz);

	x = r * sin(DTOR * theta) * cos(DTOR * phi);
	y = r * sin(DTOR * phi);
	//z = -ZTRANS + r * cos(DTOR * theta) * cos(DTOR * phi);
	z = r * cos(DTOR * theta) * cos(DTOR * phi);
	glVertex4f(x, y, z, 1.0);
}

static void
DrawSphere(float del)
{
	//glLoadIdentity();
	float phi, phi2, theta;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	for (phi = -90.0f; phi < 90.0f; phi += del) {
		glBegin(GL_TRIANGLE_STRIP);

		phi2 = phi + del;

		for (theta = -90.0f; theta <= 90.0f; theta += del) {
			vert(theta, phi);
			vert(theta, phi2);
		}
		glEnd();
	}
	
	//glTranslatef(0, 0, -1);
	glLoadIdentity();
	angle++;
	glRotatef(angle, 0, 1, 0);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", mvmatrix[0], mvmatrix[1], mvmatrix[2], mvmatrix[3], mvmatrix[4], mvmatrix[5], mvmatrix[6], mvmatrix[7], mvmatrix[8], mvmatrix[9], mvmatrix[10], mvmatrix[11], mvmatrix[12], mvmatrix[13], mvmatrix[14], mvmatrix[15]);
	glGetDoublev(GL_MODELVIEW_MATRIX, txmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", txmatrix[0], txmatrix[1], txmatrix[2], txmatrix[3], txmatrix[4], txmatrix[5], txmatrix[6], txmatrix[7], txmatrix[8], txmatrix[9], txmatrix[10], txmatrix[11], txmatrix[12], txmatrix[13], txmatrix[14], txmatrix[15]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glPushMatrix();
	//gluLookAt(0, 0, 1.25, 0, 0, 0, 0, 1, 0);
	//gluPerspective(90, (GLfloat)window_width / (GLfloat)window_height, .1, 1000.0);
	//gluOrtho2D(-1, 1, -1, 1);

	gluLookAt(-0.025 * sinf((float)angle * PI / 180.0), 0, -0.025 * cosf((float)angle * PI / 180.0), 0, 0, 0, 0, 1, 0);
	//gluLookAt(0, 0, 1.25, 0, 0, 0, 0, 1, 0);
	//glPopMatrix();
	glGetDoublev(GL_PROJECTION_MATRIX, pjmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pjmatrix[0], pjmatrix[1], pjmatrix[2], pjmatrix[3], pjmatrix[4], pjmatrix[5], pjmatrix[6], pjmatrix[7], pjmatrix[8], pjmatrix[9], pjmatrix[10], pjmatrix[11], pjmatrix[12], pjmatrix[13], pjmatrix[14], pjmatrix[15]);
	glGetDoublev(GL_MODELVIEW_MATRIX, txmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", txmatrix[0], txmatrix[1], txmatrix[2], txmatrix[3], txmatrix[4], txmatrix[5], txmatrix[6], txmatrix[7], txmatrix[8], txmatrix[9], txmatrix[10], txmatrix[11], txmatrix[12], txmatrix[13], txmatrix[14], txmatrix[15]);

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glTranslatef(0, 0, 1);
}

static float
Dot3(float *a, float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static float *
Scale3(float *result, float *a, float scale)
{
	result[0] = a[0] * scale;
	result[1] = a[1] * scale;
	result[2] = a[2] * scale;
	return result;
}

static float *
Normalize3(float *result, float *a)
{
	float length;

	length = (float)sqrt(Dot3(a, a));
	return Scale3(result, a, 1 / length);
}

static unsigned char *
CubeFunc(unsigned char resultColor[3], float vec[3])
{
	int i;
	float faceVec[3];

	if (vec[0] == 1.0) {
		resultColor[0] = 255;
		resultColor[1] = 0;
		resultColor[2] = 0;
	}
	else if (vec[1] == 1.0) {
		resultColor[0] = 0;
		resultColor[1] = 255;
		resultColor[2] = 0;
	}
	else if (vec[2] == 1.0) {
		resultColor[0] = 0;
		resultColor[1] = 0;
		resultColor[2] = 255;
	}
	else if (vec[0] == -1.0) {
		resultColor[0] = 255;
		resultColor[1] = 0;
		resultColor[2] = 255;
	}
	else if (vec[1] == -1.0) {
		resultColor[0] = 255;
		resultColor[1] = 255;
		resultColor[2] = 0;
	}
	else if (vec[2] == -1.0) {
		resultColor[0] = 0;
		resultColor[1] = 255;
		resultColor[2] = 255;
	}
	return resultColor;

	Normalize3(faceVec, vec);
	for (i = 0; i < 3; i++) {
		resultColor[i] = 255 * (sin(6 * (faceVec[i] + faceVec[(i + 1) % 3])) + 1) / 2.0;
	}
	return resultColor;
}

GLenum cubefaces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
};

static void
InitGraphics(void)
{
	glGetDoublev(GL_PROJECTION_MATRIX, pjmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pjmatrix[0], pjmatrix[1], pjmatrix[2], pjmatrix[3], pjmatrix[4], pjmatrix[5], pjmatrix[6], pjmatrix[7], pjmatrix[8], pjmatrix[9], pjmatrix[10], pjmatrix[11], pjmatrix[12], pjmatrix[13], pjmatrix[14], pjmatrix[15]);
	glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture  
	glEnable(GL_DEPTH_TEST); // Enable depth testing  

	glDepthFunc(GL_LEQUAL);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	//glEnable(GL_CULL_FACE);

	//initFrameBuffer(); // Create our frame buffer object

	int i, j, k;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glFrustum(-1.0, 1.0, -0.75, 0.75, ZTRANS - 3.0, ZTRANS + 15.0); // match 
																   // 640x480

	//glFrustum(-1.0, 1.0, -1.0, 1.0, ZTRANS - 3.0, ZTRANS + 15.0); // match
																	// 800x800

	//gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", mvmatrix[0], mvmatrix[1], mvmatrix[2], mvmatrix[3], mvmatrix[4], mvmatrix[5], mvmatrix[6], mvmatrix[7], mvmatrix[8], mvmatrix[9], mvmatrix[10], mvmatrix[11], mvmatrix[12], mvmatrix[13], mvmatrix[14], mvmatrix[15]);

	glGetDoublev(GL_PROJECTION_MATRIX, pjmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pjmatrix[0], pjmatrix[1], pjmatrix[2], pjmatrix[3], pjmatrix[4], pjmatrix[5], pjmatrix[6], pjmatrix[7], pjmatrix[8], pjmatrix[9], pjmatrix[10], pjmatrix[11], pjmatrix[12], pjmatrix[13], pjmatrix[14], pjmatrix[15]);
	//gluLookAt(0, 0, 1.25, 0, 0, 0, 0, 1, 0);
	//glViewport(0, 0, (GLfloat)window_width, (GLfloat)window_height);
	gluPerspective(90, (GLfloat)window_width / (GLfloat)window_height, 0.1, 1000.0);
	//gluOrtho2D(-1, 1, -1, 1);
	//gluLookAt(0, 0, 1.25, 0, 0, 0, 0, 1, 0);
	
	//glTranslatef(0, 0,-2);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", mvmatrix[0], mvmatrix[1], mvmatrix[2], mvmatrix[3], mvmatrix[4], mvmatrix[5], mvmatrix[6], mvmatrix[7], mvmatrix[8], mvmatrix[9], mvmatrix[10], mvmatrix[11], mvmatrix[12], mvmatrix[13], mvmatrix[14], mvmatrix[15]);
	glGetDoublev(GL_MODELVIEW_MATRIX, txmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", txmatrix[0], txmatrix[1], txmatrix[2], txmatrix[3], txmatrix[4], txmatrix[5], txmatrix[6], txmatrix[7], txmatrix[8], txmatrix[9], txmatrix[10], txmatrix[11], txmatrix[12], txmatrix[13], txmatrix[14], txmatrix[15]);


	glGetDoublev(GL_PROJECTION_MATRIX, pjmatrix);
	//printf("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pjmatrix[0], pjmatrix[1], pjmatrix[2], pjmatrix[3], pjmatrix[4], pjmatrix[5], pjmatrix[6], pjmatrix[7], pjmatrix[8], pjmatrix[9], pjmatrix[10], pjmatrix[11], pjmatrix[12], pjmatrix[13], pjmatrix[14], pjmatrix[15]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0, 0, 2, 0, 0, 0, 0, 1, 0);

	for (i = 0; i < CUBE_MAP_SIZE; i++) {
		float t = 1.0 / (2 * CUBE_MAP_SIZE) + (float)i / CUBE_MAP_SIZE;
		t = 2.0 * t - 1.0;
		for (j = 0; j < CUBE_MAP_SIZE; j++) {
			float s = 1.0 / (2 * CUBE_MAP_SIZE) + (float)j / CUBE_MAP_SIZE;
			float pt[3];
			s = 2.0 * s - 1.0;
			pt[0] = 1;
			pt[1] = t;
			pt[2] = -s;
			CubeFunc(CubeMap[CUBE_POS_X][i][j], pt);
			pt[0] = -1;
			pt[1] = t;
			pt[2] = s;
			CubeFunc(CubeMap[CUBE_NEG_X][i][j], pt);

			pt[1] = 1;
			pt[0] = s;
			pt[2] = -t;
			CubeFunc(CubeMap[CUBE_POS_Y][i][j], pt);
			pt[1] = -1;
			pt[0] = s;
			pt[2] = t;
			CubeFunc(CubeMap[CUBE_NEG_Y][i][j], pt);

			pt[2] = 1;
			pt[0] = s;
			pt[1] = t;
			CubeFunc(CubeMap[CUBE_POS_Z][i][j], pt);
			pt[2] = -1;
			pt[0] = -s;
			pt[1] = t;
			CubeFunc(CubeMap[CUBE_NEG_Z][i][j], pt);
			for (k = CUBE_POS_X; k <= CUBE_NEG_Z; k++) {
				CubeMap[k][i][j][3] = 255;
			}
		}
	}

	CreateSphere(R, 0, 0, 0);
	//glEnable(GL_DEPTH_TEST);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	for (i = 0; i < 6; i++) {
		glPushMatrix();
		glClearDepth(1);

		glClearColor(0.0, 0.0, 0.0, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLoadIdentity();
		
		DisplaySphere(i);
		glPopMatrix();
		
		/*
		glTexImage2D(
			cubefaces[i],
			0,                  // level
			GL_RGBA8,          // internal format
			CUBE_MAP_SIZE,     // width
			CUBE_MAP_SIZE,     // height
			0,                 // border
			GL_RGBA,           // format
			GL_UNSIGNED_BYTE,   // type
			CubeMap[CUBE_POS_X + i]); // pixel data
		*/

		glCopyTexImage2D(cubefaces[i], 0, GL_RGBA, 0, 0, window_width, window_height, 0);
		//glLoadIdentity();
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);

	//glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	//glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	//glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glEnable(GL_TEXTURE_CUBE_MAP_EXT);
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_NORMALIZE);
}

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawSphere(1.0);
	//glRotatef(1, 0, 1, 0);
	//glLoadIdentity();
	//glTranslatef(0, 0, 1);
	//angle += 1;

	glutSwapBuffers();
	if (exitAfterOneFrame) {
		exit(0);
	}
}

static void
keyboard(unsigned char c, int x, int y)
{
	switch (c) {
	case 27:
		exit(0);
		break;
	case ' ':  /* Space bar redraws. */
		glutPostRedisplay();
		break;
	}
}

int
main(int argc, char **argv)
{
	PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;
	int i;

	//glutInitWindowSize(640, 480);
	glutInitWindowSize(window_width, window_height);
	glutInit(&argc, argv);
	for (i = 1; i < argc; i++) {
		if (!strcmp("-e", argv[i])) {
			exitAfterOneFrame = 1;
		}
	}
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	//glutInitWindowSize(800, 800);

	//glutInitWindowPosition(100, 100);
	glutCreateWindow("cubemap");

	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		// this is another function from WGL_EXT_swap_control extension
		wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglSwapIntervalEXT(-1);
	}

	/* Run-time extension check. */

	glutDisplayFunc(display);
	glutIdleFunc(display);
	//glutTimerFunc(display, 10);
	glutKeyboardFunc(keyboard);
	InitGraphics();
	glutMainLoop();
	return 0;
}
