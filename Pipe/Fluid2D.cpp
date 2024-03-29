/******************************************************************               
 *
 * Fluid2D.cpp
 *
 * Description: Implementation of functions for setting up 2D Euler
 * flow scene & solving the underlying differential equations via
 * operator splitting; semi-Lagrangian advection is employed, the
 * pressures are obtained via Gauss-Seidel iteration; artificial
 * turbulence is added following Fedkiw et al. (vorticity confinement)
 *
 * Physically-Based Simulation Proseminar WS 2016
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/

/* Standard includes */
#include <sstream>
#include <stdlib.h>

/* Local includes */
#include "Fluid2D.h"

/* External functions for implementing advection, the pressure
 solver, and the correction of velocities */
extern void AdvectWithSemiLagrange(int xRes, int yRes, double dt,
		double* xVelocity, double* yVelocity, double *field, double *tempField,
		double defaultValue);

extern void SolvePoisson(int xRes, int yRes, int iterations, double accuracy,
		double* pressure, double* divergence);

extern void CorrectVelocities(int xRes, int yRes, double dt,
		const double* pressure, double* xVelocity, double* yVelocity);

Fluid2D::Fluid2D(int _xRes, int _yRes) :
		xRes(_xRes), yRes(_yRes) {
	dt = 0.15; /* Time step */
	totalSteps = 0;
	bndryCond = 1;
	addVort = 0;

	iterations = solverIterations;
	accuracy = solverAccuracy;

	/* Allocate fields */
	totalCells = xRes * yRes;
	divergence = new double[totalCells];
	pressure = new double[totalCells];
	xVelocity = new double[totalCells];
	yVelocity = new double[totalCells];
	xVelocityTemp = new double[totalCells];
	yVelocityTemp = new double[totalCells];
	xForce = new double[totalCells];
	yForce = new double[totalCells];
	density = new double[totalCells];
	densityTemp = new double[totalCells];
	curl = new double[totalCells];
	xCurlGrad = new double[totalCells];
	yCurlGrad = new double[totalCells];

	/* Initialize fields */
	reset(vector<int>());
}

Fluid2D::~Fluid2D() {
	if (density)
		delete[] density;
	if (densityTemp)
		delete[] densityTemp;
	if (divergence)
		delete[] divergence;
	if (pressure)
		delete[] pressure;
	if (xVelocity)
		delete[] xVelocity;
	if (yVelocity)
		delete[] yVelocity;
	if (xVelocityTemp)
		delete[] xVelocityTemp;
	if (yVelocityTemp)
		delete[] yVelocityTemp;
	if (xForce)
		delete[] xForce;
	if (yForce)
		delete[] yForce;
	if (curl)
		delete[] curl;
	if (xCurlGrad)
		delete[] xCurlGrad;
	if (yCurlGrad)
		delete[] yCurlGrad;
}

/*----------------------------------------------------------------*/

void Fluid2D::reset(vector<int> zeroIndices) {
	/* Initialize fields */
	for (int i = 0; i < totalCells; i++) {
		divergence[i] = 0.0;
		pressure[i] = 1.0;
		xVelocity[i] = 0.0;
		yVelocity[i] = 0.0;
		xVelocityTemp[i] = 0.0;
		yVelocityTemp[i] = 0.0;
		xForce[i] = 0.0;
		yForce[i] = 0.0;
		density[i] = 0.0;
		densityTemp[i] = 0.0;
		curl[i] = 0.0;
		xCurlGrad[i] = 0.0;
		yCurlGrad[i] = 0.0;
	}

	vector<bool> reachabilityGrid(xRes * yRes, false);
	getReachablePoints(zeroIndices, reachabilityGrid);
	for (int index = 0; index < yRes * xRes; index++)
		if (reachabilityGrid[index])
			xVelocity[index] = 2;

//set the boundaries to their designated values
	enforceBoundaries();
}

void Fluid2D::scaleFlow(const vector<int>& zeroIndices) {
	/* Ensure global flow stays constant by scaling the columns */
	vector<bool> reachabilityGrid(xRes * yRes, false);
	getReachablePoints(zeroIndices, reachabilityGrid);
	double targetFlow = (yRes - 2) * 2.;
	for (int x = 1; x < xRes - 1; x++) {
		//get the flow of the current cut
		double currentFlow = 0.;
		for (int y = 0; y < yRes; y++)
			currentFlow += xVelocity[y * xRes + x];
		double scaling = targetFlow / currentFlow;
		for (int y = 0; y < yRes; y++)
			xVelocity[y * xRes + x] *= scaling;
	}
}

/*------------------------------------------------------------------
 | Time step for solving the Euler flow equations using operator
 | splitting
 ------------------------------------------------------------------*/

void Fluid2D::step(vector<int> zeroIndices, bool enforce, bool scale) {
	AdvectWithSemiLagrange(xRes, yRes, dt, xVelocity, yVelocity, xVelocity,
			xVelocityTemp, 2.);
	AdvectWithSemiLagrange(xRes, yRes, dt, xVelocity, yVelocity, yVelocity,
			yVelocityTemp, 0.);

	/* Copy/update advected fields */
	copyFields();

	if (enforce)
		enforceBoundaries();

	/* Zero the obstacle blocks */
	zeroObstacles(zeroIndices);

	/* Solve for pressure, ensuring divergence-free velocity field */
	solvePressure();

	/* Ensure global flow stays constant by scaling the columns */
	if (scale)
		scaleFlow(zeroIndices);
	totalSteps++;
}

void Fluid2D::zeroObstacles(vector<int> zeroIndices) {

	/* Zero the obstacle blocks */
	for (int index : zeroIndices) {
		xVelocity[index] = 0;
		yVelocity[index] = 0;
	}

}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

void Fluid2D::solvePressure() {
	/* Set appropriate boundary conditions, we use zero slip */
	setZeroY(yVelocity);
	setZeroY(xVelocity);

	/* Compute velocity field divergence */
	computeDivergence();

	copyBorderX(pressure);
	copyBorderY(pressure);

	/* Solve for pressures and make field divergence-free */
	SolvePoisson(xRes, yRes, iterations, accuracy, pressure, divergence);
	CorrectVelocities(xRes, yRes, dt, pressure, xVelocity, yVelocity);
}

void Fluid2D::enforceBoundaries() {
	for (int y = 1; y < yRes - 1; y++) {
		int leftCoord = y * xRes;
		int rightCoord = leftCoord + xRes - 1;

		pressure[leftCoord] = 1.;
		xVelocity[leftCoord] = xVelocity[rightCoord] = 2.;
	}
}

void Fluid2D::computeDivergence() {
	const double dx = 1.0 / xRes;
	const double idtx = 1.0 / (2.0 * (dt * dx));

	for (int y = 1; y < yRes - 1; y++)
		for (int x = 1; x < xRes - 1; x++) {
			const int index = y * xRes + x;
			const double xComponent = (xVelocity[index + 1]
					- xVelocity[index - 1]) * idtx;
			const double yComponent = (yVelocity[index + xRes]
					- yVelocity[index - xRes]) * idtx;
			divergence[index] = -(xComponent + yComponent);
		}
}

void Fluid2D::copyFields() {
	int size = totalCells * sizeof(double);
	memcpy(xVelocity, xVelocityTemp, size);
	memcpy(yVelocity, yVelocityTemp, size);
}

void Fluid2D::getReachablePoints(vector<int> zeroIndices,
		vector<bool> retGrid) {
	//the last row is always reachable
	for (int y = 0; y < yRes; y++) {
		if (find(zeroIndices.begin(), zeroIndices.end(), (y + 1) * xRes - 1)
				== zeroIndices.end())
			retGrid[(y + 1) * xRes - 1] = true;
	}

	for (int x = xRes - 2; x >= 0; x--) {
		for (int y = 0; y < yRes; y++) {
			if (find(zeroIndices.begin(), zeroIndices.end(), y * xRes + x)
					== zeroIndices.end()) {
				for (int offsetY = y > 0 ? -1 : 0;
						offsetY < (y < yRes - 1 ? 1 : 0); offsetY++) {
					if (retGrid[(y + offsetY) * xRes + x + 1])
						retGrid[y * xRes + x] = true;
				}
			}
		}
	}
}

/*----------------------------------------------------------------*/

void Fluid2D::setNeumannX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = field[index + 1];
		field[index + xRes - 1] = field[index + xRes - 2];
	}
}

void Fluid2D::setNeumannY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = field[index + xRes];

		index = x + (yRes - 1) * xRes;
		field[index] = field[index - xRes];
	}
}

void Fluid2D::setZeroX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = 0.0;
		field[index + xRes - 1] = 0.0;
	}
}

void Fluid2D::setZeroY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = 0.0;

		index = x + (yRes - 1) * xRes;
		field[index] = 0.0;
	}
}

void Fluid2D::copyBorderX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = field[index + 1];
		field[index + xRes - 1] = field[index + (xRes - 1) - 1];
	}
}

void Fluid2D::copyBorderY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = field[index + xRes];

		index = x + (yRes - 1) * xRes;
		field[index] = field[index - xRes];
	}
}
