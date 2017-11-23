/******************************************************************
 *
 * LinTriElement.cpp
 *
 * Description: Implementation of functions for handling linear
 * triangular elements
 *
 * Physically-Based Simulation Proseminar WS 2015
 *
 * Interactive Graphics and Simulation Group
 * Institute of Computer Science
 * University of Innsbruck
 *
 *******************************************************************/

#include <math.h>
#include <stdio.h>
#include "FEModel.h"

double LinTriElement::area = nan("");

//Didn't see a reason to use this function when basis-function coefficients are computed
// as shown in PS slides
void LinTriElement::ComputeBasisDeriv(const FEModel *model) {
	//task 1

	//Get (x1,y1), (x2,y2) and (x3,y3)
	Vector2 pos1 = model->GetNodePosition(GetGlobalID(0));
	Vector2 pos2 = model->GetNodePosition(GetGlobalID(1));
	Vector2 pos3 = model->GetNodePosition(GetGlobalID(2));

	//Arrange them in matrix for solving linear equation system as in PS Slides
	Matrix3x3 positionMat = Matrix3x3();
	positionMat.Zero();
	positionMat(0, 0) = 1.0;
	positionMat(0, 1) = pos1.x();
	positionMat(0, 2) = pos1.y();
	positionMat(1, 0) = 1.0;
	positionMat(1, 1) = pos2.x();
	positionMat(1, 2) = pos2.y();
	positionMat(2, 0) = 1.0;
	positionMat(2, 1) = pos3.x();
	positionMat(2, 2) = pos3.y();

	//Solve the system. This only works with unsafe Inversion, maybe not correct...
	coefficients = positionMat.Inverse();

	//read off partial derivatives of basis function.
	derivatives[0] = Vector2(coefficients(1, 0), coefficients(2, 0));
	derivatives[1] = Vector2(coefficients(1, 1), coefficients(2, 1));
	derivatives[2] = Vector2(coefficients(1, 2), coefficients(2, 2));
}

double LinTriElement::GetArea(FEModel *model) const {
	//This is a special case optimization for this model, where every triangle has the same area
	if (std::isnan(LinTriElement::area)) {
		Vector2 pos1 = model->GetNodePosition(GetGlobalID(0));
		Vector2 pos2 = model->GetNodePosition(GetGlobalID(1));
		Vector2 pos3 = model->GetNodePosition(GetGlobalID(2));

		//shoelace formula
		LinTriElement::area =
				0.5
						* fabs(
								((pos1.x() - pos3.x()) * (pos2.y() - pos1.y()))
										- ((pos1.x() - pos2.x())
												* (pos3.y() - pos1.y())));
	}
	return LinTriElement::area;
}

Vector2 LinTriElement::GetCenter(FEModel *model) {
	if (std::isnan(center.x())) {
		Vector2 pos1 = model->GetNodePosition(GetGlobalID(0));
		Vector2 pos2 = model->GetNodePosition(GetGlobalID(1));
		Vector2 pos3 = model->GetNodePosition(GetGlobalID(2));

		center = (pos1 + pos2 + pos3) / 3;
	}

	return center;
}

void LinTriElement::AssembleElement(FEModel *model) {
	//task 2
	double area = GetArea(model);
	ComputeBasisDeriv(model);

	//this is the inner sum of the i,j't entry in the stiffness matrix.
	// outer sum is the repeated call of this function. This should be correct.
	double val;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			val = (derivatives[i].x() * derivatives[j].x()
					+ derivatives[i].y() * derivatives[j].y()) * area;

			//Had to change that function, see definition
			model->AddToStiffnessMatrix(GetGlobalID(i), GetGlobalID(j), val);
		}
	}
}

double LinTriElement::evaluateN(FEModel *model, int globalID) {
	int index = 0;
	for (; index < 3; index++)
		if (nodeID[index] == globalID)
			break;

	if(index==4)
		return 0.;

	Vector2 center = GetCenter(model);
	return coefficients(0, index) + coefficients(1, index) * center.x()
			+ coefficients(2, index) * center.y();

}
