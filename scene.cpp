#include <math.h>

#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "scene.h"

/*
** �^�C���̕`��
*/
static void tile(double w, double d, int nw, int nd)
{
  /* �^�C���̐F */
  static const GLfloat color[][4] = {
    { 0.6, 0.6, 0.6, 1.0 },
    { 0.3, 0.3, 0.3, 1.0 }
  };
  
  int i, j;

  glNormal3d(0.0, 1.0, 0.0);
  glBegin(GL_QUADS);
  for (j = 0; j < nd; ++j) {
    GLdouble dj = d * j, djd = dj + d;

    for (i = 0; i < nw; ++i) {
      GLdouble wi = w * i, wiw = wi + w;

      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color[(i + j) & 1]);
      glVertex3d(wi,  0.0, dj);
      glVertex3d(wi,  0.0, djd);
      glVertex3d(wiw, 0.0, djd);
      glVertex3d(wiw, 0.0, dj);
    }
  }
  glEnd();
}

/*
** ���̕`��
*/
static void box(double x, double y, double z)
{
  /* ���_�f�[�^ */
  const GLdouble vertex[][3] = {
    { 0.0, 0.0, 0.0 },
    {   x, 0.0, 0.0 },
    {   x,   y, 0.0 },
    { 0.0,   y, 0.0 },
    { 0.0, 0.0,   z },
    {   x, 0.0,   z },
    {   x,   y,   z },
    { 0.0,   y,   z },
  };
  
  /* �ʃf�[�^ */
  static const GLdouble *face[][4] = {
    { vertex[0], vertex[1], vertex[2], vertex[3] },
    { vertex[1], vertex[5], vertex[6], vertex[2] },
    { vertex[5], vertex[4], vertex[7], vertex[6] },
    { vertex[4], vertex[0], vertex[3], vertex[7] },
    { vertex[4], vertex[5], vertex[1], vertex[0] },
    { vertex[3], vertex[2], vertex[6], vertex[7] },
  };
  
  /* �ʂ̖@���x�N�g�� */
  static const GLdouble normal[][3] = {
    { 0.0, 0.0,-1.0 },
    { 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0 },
    {-1.0, 0.0, 0.0 },
    { 0.0,-1.0, 0.0 },
    { 0.0, 1.0, 0.0 },
  };
  
  /* ���̐F */
  static const GLfloat color[] = { 0.8, 0.8, 0.2, 1.0 };
  
  int i, j;
  
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
  glBegin(GL_QUADS);
  for (j = 0; j < 6; j++) {
    glNormal3dv(normal[j]);
    for (i = 4; --i >= 0;) {
      glVertex3dv(face[j][i]);
    }
  }
  glEnd();
}

/*
** �V�[���̕`��
*/
void scene(void)
{
  static const GLfloat red[] = { 0.8, 0.2, 0.2, 1.0 };

  /* �^�C����`�� */
  glPushMatrix();
  glTranslated(-3.0, -2.0, -3.0);
  tile(1.0, 1.0, 6, 6);
  glPopMatrix();
  
  /* ����`�� */
  glPushMatrix();
  glTranslated(-1.0, -1.5, -1.0);
  box(2.0, 1.0, 2.0);
  glPopMatrix();
  
  /* ����`�� */
  glPushMatrix();
  glTranslated(1.0, 1.0, 1.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  glutSolidTeapot(1.0);
  glPopMatrix();
}
