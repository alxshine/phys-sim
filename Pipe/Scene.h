/******************************************************************
 *
 * Scene.h
 *
 * Description: Class definition for scene description
 *
 * Physically-Based Simulation Proseminar WS 2015
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/

#ifndef __SCENE_H__
#define __SCENE_H__

/* Standard includes */
#define _USE_MATH_DEFINES

#include <GL/glut.h>

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <vector>

using namespace std;

#include "Spring.h"
#include "Point.h"
#include "Vec2.h"

class Scene {
private:
	int resolutionX, resolutionY;
//	vector<double> gridX, gridY;
	double topBorder, rightBorder, bottomBorder, leftBorder;
	double crossHalfLength;

	void drawGridPoint(double locX, double locY);

public:

	Scene(void);

	Scene(int argc, char *argv[]);

	~Scene(void);

	void Init(void);

	void PrintSettings(void);
	void Render(); /* Draw scene */

	void Update(); /* Execute time step */
};

#endif
