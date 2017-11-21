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


//Didn't see a reason to use this function when basis-function coefficients are computed
// as shown in PS slides
void LinTriElement::ComputeBasisDeriv(int nodeId, Vector2 &basisDeriv, const FEModel *model) const
{

}

//TODO: Only compute once
double LinTriElement::GetArea(FEModel *model) const {
    Vector2 pos1 = model->GetNodePosition(this->GetGlobalID(0));
    Vector2 pos2 = model->GetNodePosition(this->GetGlobalID(1));
    Vector2 pos3 = model->GetNodePosition(this->GetGlobalID(2));

    //shoelace formula
    return 0.5 * fabs(((pos1.x()-pos3.x()) * (pos2.y()-pos1.y())) - ((pos1.x()-pos2.x()) * (pos3.y()-pos1.y())));
}

Vector2 LinTriElement::GetCenter(FEModel *model) const {
    Vector2 pos1 = model->GetNodePosition(this->GetGlobalID(0));
    Vector2 pos2 = model->GetNodePosition(this->GetGlobalID(1));
    Vector2 pos3 = model->GetNodePosition(this->GetGlobalID(2));

    return (pos1 + pos2 + pos3)/3;
}

void LinTriElement::AssembleElement(FEModel *model) const
{   
    //Get (x1,y1), (x2,y2) and (x3,y3)
    Vector2 pos1 = model->GetNodePosition(this->GetGlobalID(0));
    Vector2 pos2 = model->GetNodePosition(this->GetGlobalID(1));
    Vector2 pos3 = model->GetNodePosition(this->GetGlobalID(2));

    
    //Arrange them in matrix for solving linear equation system as in PS Slides
    Matrix3x3 positionMat = Matrix3x3();
    positionMat.Zero();
    positionMat(0,0) = 1.0;
    positionMat(0,1) = pos1.x();
    positionMat(0,2) = pos1.y();
    positionMat(1,0) = 1.0;
    positionMat(1,1) = pos2.x();
    positionMat(1,2) = pos2.y();
    positionMat(2,0) = 1.0;
    positionMat(2,1) = pos3.x();
    positionMat(2,2) = pos3.y();

    //Solve the system. This only works with unsafe Inversion, maybe not correct...
    Matrix3x3 coefficents = positionMat.Inverse();

    //read off partial derivatives of basis function. 
    Vector2 basis1_nabla = Vector2(coefficents(1,0), coefficents(2,0));
    Vector2 basis2_nabla = Vector2(coefficents(1,1), coefficents(2,1));
    Vector2 basis3_nabla = Vector2(coefficents(1,2), coefficents(2,2));
    Vector2 all[3] = {basis1_nabla, basis2_nabla, basis3_nabla};

    //actually always the same area, TODO: only compute once
    double area = this->GetArea(model);

    //this is the inner sum of the i,j't entry in the stiffness matrix.
    // outer sum is the repeated call of this function. This should be correct.
    double val;
    for(int i=0; i<3; i++){
        for (int j=0; j<3; j++){
            val = (all[i].x()*all[j].x() + all[i].y()*all[j].y())*area;

        
            //Had to change that function, see definition
            model->AddToStiffnessMatrix(this->GetGlobalID(i),this->GetGlobalID(j), val);
        }
    }
}

