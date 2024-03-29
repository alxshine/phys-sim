/*
 * Pipe.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: bambusbjorn
 */

#include <GL/freeglut.h>
#include "Scene.h"

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

Scene scene;
bool needToRun = true;
int winWidth, winHeight;
bool rendPressure = true;
bool scaleVelocities = false;

void Reshape(int width, int height) {
	glViewport(0, 0, width, height);
	winWidth = width;
	winHeight = height;
}

void Idle(void) {
	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'p':
		rendPressure = !rendPressure;
		break;
	case 's':
		scaleVelocities = !scaleVelocities;
		break;
	}
}

void Mouse(int button, int state, int x, int y) {
//calculate position in projection space
	double projectionX = (x - winWidth / 2) / (double) winWidth * 6;
//the origin is in the top left corner, so y has to be reversed
	double projectionY = (winHeight / 2 - y) / (double) winWidth * 6;

	scene.HandleMouse(projectionX, projectionY);
	needToRun = true;
}

void Simulate() {
	if (needToRun) {
		scene.Solve(1, scaleVelocities);
		sleep(0.1);
//		needToRun = false;
	}
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);

	Simulate();

	scene.Render(rendPressure);

	glutPostRedisplay();
	glutSwapBuffers();

	sleep(0.1);
}

void Init(void) {
	/* Set background (clear) color to black */
	glClearColor(0.0, 0.0, 0.0, 0.0);

	/* Set appropriate mode for specifying projection matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* Use orthographic projection */
	glOrtho(-3.0, 3.0, -3.0, 3.0, -3.0, 3.0);

	/* Back to modelview mode for object rendering */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Pipe project");

	Init();
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutIdleFunc(Idle);

	glutMainLoop();

	return EXIT_SUCCESS;
}
