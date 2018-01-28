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
		double* xVelocity, double* yVelocity, double *field, double *tempField);

extern void SolvePoisson(int xRes, int yRes, int iterations, double accuracy,
		double* pressure, double* divergence);

extern void CorrectVelocities(int xRes, int yRes, double dt,
		const double* pressure, double* xVelocity, double* yVelocity);

Fluid_2D::Fluid_2D(int _xRes, int _yRes) :
		xRes(_xRes), yRes(_yRes) {
	dt = 0.1; /* Time step */
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
	for (int i = 0; i < totalCells; i++) {
		divergence[i] = 0.0;
		pressure[i] = 1.0;
		xVelocity[i] = 2.0;
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
}

Fluid_2D::~Fluid_2D() {
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

/*------------------------------------------------------------------
 | Time step for solving the Euler flow equations using operator
 | splitting
 ------------------------------------------------------------------*/

void Fluid_2D::step() {

	AdvectWithSemiLagrange(xRes, yRes, dt, xVelocity, yVelocity, xVelocity,
			xVelocityTemp);
	AdvectWithSemiLagrange(xRes, yRes, dt, xVelocity, yVelocity, yVelocity,
			yVelocityTemp);

	/* Copy/update advected fields */
	copyFields();

	/* Solve for pressure, ensuring divergence-free velocity field */
	solvePressure();

	totalSteps++;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

void Fluid_2D::solvePressure() {
	/* Set appropriate boundary condition - open vs. closed domain */
//	setNeumannX(xVelocity);
	setNeumannY(yVelocity);
	setZeroY(xVelocity);
//	setZeroX(yVelocity);

	/* Compute velocity field divergence */
	computeDivergence();

	copyBorderX(pressure);
	copyBorderY(pressure);

	/* Solve for pressures and make field divergence-free */
	SolvePoisson(xRes, yRes, iterations, accuracy, pressure, divergence);
	CorrectVelocities(xRes, yRes, dt, pressure, xVelocity, yVelocity);
}

void Fluid_2D::computeDivergence() {
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

void Fluid_2D::copyFields() {
	int size = totalCells * sizeof(double);
	memcpy(xVelocity, xVelocityTemp, size);
	memcpy(yVelocity, yVelocityTemp, size);
}

/*----------------------------------------------------------------*/

void Fluid_2D::setNeumannX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = field[index + 1];
		field[index + xRes - 1] = field[index + xRes - 2];
	}
}

void Fluid_2D::setNeumannY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = field[index + xRes];

		index = x + (yRes - 1) * xRes;
		field[index] = field[index - xRes];
	}
}

void Fluid_2D::setZeroX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = 0.0;
		field[index + xRes - 1] = 0.0;
	}
}

void Fluid_2D::setZeroY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = 0.0;

		index = x + (yRes - 1) * xRes;
		field[index] = 0.0;
	}
}

void Fluid_2D::copyBorderX(double* field) {
	for (int y = 0; y < yRes; y++) {
		int index = y * xRes;
		field[index] = field[index + 1];
		field[index + xRes - 1] = field[index + (xRes - 1) - 1];
	}
}

void Fluid_2D::copyBorderY(double* field) {
	for (int x = 0; x < xRes; x++) {
		int index = x;
		field[index] = field[index + xRes];

		index = x + (yRes - 1) * xRes;
		field[index] = field[index - xRes];
	}
}