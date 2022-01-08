#define _CRT_SECURE_NO_WARNINGS
#include "glut.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

const double PI = 3.14;
const int W = 600;
const int H = 600;
const int TW = 512;
const int TH = 512;
const int TREE_NUM = 15;
const int SMOKE = 150;

const int GSZ = 150; // ground size

double ground[GSZ][GSZ] = { 0 }; // this is the matrix of heights

//Tree struct for none-random position
typedef struct Tree {
	int x;
	int y;
	float h;
} Tree;

Tree trees[TREE_NUM];

typedef struct Train {
	double x, y;
} Train;

Train train = {55,5.5};


// texture definitions
unsigned char railRoad[TH][TW][3]; // 3 stands for rgb
unsigned char tx3[1024][2048][3]; // sunrise 
unsigned char water[256][256][3]; //water texture
unsigned char stone[256][256][3]; //stone texture
unsigned char wood[TH][TW][3]; //wood texture
unsigned char leaves[1024][1024][3]; // leaves texture
unsigned char smoke[256][256][3]; // smoke texture


unsigned char* bmp;

// locomotion (ego-motion)
double eyex=0, eyey=14, eyez = 26;

double yaw=PI, pitch = -0.1, roll;
double speed = 0, angularSpeed = 0;
double dir[3] = {sin(yaw),sin(pitch),cos(yaw)};
// aircraft motion
double airyaw = PI, airpitch = 0, airroll;
double airspeed = 0, airangularSpeed = 0;
double airdir[3] = { sin(airyaw),sin(airpitch),cos(airyaw) };
double airLocation[3] = {0,18,0};

double offset = 0;

bool buildTerrain = false;

void Smooth();

// first light source
float lamb0[4] = { 0.2,0.2,0.2,0 };
float ldiff0[4] = { 0.8,0.5,0.2,0 };
float lspec0[4] = { 0.5,0.5,0.5,0 };
float lpos0[4] = { -1,0.2,0,0};// the last element defines kind of light source:
// if it is 1 the the light is positional; if it is 0 the the light is directional

// second light source
float lamb1[4] = { 0.2,0.2,0.2,0 };
float ldiff1[4] = { 0.8,0.8,0.8,0 };
float lspec1[4] = { 0.7,0.7,0.7,0 };
float lpos1[4] = { 1,1,1,0 }; // directional




void ReadBitmap(const char fname[])
{
	FILE* pf;
	// BITMAP file starts with 
	BITMAPFILEHEADER bf; 
	// then goes
	BITMAPINFOHEADER bi;
	// and then goes stream: bgrbgrbgr.....
	int size; // how many BYTES will be read

	pf = fopen(fname, "rb");
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, pf);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, pf);
	size = bi.biHeight * bi.biWidth * 3;

	bmp = (unsigned char*) malloc(size);
	fread(bmp, 1, size, pf);
	fclose(pf);
}




double Distance(int i1, int j1, int i2, int j2)
{
	return sqrt((double)(i1 - i2) * (i1 - i2) + (j1 - j2) * (j1 - j2));
}


void SetTexture(int kind)
{
	int i, j,delta,k;
	switch (kind)
	{

	case 0: //stone texture
		for (i = 0, k = 0; i < 256; i++)
			for (j = 0; j < 256; j++, k += 3)
			{
				stone[i][j][0] = bmp[k + 2]; // red
				stone[i][j][1] = bmp[k + 1]; // green
				stone[i][j][2] = bmp[k]; // blue
			}
		break;

	case 1: // rail_road
		for (i = 0; i < TH; i++)
			for (j = 0; j < TW; j++)
			{
				delta = -20 + rand() % 60; // random value in range [-20,20)
				if (j < TH/3 || i > 64 && i < 128  || i < TW - 64 && i > TW -128)
				{
					railRoad[i][j][0] = 165 + delta;//red
					railRoad[i][j][1] = 42 + delta;//green
					railRoad[i][j][2] = 42 + delta;//blue
				}
				else				//road
				{
					railRoad[i][j][0] = 200 + delta;//red
					railRoad[i][j][1] = 200 + delta;//green
					railRoad[i][j][2] = 200 + delta;//blue
				}
			}
		break;

	case 2: // wood texture
		for (i = 0, k = 0; i < TH; i++)
			for (j = 0; j < TW; j++, k += 3)
			{
				wood[i][j][0] = bmp[k + 2]; // red
				wood[i][j][1] = bmp[k + 1]; // green
				wood[i][j][2] = bmp[k]; // blue
			}
		break;

	case 3: // leaves texture
		for (i = 0, k = 0; i < 1024; i++)
			for (j = 0; j < 1024; j++, k += 3)
			{
				leaves[i][j][0] = bmp[k + 2]; // red
				leaves[i][j][1] = bmp[k + 1]; // green
				leaves[i][j][2] = bmp[k]; // blue
			}
		break;

	case 4: // smoke texture
		for (i = 0, k = 0; i < 256; i++)
			for (j = 0; j < 256; j++, k += 3)
			{
				leaves[i][j][0] = bmp[k + 2]; // red
				leaves[i][j][1] = bmp[k + 1]; // green
				leaves[i][j][2] = bmp[k]; // blue
			}
		break;

	case 6: // water texture
		for (i = 0, k = 0; i < 256; i++)
			for (j = 0; j < 256; j++, k += 3)
			{
				water[i][j][0] = bmp[k + 2]; // red
				water[i][j][1] = bmp[k + 1]; // green
				water[i][j][2] = bmp[k]; // blue
			}
		break;

	case 7: // sunrise
		for (i = 0, k = 0; i < 1024; i++)
			for (j = 0; j < 2048; j++, k += 3)
			{
				tx3[i][j][0] = bmp[k + 2]; // red
				tx3[i][j][1] = bmp[k + 1]; // green
				tx3[i][j][2] = bmp[k]; // blue
			}
		break;
	}
}

// align heights along z=0
//add moving element?
void DisplayRiver()
{
	int j,i;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 6); // water texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE

	for (i = 0; i < GSZ ; i++)
	{
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3d(-11, ground[i][GSZ / 2]+0.1 , i - GSZ/2); // point 1;
		glTexCoord2d(1, 0);
		glVertex3d(-11, ground[i + 1][GSZ / 2]+0.1 , i - GSZ/2 +1); // point 2
		glTexCoord2d(1, 2);
		glVertex3d(11, ground[i + 1][GSZ / 2]+0.1 , i - GSZ/2 + 1); // point 3
		glTexCoord2d(0, 2);
		glVertex3d(11, ground[i][GSZ / 2]+0.1 , i - GSZ/2); // point 4
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

		

}

void PrepareRailRoad()
{
	int i,j;
	for (i = -2; i < 2; i++) 
	{
		for (j = 0; j < GSZ; j++)
			if (j <= GSZ / 2 - 10 || j >= GSZ / 2 + 10)
			{
				ground[GSZ / 2 + 1+i][j] = ground[GSZ / 2 - 1 + i][j] = ground[GSZ / 2 + i][j] = 5;
			}
	}

			
}

void PrepareGround(int startx, int startz, int endx, int endz, int height)
{

	float random_delta;
	int i, j;
	for (i = startx; i < endx; i++)
	{
		ground[i][startz -1] = height;

		for (j = startz; j < endz; j++)
		{
			ground[0][j] = height;
			random_delta = (float)rand() / (float)(RAND_MAX/5);
			if (rand() % 2 == 0)
			{
				random_delta = -random_delta;
			}
			ground[i][j] = (ground[i][j - 1] + ground[i - 1][j] + ground[i - 1][j - 1] + random_delta)/3;
	}

	}
		

}

void initTrees() 
{
	int i, x=0, y=0;
	float random_h;
	int size;

	for (i = 0; i < TREE_NUM; i++)
	{
		while (x < 10 || x > 65) {
			x = rand() % GSZ / 2;
		}
		while (y < 10 || y > 65)
		{
			y = rand() % GSZ / 2;
		}
		random_h = (float)rand() / (float)(RAND_MAX / 5);

		int randomQ;
		randomQ = rand() % 4;

		if (randomQ % 4 == 0)
		{
		}
		else if (randomQ % 4 == 1) {
			x = -x;
		}
		else if (randomQ % 4 == 2) {
			y = -y;
		}
		else if (randomQ % 4 == 3) {
			x = -x;
			y = -y;
		}
		trees[i].x = x;
		trees[i].y = y;
		trees[i].h = random_h;
	}
}


void init()
{
	int i, j;
	srand(time(0)); // seed or init random numbers generator
	
	glClearColor(0.0, 0.6, 0.8, 0);// color of window background
	glEnable(GL_DEPTH_TEST); // allow to show the nearest object

	Smooth();
	Smooth();

	PrepareGround(1, GSZ / 2 +10, GSZ, GSZ, 5);
	PrepareGround(1, 1, GSZ, GSZ / 2 - 10,5);
	PrepareRailRoad();

	//init tree positions
	initTrees();

	glEnable(GL_NORMALIZE);// needs for lighting to normalize the vectors
	
	// Stone texture
	ReadBitmap("stoneTile.bmp");
	SetTexture(0);
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, stone);

	// Textures definitions
	SetTexture(1); // road
	glBindTexture(GL_TEXTURE_2D, 1); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TW, TH, 0, GL_RGB, GL_UNSIGNED_BYTE, railRoad);

	// Textures definitions
	ReadBitmap("wood2.bmp");
	SetTexture(2); // wood texture
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TW, TH, 0, GL_RGB, GL_UNSIGNED_BYTE, wood);

	// Textures definitions
	ReadBitmap("leaves.bmp");
	SetTexture(3); // leaves texture
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, leaves);

	ReadBitmap("smoke3.bmp");
	SetTexture(4); // smoke texture
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, smoke);

	// Water texture
	ReadBitmap("water2.bmp");
	SetTexture(6);
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, water);

	// read sunrise from file
	ReadBitmap("sunrise.bmp");
	SetTexture(7);
	free(bmp);
	glBindTexture(GL_TEXTURE_2D, 7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, tx3);
}

void Smooth()
{
	double tmp[GSZ][GSZ] ; 
	int i, j;

	// compute smoothing signal
	for (i = 1; i < GSZ - 1; i++)
		for (j = 1; j < GSZ - 1; j++)
			tmp[i][j] = (0.25*ground[i-1][j-1] + ground[i - 1][j]+ 0.25*ground[i - 1][j + 1]+
							ground[i][j - 1] + 4*ground[i][j] + ground[i][j + 1]+
							0.25 * ground[i + 1][j - 1] + ground[i + 1][j] + 0.25 * ground[i + 1][j + 1]) / 9.0;

	// copy the new signal
	for (i = 1; i < GSZ - 1; i++)
		for (j = 1; j < GSZ - 1; j++)
			ground[i][j] = tmp[i][j];

}

// height is +-4
void SetColor(double h)
{
	if (fabs(h) < 0.2) // sand
		glColor3d(0.8, 0.7, 0.5);
	else
	if (fabs(h) < 11) // green fields
		glColor3d(0.1 + h / 50, 0.6 - fabs(h) / 22.5, 0);
	else // rocks and snow
		glColor3d(sqrt(h)/5, sqrt(h)/5, sqrt(h)/4.0);
}

void SetNormal(int i, int j)
{
	double n[3];

	n[0] = ground[i][j] - ground[i][j + 1];
	n[1] = 1;
	n[2] = ground[i][j] - ground[i + 1][j];
	glNormal3d(n[0], n[1], n[2]);

}

void DrawRoad()
{
	int j,i;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1); // road texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE

	// RailRoad from left to right
	for (j = 0; j < GSZ - 1; j++)
	{
			glBegin(GL_POLYGON);
			glTexCoord2d(0, 0);   glVertex3f(j - GSZ / 2, ground[GSZ / 2][j] + 0.1, -1);// point 1;
			glTexCoord2d(1, 0);   glVertex3f(j - GSZ / 2 + 1, ground[GSZ / 2][j + 1] + 0.1, -1); // point 2
			glTexCoord2d(1, 1);   glVertex3f(j - GSZ / 2 + 1, ground[GSZ / 2][j + 1] + 0.1, 1); // point 3
			glTexCoord2d(0, 1);   glVertex3f(j - GSZ / 2, ground[GSZ / 2][j] + 0.1, 1); // point 4

			glEnd();
		//}
	}
/*
	// Road from far to near
	for (i = 0; i < GSZ - 1; i++)
	{
		//if (i<GSZ / 2 - 2 || i>GSZ / 2+1) // not on crossroads
		//{
			glBegin(GL_POLYGON);
			glTexCoord2d(0, 0);
			if (ground[i][GSZ / 2] > 0) 			glVertex3d(-1, ground[i][GSZ / 2] + 0.1, i - GSZ / 2); // point 1;
			else  glVertex3d(-1, 1, i - GSZ / 2); // bridge
			glTexCoord2d(1, 0);
			if (ground[i + 1][GSZ / 2] > 0)
			{
				glVertex3d(-1, ground[i + 1][GSZ / 2] + 0.1, i - GSZ / 2 + 1); // point 2
				glTexCoord2d(1, 2);   		glVertex3d(1, ground[i + 1][GSZ / 2] + 0.1, i - GSZ / 2 + 1); // point 3
			}
			else
			{
				glVertex3d(-1, 1, i - GSZ / 2 + 1);
				glTexCoord2d(1, 2);   		glVertex3d(1, 1, i - GSZ / 2 + 1); // point 3 on bridge
			}

			glTexCoord2d(0, 2);
			if (ground[i][GSZ / 2] > 0) 	glVertex3d(1, ground[i][GSZ / 2] + 0.1, i - GSZ / 2); // point 4
			else glVertex3d(1, 1, i - GSZ / 2); // point 4 on bridge
			glEnd();
		//}
	}
	*/
	/*
	// Cross roads
	if (ground[GSZ / 2][GSZ / 2] > 0)
	{
		glBegin(GL_POLYGON);
		glTexCoord2d(0.5, 0.1);		glVertex3d(-1, ground[GSZ / 2][GSZ / 2] + 0.1, -1);
		glTexCoord2d(1, 0.1);		glVertex3d(1, ground[GSZ / 2][GSZ / 2] + 0.1, -1);
		glTexCoord2d(1, 0.9);    	glVertex3d(1, ground[GSZ / 2][GSZ / 2] + 0.1, 1);
		glTexCoord2d(0.5, 0.9);    	glVertex3d(-1, ground[GSZ / 2][GSZ / 2] + 0.1, 1);
		glEnd();
	}
	else // this is a bridge
	{
		glBegin(GL_POLYGON);
		glTexCoord2d(0.5, 0.1);		glVertex3d(-1, 1, -1);
		glTexCoord2d(1, 0.1);		glVertex3d(1, 1, -1);
		glTexCoord2d(1, 0.9);    	glVertex3d(1,1, 1);
		glTexCoord2d(0.5, 0.9);    	glVertex3d(-1, 1, 1);
		glEnd();
	}

	// pedestrian walk
	glBindTexture(GL_TEXTURE_2D, 2); // zebra texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE

	// 1: left side of crossroads
	
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);	
	if(ground[GSZ / 2][GSZ / 2 - 2]>0) glVertex3d(-2, ground[GSZ / 2][GSZ / 2-2] + 0.1, -1);
	else glVertex3d(-2, 1, -1);
	glTexCoord2d(1, 0);		
	if (ground[GSZ / 2][GSZ / 2 - 1] > 0)
	{
		glVertex3d(-1, ground[GSZ / 2][GSZ / 2 - 1] + 0.1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(-1, ground[GSZ / 2][GSZ / 2 - 1] + 0.1, 1);
	}
	else // bridge
	{
		glVertex3d(-1, 1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(-1, 1, 1);
	}
	glTexCoord2d(0, 7.5);  
	if (ground[GSZ / 2][GSZ / 2 - 2] > 0) glVertex3d(-2, ground[GSZ / 2][GSZ / 2-2] + 0.1, 1);
	else glVertex3d(-2, 1, 1);
	glEnd();

	//2. right side of crossroads
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	if (ground[GSZ / 2][GSZ / 2 +1 ] > 0) glVertex3d(1, ground[GSZ / 2][GSZ / 2 +1] + 0.1, -1);
	else glVertex3d(1, 1, -1);
	glTexCoord2d(1, 0);
	if (ground[GSZ / 2][GSZ / 2 + 2] > 0)
	{
		glVertex3d(2, ground[GSZ / 2][GSZ / 2 +2] + 0.1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(2, ground[GSZ / 2][GSZ / 2 +2] + 0.1, 1);
	}
	else // bridge
	{
		glVertex3d(2, 1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(2, 1, 1);
	}
	glTexCoord2d(0, 7.5);
	if (ground[GSZ / 2][GSZ / 2 +1] > 0) glVertex3d(1, ground[GSZ / 2][GSZ / 2 +1] + 0.1, 1);
	else glVertex3d(1, 1, 1);
	glEnd();


	// 3: far side of crossroads

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	if (ground[GSZ / 2-2][GSZ / 2+1 ] > 0) glVertex3d(1, ground[GSZ / 2-2][GSZ / 2+1 ] + 0.1, -2);
	else glVertex3d(1, 1, -2);
	glTexCoord2d(1, 0);
	if (ground[GSZ / 2-1][GSZ / 2 + 1] > 0)
	{
		glVertex3d(1, ground[GSZ / 2-1][GSZ / 2 + 1] + 0.1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(-1, ground[GSZ / 2-1][GSZ / 2 - 1] + 0.1, -1);
	}
	else // bridge
	{
		glVertex3d(1, 1, -1);
		glTexCoord2d(1, 7.5);    	glVertex3d(-1, 1, -1);
	}
	glTexCoord2d(0, 7.5);
	if (ground[GSZ / 2-2][GSZ / 2 - 1] > 0) glVertex3d(-1, ground[GSZ / 2-2][GSZ / 2 - 1] + 0.1, -2);
	else glVertex3d(-1, 1, -2);
	glEnd();


	// 4: near side of crossroads

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	if (ground[GSZ / 2+ 2][GSZ / 2 - 1] > 0) glVertex3d(-1, ground[GSZ / 2 + 2][GSZ / 2 - 1] + 0.1, 2);
	else glVertex3d(-1, 1, 2);
	glTexCoord2d(1, 0);
	if (ground[GSZ / 2 + 1][GSZ / 2 - 1] > 0)
	{
		glVertex3d(-1, ground[GSZ / 2 + 1][GSZ / 2 - 1] + 0.1, 1);
		glTexCoord2d(1, 7.5);    	glVertex3d(1, ground[GSZ / 2 + 1][GSZ / 2 + 1] + 0.1, 1);
	}
	else // bridge
	{
		glVertex3d(-1, 1, 1);
		glTexCoord2d(1, 7.5);    	glVertex3d(1, 1, 1);
	}
	glTexCoord2d(0, 7.5);
	if (ground[GSZ / 2 + 2][GSZ / 2 + 1] > 0) glVertex3d(1, ground[GSZ / 2 + 2][GSZ / 2 + 1] + 0.1, 2);
	else glVertex3d(1, 1, 2);
	glEnd();
	*/

	glDisable(GL_TEXTURE_2D);
}


// draws the ground matrix
void DrawFloor()
{
	int i, j;

	glColor3d(0, 0, 0.3);
	// i goes on z, j goes on x, y is taken from ground[i][j]
	for (i = 0; i < GSZ - 2; i++)
		for (j = 0; j < GSZ - 2; j++)
		{
	/*		if (fabs(ground[i][j]) >= 6) // snow
			{
				// set material: silver
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mamb3);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mdiff3);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mspec3);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);

			}
			else // sand
			{
				// gold
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mamb2);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mdiff2);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mspec2);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80);

			}
*/			glBegin(GL_POLYGON);// GL_LINE_LOOP);
		SetColor(ground[i][j]);
//			SetNormal(i, j);
			glVertex3d(j - GSZ / 2, ground[i][j], i - GSZ / 2);
			SetColor(ground[i][j+1]);
//			SetNormal(i, j+1);
			glVertex3d(j + 1 - GSZ / 2, ground[i][j + 1], i - GSZ / 2);
			SetColor(ground[i+1][j+1]);
//			SetNormal(i+1, j+1);
			glVertex3d(j + 1 - GSZ / 2, ground[i+1][j + 1], i +1- GSZ / 2);
			SetColor(ground[i+1][j]);
//			SetNormal(i+1, j);
			glVertex3d(j  - GSZ / 2, ground[i + 1][j ], i + 1 - GSZ / 2);
			glEnd();
		}

	DrawRoad();

// add water surface
//	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4d(0, 0.3, 0.7,0.6);
	glBegin(GL_POLYGON);
	glVertex3d(-GSZ / 2, 0, -GSZ / 2);
	glVertex3d(-GSZ / 2, 0, GSZ / 2);
	glVertex3d(GSZ / 2, 0, GSZ / 2);
	glVertex3d(GSZ / 2, 0, -GSZ / 2);
	glEnd();
	glDisable(GL_BLEND);
//	glEnable(GL_LIGHTING);

}



void DrawCylinder(int n)
{
	double alpha, teta = 2 * PI / n;

	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side

		glBegin( GL_POLYGON);
		glColor3d(fabs(sin(alpha)),(1 + cos(alpha)) / 2 ,1 - fabs(sin(alpha+PI/2)));
		glVertex3d(sin(alpha), 1, cos(alpha)); // vertex 1
		glVertex3d(sin(alpha + teta), 1, cos(alpha + teta)); // vertex 2
		glColor3d(fabs(sin(alpha))/2, (1 + cos(alpha)) / 4, 1 - fabs(sin(alpha+PI/2)));
		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta )); // vertex 3
		glVertex3d(sin(alpha), 0, cos(alpha)); // vertex 4
		glEnd();
	}
}

// tnum is texture number
// num_repeat is a number of texture repeats
void DrawTexCylinder(int n, int tnum, int num_repeat)
{
	double alpha, teta = 2 * PI / n;
	double part = num_repeat / (double)n;
	int counter;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tnum); // wall with window texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_REPLACE


	for (alpha = 0,counter = 0; alpha <= 2 * PI; alpha += teta,counter++)
	{
		// defines one side

		glBegin(GL_POLYGON);
		glColor3d(0.3+0.7*fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)));
		glTexCoord2d(counter*part, 1);    glVertex3d(sin(alpha), 1, cos(alpha)); // vertex 1
		glTexCoord2d((counter+1) * part, 1);    		glVertex3d(sin(alpha + teta), 1, cos(alpha + teta)); // vertex 2
		glTexCoord2d((counter + 1) * part, 0);    		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // vertex 3
		glTexCoord2d(counter * part, 0);    		glVertex3d(sin(alpha), 0, cos(alpha)); // vertex 4
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

}


// tnum is texture number
// num_repeat is a number of texture repeats
void DrawTexCylinder1(int n, int tnum, int num_repeat, double tr, double br)
{
	double alpha, teta = 2 * PI / n;
	double part = num_repeat / (double)n;
	int counter;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tnum); // wall with window texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_REPLACE


	for (alpha = 0, counter = 0; alpha <= 2 * PI; alpha += teta, counter++)
	{
		// defines one side

		glBegin(GL_POLYGON);
		glColor3d(0.3 + 0.7 * fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)));
		glTexCoord2d(counter * part, 1);    glVertex3d(tr*sin(alpha), 1, tr*cos(alpha)); // vertex 1
		glTexCoord2d((counter + 1) * part, 1);    		glVertex3d(tr*sin(alpha + teta), 1, tr*cos(alpha + teta)); // vertex 2
		glTexCoord2d((counter + 1) * part, 0);    		glVertex3d(br*sin(alpha + teta), 0, br*cos(alpha + teta)); // vertex 3
		glTexCoord2d(counter * part, 0);    		glVertex3d(br*sin(alpha), 0, br*cos(alpha)); // vertex 4
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

}

// DrawTexCylinder2 attaches vertical and horizontal part of texture
// on cylinder
// tnum is texture number
// num_repeat is a number of texture repeats
// tpart and bpart are vertical texture coordinates
void DrawTexCylinder2(int n, int tnum, int num_repeat, double tr, double br,double tpart, double bpart)
{
	double alpha, teta = 2 * PI / n;
	double part = num_repeat / (double)n;
	int counter;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tnum); // wall with window texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 


	for (alpha = 0, counter = 0; alpha <= 2 * PI; alpha += teta, counter++)
	{
		// defines one side

		glBegin(GL_POLYGON);
//		glColor3d(0.3 + 0.7 * fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)), 0.3 + 0.7 * fabs(sin(alpha)));
		glTexCoord2d(counter * part, tpart);    glVertex3d(tr * sin(alpha), 1, tr * cos(alpha)); // vertex 1
		glTexCoord2d((counter + 1) * part, tpart);    		glVertex3d(tr * sin(alpha + teta), 1, tr * cos(alpha + teta)); // vertex 2
		glTexCoord2d((counter + 1) * part, bpart);    		glVertex3d(br * sin(alpha + teta), 0, br * cos(alpha + teta)); // vertex 3
		glTexCoord2d(counter * part, bpart);    		glVertex3d(br * sin(alpha), 0, br * cos(alpha)); // vertex 4
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

}


// topr is top radius, bottomr is bottom radius
void DrawCylinder1(int n, double topr, double bottomr)
{
	double alpha, teta = 2 * PI / n;

	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side
		glBegin(GL_POLYGON);

		glVertex3d(topr*sin(alpha), 1, topr * cos(alpha)); // vertex 1
		glVertex3d(topr * sin(alpha + teta), 1, topr * cos(alpha + teta)); // vertex 2
		glVertex3d(bottomr*sin(alpha + teta), 0, bottomr * cos(alpha + teta)); // vertex 3
		glVertex3d(bottomr * sin(alpha), 0, bottomr * cos(alpha)); // vertex 4
		glEnd();
	}
}

// topr is top radius, bottomr is bottom radius
// defines vector of normal
void DrawLitCylinder1(int n, double topr, double bottomr)
{
	double alpha, teta = 2 * PI / n,h,len;

	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		h = (bottomr - topr) * bottomr;
		
		// defines one side
		glBegin(GL_POLYGON);
			glNormal3d(topr*sin(alpha), h, topr*cos(alpha));
		glVertex3d(topr * sin(alpha), 1, topr * cos(alpha)); // vertex 1
			glNormal3d(topr * sin(alpha+teta), h, topr * cos(alpha+teta));
		glVertex3d(topr * sin(alpha + teta), 1, topr * cos(alpha + teta)); // vertex 2
			glNormal3d(bottomr * sin(alpha + teta), h, bottomr * cos(alpha + teta));
		glVertex3d(bottomr * sin(alpha + teta), 0, bottomr * cos(alpha + teta)); // vertex 3
			glNormal3d(bottomr * sin(alpha), h, bottomr * cos(alpha));
		glVertex3d(bottomr * sin(alpha), 0, bottomr * cos(alpha)); // vertex 4
		glEnd();
	}
}

void DrawConus(int n)
{
	double alpha, teta = 2 * PI / n;

	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side

		glBegin(GL_POLYGON);
		glColor3d(1,1,1);
		glVertex3d(0, 1, 0); // vertex 1
		glColor3d(fabs(sin(alpha)) / 2, (1 + cos(alpha)) / 4, 1 - fabs(sin(alpha + PI / 2)));
		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // vertex 2
		glVertex3d(sin(alpha), 0, cos(alpha)); // vertex 3
		glEnd();
	}
}

// n is the number of sectors, sl is the number of slices
void DrawSphere(int n, int sl)
{
	double beta, gamma = PI / sl;


	for (beta = -PI / 2; beta <= PI / 2; beta+=gamma)
	{
		glPushMatrix();
		glTranslated(0, sin(beta), 0);
		glScaled(1, sin(beta+gamma)-sin(beta), 1);
		DrawCylinder1(n, cos(beta+gamma),cos(beta) );
		glPopMatrix();
	}
}

// n is the number of sectors, sl is the number of slices
// num_rep is amount of horizontal repeats of a texture
// vert_rep is amount of vertical repeats of a texture
void DrawTexSphere(int n, int sl,int tnum,int num_rep,int vert_rep)
{
	double beta, gamma = PI / sl;
	int counter;
	double part= vert_rep/(double)sl;

	for (beta = -PI / 2,counter=0; beta <= PI / 2; beta += gamma,counter++)
	{
		glPushMatrix();
		glTranslated(0, sin(beta), 0);
		glScaled(1, sin(beta + gamma) - sin(beta), 1);
		DrawTexCylinder2(n, tnum,num_rep,cos(beta + gamma), cos(beta),(counter+1)*part,counter*part);
		glPopMatrix();
	}
}

// n is the number of sectors, sl is the number of slices
void DrawLitSphere(int n, int sl)
{
	double beta, gamma = PI / sl;


	for (beta = -PI / 2; beta <= PI / 2; beta += gamma)
	{
		glPushMatrix();
		glTranslated(0, sin(beta), 0);
		glScaled(1, sin(beta + gamma) - sin(beta), 1);
		DrawLitCylinder1(n, cos(beta + gamma), cos(beta));
		glPopMatrix();
	}
}

void DrawBridgeArc()
{
	double alpha, teta = 2 * PI / 180;

for (int i = 5; i >= -5; i -= 5)
	{
		for (double alpha = - 2 * PI / 18; alpha <= PI; alpha += 2 * PI / 18)
		{
			glBegin(GL_POLYGON);
			glColor3d(0.4375, 0.5, 0.5625);
			glTexCoord2d(0, 1); glVertex3d(i+2 * cos(alpha),2+3 * sin(alpha),  -5);
			glTexCoord2d(1, 1);  glVertex3d(i+2 * cos(alpha + 2 * PI / 18),2+3 * sin(alpha + 2 * PI / 18),  -5);
			glTexCoord2d(1, 2);    glVertex3d(i+2 * cos(alpha + 2 * PI / 18),2+3 * sin(alpha + 2 * PI / 18),  5);
			glTexCoord2d(0, 2);   glVertex3d(i+2 * cos(alpha),2+3 * sin(alpha),  5);
			glEnd();
		}
	}
}

void DrawBridgeBody()
{
	double alpha, teta = 2 * PI / 180;

	for (int i = 5; i >= -10; i -= 5)
	{
		for (double alpha = -2 * PI / 18; alpha <= PI; alpha += 2 * PI / 18)
		{
			glBegin(GL_POLYGON);
			glColor3d(0.4375, 0.5, 0.5625);
			glTexCoord2d(0, 1); glVertex3d(i , 2 + 3 * sin(alpha), -5);
			glTexCoord2d(1, 1);  glVertex3d(i + 5 , 2 + 3 * sin(alpha + 2 * PI / 18), -5);
			glTexCoord2d(1, 2);    glVertex3d(i+5 , 2 + 3 * sin(alpha + 2 * PI / 18), 5);
			glTexCoord2d(0, 2);   glVertex3d(i , 2 + 3 * sin(alpha), 5);
			glEnd();
		}
	}
}

void DrawBridgeArc1()
{
	double alpha, teta = 2 * PI / 180;


		for (double alpha = -2 * PI / 18; alpha <= PI; alpha += 2 * PI / 18)
		{
			glBegin(GL_POLYGON);
			glColor3d(0.4375, 0.5, 0.5625);
			glTexCoord2d(0, 1); glVertex3d(-10, 5 + 5 * sin(alpha), 4 * cos(alpha));
			glTexCoord2d(1, 1);  glVertex3d(-10, 5 + 5 * sin(alpha + 2 * PI / 18), 4 * cos(alpha + 2 * PI / 18));
			glTexCoord2d(1, 2);    glVertex3d(10, 5 + 5 * sin(alpha + 2 * PI / 18), 4 * cos(alpha + 2 * PI / 18));
			glTexCoord2d(0, 2);   glVertex3d(10, 5 + 5 * sin(alpha), 4 * cos(alpha));
			glEnd();
		}

}

void DrawBridgeBody1()
{
	double alpha, teta = 2 * PI / 180;

	for (int i = 5; i >= -10; i -= 5)
	{
		for (double alpha = -2 * PI / 90; alpha <= PI; alpha += 2 * PI / 90)
		{
			glBegin(GL_POLYGON);
			glColor3d(0.4375, 0.5, 0.5625);
			glTexCoord2d(0, 1); glVertex3d(i, 2 + 3 * sin(alpha), -5);
			glTexCoord2d(1, 1);  glVertex3d(i + 5, 2 + 3 * sin(alpha + 2 * PI / 18), -5);
			glTexCoord2d(1, 2);    glVertex3d(i + 5, 2 + 3 * sin(alpha + 2 * PI / 18), 5);
			glTexCoord2d(0, 2);   glVertex3d(i, 2 + 3 * sin(alpha), 5);
			glEnd();
		}
	}
}


void DrawBridge()
{

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_REPLACE


	DrawBridgeArc1();
	DrawBridgeBody1();

	glDisable(GL_TEXTURE_2D);
}

void DrawTopOFTree(double x, double y, double h)
{

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 3); // leaves texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_REPLACE

	float greenColor = (rand() % 20) * 0.01 + 0.8;
	float redColor = (rand() % 10) * 0.01 +0.2;
	glColor3d(redColor, greenColor, 0);

	glPushMatrix();

	glBegin(GL_POLYGON);	
	glVertex3d(x, 8+h, y);
	glVertex3d(x+2, h+6 , y);
	glVertex3d(x , h+4 , y);
	glVertex3d(x-2, h+6, y);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}
void DrawBranch(double x, double y, double height)
{
	double alpha, teta = 2 * PI / 180;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 2); // wood texture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_REPLACE


	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side
		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 4 +x, 5 + height, cos(alpha) / 4 +y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 4 +x, 5 + height, cos(alpha + teta) / 4 +y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 4 +x, 0 + height, cos(alpha + teta) / 4 +y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 4 +x, 0 + height, cos(alpha) / 4 +y); // vertex 4
		glEnd();
		/*
		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 11 + x + height*0.7, height * 0.55 + height, cos(alpha) / 11 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 11 + x + height * 0.7, height * 0.55 + height, cos(alpha + teta) / 11 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 11 + x, 0.4* height + height, cos(alpha + teta) / 11 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 11 + x, 0.4 * height+ height, cos(alpha) / 11 + y); // vertex 4
		glEnd();
		DrawLeaf(sin(alpha) / 11 + x, 0.4 * height + height);

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 16 + x - height * 0.7, height * 0.55 + height, cos(alpha) / 16 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 16 + x - height * 0.7, height * 0.55 + height, cos(alpha + teta) / 16 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 16 + x, 0.4 * height + height, cos(alpha + teta) / 16 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 16 + x, 0.4 * height + height, cos(alpha) / 16 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 17 + x + height * 0.6, height * 0.8 + height, cos(alpha) / 17 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 17 + x + height * 0.6, height * 0.8 + height, cos(alpha + teta) / 17 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 17 + x, 0.6 * height + height, cos(alpha + teta) / 17 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 17 + x, 0.6 * height + height, cos(alpha) / 17 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 18 + x - height * 0.6, height * 0.8 + height, cos(alpha) / 18 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 18 + x - height * 0.6, height * 0.8 + height, cos(alpha + teta) / 18 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 18 + x, 0.6 * height + height, cos(alpha + teta) / 18 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 18 + x, 0.6 * height + height, cos(alpha) / 18 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 17 + x + height * 0.4, 2* height, cos(alpha) / 17 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 17 + x + height * 0.4, 2* height, cos(alpha + teta) / 17 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 17 + x, 0.8 * height + height, cos(alpha + teta) / 17 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 17 + x, 0.8 * height + height, cos(alpha) / 17 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 14 + x - height * 0.4, 2* height, cos(alpha) / 14 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 14 + x - height * 0.4, 2* height, cos(alpha + teta) / 14 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 14 + x, 0.8 * height + height, cos(alpha + teta) / 14 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 14 + x, 0.8 * height + height, cos(alpha) / 14 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 17 + x + height * 0.2, 2* height, cos(alpha) / 17 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 17 + x + height * 0.2, 2* height, cos(alpha + teta) / 17 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 17 + x, 0.8 * height + height, cos(alpha + teta) / 17 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 17 + x, 0.8 * height + height, cos(alpha) / 17 + y); // vertex 4
		glEnd();

		glBegin(GL_POLYGON);
		glColor3d(0.384, 0.305, 0.172); //'Tree Bark Brown'
		glTexCoord2d(0, 1);    glVertex3d(sin(alpha) / 17 + x - height * 0.2, 2.15 * height, cos(alpha) / 17 + y); // vertex 1
		glTexCoord2d(1, 1);    		glVertex3d(sin(alpha + teta) / 17 + x - height * 0.2, 2.15 * height, cos(alpha + teta) / 17 + y); // vertex 2
		glTexCoord2d(1, 2);    		glVertex3d(sin(alpha + teta) / 17 + x, 0.9 * height + height, cos(alpha + teta) / 17 + y); // vertex 3
		glTexCoord2d(0, 2);    		glVertex3d(sin(alpha) / 17 + x, 0.9 * height + height, cos(alpha) / 17 + y); // vertex 4
		glEnd();
		*/
	}

	glDisable(GL_TEXTURE_2D);
}


void DrawTrees()
{
	int i;
	for (i = 0; i < TREE_NUM; i++)
	{
		DrawBranch(trees[i].x, trees[i].y,trees[i].h);
		DrawTopOFTree(trees[i].x, trees[i].y, trees[i].h);
	}
}

void DrawCylinder3(int n, double r, double g, double b)
{
	double alpha, teta = 2 * PI / n;

	glColor3d(r, g, b);
	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side

		glBegin(GL_POLYGON);
		glVertex3d(sin(alpha), 0.2, cos(alpha)); // vertex 1
		glVertex3d(sin(alpha + teta), 0.2, cos(alpha + teta)); // vertex 2
		glVertex3d(sin(alpha + teta), 0, cos(alpha + teta)); // vertex 3
		glVertex3d(sin(alpha), 0, cos(alpha)); // vertex 4
		glEnd();
	}
}

void DrawTrainWheel(int n,double outer, double inner,double r, double g, double b ,int jump)
{
	double alpha, teta = 2*PI/n;
	double x, z;
	glColor3d(r, g, b);
	for (alpha = 0; alpha < 2 * PI; alpha += jump * teta)
	{
		glBegin(GL_POLYGON);
		x = outer * sin(alpha);
		z = outer * cos(alpha);
		glVertex3d(x, 0, z); // 1
		x = inner * sin(alpha);
		z = inner * cos(alpha);
		glVertex3d(x, 0, z); // 2
		x = inner * sin(alpha+teta);
		z = inner * cos(alpha + teta);
		glVertex3d(x, 0, z); // 3
		x = outer * sin(alpha + teta);
		z = outer * cos(alpha + teta);
		glVertex3d(x, 0, z); // 4
		glEnd();
	}
}

void DrawWheel()
{
	glPushMatrix();
	glRotated(-5*fabs(offset), 0, 1, 0);
	DrawCylinder3(50,0.5,0.5,0.5);
	glPushMatrix();
	glScaled(0.99, 1, 0.99);
	DrawCylinder3(50, 0, 0, 0);
	glPopMatrix();

	DrawTrainWheel(50,0.25, 0, 0.6875,0, 0.15625,1);
	DrawTrainWheel(30, 0.99, 0.25, 0.6875, 0, 0.15625,3);
	glPushMatrix();
	glTranslated(0, 0.1, 0);
	DrawTrainWheel(50, 1, 0, 0, 0, 0, 1);
	glTranslated(0, 0.1, 0);
	DrawTrainWheel(50, 0.25, 0, 0.6875, 0, 0.15625, 1);
	DrawTrainWheel(30, 0.99, 0.25, 0.6875, 0, 0.15625, 3);
	
	glPopMatrix();
	glPopMatrix();
}

void DrawTrainWheels(double x, double y)
{
	glPushMatrix();
	glTranslated(x+5, y, 0.3);
	glRotated(-90, 1, 0, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x-1, y, 0.4);
	glRotated(-90, 1, 0, 0);
	glScaled(0.6, 0.6, 0.6);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+3, y, 0.4);
	glRotated(-90, 1, 0, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+1, y, 0.3);
	glRotated(-90, 1, 0, 0);
	glScaled(0.6, 0.6, 0.6);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+5, y, -0.7);
	glRotated(-90, 1, 0, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x-1, y, -0.7);
	glRotated(-90, 1, 0, 0);
	glScaled(0.6, 0.6, 0.6);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+3, y, -0.7);
	glRotated(-90, 1, 0, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+1, y, -0.7);
	glRotated(-90, 1, 0, 0);
	glScaled(0.6, 0.6, 0.6);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();
}

void DrawTrainBody()
{

	int delta = rand() % 5; //rnadom value between 0 and 5

	//Pipe
	glPushMatrix();
	glTranslated(train.x-1.5, train.y+2, 0);
	glScaled(0.2, 6, 0.2);
	DrawCylinder3(50, 4 * 0.01 * delta, 3 * 0.01 * delta, 2 * 0.01 * delta);
	glPopMatrix();

	//Train Top accesory x1
	glPushMatrix();
	glTranslated(train.x-0.5, train.y+2, 0);
	glScaled(0.25, 0.7, 0.25);
	DrawLitCylinder1(30, 2, 2);
	glPopMatrix();

	//Train Top accesory x2
	glPushMatrix();
	glTranslated(train.x +1, train.y+2, 0);
	glScaled(0.25, 0.7, 0.25);
	DrawLitCylinder1(30, 2, 2);
	glPopMatrix();

	//Train Body
	glPushMatrix();
	glTranslated(train.x+ 2, train.y+ 1.5, 0);
	glRotated(90, 0, 0, 1);
	glScaled(0.4, 4, 0.4);
	DrawLitCylinder1(30, 2,2);
	glPopMatrix();

	// Conductor cell
	glPushMatrix();
	glTranslated(train.x + 3.5, train.y + 1.5, 0);
	glRotated(45, 0, 1, 0);
	glScaled(1, 2, 1);
	DrawLitCylinder1(4, 2, 2);
	glPopMatrix();

	//conductor roof
	glBegin(GL_POLYGON);
	glVertex3d(train.x +2, train.y +3.5, -1.5); // vertex 1, 
	glVertex3d(train.x +5, train.y +3.5, -1.5); // vertex 2
	glVertex3d(train.x +5, train.y +3.5, 1.5); // vertex 3
	glVertex3d(train.x +2, train.y +3.5, 1.5); // vertex 4
	glEnd();

	//Train "head" aka the red arrow up front
	double alpha, teta = 2 * PI / 30;

	for (alpha = 0; alpha <= 2 * PI; alpha += teta)
	{
		// defines one side
		glBegin(GL_POLYGON);
		glColor3d(0.6875 + 0.03*delta, 0, 0.15625 + 0.03*delta);
		glVertex3d(train.x-3, train.y + 1.5, 0); // vertex 1, tip on the train
		glVertex3d(train.x -2, train.y + 1.5 + sin(alpha + teta)*0.6, cos(alpha + teta)); // vertex 2
		glVertex3d(train.x -2, train.y + 1.5+ sin(alpha)*0.6, cos(alpha)); // vertex 3
		glEnd();

	}


}

void DrawMirrors(double x, double y) 
{
	int i;
	int delta = rand() % 20; //rnadom value between 0 and 5

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glColor4d(0.6, 0.3, 0.0, 0.5);
	glColor4d(0.5078125 +0.01 * delta, 0.8046875 +0.01 * delta, 0.9765625 +0.01 * delta,0.5);

	for (i = 0; i < 8; i++) {
		// right side
		glBegin(GL_POLYGON);
		glVertex3d(x + 0.25+i, y + 2.5, 1 + 0.01);
		glVertex3d(x + 0.75+i, y + 2.5, 1 + 0.01);
		glVertex3d(x + 0.75+i, y + 1.5, 1 + 0.01);
		glVertex3d(x + 0.25+i, y + 1.5, 1 + 0.01);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex3d(x + 0.25 + i, y + 2.5, -1 - 0.01);
		glVertex3d(x + 0.75 + i, y + 2.5, -1 - 0.01);
		glVertex3d(x + 0.75 + i, y + 1.5, -1 - 0.01);
		glVertex3d(x + 0.25 + i, y + 1.5, -1 - 0.01);
		glEnd();
	}

	glDisable(GL_BLEND);
}

void DrawCart(double x, double y, float r, float g, float b)
{
	int delta = rand() % 5; //rnadom value between 0 and 5

	//glColor3f(0.96875 + 0.01*delta, 0.06640625+ 0.01 * delta, 0.06640625 + 0.01 * delta);
	glColor3f(r + 0.01 * delta, g + 0.01 * delta, b + 0.01 * delta);

	// top side
	glBegin(GL_POLYGON);
	glVertex3d(x, y+3, -1);
	glVertex3d(x+8, y+3, -1);
	glVertex3d(x + 8, y+3, 1);
	glVertex3d(x, y+3, 1);
	glEnd();

	// bottom side
	glBegin(GL_POLYGON);
	glVertex3d(x, y+0.5, -1);
	glVertex3d(x+8, y + 0.5, -1);
	glVertex3d(x + 8, y + 0.5, 1);
	glVertex3d(x, y + 0.5, 1);
	glEnd();

	// front side -  aka from user view
	glBegin(GL_POLYGON);
	glVertex3d(x, y+3, 1);
	glVertex3d(x+8, y+3, 1);
	glVertex3d(x+8, y + 0.5, 1);
	glVertex3d(x, y + 0.5, 1);
	glEnd();

	// rear side
	glBegin(GL_POLYGON);
	glVertex3d(x, y + 3, -1);
	glVertex3d(x + 8, y + 3, -1);
	glVertex3d(x + 8, y + 0.5, -1);
	glVertex3d(x, y + 0.5, -1);
	glEnd();

	// left side
	glBegin(GL_POLYGON);
	glVertex3d(x, y + 0.5, 1);
	glVertex3d(x, y+0.5, -1);
	glVertex3d(x, y+3, -1);
	glVertex3d(x, y+3, 1);
	glEnd();

	// right side
	glBegin(GL_POLYGON);
	glVertex3d(x + 8, y + 0.5, 1);
	glVertex3d(x + 8, y + 0.5, -1);
	glVertex3d(x + 8, y + 3, -1);
	glVertex3d(x + 8, y + 3, 1);
	glEnd();

	//DrawTrainWheels();

	//TBD: REPLACE

	glPushMatrix();
	glTranslated(x+0.5, y, 0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+1.5, y, 0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+0.5, y, -0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+1.5, y, -0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+7.5, y, 0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+6.5, y, 0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x+7.5, y, -0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	glPushMatrix();
	glTranslated(x + 6.5, y, -0.5);
	glRotated(-90, 1, 0, 0);
	glScaled(0.4, 0.4, 0.4);
	glTranslated(0, -0.5, 0);
	DrawWheel();
	glPopMatrix();

	DrawMirrors(x,y);
}

void DrawSmoke(double x, double y)
{
	int i;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	double delta = -10 + rand() % 20; // random value in range [-10,10)


	glColor4d(0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.5);
	glPushMatrix();
	glTranslated(x , y +1.3, 0);
	glScaled(0.1, 0.1, 0.1);
	DrawSphere(25, 6);
	glPopMatrix();

	glColor4d(0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.5 + 0.03 * delta);
	glPushMatrix();
	glTranslated(x+0.4, y + 1.6, 0);
	glScaled(0.2, 0.2, 0.2);
	DrawSphere(25, 6);
	glPopMatrix();

	glColor4d(0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.9 + 0.01 * delta, 0.5 + 0.06 * delta);
	glPushMatrix();
	glTranslated(x + 1, y + 2.1, 0);
	glScaled(0.3, 0.3, 0.3);
	DrawSphere(25, 6);
	glPopMatrix();

	glDisable(GL_BLEND);
}


void DrawTrain()
{
	DrawTrainWheels(train.x, train.y+0.5);
	DrawTrainBody();
	DrawSmoke(train.x-1.5, train.y+2);

	DrawCart(train.x + 6, train.y, 0.375, 0.375, 0.375);
	//DrawCart(6, 10, 0.2109375, 0.609375, 0.44921875);
	DrawCart(train.x + 14.5, train.y, 0.5, 0.5, 0.5);

	//change for speed
	train.x -= 0.05;
	if (train.x <= -70)
		train.x = 60;

}

void DrawView()
{
	DrawFloor();

	glPushMatrix();
	glTranslated(0, -30, 0);
	glRotated(offset / 10, 0, 1, 0);
	glScaled(180, 180, 180);
	DrawTexSphere(140, 40, 7, 1, 1);
	glPopMatrix();
	glPushMatrix();
	//Move the river along z
	glTranslatef(1, 1, offset / 100);
	DisplayRiver();
	glPopMatrix();

	DrawBridge();
	DrawTrees();
	DrawTrain();
}

void display()
{
	double x, y,beta;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // clean frame buffer and depth buffer
	glViewport(0, 0, W, H);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // loads the Identity matrix to the Projection Matrix
	// define the camera model
	glFrustum(-1, 1, -1, 1, 1, 300);
	// define the viewing parameters
	gluLookAt(eyex, eyey, eyez, // eye coordinates
		eyex+dir[0],eyey+dir[1], eyez+dir[2], // point of interest coordinates
		0, 1, 0); // vector UP reflects roll

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix

	DrawFloor();

	// textured sphere - SUNSET
	glPushMatrix();
	glTranslated(0, -30, 0);
	glRotated(offset/10, 0, 1, 0);
	glScaled(180, 180, 180);
	DrawTexSphere(140, 40, 7, 1,1);
	glPopMatrix();

	glPushMatrix();
	//Move the river along z
	glTranslatef(1,1,offset/100);
	DisplayRiver();
	glPopMatrix();

	DrawBridge();
	DrawTrees();
	DrawTrain();

	glutSwapBuffers(); // show all
}

void displayFromWindow()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean frame buffer and depth buffer
	glViewport(0, 0, W, H);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity(); // loads the Identity matrix to the Projection Matrix
	// define the camera model
	glFrustum(-1, 1, -1, 1, 1, 300);
	// define the viewing parameters
	gluLookAt(train.x + 10, 6.6, -1, // eye coordinates
		train.x + 10, 6.6, -1.5, // point of interest coordinates
		0, 1, 0); // vector UP reflects roll

	//Other Point of view just in case 
	//gluLookAt(train.x + 3, 8, 0, // eye coordinates
	//train.x - 1, 8, 0, // point of interest coordinates
		//0, 1, 0); // vector UP reflects roll

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix

	DrawView();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4d(0.5078125, 0.8046875, 0.9765625, 0.5);
	glBegin(GL_QUADS);
	glVertex3i(-1, -1, -1);
	glVertex3i(1, -1, -1);
	glVertex3i(1, 1, -1);
	glVertex3i(-1, 1, -1);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);
	// to use model transformations
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix
	glColor3d(0.5078125, 0.8046875, 0.9765625);
	glBegin(GL_POLYGON);
	glVertex2d(-1, -1);
	glVertex2d(-1, 1);
	glVertex2d(1, 1);
	glVertex2d(1, -1);
	glEnd();
	glEnable(GL_DEPTH_TEST); //before 3D
	glutSwapBuffers(); // show all
}

void engineView()
{

	double x, y, beta;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean frame buffer and depth buffer
	glViewport(0, 0, W, H);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // loads the Identity matrix to the Projection Matrix
	// define the camera model
	glFrustum(-1, 1, -1, 1, 1, 300);
	// define the viewing parameters
	gluLookAt(train.x - 5, 7, 0, // eye coordinates
		train.x - 6, 7, 0, // point of interest coordinates
		0, 1, 0); // vector UP reflects roll

	//Other Point of view just in case 
	//gluLookAt(train.x + 3, 8, 0, // eye coordinates
	//train.x - 1, 8, 0, // point of interest coordinates
		//0, 1, 0); // vector UP reflects roll

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix

	DrawView();

	// to use model transformations
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix
	glColor3d(0.8, 0.8, 0);
	glBegin(GL_POLYGON);
	glVertex2d(-1, -1);
	glVertex2d(-1, 1);
	glVertex2d(1, 1);
	glVertex2d(1, -1);
	glEnd();
	glEnable(GL_DEPTH_TEST); //before 3D
	glutSwapBuffers(); // show all
}

void sideDisplay()
{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean frame buffer and depth buffer
		glViewport(0, 0, W, H);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); // loads the Identity matrix to the Projection Matrix
		// define the camera model
		glFrustum(-1, 1, -1, 1, 1, 300);
		// define the viewing parameters
		gluLookAt(train.x, 25, 40, // eye coordinates
			train.x, 7, -10, // point of interest coordinates
			0, 1, 0); // vector UP reflects roll

		//Other Point of view just in case 
		//gluLookAt(train.x + 3, 8, 0, // eye coordinates
		//train.x - 1, 8, 0, // point of interest coordinates
			//0, 1, 0); // vector UP reflects roll

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix

		DrawView();

		// to use model transformations
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity(); // loads the Identity matrix to the TRASFORMATION Matrix
		glColor3d(0.8, 0.8, 0);
		glBegin(GL_POLYGON);
		glVertex2d(-1, -1);
		glVertex2d(-1, 1);
		glVertex2d(1, 1);
		glVertex2d(1, -1);
		glEnd();
		glEnable(GL_DEPTH_TEST); //before 3D
		glutSwapBuffers(); // show all
}

void idle()
{
	int i, j;
	double dist;
	if (offset < 500) {
		offset += 0.15;
	}
	else {
		offset = 0;
	}

	
	// set locomotion direction 
	yaw += angularSpeed;
	// setup the sight direction
	dir[0] = sin(yaw);
	dir[1] = sin(pitch);
	dir[2] = cos(yaw);
	// setup the motion
	eyex += speed * dir[0];
	eyey += speed * dir[1];
	eyez += speed * dir[2];

	// set airplane direction
	airyaw += airangularSpeed;
	// setup the sight direction
	airdir[0] = sin(airyaw);
	airdir[1] = sin(airpitch);
	airdir[2] = cos(airyaw);
	// update aircraft location
	airLocation[0] += airspeed * airdir[0];
	airLocation[1] += airspeed * airdir[1];
	airLocation[2] += airspeed * airdir[2];


	glutPostRedisplay(); // posts message (with request to show the frame ) to main window
}


void special_key(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		angularSpeed += 0.001;
		break;
	case GLUT_KEY_RIGHT:
		angularSpeed -= 0.001;
		break;
	case GLUT_KEY_UP: 
		speed += 0.01;
		break;
	case GLUT_KEY_DOWN:
		speed -= 0.01;
		break;
	case GLUT_KEY_PAGE_UP:
		pitch+= 0.1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		pitch -= 0.1;
		break;


	}
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		airangularSpeed += 0.001;
		break;
	case 'w':
		airspeed += 0.01;
		break;
	case 's':
		airspeed -= 0.01;
		break;
	case 'd':
		airangularSpeed -= 0.001;
		break;
	}
}

void menu(int choice)
{
	switch (choice)
	{
	case 1:
		glutDisplayFunc(display); // refresh window function
		break;
	case 2:
		glutDisplayFunc(displayFromWindow);
		break;
	case 3:
		glutDisplayFunc(engineView);
		break;
	case 4:
		glutDisplayFunc(sideDisplay);
		break;
	}
}

void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	// defines BUFFERS: Color buffer (frame buffer) and Depth buffer
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
	glutInitWindowSize(W, H);
	glutInitWindowPosition(400, 200);
	glutCreateWindow("Final Computer Graphics Project");

	glutDisplayFunc(display); // refresh window function
	glutIdleFunc(idle); // kind of timer function

	glutSpecialFunc(special_key);
	glutKeyboardFunc(keyboard);

	// menu
	glutCreateMenu(menu);
	glutAddMenuEntry("Regular view", 1);
	glutAddMenuEntry("Window view", 2);
	glutAddMenuEntry("Engine view", 3);
	glutAddMenuEntry("Side view", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutMainLoop();
}