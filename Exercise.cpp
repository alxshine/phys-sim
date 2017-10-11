/******************************************************************
*
* Exercise.cpp  
*
* Description: In this file - in the function TimeStep() - the various 
* numerical solvers of the first programming assignment have to be
* implemented. Feel free to add new local functions into this file. 
* Changes to other source files of the framework should not be 
* required. It is acceptable to assume knowledge about the scene
* topology (i.e. how springs and points are connected).
*
* Physically-Based Simulation Proseminar WS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
*******************************************************************/

/* Standard includes */
#include <iostream>
#include <vector>

using namespace std;

/* Local includes */
#include "Vec2.h"
#include "Point.h"
#include "Scene.h"


/******************************************************************
*
* TimeStep
*
* This function is called every time step of the dynamic simulation.
* Positions, velocities, etc. should be updated to simulate motion
* of the mass points of the 2D scene. The selected solver is passed
* to the function as well as the time step, the spings, and the 
* mass points. 
*
*******************************************************************/

void TimeStep(double dt, Scene::Method method,
              vector<Point> &points, vector<Spring> &springs, bool interaction) {
    switch (method) {
        case Scene::EULER: {
            /**
             * The following things need to be calculated:
             * position for t+h
             * forces for t
             * acceleration for t
             * velocity for t+h
             */

            for (Point &p : points) {
                if (!p.isFixed())
                    p.setPos(p.getPos() + p.getVel() * dt);

                //reset force to gravity
                p.setForce(Vec2(0, 9.81 * p.getMass()));
                p.addForce(-p.getVel()*p.getDamping());
            }

            //add spring force
            for (Spring &s : springs)
                s.applyForce();

            for (Point &p : points) {
                Vec2 a = p.getForce() / p.getMass();
                Vec2 v = p.getVel() + a * dt;
                p.setVel(v);
            }

            //TODO: add collision
            break;
        }

        case Scene::SYMPLECTIC: {
            break;
        }

        case Scene::LEAPFROG: {
            break;
        }

        case Scene::MIDPOINT: {
            break;
        }
    }
}       

