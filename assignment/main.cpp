//!Includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>

#define printOpenGLError() printOglError(__FILE__, __LINE__)
int printOglError(char *file, int line);

//!Function Prototypes
bool initGL(int argc, char** argv);
void initShader();
void initGeometry();
void drawGeometry();
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initTexture(std::string filename, GLuint & textureID);

//Draw functions
void drawCrate();
void drawTank();

//! Screen size
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//Global Variables
GLuint shaderProgramID;
GLuint vertexPositionAttribute;
GLuint vertexNormalAttribute;		

//Texture
GLuint vertexTexcoordAttribute;
GLuint TextureMapUniformLocation;

//Mesh
Mesh crate;
GLuint crateTexture;
Mesh chassis;
GLuint tankTexture;

//Viewing
SphericalCameraManipulator cameraManip;
Matrix4x4 ModelViewMatrix;
GLuint MVMatrixUniformLocation;
Matrix4x4 ProjectionMatrix;	
GLuint ProjectionUniformLocation;	

//! Array of key states
bool keyStates[256];


//! Main Program Entry
int main(int argc, char** argv)
{	
	//init OpenGL
	if(!initGL(argc, argv))
		return -1;

    //Init Key States to false;    
    for(int i = 0 ; i < 256; i++)
        keyStates[i] = false;
    
    //Set up your program
    initShader();
    crate.loadOBJ("../models/test_cube.obj");   
	initTexture("../models/Crate.bmp", crateTexture);

	chassis.loadOBJ("../models/chassis.obj");
	initTexture("../models/humvee.bmp", tankTexture);

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(crate.getMeshCentroid());

    //Enter main loop
    glutMainLoop();

    //Delete shader program
	glDeleteProgram(shaderProgramID);

    return 0;
}

//! Function to Initlise OpenGL
bool initGL(int argc, char** argv)
{
	//Init GLUT
    glutInit(&argc, argv);
    
	//Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//Set Window Size
    glutInitWindowSize(screenWidth, screenHeight);
    
    // Window Position
    glutInitWindowPosition(200, 200);

	//Create Window
    glutCreateWindow("Tank Assignment");
    
    // Init GLEW
	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}
	
	//Set Display function
    glutDisplayFunc(display);
	
	//Set Keyboard Interaction Functions
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp); 

	//Set Mouse Interaction Functions
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);

    //Start start timer function after 100 milliseconds
    glutTimerFunc(100,Timer, 0);

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

	return true;
}

//Init Shader
void initShader(){
	
	shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");

	// Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");

    //!
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID,     "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID,   "ProjMatrix_uniform");

	vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	TextureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");

}

void initTexture(std::string filename, GLuint & textureID)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	//Get texture Data
	int width, height;
	char* data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,data);
	
	//Cleanup data as copied to GPU
	delete[] data;
}

//! Display Loop
void display(void)
{
    //Handle keys
    handleKeys();

	//Set Viewport
	glViewport(0,0,screenWidth, screenHeight);
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//Use shader
	glUseProgram(shaderProgramID);

	//Projection Matrix - Perspective Projection
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);

    //Set Projection Matrix
    glUniformMatrix4fv(	
	ProjectionUniformLocation,  //Uniform location
	1,							//Number of Uniforms
	false,						//Transpose Matrix
	ProjectionMatrix.getPtr());	//Pointer to ModelViewMatrixValues

	//Apply Camera Manipluator to Set Model View Matrix on GPU
    ModelViewMatrix.toIdentity();
    Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    m.getPtr());	        //Pointer to Matrix Values

    //Draw your scene
	drawCrate();
    drawTank();
    
    
    
    //Unuse Shader
	glUseProgram(0);

    //Swap Buffers and post redisplay
	glutSwapBuffers();
	glutPostRedisplay();
}

void drawCrate(){

	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crateTexture);
	glUniform1i(TextureMapUniformLocation, 0);

	//Call Draw Geometry Function
    crate.Draw(vertexPositionAttribute,-1,vertexTexcoordAttribute);

}

void drawTank(){

	//Draw chassis
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tankTexture);
	glUniform1i(TextureMapUniformLocation, 0);

	//Call Draw Geometry Function
    chassis.Draw(vertexPositionAttribute,-1,vertexTexcoordAttribute);

}

//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}
    
    //Set key status
    keyStates[key] = true;
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{
    //keys should be handled here
	if(keyStates['a'])
    {
        
    }
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
    cameraManip.handleMouse(button, state,x,y);
    glutPostRedisplay(); 
}

//! Motion
void motion(int x, int y)
{
    cameraManip.handleMouseMotion(x,y);
    glutPostRedisplay(); 
}

//! Timer Function
void Timer(int value)
{
    
    //Call function again after 10 milli seconds
	glutTimerFunc(10,Timer, 0);
}

int printOglError(char *file, int line)
{
	GLenum glErr;
	int retCode = 0;
	glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n",
		file, line, gluErrorString(glErr));
		retCode = 1;
	}
	return retCode;
}