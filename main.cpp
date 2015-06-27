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
** Depth Peeling のレイヤー
*/
static int layer = 0;

/*
** 光源
*/
static const GLfloat lightpos[] = { 4.0, 9.0, 5.0, 1.0 }; /* 位置　　　　　　　 */
static const GLfloat lightcol[] = { 1.0, 1.0, 1.0, 1.0 }; /* 直接光強度　　　　 */
static const GLfloat lightamb[] = { 0.1, 0.1, 0.1, 1.0 }; /* 環境光強度　　　　 */

/*
** テクスチャ
*/
#define TEXWIDTH  1024//512                                     /* テクスチャの幅　　 */
#define TEXHEIGHT 1024//512                                     /* テクスチャの高さ　 */

/*
**STL読み込み
*/
// STLデータのファイル名．
//#define STL_FILE "sample.stl"
#define STL_FILE "iPhone_Basic.stl"
//#define STL_FILE "T01_FINISH.stl"
//#define STL_FILE "FINISH_Ascii.stl"

// STLファイルの読み込み．
extern bool loadSTLFile(const char* STL_file);


/*
** 初期化
*/
static void init(void)
{
	/* テクスチャの割り当て－このテクスチャを前方のデプスバッファとして使う */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEXWIDTH, TEXHEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	/* テクスチャを拡大・縮小する方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* テクスチャの繰り返し方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* 書き込むポリゴンのテクスチャ座標値のＲとテクスチャとの比較を行うようにする */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	/* もしＲの値がテクスチャの値を超えていたら真（フラグメントを描く） */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

	/* 比較の結果をアルファ値として得る */
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);

	/* アルファテストの比較関数（しきい値） */
	glAlphaFunc(GL_GEQUAL, 0.5f);

	/* テクスチャ座標に視点座標系における物体の座標値を用いる */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

	/* 生成したテクスチャ座標をそのまま (S, T, R, Q) に使う */
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

	/* 初期設定 */
	glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	/* 光源の初期設定 */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);

	/* Polygon Offset の設定 */
	glPolygonOffset(1.0f, 1.0f);
}


/****************************
** GLUT のコールバック関数 **
****************************/

/* トラックボール処理用関数の宣言 */
#include "trackball.h"

/* シーンを描く関数の宣言 */
#include "scene.h"


extern void displayModel(void);

static void display(void)
{
	/*STLの表示*/
	//displayModel();


	GLdouble projection[16]; /* 透視変換行列の保存用 */
	int i;

	/* モデルビュー変換行列の設定 */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* 視点の位置を設定する（物体の方を奥に移動する）*/
	glTranslated(0.0, 0.0, -10.0);

	/* トラックボール式の回転を与える */
	glMultMatrixd(trackballRotation());

	/* 光源の位置を設定する */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	/* テクスチャ変換行列を設定する */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	/* テクスチャ座標の [-1,1] の範囲を [0,1] の範囲に収める */
	glTranslated(0.5, 0.5, 0.5);
	glScaled(0.5, 0.5, 0.5);

	/* 現在の透視変換行列を取り出す */
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	/* 透視変換行列をテクスチャ変換行列に設定する */
	glMultMatrixd(projection);

	/* モデルビュー変換行列に戻す */
	glMatrixMode(GL_MODELVIEW);

	/* デプスバッファを０でクリア */
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* デプスバッファの消去値を１に戻す */
	glClearDepth(1.0);

	/* テクスチャマッピングとテクスチャ座標の自動生成を有効にする */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	/* アルファテストを有効にする */
	glEnable(GL_ALPHA_TEST);

	/* Polygon Offset を有効にする */
	glEnable(GL_POLYGON_OFFSET_FILL);

	for (i = 0; i <= layer; ++i) {

		/* デプスバッファをデプステクスチャにコピー */
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, TEXWIDTH, TEXHEIGHT);

		/* フレームバッファとデプスバッファをクリアする */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* シーンを描画する */
		scene();
	}

	/* Polygon Offset を無効にする */
	glDisable(GL_POLYGON_OFFSET_FILL);

	/* アルファテストを無効にする */
	glDisable(GL_ALPHA_TEST);

	/* テクスチャマッピングとテクスチャ座標の自動生成を無効にする */
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	glDisable(GL_TEXTURE_2D);

	/* ダブルバッファリング */
	glutSwapBuffers();
}

static void resize(int w, int h)
{
	/* ウィンドウサイズの拡大縮小を制限する */
	if (w != TEXWIDTH || h != TEXHEIGHT) {
		if (w != TEXWIDTH) w = TEXWIDTH;
		if (h != TEXHEIGHT) h = TEXHEIGHT;
		glutReshapeWindow(w, h);
	}

	/* トラックボールする範囲 */
	trackballRegion(w, h);

	/* ウィンドウ全体をビューポートにする */
	glViewport(0, 0, w, h);

	/* 透視変換行列の指定 */
	glMatrixMode(GL_PROJECTION);

	/* 透視変換行列の初期化 */
	glLoadIdentity();
	gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 3.0, 15.0);
}

static void idle(void)
{
	/* 画面の描き替え */
	glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			/* トラックボール開始 */
			trackballStart(x, y);
			glutIdleFunc(idle);
			break;
		case GLUT_UP:
			/* トラックボール停止 */
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
	/* トラックボール移動 */
	trackballMotion(x, y);
}



// 表示モード．
#define DISPLAY_MODEL 0

static void keyboard(unsigned char key, int x, int y)
{
	if (key >= '0' && key <= '9') {
		/* '0'～'9' のキーで Depth Peeling のレイヤー変更 */
		layer = key - '0';
		glutPostRedisplay();
	}
	else {
		switch (key) {
		case 'q':
		case 'Q':
		case '\033':
			/* ESC か q か Q をタイプしたら終了 */
			exit(0);
			break;
			/*
			// STLファイルの読み込み．
			case 'r':
			case 'R':
			if (!loadSTLFile(STL_FILE))
			break;
			//display_mode = DISPLAY_MODEL;
			glutPostRedisplay();
			break;

			// STLモデルの表示．
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
** メインプログラム
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
