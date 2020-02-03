/*
 *  Project: An Interactive 3D Maze Game 
 *  Project.cpp
 * 
 * -----------------------------------------------------------------------------
 *  Student Information
 * -----------------------------------------------------------------------------
 *  Student Name: Lyu Peiying
 *  Student ID:   1409853G-I011-0186
 *  E-mail:		  415887370@qq.com
 *  Major: Computer Technology / Electronic Information / Software Technology
 *  Year: 3
 * ------------------------------------------------------------------------------
 */
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "Bitmap.h"

#define M_PI 3.141592654

//===============player=============================

// ============ Global variables =======================
// Maze information
#define MAX_MAZESIZE 20
static int _mapx, _mapz; // Size of the maze	
static int _map[MAX_MAZESIZE][MAX_MAZESIZE];
int _initpos[2];         // Initial position of the player

static GLfloat _wallHeight = 1.0; // Height of the wall
static GLfloat _wallScale = 2.0;  // Scale of the width of the wall

// Camera setting
GLfloat _viewangle = 45.0; // Angle of view
GLfloat _viewdepth = 20.0; // View depth

// Define the player information structure
typedef struct _playerInfo {
   GLfloat degree;  // Object orientation
   GLfloat rotate;  // Object orientation
   GLfloat forward, spin;
   GLfloat pos[3];	// User position
   GLfloat mySize;	// User radial size
   GLfloat forwardStepSize;	// Step size
   GLfloat spinStepSize;	// Rotate step size
} playerInfo;

/**set the texture **/
GLuint	groundTextureID = 0;
GLuint  wallTextureID = 0;
GLuint  doorTextureID = 0;
GLuint  playerTextureID = 0;
/********************/

playerInfo _player;

int _drawmode = 0;

void init();
void initplayer();

// Capture the BMP file  //do not change here
GLubyte* TextureLoadBitmap(char *filename, int *w, int *h) // Bitmap file to load
{
   BITMAPINFO *info; // Bitmap information
   void       *bits; // Bitmap pixel bits
   GLubyte    *rgb;  // Bitmap RGB pixels

   // Try loading the bitmap and converting it to RGB...
   bits = LoadDIBitmap(filename, &info);
   
   if (bits==NULL) 
	  return (NULL);
   
   rgb = ConvertRGB(info, bits);
   
   if (rgb == NULL) {
      free(info);
      free(bits);
   }

   *w = info->bmiHeader.biWidth;
   *h = info->bmiHeader.biHeight;

   // Free the bitmap and RGB images, then return 0 (no errors).
   
   free(info);
   free(bits);
   return (rgb);
}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(_viewangle, (GLfloat) w / (GLfloat) h, 0.8, _viewdepth);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

//====== Drawing functions ===============
void DrawGround()
{
   // Draw the ground here
   glPushMatrix();
   	  // covering the floor following the x-z  plane
      glTranslatef(_wallScale * _mapx / 2.0, 0.0, _wallScale * _mapz / 2.0);
      glScalef(_wallScale * _mapx, 1.0, _wallScale * _mapz);
	  
	  glColor3f(1.0, 1.0, 1.0);
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, groundTextureID);

      glBegin(GL_QUADS);
	  //setting the texture coordinates
	  //point 1
	  glTexCoord2f(0.0, 0.0);
      glVertex3f(-0.5, 0.0, 0.5);

	  //point 2
	  glTexCoord2f(_mapx, 0.0);
      glVertex3f(0.5, 0.0, 0.5);

	  //point 3
	  glTexCoord2f(_mapx, _mapz);
      glVertex3f(0.5, 0.0, -0.5);

	  //point 4
	  glTexCoord2f(0.0, _mapz);
      glVertex3f(-0.5, 0.0, -0.5);

      glEnd();
	  glDisable(GL_TEXTURE_2D);
      glPopMatrix();
}
//枚举， 就四个方向 
enum wallDir {
	NORTH,
	WEST,
	SOUTH,
	EAST
};
void DrawWall(float cx, float cy, wallDir dir, GLuint textureID) {
	glBindTexture(GL_TEXTURE_2D, textureID);

	glPushMatrix();
	glScalef(1.0, 0.5, 1.0);
	glTranslatef(cx, 0.5*_wallScale, cy);
	glRotatef(dir*90.0, 0, 1, 0);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-_wallScale*0.5, -_wallScale*0.5, -_wallScale*0.5);

	glTexCoord2f(0, 1);
	glVertex3f(-_wallScale*0.5, _wallScale*0.5, -_wallScale*0.5);

	glTexCoord2f(1, 1);
	glVertex3f(_wallScale*0.5, _wallScale*0.5, -_wallScale*0.5);

	glTexCoord2f(1, 0);
	glVertex3f(_wallScale*0.5, -_wallScale*0.5, -_wallScale*0.5);

	glEnd();
	glPopMatrix();

}

void DrawWalls()
{
   // Draw the maze's walls here
	glEnable(GL_TEXTURE_2D);

	for (int i = 0; i < _mapz; ++i) {
		for (int j = 0; j < _mapx; ++j) {
			if (_map[i][j] % 2 != 0) {
				GLuint texture = _map[i][j] == 1 ? wallTextureID : doorTextureID;
				if (i - 1 >= 0 && !(_map[i - 1][j] % 2))		//north
				{
					DrawWall((j + 0.5)*_wallScale, (i + 0.5)*_wallScale, NORTH, texture);
				}
				if (j + 1 < _mapx && !(_map[i][j + 1] % 2))		//east
				{
					DrawWall((j + 0.5)*_wallScale, (i + 0.5)*_wallScale, EAST, texture);
				}
				if (i + 1 < _mapx && !(_map[i + 1][j] % 2))		//south
				{
					DrawWall((j + 0.5)*_wallScale, (i + 0.5)*_wallScale, SOUTH, texture);
				}
				if (j - 1 >= 0 && !(_map[i][j - 1] % 2))		//west
				{
					DrawWall((j + 0.5)*_wallScale, (i + 0.5)*_wallScale, WEST, texture);
				}
			}
		}
	}
	glDisable(GL_TEXTURE_2D);
}

//==================player========================

void DrawPlayer()
{
   // Draw your player here
	GLfloat a_ambient[] = { 0.0,0.0,1.0,1.0 };  //blue
	GLfloat a_diffuse[] = { 0.0,0.0,1.0,1.0 };   //blue
	GLfloat a_shininess[] = { 10 };


	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, playerTextureID);
	glPushMatrix();

	//drew the plater and set it into this position
	glTranslatef(_player.pos[0], _player.pos[1], _player.pos[2]);
	glScalef(_player.pos[1], _player.pos[1], _player.pos[1]);
	glRotatef(_player.degree, 0, 1, 0);
	glRotatef(_player.rotate, 1, 0, 0);

	glBegin(GL_TRIANGLE_STRIP); //三角网 

	//materica state
	glMaterialfv(GL_FRONT, GL_AMBIENT, a_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, a_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, a_shininess);

	for (int i = 0; i < 61; ++i)
	{
		//弧度转角度
		glTexCoord2f(0, i*0.05);
		glVertex3f(-0.5, 0.9*sin(i * 6 * M_PI / 180.0), 0.9*cos(i * 6 * M_PI / 180.0));
		glTexCoord2f(0.1, i*0.05);
		glVertex3f(-0.4, sin(i * 6 * M_PI / 180.0), cos(i * 6 * M_PI / 180.0));
	}

	for (int i = 0; i < 61; ++i)
	{
		glTexCoord2f(0.1, i*0.05);
		glVertex3f(-0.4, sin(i * 6 * M_PI / 180.0), cos(i * 6 * M_PI / 180.0));
		glTexCoord2f(0.9, i*0.05);
		glVertex3f(0.4, sin(i * 6 * M_PI / 180.0), cos(i * 6 * M_PI / 180.0));
	}

	for (int i = 0; i < 61; ++i)
	{
		glTexCoord2f(0.9, i*0.05);
		glVertex3f(0.4, sin(i * 6 * M_PI / 180.0), cos(i * 6 * M_PI / 180.0));
		glTexCoord2f(1.0, i*0.05);
		glVertex3f(0.5, 0.9*sin(i * 6 * M_PI / 180.0), 0.9*cos(i * 6 * M_PI / 180.0));
	}

	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//glutSwapBuffers();
	/********************
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, playerTextureID);

	glPushMatrix();
	glTranslatef(_player.pos[0], _player.pos[1], _player.pos[2]);

	glColor3f(1.0, 0, 1.0);
	glutSolidSphere(_player.mySize, 15, 15);

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
*******/
}

// For debugging collision detection
void DrawSphere()
{
   glPushMatrix();
	  glTranslatef(_player.pos[0], _player.pos[1], _player.pos[2]);
	  glColor3f(1.0, 0.0, 1.0); 
	  glutWireSphere(_player.mySize, 15, 15);
   glPopMatrix();
}

//JUST ADD a depth buffer and do not change any thinking here
void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glColor3f(1.0, 1.0, 1.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
	  gluLookAt(_player.pos[0] - 2.0 * sin(_player.degree * M_PI / 180.0), // eye
		        _player.pos[1] + 0.25, 
				_player.pos[2] - 2.0 * cos(_player.degree* M_PI / 180.0), 
				_player.pos[0], // at
				_player.pos[1],
				_player.pos[2],
				0.0, 1.0, 0.0); // up

 	  DrawGround();
  	  DrawWalls();
	  
	  if (_drawmode == 0)
		 DrawPlayer();
	  else
		 DrawSphere();
   glPopMatrix();

   glutSwapBuffers();
}

void checkcollide()
{
	//auto variable
	static auto timeStamp = clock() / 1000.0f;
	if (_player.forward == 0.0)
	{
		timeStamp = clock() / 1000.0f;
		return;
	}

   float dx, dz;
   // Check collision of walls here //触碰

   // Update the current position
   auto nowTime = clock() / 1000.0f;

   //play Information
   dx = _player.forward *(nowTime - timeStamp)* sin(_player.degree * M_PI / 180.0);
   dz = _player.forward *(nowTime - timeStamp)* cos(_player.degree * M_PI / 180.0);

   //玩家选装的角度
   _player.rotate += (360 / (2 * M_PI * _player.pos[1] / _player.forward)) * (nowTime - timeStamp);

   _player.pos[0] += dx;
   _player.pos[2] += dz;

   int i = _player.pos[2] / _wallScale;
   int j = _player.pos[0] / _wallScale;

   if (_map[i][j] == 3) {
	   MessageBox(NULL, "Arrival terminal", "congratulations", MB_OK);
	   initplayer();
   }

   //check north
   if (_player.pos[2] - _player.pos[1]<i*_wallScale&&_map[i - 1][j] == 1){
	   _player.pos[2] = i*_wallScale + _player.pos[1];
   }
   //check east
   if (_player.pos[0] + _player.pos[1]>(j + 1)*_wallScale&&_map[i][j + 1] == 1){
	   _player.pos[0] = (j + 1)*_wallScale - _player.pos[1];
   }
   //check south
   if (_player.pos[2] + _player.pos[1]>(i + 1)*_wallScale&&_map[i + 1][j] == 1){
	   _player.pos[2] = (i + 1)*_wallScale - _player.pos[1];
   }
   //check west
   if (_player.pos[0] - _player.pos[1]<j*_wallScale&&_map[i][j - 1] == 1){
	   _player.pos[0] = j*_wallScale + _player.pos[1];
   }

   timeStamp = nowTime;
}

void move(void)
{
	/*********************************************/
	static auto timeStamp = clock() / 1000.0f;
	auto nowTime = clock() / 1000.0f;
	/********************************************/
   if (_player.spin != 0.0) {
	   _player.degree += _player.spin*(nowTime - timeStamp);
	  if (_player.degree > 360.0) {
		  _player.degree -= 360.0;
	  }
	  else if (_player.degree < -360.0) {
		 _player.degree += 360.0;
	  }
   }
   timeStamp = nowTime;

   //if (_player.forward != 0.0) {
	  checkcollide();
   //}
   glutPostRedisplay();
}

void keyboard(unsigned char key,int x, int y)
{
   switch (key) {
	  case 's':
	  case 'S':
	     // Change to use sphere for the object
		 _drawmode++;
		 _drawmode %= 2;
		 break;
      case 27:
	  case 'q':
	  case 'Q':
	     exit(0);
   }
}

// Please read this function carefully, you can learn how to 
// make use the mouse buttons to control the Test Object/Player
void mouse(int button, int state, int x, int y)
{
   static int buttonhold = 0;
   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
	  if (buttonhold >= 2) {
	     // Stop forward and turn right
		 _player.forward = 0.0;
		 _player.spin = -_player.spinStepSize; // Turn right
	  }
	  else
		 _player.spin = 0.0; // Stop turn left
	  buttonhold--;
   }

   if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
	  if (buttonhold >= 2) {
		 // Stop forward and turn left
		 _player.forward = 0.0;
		 _player.spin = _player.spinStepSize; // Turn left
	  }
	  else
	 	 _player.spin = 0.0; // Stop turn right
	  buttonhold--;
   }

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
	  _player.forward = 0.0;
   }

   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
	  if (buttonhold > 0) {
		 _player.forward = _player.forwardStepSize;
		 _player.spin = 0.0;
	  }
	  else
		 _player.spin = _player.spinStepSize; // Turn left
	  buttonhold++;
   }

   if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
	  if (buttonhold > 0) {
	     _player.forward = _player.forwardStepSize;
		 _player.spin = 0.0;
	  }
	  else
		 _player.spin = -_player.spinStepSize; // Turn right
	  buttonhold++;
   }

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
	   _player.forward = _player.forwardStepSize;
   }
}

//changed size
void initplayer()
{
   // Initilaize the player
   // You may try change the values as you like
   _player.degree = 0.0; // User orientation
   _player.rotate = 0.0; // User orientation
   _player.mySize = 0.2; // User radial size
   _player.forward = 0.0;

   _player.forwardStepSize = 5.0; // Step size
   _player.spin = 0.0;
   _player.spinStepSize = 60.0; // Rotate step size

   // Init the player's position based on the postion on the map
   _player.pos[0] = _initpos[1] * _wallScale + _wallScale / 2.0;
   _player.pos[1] = _player.mySize;
   _player.pos[2] = _initpos[0] * _wallScale + _wallScale / 2.0;
}

void init()
{
   initplayer();
  // glClearColor(1, 1, 1, 1.0);
   glClearColor(0.2, 0.2, 0.2, 1.0);
   //glClearColor(0.0, 0.0, 0.0, 1.0);
   glClearDepth(1.0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   //============lighting=====================
   GLfloat light1_pos[] = { 10.0,10.0,-10.0,1.0 };
   GLfloat light1_dif[] = { 1.0,1.0,1.0, }; //whlit
   GLfloat light1_spe[] = { 0.0,0.0,1.0,1.0 };   //blue
   glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_dif);
   glLightfv(GL_LIGHT1, GL_SPECULAR, light1_spe);
   glEnable(GL_LIGHT1);

   //============fog===============================
   float fogColor[] = { 1.0, 0.0, 0.0, 1.0 };
   glFogi(GL_FOG_MODE, GL_LINEAR);		//kind of fog
   glFogfv(GL_FOG_COLOR, fogColor);		//color
   glFogf(GL_FOG_DENSITY, 0.05f);		//density
   glHint(GL_FOG_HINT, GL_DONT_CARE);   //rendering method
   glFogf(GL_FOG_START, 1.0f);		
   glFogf(GL_FOG_END, 15.0f);
   glEnable(GL_FOG);

   //call 4 texture fun()
   glGenTextures(1, &groundTextureID);
   glGenTextures(1, &wallTextureID);
   glGenTextures(1, &doorTextureID);
   glGenTextures(1, &playerTextureID);

   int height;
   int width;
   GLubyte *data = NULL;

   //ground
   glBindTexture(GL_TEXTURE_2D, groundTextureID);
   data = TextureLoadBitmap("texture//floor.bmp", &width, &height);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   //wall
   glBindTexture(GL_TEXTURE_2D, wallTextureID);
   data = TextureLoadBitmap("texture//wall.bmp", &width, &height);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   //door
   glBindTexture(GL_TEXTURE_2D, doorTextureID);
   data = TextureLoadBitmap("texture//door.bmp", &width, &height);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   //play
  // uquadric = gluNewQuadric();
   glBindTexture(GL_TEXTURE_2D, playerTextureID);
   data = TextureLoadBitmap("texture//player.bmp", &width, &height);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  
}

// Read in the maze map //do not change
int readmap(char* filename) 
{
   FILE* fp;
   int i, j;
   char tmp[MAX_MAZESIZE];

   fp = fopen(filename,"r");

   if (fp) {
      fscanf(fp, "%d", &_mapx);
	  fscanf(fp, "%d", &_mapz);
	  for (j = 0; j < _mapz; j++) {
 	     fscanf(fp, "%s", tmp);
	     for (i = 0; i < _mapx; i++) {
			_map[i][j] = tmp[i] - '0';
			if (_map[i][j] == 2) {
			   // Save the initial position
			   _initpos[0] = i;
			   _initpos[1] = j;
			}
			printf("%d", _map[i][j]);
		 }
 		 printf("\n");
	  }
	  fclose(fp);
   }
   else {
	  printf("Error Reading Map file!\n");
	  return 0;
   }
   return 1;
}

void main(int argc,char **argv)
{
   if (argc >= 2) {
	  srand(time(NULL));
	  if (readmap(argv[1]) == 0 )
	  	 exit(0);
	  
	  glutInit(&argc, argv);
	  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	  glutInitWindowSize(400, 300);
	  glutInitWindowPosition(250, 250);
	  if (glutCreateWindow("An Interactive 3D Maze Game (Skeleton)") == GL_FALSE)
	     exit(-1);
	  init();
	  glutDisplayFunc(display);
	  glutReshapeFunc(reshape);
	  glutKeyboardFunc(keyboard);
	  glutMouseFunc(mouse);
	  glutIdleFunc(move);
      glutMainLoop();
   } 
   else
 	  printf("Usage %s <mapfile>\n", argv[0]);
}

