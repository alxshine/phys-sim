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

Scene::Scene(void) :
		resolutionX(20), resolutionY(20), topBorder(2), rightBorder(2), bottomBorder(
				-2), leftBorder(-2), crossHalfLength(0.02), fluid(resolutionX,
				resolutionY) {
	setUpTestCase();
	Init();
	PrintSettings();
}

Scene::Scene(int argc, char *argv[]) :
		Scene() {
}

Scene::~Scene(void) {
}

void Scene::setUpTestCase() {
	//long narrow chain bottom, also the narrow passages
	zeroBlocks.push_back(51);
	zeroBlocks.push_back(72);
	zeroBlocks.push_back(92);
	zeroBlocks.push_back(112);
	zeroBlocks.push_back(132);

	zeroBlocks.push_back(172);
	zeroBlocks.push_back(192);

	zeroBlocks.push_back(252);
	zeroBlocks.push_back(272);

	//single block left
	zeroBlocks.push_back(222);

	//bay top middle
	zeroBlocks.push_back(370);
	zeroBlocks.push_back(350);
	zeroBlocks.push_back(330);
	zeroBlocks.push_back(329);
	zeroBlocks.push_back(328);

	//3x3 block bottom left
	zeroBlocks.push_back(42);
	zeroBlocks.push_back(43);
	zeroBlocks.push_back(44);
	zeroBlocks.push_back(62);
	zeroBlocks.push_back(63);
	zeroBlocks.push_back(64);
	zeroBlocks.push_back(82);
	zeroBlocks.push_back(83);
	zeroBlocks.push_back(84);

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

}

void Scene::Solve(int iterations) {
	for (int i = 0; i < iterations; i++)
		fluid.step(zeroBlocks);

	double* xVel = fluid.get_xVelocity();
	double* yVel = fluid.get_yVelocity();

	for (int i = 0; i < resolutionX * resolutionY; i++) {
		vel[i].x = xVel[i];
		vel[i].y = yVel[i];
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

void Scene::drawGridArrow(double locX, double locY, Vec2 currentVel) {
	Vec2 arrowEnd = Vec2(locX + currentVel.x / 10, locY + currentVel.y / 10);

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
	Vec2 pointA = headStart + Vec2(offsetX, -offsetY) * 0.02;
	Vec2 pointB = headStart + Vec2(-offsetX, offsetY) * 0.02;

	//the head of the arrow is the triangle between pointA, pointB and arrowEnd
	glBegin(GL_TRIANGLES);
	glVertex2d(arrowEnd.x, arrowEnd.y);
	glVertex2d(pointA.x, pointA.y);
	glVertex2d(pointB.x, pointB.y);
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

	//draw the obstacles
	glColor3f(0.6, 0.6, 0.6);
	for (int index : zeroBlocks) {
		int x = index % resolutionX;
		double locX = leftBorder + xStep * x;
		int y = index / resolutionX;
		double locY = bottomBorder + yStep * y;

		glBegin(GL_QUADS);
		glVertex2d(locX, locY);
		glVertex2d(locX + xStep, locY);
		glVertex2d(locX + xStep, locY + yStep);
		glVertex2d(locX, locY + yStep);
		glEnd();
	}

	//draw the arrows for velocity
	glColor3f(0.4, 0.4, 0.8);

	for (int j = 0; j < resolutionY; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX; i++) {
			double locX = leftBorder + xStep * i;
			Vec2 currentVel = vel[j * resolutionX + i];
			bool draw = true;

			//check if we would be drawing into an obstacle
			for (int index : zeroBlocks) {
				int blockX = index % resolutionX;
				int blockY = index / resolutionY;

				if (blockY == j) {
					if (blockX == i) {
						//bottom left corner
						if (currentVel.x > 0 && currentVel.y > 0) {
							draw = false;
							break;
						}
					} else if (blockX + 1 == i) {
						//bottom right corner
						if (currentVel.x < 0 && currentVel.y > 0) {
							draw = false;
							break;
						}
					}
				} else if (blockY + 1 == j) {
					if (blockX == i) {
						//top left corner
						if (currentVel.x > 0 && currentVel.y < 0) {
							draw = false;
							break;
						}
					} else if (blockX + 1 == i) {
						//top right corner
						if (currentVel.x < 0 && currentVel.y < 0) {
							draw = false;
							break;
						}
					}
				}
			}

			if (draw)
				drawGridArrow(locX, locY, currentVel);
		}
	}
}
