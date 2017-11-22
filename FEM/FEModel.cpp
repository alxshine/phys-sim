/******************************************************************
 *
 * FEModel.cpp
 *
 * Description: Implements a Finite Element Solver for finding the
 * solution to Poisson�s equation; boundary conditions given in
 * Dirichlet form;
 * Note: equation system has to be set up before calling solver
 *
 * Physically-Based Simulation Proseminar WS 2015
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/

#include "GL/glut.h"  
#include <math.h>
#include <stdio.h>

#include "HSV2RGB.h"
#include "FEModel.h"

/*----------------------------------------------------------------*/
double Boundary_u(double x, double y) {
	/* Funcion on boundary, and also exact soluion */
	return 3.0 * x * x + 2.0 * y * y * y * x;
}

double Source_Term_f(double x, double y) {
	/* Source term in Poisson´s equation */
	return -6.0 - 12.0 * y * x;
}

/*----------------------------------------------------------------*/
void FEModel::CreateUniformGridMesh(int nodesX, int nodesY) {
	double lenX = (double) (nodesX - 1);
	double lenY = (double) (nodesY - 1);

	for (int y = 0; y < nodesY; y++) {
		for (int x = 0; x < nodesX; x++) {
			Vector2 pos = Vector2((double) x / lenX, (double) y / lenY);
			nodes.push_back(pos);
			num_nodes++;
		}
	}

	for (int y = 0; y < nodesY - 1; y++) {
		for (int x = 0; x < nodesX - 1; x++) {
			int node00 = y * nodesX + x;
			int node10 = node00 + 1;
			int node01 = node00 + nodesX;
			int node11 = node00 + nodesX + 1;

			elements.push_back(LinTriElement(node00, node10, node11));
			elements.push_back(LinTriElement(node00, node11, node01));
			num_elems += 2;
		}
	}

	solution.resize(num_nodes);
	error.resize(num_nodes);
	abserror.resize(num_nodes);
	rhs.resize(num_nodes);

	K_matrix.ClearResize(num_nodes);
}

void FEModel::AssembleStiffnessMatrix() {
	for (int i = 0; i < num_elems; i++)
		elements[i].AssembleElement(this);
}

void FEModel::SetBoundaryConditions() {
	for (int i = 0; i < num_nodes; i++) {
		const Vector2 &pos = GetNodePosition(i);

		if (pos[0] <= 0.0 || pos[0] >= 1.0 || pos[1] <= 0.0 || pos[1] >= 1.0) {
			double x = pos[0];
			double y = pos[1];

			double val = Boundary_u(x, y);

			boundaryConds.push_back(BoundaryCondition(i, val));
		}
	}
}

//Not sure if this is correct, didn't know where to get that N_j(x_c, y_c) from.
void FEModel::ComputeRHS() {
	// task 3

	//this is incorrect
	//what we need to do is:
	// - initialize rhs with zeroes
	// - for every node n_j, check every triangle e if it contains n_j
	// - if it does, add A_e * f(c_e.x(), c_e.y()) * N_j(c_e.x(), c_e.y())

	//In theory N_j is only dependent on n_j (it is it's basis function)
	//but in this practical case, the element (triangle) we are in determines
	//where to find the correct PART of the basis function (ie. the one for the barycenter of e).
	//This part of the basis function needed for e is conveniently saved in e.
	for (unsigned int i = 0; i < rhs.size(); i++)
		rhs[i] = 0;

	for (unsigned int j = 0; j < nodes.size(); j++) {
		for (LinTriElement e : elements) {
			if (e.contains(j)) {
				Vector2 center = e.GetCenter(this);
				rhs[j] += Source_Term_f(center.x(), center.y())
						* e.GetArea(this) * e.evaluateN(this, j);
			}
		}
	}

//	for (int i = 0; i < K_matrix.GetNumCols(); i++) {
//		Vector2 center = elements[i].GetCenter(this);
//		rhs[i] = Source_Term_f(center.x(), center.y())
//				* elements[i].GetArea(this); // * N_j(center.x, center.y), there should be something missing, but it works ?!
//	}
}

void FEModel::Solve() {
	vector<double> tmp_rhs = rhs;
	SparseSymmetricMatrix tmp_K_matrix = K_matrix;

	/* Adjust K matrix to accommodate for known values of u on boundary */
	for (int i = 0; i < (int) boundaryConds.size(); i++)
		tmp_K_matrix.FixSolution(tmp_rhs, boundaryConds[i].GetID(),
				boundaryConds[i].GetValue());

	SparseLinSolverPCGT<double> solver;
	/* Use preconditioned conjugate gradient solver, with residual 1e-6, and
	 maximum number of iterations 1000 */
	solver.SolveLinearSystem(tmp_K_matrix, solution, tmp_rhs, (double) 1e-6,
			1000);
}

double FEModel::ComputeError() {
	double err_nrm = 0.0;

	for (int i = 0; i < num_nodes; i++) {
		const Vector2 &pos = GetNodePosition(i);
		error[i] = Boundary_u(pos[0], pos[1]) - solution[i];
	}

	abserror = error;
	for (int i = 0; i < num_nodes; i++)
		abserror[i] = fabs(abserror[i]);

	/* Compute inner product error norm:  err = sqrt(v*K*v) */

	// Task 4
	return err_nrm;
}

void FEModel::Render(int toggle_vis) {
	vector<double> data;

	/* Select data to display */
	if (toggle_vis)
		data = abserror;
	else
		data = solution;

	double maxValue = 0;
	for (int i = 0; i < (int) data.size(); i++)
		maxValue = std::max(data[i], maxValue);

	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < num_elems; i++) {
			for (int j = 0; j < 3; j++) {
				int nodeID = elements[i].GetGlobalID(j);

				const Vector2 &pos = GetNodePosition(nodeID);
				double val = data[nodeID] / maxValue;

				/* Map values in interval to HSV hue range (blue = 0, red = max) */
				double s = 1.0;
				double v = 1.0;

				if (val < 0.0) {
					val = 0.0;
					v = 0.0;
				}
				if (val > 1.0) {
					val = 1.0;
					v = 359.0;
				}

				double h = (1.0 - val) * 240.0;

				double r, g, b;
				r = g = b = 0.0;
				HSV2RGB(h, s, v, r, g, b);

				glColor3f(r, g, b);
				glVertex3f(pos[0], pos[1], 0);
			}
		}
	}
	glEnd();

	/* Overlay triangle edges as black lines */
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	{
		for (int i = 0; i < num_elems; i++) {
			for (int j = 0; j < 3; j++) {
				int nodeID1 = elements[i].GetGlobalID(j);
				int nodeID2 = elements[i].GetGlobalID((j + 1) % 3);

				const Vector2 &pos1 = GetNodePosition(nodeID1);
				const Vector2 &pos2 = GetNodePosition(nodeID2);

				glVertex3f(pos1[0], pos1[1], 0);
				glVertex3f(pos2[0], pos2[1], 0);
			}
		}
	}
	glEnd();
}

