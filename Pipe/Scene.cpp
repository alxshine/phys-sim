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
	boundarySpeed = 2;
	pZero = 10.0;
	boundaryPressure = pZero - 0.5*boundarySpeed*boundarySpeed;

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
	vel.reserve(resolutionX * resolutionY);
	pressure.reserve(resolutionX * resolutionY);
	divergence.reserve(resolutionX * resolutionY);

	//set border velocities
	for (int i = 1; i < resolutionY - 1; i++) {
		vel[i * resolutionX] = Vec2(boundarySpeed, 0);
		vel[(i + 1) * resolutionX - 1] = Vec2(boundarySpeed, 0);
		pressure[i * resolutionX] = boundaryPressure;
		pressure[(i+1) * resolutionX -1] = boundaryPressure;
	}
}


void Scene::Update(){

}

void Scene::Solve(int iterations) {

	for (int i = 0; i < iterations; i++) {
		SolvePressure(20);
		UpdateVelocity();
		ComputeDivergence();
	}

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
	glVertex2d(leftBorder, topBorder);
	glVertex2d(rightBorder, topBorder);
	glEnd();

	glBegin(GL_LINES);
	glVertex2d(leftBorder, bottomBorder);
	glVertex2d(rightBorder, bottomBorder);
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

	//draw the arrows for velocity
	glColor3f(0.4, 0.4, 0.8);

	for (int j = 0; j < resolutionY; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX; i++) {
			double locX = leftBorder + xStep * i;
			Vec2 currentVel = vel[j * resolutionX + i];
			if (currentVel.isZero())
				continue;

			Vec2 arrowEnd = Vec2(locX + currentVel.x / 10,
					locY + currentVel.y / 10);

			//draw the arrow body
			glBegin(GL_LINES);
			glVertex2d(locX, locY);
			glVertex2d(arrowEnd.x, arrowEnd.y);
			glEnd();

			//draw the head for the arrow
			Vec2 direction = currentVel.normalize();
			Vec2 headStart = arrowEnd - direction * 0.04;
			double offsetX = 0, offsetY = 0;
			if (direction.y == 0 && direction.x == 0) {
				//leave the offset 0
			} else if (direction.y == 0) {
				offsetY = 1;
			} else if (direction.x == 0) {
				offsetX = 1;
			} else {
				offsetX = direction.x;
				offsetY = direction.y;
			}
			Vec2 pointA = headStart + Vec2(offsetX, -offsetY)*0.02;
			Vec2 pointB = headStart + Vec2(-offsetX, offsetY)*0.02;

			//the head of the arrow is the triangle between pointA, pointB and arrowEnd
			glBegin(GL_TRIANGLES);
			glVertex2d(arrowEnd.x, arrowEnd.y);
			glVertex2d(pointA.x, pointA.y);
			glVertex2d(pointB.x, pointB.y);
			glEnd();
		}
	}
}

void Scene::SolvePressure(int iterations) {

	double h = 1.0 / resolutionX;

	for (int i = 0; i < iterations; i++) {
		//solver iteration
		for (int y = 0; y < resolutionY; y++) {
			for (int x = 0; x < resolutionX; x++) {
				int c = y * resolutionX + x;
				//handle boundaries
				double above = y < resolutionY - 1 ? pressure[c + resolutionX] : pZero;
				double below = y > 0 ? pressure[c - resolutionX] : pZero;
				double left = x > 0 ? pressure[c - 1] : boundaryPressure;
				double right = x < resolutionX - 1 ? pressure[c + 1] : boundaryPressure;


				pressure[c] = (h * h * divergence[c] + above + below + left
						+ right) / 4;
			}
		}
	}

}

void Scene::UpdateVelocity(void) {

	double h = 1.0 / resolutionX;
	for (int y = 0; y < resolutionY; y++) {
		for (int x = 0; x < resolutionX; x++) {
			int c = y * resolutionX + x;

			double xVel = -(1 / h * (pressure[c] - pressure[c - 1]));
			double yVel = -(1 / h * (pressure[c] - pressure[c - resolutionX]));

			
			Vec2 velTemp = Vec2(xVel, yVel).normalize();



			velTemp = velTemp * sqrt(abs(pZero - pressure[c])) * 2;



			vel[c] = velTemp;

		}
	}

}

void Scene::ComputeDivergence(void) {
	const double dx = 1.0 / resolutionX;
	const double idtx = 1.0 / (2.0 * (dx * dx));

	for (int y = 1; y < resolutionY - 1; y++)
		for (int x = 1; x < resolutionX - 1; x++) {
			const int index = y * resolutionX + x;
			const double xComponent = (vel[index + 1].x
					- vel[index - 1].x) * idtx;
			const double yComponent = (vel[index + resolutionX].y
					- vel[index - resolutionX].y) * idtx;
			divergence[index] = -(xComponent + yComponent);
		}
}
