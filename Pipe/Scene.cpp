/******************************************************************
 *
 * Scene.cpp
 *
 * Description: Setup of 2D simulation scenes; three different scene
 * configurations are hard-coded - a hanging mass, a hanging triangle,
 * a falling triangle;
 *
 * Physically-Based Simulation Proseminar WS 2015
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/

using namespace std;

/* Local includes */
#include "Scene.h"

Scene::Scene(void) {
	resolutionX = 20;
	resolutionY = 20;
	topBorder = 2;
	bottomBorder = -2;
	leftBorder = -2;
	rightBorder = 2;
	crossHalfLength = 0.02;

	Init();
	PrintSettings();
}

Scene::Scene(int argc, char *argv[]) :
		Scene() {
}

Scene::~Scene(void) {
}

void Scene::PrintSettings(void) {
	cerr << endl << "Settings for pipe:" << endl;

	cerr << "\t-resolution " << resolutionX << "x" << resolutionY << endl;
}

/******************************************************************
 *
 * Init
 *
 * Setup 2D simulation scenes; geometry for all three cases is
 * hard-coded
 *
 *******************************************************************/

void Scene::Init(void) {
//	gridX = vector<double>(resolutionX);
//	gridY = vector<double>(resolutionY);
}

void Scene::Update(void) {
}

void Scene::drawGridPoint(double locX, double locY) {
	glBegin(GL_LINES);
	glVertex2d(locX - crossHalfLength, locY);
	glVertex2d(locX + crossHalfLength, locY);
	glEnd();
	glBegin(GL_LINES);
	glVertex2d(locX, locY - crossHalfLength);
	glVertex2d(locX, locY + crossHalfLength);
	glEnd();
}

void Scene::Render(void) {
	//render the pipe itself
	glColor3f(0.5, 0.5, 0.5);
	glLineWidth(5);
	glBegin(GL_LINES);
	glVertex3d(leftBorder, topBorder, 0.0);
	glVertex3d(rightBorder, topBorder, 0.0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3d(leftBorder, bottomBorder, 0);
	glVertex3d(rightBorder, bottomBorder, 0);
	glEnd();

	//render the grid with little crosses
	glColor3f(0.7, 0.7, 0.7);
	glLineWidth(1);
	//we need to divide by resolution-1 because otherwise the array and visualization shape would not match
	double xStep = (rightBorder - leftBorder) / (resolutionX - 1);
	double yStep = (topBorder - bottomBorder) / (resolutionY - 1);
	for (int j = 0; j < resolutionY; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX; i++) {
			double locX = leftBorder + xStep * i;

			drawGridPoint(locX, locY);
		}
	}
}
