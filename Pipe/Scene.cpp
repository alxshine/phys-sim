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
				resolutionY), blockSideLength(1) {
	vel.resize(resolutionX * resolutionY);
	pressure.resize(resolutionX * resolutionY);

	setUpTestCase();
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

void Scene::Solve(int iterations) {
	//convert the zeroBlocks into zeroIndices
	vector<int> zeroIndices;
	zeroIndices.reserve(zeroBlocks.size() * blockSideLength * blockSideLength);
	for (int block : zeroBlocks) {
		int blockX = block % resolutionX;
		int blockY = block / resolutionY;
		for (int offsetX = 0; offsetX <= blockSideLength; offsetX++) {
			for (int offsetY = 0; offsetY <= blockSideLength; offsetY++) {
				int index = (blockY + offsetY) * resolutionX + blockX + offsetX;
				if (find(zeroIndices.begin(), zeroIndices.end(), index)
						== zeroIndices.end())
					zeroIndices.emplace_back(index);
			}
		}
	}

	fluid.reset(zeroIndices);
	for (int i = 0; i < iterations - 1; i++)
		fluid.step(zeroIndices, true);
	fluid.step(zeroIndices, false);

	double* xVel = fluid.get_xVelocity();
	double* yVel = fluid.get_yVelocity();
	double* p = fluid.get_pressure();

	for (int i = 0; i < resolutionX * resolutionY; i++) {
		vel[i].x = xVel[i];
		vel[i].y = yVel[i];
		pressure[i] = p[i];
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

void Scene::CreatePressureVertex(double locX, double locY, double val,
		double maxValue, double minValue) {
	double s = 1.0;
	double v = 1.0;

	val -= minValue;
	val /= maxValue - minValue;

	double h = (1. - val) * 240;
	double r, g, b;
	r = g = b = 0.;
	HSV2RGB(h, s, v, r, g, b);
	glColor3f(r, g, b);
	glVertex2d(locX, locY);
}

void Scene::renderPipe() {
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
}

void Scene::renderPressure(double yStep, double xStep) {
	//render the pressure as heatmap
	double maxValue = *max_element(pressure.begin(), pressure.end());
	double minValue = *min_element(pressure.begin(), pressure.end());

	glBegin(GL_QUADS);
	for (int j = 0; j < resolutionY - 1; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX - 1; i++) {
			double locX = leftBorder + xStep * i;
			int index = j * resolutionX + i;
			CreatePressureVertex(locX, locY, pressure[index], maxValue,
					minValue);
			CreatePressureVertex(locX + xStep, locY, pressure[index + 1],
					maxValue, minValue);
			CreatePressureVertex(locX + xStep, locY + yStep,
					pressure[index + resolutionX + 1], maxValue, minValue);
			CreatePressureVertex(locX, locY + yStep,
					pressure[index + resolutionX], maxValue, minValue);
		}
	}
	glEnd();
}

void Scene::renderGrid(double yStep, double xStep) {
	//render the grid with little crosses
	glColor3f(0.7, 0.7, 0.7);
	glLineWidth(1);
	for (int j = 0; j < resolutionY; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX; i++) {
			double locX = leftBorder + xStep * i;
			drawGridPoint(locX, locY);
		}
	}
}

void Scene::renderObstacles(double xStep, double yStep) {
	//draw the obstacles
	glColor3f(0.6, 0.6, 0.6);
	for (int index : zeroBlocks) {
		int x = index % resolutionX;
		double locX = leftBorder + xStep * x;
		int y = index / resolutionX;
		double locY = bottomBorder + yStep * y;
		glBegin(GL_QUADS);
		glVertex2d(locX, locY);
		glVertex2d(locX + blockSideLength * xStep, locY);
		glVertex2d(locX + blockSideLength * xStep,
				locY + blockSideLength * yStep);
		glVertex2d(locX, locY + blockSideLength * yStep);
		glEnd();
	}
}

void Scene::renderVelocities(double yStep, double xStep) {
	//draw the arrows for velocity
	glColor3f(0.4, 0.4, 0.8);
	for (int j = 0; j < resolutionY; j++) {
		double locY = bottomBorder + yStep * j;
		for (int i = 0; i < resolutionX; i++) {
			double locX = leftBorder + xStep * i;
			Vec2 currentVel = vel[j * resolutionX + i];

			if (!isAlreadyInObstacle(i, j))
				drawGridArrow(locX, locY, currentVel);
		}
	}
}

void Scene::Render(bool renderP) {
	//we need to divide by resolution-1 because otherwise the array and visualization shape would not match
	double xStep = (rightBorder - leftBorder) / (resolutionX - 1);
	double yStep = (topBorder - bottomBorder) / (resolutionY - 1);

	//render the pipe itself
	renderPipe();

	if (renderP)
		renderPressure(yStep, xStep);
	renderGrid(yStep, xStep);
	renderObstacles(xStep, yStep);
	renderVelocities(yStep, xStep);
}

bool Scene::isAlreadyInObstacle(int gridX, int gridY) {
	//check if the clicked block is already an obstacle
	for (int index : zeroBlocks) {
		int blockX = index % resolutionX;
		int blockY = index / resolutionY;
		bool xContained = false;
		for (int offsetX = 0; offsetX < blockSideLength; offsetX++) {
			if (gridX == blockX + offsetX) {
				xContained = true;
				break;
			}
		}
		if (xContained) {
			for (int offsetY = 0; offsetY < blockSideLength; offsetY++) {
				if (gridY == blockY + offsetY) {
					return true;
				}
			}
		}
	}
	return false;
}

void Scene::HandleMouse(double x, double y) {
//check if the user actually clicked inside the grid
	if (x < leftBorder || x > rightBorder)
		return;
	if (y < bottomBorder || y > topBorder)
		return;

//calculate the position of the click on the grid
	double xStep = (rightBorder - leftBorder) / (resolutionX - 1);
	double yStep = (topBorder - bottomBorder) / (resolutionY - 1);

	x -= leftBorder;
	y -= bottomBorder;

	int gridX = x / xStep;
	int gridY = y / yStep;

//check if the clicked block is already an obstacle
	if (!isAlreadyInObstacle(gridX, gridY))
		zeroBlocks.emplace_back(gridY * resolutionX + gridX);
}

void Scene::HSV2RGB(double h, double s, double v, double &r, double &g,
		double &b) {
	h /= 360.0;
	if (fabs(h - 1.0) < 0.000001)
		h = 0.0;

	h *= 6.0;

	int i = floor(h);

	double f = h - i;
	double p = v * (1.0 - s);
	double q = v * (1.0 - (s * f));
	double t = v * (1.0 - (s * (1.0 - f)));

	switch (i) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	case 5:
		r = v;
		g = p;
		b = q;
		break;
	}
}
