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

void Reshape(int width, int height) {
	glViewport(0, 0, width, height);
}

void Idle(void) {
	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {

}

void Simulate() {
	scene.Solve(1);
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);

	Simulate();

	scene.Render();

	glutPostRedisplay();
	glutSwapBuffers();

	//prevent it from looping nonstop
	sleep(1);
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
	glutIdleFunc(Idle);

	glutMainLoop();

	return EXIT_SUCCESS;
}
