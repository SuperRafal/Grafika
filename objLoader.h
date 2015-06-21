/*
* obj.c -- obj model loader
* last modification: apr. 2, 2011
*
* Copyright (c) 2005-2007 David HENRY
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* gcc -Wall -ansi -lGL -lGLU -lglut obj.c -o obj
*/

#include "szescian.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




/* Vectors */
typedef float vec3_t[3];
typedef float vec4_t[4];

/* Vertex */
struct obj_vertex_t
{
	vec4_t xyzw;
};

/* Texture coordinates */
struct obj_texCoord_t
{
	vec3_t uvw;
};

/* Normal vector */
struct obj_normal_t
{
	vec3_t ijk;
};

/* Polygon */
struct obj_face_t
{
	GLenum type;        /* primitive type */
	int num_elems;      /* number of vertices */

	int *vert_indices;  /* vertex indices */
	int *uvw_indices;   /* texture coordinate indices */
	int *norm_indices;  /* normal vector indices */
};

/* OBJ model structure */
struct obj_model_t
{
	int num_verts;                     /* number of vertices */
	int num_texCoords;                 /* number of texture coords. */
	int num_normals;                   /* number of normal vectors */
	int num_faces;                     /* number of polygons */

	int has_texCoords;                 /* has texture coordinates? */
	int has_normals;                   /* has normal vectors? */

	struct obj_vertex_t *vertices;     /* vertex list */
	struct obj_texCoord_t *texCoords;  /* tex. coord. list */
	struct obj_normal_t *normals;      /* normal vector list */
	struct obj_face_t *faces;          /* model's polygons */
	GLuint *texture[1];

	/* Position */

	float pos_x;
	float pos_y;
	float pos_z;
	float angle;
}gasienice, kadlub, lufa, wieza, trawa, ziemia, box, drzewo;

void SetPosition(struct obj_model_t *mdl, float x, float y, float z);

void SetRotate(struct obj_model_t *mdl, float angle);

/*** An OBJ model ***/
struct obj_model_t objfile;


/**
* Free resources allocated for the model.
*/
void FreeModel(struct obj_model_t *mdl);

/**
* Allocate resources for the model after first pass.
*/
int MallocModel(struct obj_model_t *mdl);
/**
* Load an OBJ model from file -- first pass.
* Get the number of triangles/vertices/texture coords for
* allocating buffers.
*/
int FirstPass(FILE *fp, struct obj_model_t *mdl);

/**
* Load an OBJ model from file -- first pass.
* This time, read model data and feed buffers.
*/
int SecondPass(FILE *fp, struct obj_model_t *mdl);
/**
* Load an OBJ model from file, in two passes.
*/
int ReadOBJModel(const char *filename, struct obj_model_t *mdl);

/**
* Draw the OBJ model.
*/

void RenderOBJModel(struct obj_model_t *mdl);
/*
void
init(const char *filename)
{
	GLfloat lightpos[] = { 5.0f, 10.0f, 0.0f, 1.0f };

	// Initialize OpenGL context
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	// Load OBJ model file
	if (!ReadOBJModel(filename, &objfile))
		exit(EXIT_FAILURE);
}*/
/*
void
cleanup()
{
	FreeModel(&objfile);
}
*/
/*
void
reshape(int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, w / (GLdouble)h, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutPostRedisplay();
}
*/
/*
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -15.0f);

	// Draw the model 
	RenderOBJModel(&objfile);

	glutSwapBuffers();
}*/

/*void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) //escape
		exit(0);
}*/

/*int
main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <filename.obj>\n", argv[0]);
		return -1;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("OBJ Model");

	atexit(cleanup);
	init(argv[1]);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}*/