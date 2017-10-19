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

int halfStep = 0;
int initialized = 0;

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
                if (p.isFixed())
                    continue;

                //reset force to gravity
                p.setForce(Vec2(0, -9.81 * p.getMass()));
                p.addForce(-p.getVel() * p.getDamping());

                //calculate penalty force for collision
                double penaltyStiffness = 1000;
                double groundHeight = -2;
                if (p.getPos().y < groundHeight) {
                    double penetrationDepth = p.getPos().y - groundHeight;
                    Vec2 penaltyForce = Vec2(0, 1) * penaltyStiffness * -penetrationDepth;
                    p.addForce(penaltyForce);
                }
            }

            /**
             * Add spring force
             * We also apply a seperate internal spring damping force, for more realism
             */
            for (Spring &s : springs) {
                s.applyForce();
                s.applyDamping();
            }

            for (Point &p : points) {
                if (p.isFixed()) {
                    continue;
                }
                //update position
                p.setPos(p.getPos() + p.getVel() * dt);

                Vec2 a = p.getForce() / p.getMass();
                Vec2 v = p.getVel() + a * dt;

                p.setVel(v);

            }

            break;
        }

        case Scene::SYMPLECTIC: {
            break;
        }

        case Scene::LEAPFROG: {
            /**
             * Update velocity for every t+h/2, and position and acceleration for every t
             * IMPORTANT: because the step time is halved (in MassSpring.cpp), h/2 == dt and h==2*dt
             */
            if (halfStep) {
                //this saves me from using an if (which should be minimally faster)
                dt += initialized * dt;
                initialized |= 1;

                for (Point &p : points) {
                    if (p.isFixed()) {
                        continue;
                    }

                    Vec2 a = p.getForce() / p.getMass();


                    p.setVel(p.getVel() + dt * a);
                }
            } else {
                /**
                 * We need to approximate the velocity at the current point in time t.
                 * In order to not mess our velocity updates up, we need to save and restore the velocity for t-h/2.
                 */
                Vec2 oldVelocities[points.size()];

                //update positions
                for (int i = 0; i < points.size(); i++) {
                    Point &p = points[i];
                    p.setPos(p.getPos() + p.getVel() * 2 * dt);
                    oldVelocities[i] = p.getVel();

                    //approximate velocity
                    Vec2 a = p.getForce() / p.getMass();
                    p.setVel(p.getVel() + a * dt);

                    //set forces to gravity
                    p.setForce(Vec2(0, -9.81) * p.getMass());
                    p.addForce(-p.getVel() * p.getDamping());

                    //calculate penalty force for collision
                    double penaltyStiffness = 1000;
                    double groundHeight = -2;
                    if (p.getPos().y < groundHeight) {
                        double penetrationDepth = p.getPos().y - groundHeight;
                        Vec2 penaltyForce = Vec2(0, 1) * penaltyStiffness * -penetrationDepth;
                        p.addForce(penaltyForce);
                    }
                }

                /**
                 * Add spring force
                 * We also apply a seperate internal spring damping force, for more realism
                 */
                for (Spring &s : springs) {
                    s.applyForce();
                    s.applyDamping();
                }

                /**
                 * Restore velocities
                 */
                for (int i = 0; i < points.size(); i++)
                    points[i].setVel(oldVelocities[i]);
            }

            halfStep = !halfStep;
            break;
        }

        case Scene::MIDPOINT: {
            break;
        }
    }
}       

