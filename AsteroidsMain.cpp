#ifdef _WIN32
#include <Windows.h>
#endif
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif
typedef struct ship
{
double angle;//xmove, ymove
double point[3];
double incent[3];//x, y, radius
double side, back, per, area;
double len;
double wid;
double speed;
} ship;
typedef struct aster
{
	double cent[3];
	double col[3];
	double rad;
	double angle;
	double speed;
} aster;
typedef struct bolt
{
	double colour[3];
	double angle;
	double frontpos[2];
} bolt;
typedef struct image_t
{
	unsigned int size_x;
	unsigned int size_y;
	unsigned char *data;
} image_t;
ship argo;
image_t gShipim;
image_t gThr;
image_t gStars;
image_t gTitle;
char* filnam;
char* filname1;
char* starsfil;
char* titlefil;
GLuint gTexThr;
GLuint gTex;
GLuint gStarTex;
GLuint gTitleTex;
std::vector<aster> asters;
std::vector<bolt> shots;
bool lef, rig, fow, firing, lost, pause, full, kill1, kill2, kill4, ster, started;
double bsize;
double bspeed;
double maxspeed;
double regsize;
double offset;
int asteroids;
int maxasters;
int points;
int canfire;
void setup();
void display();
void initData();
void rend();
void onTime(int);
void arrows(int, int, int);
void relArrows(int, int, int);
void onKey(unsigned char, int, int);
void onKeyUp(unsigned char, int, int);
void loadGLTexture(char*, image_t*, GLuint*);
int ImageLoad(char*, image_t*);
#define msglError( ) _msglError( stderr, __FILE__, __LINE__ )
bool _msglError( FILE *out, const char *filename, int line ){
  bool ret = false;
  GLenum err = glGetError( );
  while( err != GL_NO_ERROR ) {
    ret = true;
    fprintf( out, "GL ERROR:%s:%d: %s\n",
      filename, line, (char*)gluErrorString( err ) );
    err = glGetError( );
  }
  return( ret );
}
static unsigned int getshort(FILE* fp){
  int c, c1;
  
  //get 2 bytes
  c = getc(fp);  
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}
static unsigned int getint(FILE* fp){
  int c, c1, c2, c3;

  // get 4 bytes
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}
void drawString (char *string) 
{
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *string);
        string ++;
    }
}
int main(int argc, char*argv[])
{
	//PlaySound(TEXT("mars.wav"), NULL, SND_FILENAME|SND_LOOP|SND_ASYNC|SND_NOSTOP);
	offset=.025;
	ster=false;
	started=false;
	titlefil=(char*)"Title.bmp";
	filnam= (char*)"Ship1.bmp";
	filname1=(char*)"fire.BMP";
	starsfil=(char*)"Stars.bmp";//http://a1star.com/star-space-background-7.php
	glutInitWindowSize(600, 400);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInit(&argc, argv);
	glutCreateWindow("ASTEROIDS");
	setup();
	glutDisplayFunc(display);
	glutKeyboardFunc(onKey);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialFunc(arrows);
	glutSpecialUpFunc(relArrows);
	glutTimerFunc(24, onTime,0);
	glutMainLoop();
}
void setup()
{
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glEnable( GL_DEPTH_TEST );
  glDepthRange(-1.0, 1.0);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_TEXTURE_2D);
  loadGLTexture( filnam, &gShipim, &gTex );
  loadGLTexture( filname1, &gThr, &gTexThr );
  loadGLTexture( starsfil, &gStars, &gStarTex);
  loadGLTexture(titlefil, &gTitle, &gTitleTex);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );
  glOrtho( -1, 1, -1, 1, 1, -1 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity( );
  msglError( );
}
void initData()
{
	started=true;
	srand(time(NULL));
	argo.point[0] = 0.0;
	argo.point[1] = 0.0;
	argo.point[2] = 0.0;
	argo.len = .05;
	argo.angle = 0.0;
	argo.wid = .025;
	argo.side = sqrt(argo.len*argo.len+argo.wid*argo.wid);
	argo.back = 2*argo.wid;
	argo.per = 2*argo.side+argo.back;
	argo.area = argo.wid*argo.len;
	argo.incent[2] = (2.0*argo.area)/argo.per;
	argo.incent[0] = (argo.side*argo.wid+argo.side*-argo.wid)/argo.per;
	argo.incent[1] = (argo.side*-argo.len+argo.side*-argo.len)/argo.per;
	argo.speed = 0.0;
	maxspeed = .012;
	bsize = .025;
	bspeed = .015;
	asteroids = 3;
	maxasters = 300;
	regsize = .1;
	points = 0;
	canfire = 0;
	lost=false;
	pause=false;
	kill4=false;
	shots.clear();
	asters.clear();
}
void display()
{
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 if(started)
	 {
		glColorMask(true, true, true, true);
		if(!ster)
		{
			rend();
		}
		else
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glTranslatef(-offset, 0.0, 0.0);
			glColorMask(true, false, false, false);
			rend();
			glLoadIdentity();
			glClear(GL_DEPTH_BUFFER_BIT) ;
			glColorMask(false, true, true, false);
			glTranslatef(+offset, 0.0, 0.0);
			rend();
		}
	 }
	 else
	 {
		  glEnable(GL_TEXTURE_2D);
		  glPushMatrix();
		  glBindTexture(GL_TEXTURE_2D, gTitleTex);
		  glBegin(GL_QUADS);
		  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		  glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		  glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		  glEnd();
		  glPopMatrix();
		  glDisable(GL_TEXTURE_2D);
	 }
	 glFlush();
	 msglError();
	 glutSwapBuffers();
}
void arrows(int key, int x,int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		fow=true;
			break;
	case GLUT_KEY_LEFT:
		lef=true;
			break;
	case GLUT_KEY_RIGHT:
		rig=true;
			break;
	}
}
void relArrows(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		fow=false;
			break;
	case GLUT_KEY_LEFT:
		lef=false;
			break;
	case GLUT_KEY_RIGHT:
		rig=false;
			break;
	case GLUT_KEY_DOWN:
		argo.angle+=180;
			break;
	}
}
double frand( ){
	return( double(rand()) / double(RAND_MAX) );
}
void onTime(int val)
{
	if((!lost&&!pause)||!started)
	{
	canfire++;
	if(asters.size()==0)
	{
		asteroids++;
		if(asteroids<maxasters)
		{
			while(asters.size()<asteroids)
			{
				aster a;
				a.rad=regsize;
				a.speed=.007;
				double r = frand();
				a.angle=360.0*r;
				//std::cout<<a.angle<<std::endl;
				a.cent[2]=0.0;
				int posneg=floor(frand()+.5);
				a.cent[0]=(posneg==1)?1.0:-1.0;
				int posneg2=floor(frand()+.5);
				if(posneg2==1)
					a.cent[1]=frand();
				else
					a.cent[1]=-frand();
				a.col[0]=(double)rand()/(double)RAND_MAX;
				a.col[1]=(double)rand()/(double)RAND_MAX;
				a.col[2]=(double)rand()/(double)RAND_MAX;
				asters.push_back(a);
			}
		}
	}
	
	for(int i=0; i<shots.size();i++)
	{
		for(int j=0; j<asters.size(); j++)
		{
			double cendis=sqrt((shots[i].frontpos[0]-asters[j].cent[0])*(shots[i].frontpos[0]-asters[j].cent[0])+
				(shots[i].frontpos[1]-asters[j].cent[1])*(shots[i].frontpos[1]-asters[j].cent[1]));
			if(cendis<asters[j].rad)
			{	
				#ifdef _WIN32
				PlaySound(TEXT("pop.wav"), NULL, SND_FILENAME|SND_ASYNC|SND_NOSTOP);
				#endif
				if(asters.size()<maxasters)
				{
					if(.00625!=asters[j].rad)
					{
						aster a;
						a.rad=.5*asters[j].rad;
						a.speed=.008;
						a.angle=(int)((360)*(double)rand()/(double)RAND_MAX);
						a.cent[2]=0.0;
						a.cent[0]=asters[j].cent[0];
						a.cent[1]=asters[j].cent[1];
						a.col[0]=(double)rand()/(double)RAND_MAX;
						a.col[1]=(double)rand()/(double)RAND_MAX;
						a.col[2]=(double)rand()/(double)RAND_MAX;
						asters.push_back(a);
						aster d;
						d.rad=.5*asters[j].rad;
						d.speed=.008;
						d.angle=(int)((360)*(double)rand()/(double)RAND_MAX);
						d.cent[2]=0.0;
						d.cent[0]=asters[j].cent[0];
						d.cent[1]=asters[j].cent[1];
						d.col[0]=(double)rand()/(double)RAND_MAX;
						d.col[1]=(double)rand()/(double)RAND_MAX;
						d.col[2]=(double)rand()/(double)RAND_MAX;
						asters.push_back(d);
					}
				}
				points+=int(20.0/asters[j].rad);
				asters.erase(asters.begin()+j);
				j--;
			}
		}
	}
	
	for(int i=0; i<asters.size(); i++)
	{
		asters[i].cent[1]+=cos((asters[i].angle*M_PI/180))*asters[i].speed;
		asters[i].cent[0]-=sin((asters[i].angle*M_PI/180))*asters[i].speed;
	}
	if(firing&&((canfire%2)==0))
	{
		bolt b;
		b.angle=argo.angle;
		b.frontpos[0]=argo.point[0];
		b.frontpos[1]=argo.point[1];
		b.colour[0]=(double)rand()/(double)RAND_MAX;
		b.colour[1]=(double)rand()/(double)RAND_MAX;
		b.colour[2]=(double)rand()/(double)RAND_MAX;
		shots.push_back(b);
		if(kill1&&kill2)
		{
			kill4=true;
		}
		if(kill4)
		{
		bolt c;
		c.angle=argo.angle+5;
		c.frontpos[0]=argo.point[0];
		c.frontpos[1]=argo.point[1];
		c.colour[0]=(double)rand()/(double)RAND_MAX;
		c.colour[1]=(double)rand()/(double)RAND_MAX;
		c.colour[2]=(double)rand()/(double)RAND_MAX;
		shots.push_back(c);
		bolt d;
		d.angle=argo.angle-5;
		d.frontpos[0]=argo.point[0];
		d.frontpos[1]=argo.point[1];
		d.colour[0]=(double)rand()/(double)RAND_MAX;
		d.colour[1]=(double)rand()/(double)RAND_MAX;
		d.colour[2]=(double)rand()/(double)RAND_MAX;
		shots.push_back(d);
		}
	}
	if(lef)
	{
		argo.angle+=5;
	}
	if(rig)
	{
		argo.angle-=5;
	}
	if(fow)
	{
		if(argo.speed<maxspeed)
		{
			argo.speed+=.00015;
		}
		argo.point[1]+=cos(argo.angle*3.14159265358979/180)*argo.speed;
		argo.point[0]-=sin(argo.angle*3.14159265358979/180)*argo.speed;
	}
	else
	{
		if(argo.speed>0)
		{
			argo.speed-=.00005;
		argo.point[1]+=cos(argo.angle*3.14159265358979/180)*argo.speed;
		argo.point[0]-=sin(argo.angle*3.14159265358979/180)*argo.speed;
		}
	}
	if(argo.point[0]>1||argo.point[0]<-1)
	{
		argo.point[0]=-argo.point[0];
	}
	if(argo.point[1]>1||argo.point[1]<-1)
	{
		argo.point[1]=-argo.point[1];
	}
	for(int i=0; i<asters.size(); i++)
	{
		if(asters[i].cent[0]>1||asters[i].cent[0]<-1)
		{
			asters[i].cent[0]=-asters[i].cent[0];
		}
		if(asters[i].cent[1]>1||asters[i].cent[1]<-1)
		{
			asters[i].cent[1]=-asters[i].cent[1];
		}
	}
	for(int i=0; i<shots.size(); i++)
	{
		if(shots[(shots.size()-1)-i].frontpos[0]<-1||
			shots[(shots.size()-1)-i].frontpos[0]>1||
			shots[(shots.size()-1)-i].frontpos[1]>1||
			shots[(shots.size()-1)-i].frontpos[1]<-1)
		{
			shots.erase(shots.begin()+(shots.size()-1-i));
			i--;

		}
		else
		{
			shots[(shots.size()-1-i)].frontpos[1]+=cos(shots[(shots.size()-1-i)].angle*M_PI/180)*bspeed;
			shots[(shots.size()-1-i)].frontpos[0]-=sin(shots[(shots.size()-1-i)].angle*M_PI/180)*bspeed;
		}
	}
	
	double radang=(argo.angle*M_PI/180.0);
	double inx=cos(radang)*(argo.incent[0])-sin(radang)*(argo.incent[1]);
	double iny=sin(radang)*(argo.incent[0])+cos(radang)*(argo.incent[1]);
	inx+=argo.point[0];
	iny+=argo.point[1];
	for(int i=0; i<asters.size(); i++)
	{
		double dis=sqrt((inx-asters[i].cent[0])*(inx-asters[i].cent[0])+
			(iny-asters[i].cent[1])*(iny-asters[i].cent[1]));
		//std::cout<<"dis"<<dis<<std::endl;
		if(dis<(asters[i].rad+argo.incent[2]))
		{
			#ifdef _WIN32
			PlaySound(TEXT("Boom.wav"), NULL, SND_FILENAME|SND_ASYNC|SND_NOSTOP);
			#endif
			lost=true;
			//std::cout<<"sum"<<(asters[i].rad+argo.incent[2])<<std::endl;
		}
	}
	glutPostRedisplay();
	}
	glutTimerFunc(24, onTime, 0);
}
void onKey(unsigned char c, int x, int y)
{
	switch(c)
	{
		case 'p':
			{
				pause=!pause;
			}
				break;
		case 32:
			{
				firing=true;
			}
				break;
		case 't':
			initData();
				break;
		case 'f':
			full=!full;
			if(!full)
			{
				glutReshapeWindow(600, 400);
				glutSetCursor(GLUT_CURSOR_INHERIT);
			}
			else
			{	
				glutFullScreen();
			}
				break;
		case 'q':
			exit(0);
				break;
		case 'k':
			kill1=true;
				break;
		case 'i':
			kill2=true;
				break;
	}
}
void onKeyUp(unsigned char c, int x, int y)
{
	switch(c)
	{
		case 32:
			{
				firing=false;
			}
				break;
		case 'k':
			kill1=false;
				break;
		case 'i':
			kill2=false;
				break;
		case 'd':
			ster=!ster;
			glLoadIdentity();
				break;
	}
}
void loadGLTexture( char* textureFileName, image_t* image, GLuint* texName ){	
  // allocate space for texture
  image = (image_t*)malloc(sizeof(image_t));
  if( image == NULL ){
    printf("Error allocating space for image");
    exit(0);
  }

  if( !ImageLoad(textureFileName, image) ){
    exit(1);
  }        

  // Create Textures
  glGenTextures( 1, texName );

  // linear filtered texture
  glBindTexture(GL_TEXTURE_2D, *texName);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, 3, image->size_x, image->size_y, 0,
  GL_RGB, GL_UNSIGNED_BYTE, image->data );
};
int ImageLoad( char* filename, image_t* image ){
  FILE *file;
  // size of the image in bytes.
  unsigned long size;
  // standard counter.
  unsigned long i;
  // number of planes in image (must be 1)
  unsigned short int planes;
  // number of bits per pixel (must be 24)
  unsigned short int bpp;
  // used to convert bgr to rgb color.
  char temp;
  // make sure the file is there.
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
  // seek through the bmp header, up to the width/height:
  fseek(file, 18, SEEK_CUR);
  // No 100% errorchecking anymore!!!
  // read the width
  image->size_x = getint (file);
  printf("Width of %s: %u\n", filename, image->size_x );
  // read the height 
  image->size_y = getint (file);
  printf("Height of %s: %u\n", filename, image->size_y );
  // calculate the size (assuming 24 bits or 3 bytes per pixel).
  size = image->size_x * image->size_y * 3;
  // read the planes
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }
  // read the bpp
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }

  // seek past the rest of the bitmap header.
  fseek(file, 24, SEEK_CUR);
  // read the data. 
  image->data = (unsigned char*) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0;	
  }
  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }
  /*
  for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }
  */
  // we're done.
  return 1;
}
void rend()
{	 
	 glLineWidth(3.0f);
	 for(int i=0; i<asters.size(); i++)
	 {
		 glPushMatrix();
		 glTranslated(asters[i].cent[0], asters[i].cent[1], 0.0);
		 glRotated(asters[i].angle, 0, 0, 1);
		 glBegin(GL_LINE_LOOP);
		 glColor3dv(asters[i].col);
		 int lineAmount = 20; //# of triangles used to draw circle
		 double twicePi = 2.0 * M_PI;
		 for(int j = 0; j <= lineAmount;j++) 
		 { 
			 glVertex3d((asters[i].rad * cos(j *  twicePi / lineAmount)), (asters[i].rad* sin(j * twicePi / lineAmount)), 0.0);
		 }
		 glEnd();
		 glPopMatrix();
	 }
	 for(int i=0; i<shots.size(); i++)
	 {
		 glPushMatrix();
		 glTranslated(shots[i].frontpos[0], shots[i].frontpos[1], 0.0);
		 glRotated(shots[i].angle, 0, 0, 1);
		 //glPointSize(4.0f);
		 //glBegin(GL_POINTS);
		 glBegin(GL_LINES);
		 glColor3dv(shots[i].colour);
		 glVertex3d(0.0, 0.0, 0.0);
		 glVertex3d(0.0, -bsize, 0.0);//commented out when using points
		 glEnd();
		 glPopMatrix();
	 }
	 glColor3d(1.0, 1.0, 1.0);
	 glRasterPos2d(-1.0, .9);
	 char buf[50];
	 sprintf(buf, "Points:%d", points);
	 #ifdef FREEGLUT
	 glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (unsigned char*)buf);
	 #else
	 drawString((unsigned char*)buf));
	 #endif
	 if(lost)
	 {
			glRasterPos2d(-.1, 0.0);
			char buff[40];
			sprintf(buff, "You lost! Press t to restart.");
			#ifdef FREEGLUT
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (unsigned char*)buff);
			#else
			drawString((unsigned char*)buff));
			#endif
	 }
	 glEnable(GL_TEXTURE_2D);
	 glPushMatrix();
	 glBindTexture(GL_TEXTURE_2D, gTex);
	 glTranslated(argo.point[0], argo.point[1], argo.point[2]);
	 glRotated(argo.angle, 0, 0, 1);
	 glColor3d(1.0, 1.0, 1.0);
	 glBegin(GL_QUADS);
	 //glVertex3d(0.0, 0.0, 0.0);
	 glTexCoord2f(0.0f, 0.0f); glVertex3d(-argo.wid, -argo.len, 0.0);
	 glTexCoord2f(1.0f, 0.0f); glVertex3d(argo.wid, -argo.len, 0.0);
	 glTexCoord2f(1.0f, 1.0f); glVertex3d(argo.wid, 0.0, 0.0);
	 glTexCoord2f(0.0f, 1.0f); glVertex3d(-argo.wid, 0.0, 0.0);
	 glEnd();
	 glBindTexture(GL_TEXTURE_2D, gTexThr);
	 glBegin(GL_QUADS);
	 //glColor3d(0.0, 0.0, 1.0);
	 glTexCoord2f(0.0f, 0.0f); glVertex3d(-.75*argo.wid, -argo.len-5*argo.speed, 0.0);
	 glTexCoord2f(1.0f, 0.0f); glVertex3d(.75*argo.wid, -argo.len-5*argo.speed, 0.0);
	 glTexCoord2f(1.0f, 1.0f); glVertex3d(.75*argo.wid, -argo.len, 0.0);
	 glTexCoord2f(0.0f, 1.0f); glVertex3d(-.75*argo.wid, -argo.len, 0.0);
	// glColor3d(1.0, 0.0, 1.0);
	 glEnd();
	 glPopMatrix();
	 glBindTexture(GL_TEXTURE_2D, gStarTex);
	 glBegin(GL_QUADS);
	 glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	 glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	 glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	 glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	 glEnd();
	 glDisable(GL_TEXTURE_2D);
}