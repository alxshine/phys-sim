/******************************************************************
 *
 * Exercise.cpp
 *
 * Description: In this file the functions AdvectWithSemiLagrange(),
 * SolvePoisson(), and CorrectVelocities() have to be implemented
 * for the third programming assignment.
 * Feel free to add new local functions into this file.
 * Changes to other source files of the framework should not be
 * required.
 *
 * Physically-Based Simulation Proseminar WS 2016
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/
#include <limits>
#include <cmath>

#include "Fluid2D.h"

void AdvectWithSemiLagrange(int xRes, int yRes, double dt, double *xVelocity,
		double *yVelocity, double *field, double* tempField) {
	// Task 1
	for (int y = 0; y < yRes; y++) {
		for (int x = 0; x < xRes; x++) {
			int coord = y * xRes + x;

			//calculate old position based on current velocity
			double oldX = x - xVelocity[coord] * dt * xRes;
			double oldY = y - yVelocity[coord] * dt * yRes;

			//interpolate quantity from neighbors of old coordinates
			int leftCoord = (int) oldX;
			int rightCoord = leftCoord + 1;
			int bottomCoord = (int) oldY;
			int topCoord = bottomCoord + 1;

			double bottomLeft = field[bottomCoord * xRes + leftCoord];
			double bottomRight = field[bottomCoord * xRes + rightCoord];
			double topLeft = field[topCoord * xRes + leftCoord];
			double topRight = field[topCoord * xRes + rightCoord];

			double xRatio = oldX - leftCoord;
			double yRatio = oldY - bottomCoord;

			double leftInterpolation = topLeft * yRatio
					+ bottomLeft * (1 - yRatio);
			double rightInterpolation = topRight * yRatio
					+ bottomRight * (1 - yRatio);

			tempField[coord] = rightInterpolation * xRatio
					+ leftInterpolation * (1 - xRatio);
		}
	}
}

void SolvePoisson(int xRes, int yRes, int iterations, double accuracy,
		double* pressure, double* divergence) {
// Task 2
	double h = 1.0 / xRes;

	for (int i = 0; i < iterations; i++) {
		//solver iteration
		for (int y = 0; y < yRes; y++) {
			for (int x = 0; x < xRes; x++) {
				int c = y * xRes + x;
				//handle boundaries
				double above = y < yRes - 1 ? pressure[c + xRes] : 0;
				double below = y > 0 ? pressure[c - xRes] : 0;
				double left = x > 0 ? pressure[c - 1] : 0;
				double right = x < xRes - 1 ? pressure[c + 1] : 0;
//				double above = pressure[c + xRes];
//				double below = pressure[c - xRes];
//				double left = pressure[c - 1];
//				double right = pressure[c + 1];

				pressure[c] = (h * h * divergence[c] + above + below + left
						+ right) / 4;
			}
		}

		//compute residual
		double residual = 0.;
		for (int y = 0; y < yRes; y++) {
			for (int x = 0; x < xRes; x++) {
				int c = y * xRes + x;
				//handle boundaries
				double above = y < yRes - 1 ? pressure[c + xRes] : 0;
				double below = y > 0 ? pressure[c - xRes] : 0;
				double left = x > 0 ? pressure[c - 1] : 0;
				double right = x < xRes - 1 ? pressure[c + 1] : 0;
//				double above = pressure[c + xRes];
//				double below = pressure[c - xRes];
//				double left = pressure[c - 1];
//				double right = pressure[c + 1];

				double rTemp = h * h * divergence[c] + above + below + left
						+ right - 4 * pressure[c];
				residual += rTemp * rTemp;
			}
		}
		if (sqrt(residual) < accuracy)
			return;
	}
}

void CorrectVelocities(int xRes, int yRes, double dt, const double* pressure,
		double* xVelocity, double* yVelocity) {
// Task 3
	double h = 1.0 / xRes;
	for (int y = 0; y < yRes; y++) {
		for (int x = 0; x < xRes; x++) {
			int c = y * xRes + x;

			xVelocity[c] = xVelocity[c]
					- dt * (1 / h * (pressure[c] - pressure[c - 1]));
			yVelocity[c] = yVelocity[c]
					- dt * (1 / h * (pressure[c] - pressure[c - xRes]));
		}
	}
}

