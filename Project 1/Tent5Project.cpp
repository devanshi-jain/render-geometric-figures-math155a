/*
 * TetrahedraFlatSmooth.cpp - Version 1.1 - March 31, 2022.
 *
 * This is the main program file for the programming Project #1
 *   to draw 5-sided colored tents for Math 155A in Spring 2022.
 * This starter code draws two tetrahedra instead, but has all
 * other required functionality.
 *
 * Author: Sam Buss
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG2
 */

// These libraries are needed to link the program.
// First five are usually proved by the system.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC

//#include "GL/glew.h" 
#include "GL/glew.h" 
//#include "GLFW/glfw3.h"
#include "GLFW/glfw3.h"

#include "ShaderMgrSAM.h"
#include "LinearR3.h"		// Adjust path as needed.
#include "LinearR4.h"		// Adjust path as needed.
bool check_for_opengl_errors();     // Function prototype (should really go in a header file)

// Enable standard input and output via printf(), etc.
// Put this include *after* the includes for glew and GLFW!
#include <stdio.h>

// ********************
// Animation controls and state information
// ********************
double viewAzimuth = 0.0;	// Angle of view up/down (in radians)
double viewDirection = 0.0; // Rotation of view around y-axis (in radians)
double deltaAngle = 0.01;	// Change in view angle for each up/down/left/right arrow key press
LinearMapR4 viewMatrix;		// The current view matrix, based on viewAzimuth and viewDirection.
bool wireframeMode = true;	// Equals true for polygon GL_FILL mode. False for polygon GL_LINE mode.
bool cullBackFaces = false; // Equals true to cull backfaces. Equals false to not cull backfaces.   

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************

// NumObjects: PROBABLY SHOULD EQUAL 3 TO FORM THE TENT WITH
//     (1) THREE TRIANGLES (base), (2) A TRIANGLE STRIP (side) AND (3) A TRIANGLE FAN (top).
const int NumObjects = 3;
//const int iTetra = 0;
const int iTriangles = 0;
const int iTriangleStrip = 1;
const int iTriangleFan = 2;

unsigned int myVBO[NumObjects];  // a Vertex Buffer Object holds an array of data
unsigned int myVAO[NumObjects];  // a Vertex Array Object - holds info about an array of vertex data;
//glGenVertexArrays(1, &myVBO);
//glGenBuffers(1, &myVBO);
// We create two shader programs: each consists of a vertex shader and a fragment shader
//     One program uses smooth shading, the other flat shading.
unsigned int shaderProgramSmooth;
unsigned int shaderProgramFlat;
const unsigned int aPos_loc = 0;   // Corresponds to "location = 0" in the verter shader definitions
const unsigned int aColor_loc = 1; // Corresponds to "location = 1" in the verter shader definitions
const char* projMatName = "projectionMatrix";		// Name of the uniform variable projectionMatrix
unsigned int projMatLocation_smooth;				// Location of the projectionMatrix in the "smooth" shader program.
unsigned int projMatLocation_flat;					// Location of the projectionMatrix in the "flat" shader program.
const char* modelviewMatName = "modelviewMatrix";	// Name of the uniform variable modelviewMatrix
unsigned int modelviewMatLocation_smooth;			// Location of the modelviewMatrix in the "smooth" shader program.
unsigned int modelviewMatLocation_flat;				// Location of the modelviewMatrix in the "flat" shader program.

//  The Projection matrix: Controls the "camera view/field-of-view" transformation
//     Generally is the same for all objects in the scene.
LinearMapR4 theProjectionMatrix;		//  The Projection matrix: Controls the "camera/view" transformation

// A ModelView matrix controls the placement of a particular object in 3-space.
//     It is generally different for each object.
LinearMapR4 theModelViewMatrix;
float matEntries[16];		// Holds 16 floats (since cannot load doubles into a shader that uses floats)

// YOU MAY NEED TO ADJUST THE VALUE OF THE NEXT VARIABLES TO PLACE 
// THE TWO TENTS WITHIN THE FIELD OF VIEW..
// *****************************
// These variables set the dimensions of the rectanglar region we wish to view.
// They must bound the values of all vertices, even after transformation by
//    the modelview matrix.
// They are used to help form the projection matrix.
const double Xmin = -2.7, Xmax = 2.7;
const double Ymin = -2.7, Ymax = 2.7;
const double Zmin = -3.0, Zmax = 3.0;

// *************************
// mySetupGeometries defines the scene data, especially vertex  positions and colors.
//    - It also loads all the data into the VAO's (Vertex Array Objects) and
//      into the VBO's (Vertex Buffer Objects).
// This routine is only called once to initialize the data.
// *************************
void mySetupGeometries() {

	const float sq32 = sqrt(3.0f) / 2.0f;

	// Allocate the needed Vertex Array Objects (VAO's) and Vertex Buffer Objects (VBO')
	glGenVertexArrays(NumObjects, &myVAO[0]);
	glGenBuffers(NumObjects, &myVBO[0]);

	float trianglesVerts[] = {
		// x,y,z coordinates	// R,G,B colors
		1.5f, -2.0f, 1.0f,		0.8f, 0.8f, 0.8f, // First triangle
		2.0f, -2.0f, 0.0f,		0.8f, 0.8f, 0.8f,
		-1.5f, -2.0f, 1.0f,		0.8f, 0.8f, 0.8f,
		
		2.0f, -2.0f, 0.0f,		0.8f, 0.8f, 0.8f, // Second triangle
		0.0f, -2.0f, -1.0f,		0.8f, 0.8f, 0.8f,
		-1.5f, -2.0f, 1.0f,		0.8f, 0.8f, 0.8f,
		
		-1.5f, -2.0f, 1.0f,		0.8f, 0.8f, 0.8f,	// Third triangle
		0.0f, -2.0f, -1.0f,		0.8f, 0.8f, 0.8f,
		-2.0f, -2.0f, 0.0f,		0.8f, 0.8f, 0.8f,
		
	};

	// Do the same as above, now for the three triangles.
	glBindVertexArray(myVAO[iTriangles]);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[iTriangles]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(trianglesVerts), trianglesVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aColor_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(aColor_loc);

	
	// A Triangle Strip
	// Specified 12 vertices that will be rendered with GL_TRIANGLE_STRIP
	float triangleStripVerts[] = {
		// Positions			// Colors
		-1.5f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
		-1.5f, -2.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
		1.5f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f,   // Yellow
		1.5f, -2.0f, 1.0f,		1.0f, 1.0f, 0.0f,   // Yellow
		2.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,	// Green
		2.0f, -2.0f, 0.0f,		0.0f, 1.0f, 0.0f,	// Green
		0.0f, 0.0f, -1.0f,		1.0f, 0.0f, 0.0f,	// Red
		0.0f, -2.0f, -1.0f,		1.0f, 0.0f, 0.0f,   // Red
		-2.0f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f,	// Blue
		-2.0f, -2.0f, 0.0f,		0.0f, 0.0f, 1.0f,	// Blue
		-1.5f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
		-1.5f, -2.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
	};

	//vertices forming a triangle strip.
	glBindVertexArray(myVAO[iTriangleStrip]);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[iTriangleStrip]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleStripVerts), triangleStripVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aColor_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(aColor_loc);


	// a Triangle Fan
	// Array of x,y,mx coordinates for 6 vertices for the triangles rendered with GL_TRIANGLE_FAN
	float triangleFanVerts[] = {
		// Positions			// Colors
		0.0f, 1.0f, 0.0f,		0.8f, 0.8f, 0.8f,   // Light grey
		-1.5f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
		1.5f, 0.0f, 1.0f,	    1.0f, 1.0f, 0.0f,   // Yellow
		2.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,   // Green
		0.0f, 0.0f, -1.0f,		1.0f, 0.0f, 0.0f,	// Red
		-2.0f, 0.0f,0.0f,		0.0f, 0.0f, 1.0f,	// Blue
		-1.5f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,	// Cyan
	};
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[iTriangleFan]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleFanVerts), triangleFanVerts, GL_STATIC_DRAW);
	glBindVertexArray(myVAO[iTriangleFan]); 
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aColor_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(aColor_loc);
	
	
	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}
// Main routine for rendering the scene
// myRenderScene() is called every time the scene needs to be redrawn.
// mySetupGeometries() has already created the vertex and buffer objects
//    and the model view matrices.
// setup_shaders() has already created the shader programs.
// *************************************
void myRenderScene() {

	// Clear the rendering window
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float clearDepth = 1.0f;
	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_DEPTH, 0, &clearDepth);	// Must pass in a pointer to the depth value!
	for (int i = 0; i < 2; i++) {
		if (i == 0) {
			glUseProgram(shaderProgramSmooth);		// Render smooth the first time
		}
		else {
			glUseProgram(shaderProgramFlat);		// Render the second one flat.
		}
		theModelViewMatrix = viewMatrix;	// View matrix moves the scene in front of the camera.
		if (i == 0) {
			theModelViewMatrix.Mult_glTranslate(-1.5, 0.0, 0.0);	// Translate 1.5 down the NEGATIVE x-axis.
		}
		else {
			theModelViewMatrix.Mult_glTranslate(1.5, 0.0, 0.0);		// Translate 1.5 down the POSTIVE x-axis.
		}
		// THE NEXT Mult_glScale COMMAND MAY NEED TO BE CHANGED TO MAKE THE TENT EITHER
		// SMALLER OR LARGER, IN ORDER TO FIT WELL IN THE SCENE.
		// You may also add extra translations here if the tent is not centered at the origin.
		//    For this, use Mult_glTranslate, similarly to above.
		// You can change the scale amount below to change the size.
		theModelViewMatrix.Mult_glScale(0.5);				// Scale (shrink or grow) by (0.5, 0.5, 0.5).
		theModelViewMatrix.DumpByColumns(matEntries);
		glUniformMatrix4fv(modelviewMatLocation_smooth, 1, false, matEntries);

		// THE NEXT LINES WILL NEED TO BE CHANGED TO DRAW THE TENT:
		glBindVertexArray(myVAO[iTriangles]);
		glDrawArrays(GL_TRIANGLES, 0, 9);
		glBindVertexArray(myVAO[iTriangleStrip]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);
		glBindVertexArray(myVAO[iTriangleFan]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
	}
	
	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void my_setup_SceneData() {
	mySetupGeometries();
	setup_shaders();

	// Get the locations of the projection and model view matrices in the shader programs.
	projMatLocation_smooth = glGetUniformLocation(shaderProgramSmooth, projMatName);
	projMatLocation_flat = glGetUniformLocation(shaderProgramFlat, projMatName);
	modelviewMatLocation_smooth = glGetUniformLocation(shaderProgramSmooth, modelviewMatName);
	modelviewMatLocation_flat = glGetUniformLocation(shaderProgramFlat, modelviewMatName);

	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

// *******************************************************
// Process all key press events.
// This routine is called each time a key is pressed or released.
// *******************************************************
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	static const double Pi = 3.1415926535f;
	if (action == GLFW_RELEASE) {
		return;			// Ignore key up (key release) events
	}
	switch (key) {
	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_X:
		glfwSetWindowShouldClose(window, true);
		return;
	case 'w':
	case 'W':		// Toggle wireframe mode
		if (wireframeMode) {
			wireframeMode = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			wireframeMode = true;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		return;
	case 'c':
	case 'C':		// Toggle backface culling
		if (cullBackFaces) {
			cullBackFaces = false;
			glDisable(GL_CULL_FACE);
		}
		else {
			cullBackFaces = true;
			glEnable(GL_CULL_FACE);
		}
		return;
	default:
		return;
	case 'r':
	case 'R':
		viewAzimuth = 0.0;
		viewDirection = 0.0;
		break;
	case GLFW_KEY_UP:
		viewAzimuth = Min(viewAzimuth + deltaAngle, 1.5);	// Increment, do not exceed 1/5 < pi/2
		break;
	case GLFW_KEY_DOWN:
		viewAzimuth = Max(viewAzimuth - deltaAngle, -1.5);	// Increment, do not go below -1/5 > -pi/2
		break;
	case GLFW_KEY_LEFT:
		viewDirection += deltaAngle;		// Increment
		if (viewDirection > Pi) {
			viewDirection -= 2.0*Pi;		// Wrap back to -pi after reaching +pi
		}
		break;
	case GLFW_KEY_RIGHT:
		viewDirection -= deltaAngle;		// Decrement
		if (viewDirection <= -Pi) {
			viewDirection += 2.0*Pi;			// Wrap back to +pi after reaching -pi
		}
		break;
	}
	// Set the viewMatrix based on viewAziuth and viewDirection:
	viewMatrix.Set_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	// Rotate viewAzimuth radians around x-axis
	viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);  // Rotate -viewDirection radians around y-axis
}


// *************************************************
// This function is called with the graphics window is first created,
//    and again whenever it is resized.
// The Projection View Matrix is typically set here.
//    But this program does not use any transformations or matrices.
// *************************************************
void window_size_callback(GLFWwindow* window, int width, int height) {
	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, width, height);

	// Setup the projection matrix as an orthographic view.
	// Determine the min and max values for x and y that should appear in the window.
	// The complication is that the aspect ratio of the window may not match the
	//		aspect ratio of the scene we want to view.
	double w = (width == 0) ? 1 : width;
	double h = (height == 0) ? 1 : height;
	double windowXmin, windowXmax, windowYmin, windowYmax;
	if ((Xmax - Xmin) / w < (Ymax - Ymin) / h) {
		double scale = ((Ymax - Ymin) / h) / ((Xmax - Xmin) / w);
		double center = (Xmax + Xmin) / 2.0;
		windowXmin = center - (center - Xmin)*scale;
		windowXmax = center + (Xmax - center)*scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	}
	else {
		double scale = ((Xmax - Xmin) / w) / ((Ymax - Ymin) / h);
		double center = (Ymax + Ymin) / 2;
		windowYmin = center - (center - Ymin)*scale;
		windowYmax = center + (Ymax - center)*scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}

	// Using the max & min values for x & y & z that should be visible in the window,
	//		we set up the orthographic projection.
	theProjectionMatrix.Set_glOrtho(windowXmin, windowXmax, windowYmin, windowYmax, Zmin, Zmax);

	theProjectionMatrix.DumpByColumns(matEntries);
	if (glIsProgram(shaderProgramSmooth)) {
		glUseProgram(shaderProgramSmooth);
		glUniformMatrix4fv(projMatLocation_smooth, 1, false, matEntries);
	}
	if (glIsProgram(shaderProgramFlat)) {
		glUseProgram(shaderProgramFlat);
		glUniformMatrix4fv(projMatLocation_smooth, 1, false, matEntries);
	}
	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void my_setup_OpenGL() {
	
	glEnable(GL_DEPTH_TEST);	// Enable depth buffering
	glDepthFunc(GL_LEQUAL);		// Useful for multipass shaders

	// Set polygon drawing mode for front and back of each polygon
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Disable backface culling to render both sides of polygons
	glFrontFace(GL_CCW);		// The default setting
	glCullFace(GL_BACK);		// The default setting
	glDisable(GL_CULL_FACE);

	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void error_callback(int error, const char* description)
{
	// Print error
	fputs(description, stderr);
}

void setup_callbacks(GLFWwindow* window) {
	// Set callback function for resizing the window
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	// Set callback for key up/down/repeat events
	glfwSetKeyCallback(window, key_callback);

	// Set callbacks for mouse movement (cursor position) and mouse botton up/down events.
	// glfwSetCursorPosCallback(window, cursor_pos_callback);
	// glfwSetMouseButtonCallback(window, mouse_button_callback);
}

int main() {
	glfwSetErrorCallback(error_callback);	// Supposed to be called in event of errors. (doesn't work?)
	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	const int initWidth = 800;
	const int initHeight = 600;
	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "TetrahedronFlatSmooth", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window!\n");
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (GLEW_OK != glewInit()) {
		printf("Failed to initialize GLEW!.\n");
		return -1;
	}

	// Print info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    printf("Using GLEW version %s.\n", glewGetString(GLEW_VERSION));

	printf("------------------------------\n");
	printf("Press arrow keys (up, down, left, right) to adjust view direction.\n");
	printf("Press R (Reset) to reset view position.\n");
	printf("Press W (Wireframe) to change between GL_POLYGON_MODE of GL_FILL and GL_LINES.\n");
	printf("Press C (Cull faces) to toggle whether back faces are culled.\n");
	printf("Press EXCAPE or 'X' or 'x' to exit.\n");
	
    setup_callbacks(window);
   
	// Initialize OpenGL, the scene and the shaders
    my_setup_OpenGL();
	my_setup_SceneData();
 	window_size_callback(window, initWidth, initHeight);

    // Loop while program is not terminated.
	while (!glfwWindowShouldClose(window)) {
	
		myRenderScene();				// Render into the current buffer
		glfwSwapBuffers(window);		// Displays what was just rendered (using double buffering).

		// Poll events (key presses, mouse events)
		glfwWaitEventsTimeout(1.0/60.0);	    // Use this to animate at 60 frames/sec (timing is NOT reliable)
		// glfwWaitEvents();					// Or, Use this instead if no animation.
		// glfwPollEvents();					// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}

// If an error is found, it could have been caused by any command since the
//   previous call to check_for_opengl_errors()
// To find what generated the error, you can try adding more calls to
//   check_for_opengl_errors().
char errNames[8][36] = {
	"Unknown OpenGL error",
	"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION",
	"GL_INVALID_FRAMEBUFFER_OPERATION", "GL_OUT_OF_MEMORY",
	"GL_STACK_UNDERFLOW", "GL_STACK_OVERFLOW" };
bool check_for_opengl_errors() {
	int numErrors = 0;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		numErrors++;
		int errNum = 0;
		switch (err) {
		case GL_INVALID_ENUM:
			errNum = 1;
			break;
		case GL_INVALID_VALUE:
			errNum = 2;
			break;
		case GL_INVALID_OPERATION:
			errNum = 3;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errNum = 4;
			break;
		case GL_OUT_OF_MEMORY:
			errNum = 5;
			break;
		case GL_STACK_UNDERFLOW:
			errNum = 6;
			break;
		case GL_STACK_OVERFLOW:
			errNum = 7;
			break;
		}
		printf("OpenGL ERROR: %s.\n", errNames[errNum]);
	}
	return (numErrors != 0);
}
