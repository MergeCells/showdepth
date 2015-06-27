#include <stdlib.h>
#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  include "glut.h"
#  include "glext.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

/*
** Depth Peeling �̃��C���[
*/
static int layer = 0;

/*
** ����
*/
static const GLfloat lightpos[] = { 4.0, 9.0, 5.0, 1.0 }; /* �ʒu�@�@�@�@�@�@�@ */
static const GLfloat lightcol[] = { 1.0, 1.0, 1.0, 1.0 }; /* ���ڌ����x�@�@�@�@ */
static const GLfloat lightamb[] = { 0.1, 0.1, 0.1, 1.0 }; /* �������x�@�@�@�@ */

/*
** �e�N�X�`��
*/
#define TEXWIDTH  1024//512                                     /* �e�N�X�`���̕��@�@ */
#define TEXHEIGHT 1024//512                                     /* �e�N�X�`���̍����@ */

/*
**STL�ǂݍ���
*/
// STL�f�[�^�̃t�@�C�����D
//#define STL_FILE "sample.stl"
#define STL_FILE "iPhone_Basic.stl"
//#define STL_FILE "T01_FINISH.stl"
//#define STL_FILE "FINISH_Ascii.stl"

// STL�t�@�C���̓ǂݍ��݁D
extern bool loadSTLFile(const char* STL_file);


/*
** ������
*/
static void init(void)
{
	/* �e�N�X�`���̊��蓖�ā|���̃e�N�X�`����O���̃f�v�X�o�b�t�@�Ƃ��Ďg�� */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEXWIDTH, TEXHEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	/* �e�N�X�`�����g��E�k��������@�̎w�� */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* �e�N�X�`���̌J��Ԃ����@�̎w�� */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* �������ރ|���S���̃e�N�X�`�����W�l�̂q�ƃe�N�X�`���Ƃ̔�r���s���悤�ɂ��� */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	/* �����q�̒l���e�N�X�`���̒l�𒴂��Ă�����^�i�t���O�����g��`���j */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

	/* ��r�̌��ʂ��A���t�@�l�Ƃ��ē��� */
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);

	/* �A���t�@�e�X�g�̔�r�֐��i�������l�j */
	glAlphaFunc(GL_GEQUAL, 0.5f);

	/* �e�N�X�`�����W�Ɏ��_���W�n�ɂ����镨�̂̍��W�l��p���� */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	/* ���������e�N�X�`�����W�����̂܂� (S, T, R, Q) �Ɏg�� */
	static const GLdouble genfunc[][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]);
	glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
	glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
	glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);

	/* �����ݒ� */
	glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	/* �����̏����ݒ� */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);

	/* Polygon Offset �̐ݒ� */
	glPolygonOffset(1.0f, 1.0f);
}


/****************************
** GLUT �̃R�[���o�b�N�֐� **
****************************/

/* �g���b�N�{�[�������p�֐��̐錾 */
#include "trackball.h"

/* �V�[����`���֐��̐錾 */
#include "scene.h"


extern void displayModel(void);

static void display(void)
{
	/*STL�̕\��*/
	//displayModel();


	GLdouble projection[16]; /* �����ϊ��s��̕ۑ��p */
	int i;

	/* ���f���r���[�ϊ��s��̐ݒ� */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* ���_�̈ʒu��ݒ肷��i���̂̕������Ɉړ�����j*/
	glTranslated(0.0, 0.0, -10.0);

	/* �g���b�N�{�[�����̉�]��^���� */
	glMultMatrixd(trackballRotation());

	/* �����̈ʒu��ݒ肷�� */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	/* �e�N�X�`���ϊ��s���ݒ肷�� */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	/* �e�N�X�`�����W�� [-1,1] �͈̔͂� [0,1] �͈̔͂Ɏ��߂� */
	glTranslated(0.5, 0.5, 0.5);
	glScaled(0.5, 0.5, 0.5);

	/* ���݂̓����ϊ��s������o�� */
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	/* �����ϊ��s����e�N�X�`���ϊ��s��ɐݒ肷�� */
	glMultMatrixd(projection);

	/* ���f���r���[�ϊ��s��ɖ߂� */
	glMatrixMode(GL_MODELVIEW);

	/* �f�v�X�o�b�t�@���O�ŃN���A */
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* �f�v�X�o�b�t�@�̏����l���P�ɖ߂� */
	glClearDepth(1.0);

	/* �e�N�X�`���}�b�s���O�ƃe�N�X�`�����W�̎���������L���ɂ��� */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	/* �A���t�@�e�X�g��L���ɂ��� */
	glEnable(GL_ALPHA_TEST);

	/* Polygon Offset ��L���ɂ��� */
	glEnable(GL_POLYGON_OFFSET_FILL);

	for (i = 0; i <= layer; ++i) {

		/* �f�v�X�o�b�t�@���f�v�X�e�N�X�`���ɃR�s�[ */
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, TEXWIDTH, TEXHEIGHT);

		/* �t���[���o�b�t�@�ƃf�v�X�o�b�t�@���N���A���� */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* �V�[����`�悷�� */
		scene();
	}

	/* Polygon Offset �𖳌��ɂ��� */
	glDisable(GL_POLYGON_OFFSET_FILL);

	/* �A���t�@�e�X�g�𖳌��ɂ��� */
	glDisable(GL_ALPHA_TEST);

	/* �e�N�X�`���}�b�s���O�ƃe�N�X�`�����W�̎��������𖳌��ɂ��� */
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	glDisable(GL_TEXTURE_2D);

	/* �_�u���o�b�t�@�����O */
	glutSwapBuffers();
}

static void resize(int w, int h)
{
	/* �E�B���h�E�T�C�Y�̊g��k���𐧌����� */
	if (w != TEXWIDTH || h != TEXHEIGHT) {
		if (w != TEXWIDTH) w = TEXWIDTH;
		if (h != TEXHEIGHT) h = TEXHEIGHT;
		glutReshapeWindow(w, h);
	}

	/* �g���b�N�{�[������͈� */
	trackballRegion(w, h);

	/* �E�B���h�E�S�̂��r���[�|�[�g�ɂ��� */
	glViewport(0, 0, w, h);

	/* �����ϊ��s��̎w�� */
	glMatrixMode(GL_PROJECTION);

	/* �����ϊ��s��̏����� */
	glLoadIdentity();
	gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 3.0, 15.0);
}

static void idle(void)
{
	/* ��ʂ̕`���ւ� */
	glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			/* �g���b�N�{�[���J�n */
			trackballStart(x, y);
			glutIdleFunc(idle);
			break;
		case GLUT_UP:
			/* �g���b�N�{�[����~ */
			trackballStop(x, y);
			glutIdleFunc(0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

static void motion(int x, int y)
{
	/* �g���b�N�{�[���ړ� */
	trackballMotion(x, y);
}



// �\�����[�h�D
#define DISPLAY_MODEL 0

static void keyboard(unsigned char key, int x, int y)
{
	if (key >= '0' && key <= '9') {
		/* '0'�`'9' �̃L�[�� Depth Peeling �̃��C���[�ύX */
		layer = key - '0';
		glutPostRedisplay();
	}
	else {
		switch (key) {
		case 'q':
		case 'Q':
		case '\033':
			/* ESC �� q �� Q ���^�C�v������I�� */
			exit(0);
			break;
			/*
			// STL�t�@�C���̓ǂݍ��݁D
			case 'r':
			case 'R':
			if (!loadSTLFile(STL_FILE))
			break;
			//display_mode = DISPLAY_MODEL;
			glutPostRedisplay();
			break;

			// STL���f���̕\���D
			case 'd':
			case 'D':
			//display_mode = DISPLAY_MODEL;
			glutPostRedisplay();
			break;
			*/
		default:
			break;
		}
	}
}

/*
** ���C���v���O����
*/
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(TEXWIDTH, TEXHEIGHT);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	init();
	glutMainLoop();
	return 0;
}
