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
#include <algorithm>

using namespace std;

#include "Point.h"
#include "Vec2.h"
#include "Fluid2D.h"

class Scene {
private:
	int resolutionX, resolutionY;
	double boundarySpeed;
	double boundaryPressure;
	vector<Vec2> vel;
	Fluid2D fluid;
	double topBorder, rightBorder, bottomBorder, leftBorder;
	double crossHalfLength;
	double pZero;
	vector<int> zeroBlocks;

	void drawGridPoint(double locX, double locY);
	void drawGridArrow(double locX, double locY, Vec2 currentVel);
	void setUpTestCase();

public:

	Scene(void);

	Scene(int argc, char *argv[]);

	~Scene(void);

	void Init(void);

	void PrintSettings(void);
	void Render(); /* Draw scene */

	void Update(); /* Execute time step */

	void Solve(int iterations);
	void SolvePressure(int iterations);

	void UpdateVelocity(void);

	void ComputeDivergence(void);

};

#endif
