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
	vector<double> pressure;
	Fluid2D fluid;
	double topBorder, rightBorder, bottomBorder, leftBorder;
	double crossHalfLength;
	int blockSideLength;
	double pZero;
	vector<int> zeroBlocks;

	void drawGridPoint(double locX, double locY);
	void drawGridArrow(double locX, double locY, Vec2 currentVel);
	void CreatePressureVertex(double locX, double locY, double val,
			double maxValue, double minValue);
	void HSV2RGB(double h, double s, double v, double &r, double &g,
			double &b);
	void setUpTestCase();
	void renderPipe();
	void renderPressure(double yStep, double xStep);
	void renderGrid(double yStep, double xStep);
	void renderObstacles(double xStep, double yStep);
	void renderVelocities(double yStep, double xStep);
	bool isAlreadyInObstacle(int gridX, int gridY);

public:

	Scene(void);

	Scene(int argc, char *argv[]);

	~Scene(void);

	void PrintSettings(void);
	void Render(bool renderPressure); /* Draw scene */
	void HandleMouse(double x, double y);

	void Solve(int iterations, bool scale);

};

#endif
