#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  include "glut.h"
#  include "glext.h"
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#include "glsl.h"

#define CHECK_FRAMEBUFFER_STATUS() \
{ \
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); \
  switch(status) { \
  case GL_FRAMEBUFFER_COMPLETE_EXT: \
    break; \
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT: \
    fprintf(stderr, "choose different formats\n"); \
    break; \
  default: \
    fprintf(stderr, "programming error; will fail on all hardware: %04x\n", status); \
    break; \
    } \
}

/*
** Depth Peeling のレイヤー
*/
static int layer = 0;

/*
** テクスチャの大きさ
*/
#define TEXWIDTH  1024                                     /* テクスチャの幅　　 */
#define TEXHEIGHT 1024                                     /* テクスチャの高さ　 */

/*
** テクスチャオブジェクト・フレームバッファオブジェクト
*/
static GLuint tex, fb;

/*
** シェーダオブジェクト
*/
static GLuint vertShader;
static GLuint fragShader;
static GLuint gl2Program;

/*
** 初期化
*/
static void init(void)
{
	/* シェーダプログラムのコンパイル／リンク結果を得る変数 */
	GLint compiled, linked;

#if defined(WIN32)
	glGenFramebuffersEXT =
		(PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glBindFramebufferEXT =
		(PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glFramebufferTexture2DEXT =
		(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glGenRenderbuffersEXT =
		(PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glBindRenderbufferEXT =
		(PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glRenderbufferStorageEXT =
		(PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glFramebufferRenderbufferEXT =
		(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glCheckFramebufferStatusEXT =
		(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
#endif

	/* テクスチャオブジェクトの生成 */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* デプステクスチャの割り当て */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEXWIDTH, TEXHEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	/* テクスチャを拡大・縮小する方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* テクスチャの繰り返し方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


	/* ここからshowdepth由来 */

	/* テクスチャオブジェクトの結合解除 */
	glBindTexture(GL_TEXTURE_2D, 0);

	/* フレームバッファオブジェクトの生成 */
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	/* フレームバッファオブジェクトにデプステクスチャを結合 */
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_TEXTURE_2D, tex, 0);

	/* カラーバッファを読み書きしない */
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	CHECK_FRAMEBUFFER_STATUS();

	/* フレームバッファオブジェクトの結合解除 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	/* GLSL の初期化 */
	if (glslInit()) exit(1);

	/* シェーダオブジェクトの作成 */
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	/* シェーダのソースプログラムの読み込み */
	if (readShaderSource(vertShader, "showdepth.vert")) exit(1);
	if (readShaderSource(fragShader, "showdepth.frag")) exit(1);

	/* バーテックスシェーダのソースプログラムのコンパイル */
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(vertShader);
	if (compiled == GL_FALSE) {
		fprintf(stderr, "Compile error in vertex shader.\n");
		exit(1);
	}

	/* フラグメントシェーダのソースプログラムのコンパイル */
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
	printShaderInfoLog(fragShader);
	if (compiled == GL_FALSE) {
		fprintf(stderr, "Compile error in fragment shader.\n");
		exit(1);
	}

	/* プログラムオブジェクトの作成 */
	gl2Program = glCreateProgram();

	/* シェーダオブジェクトのシェーダプログラムへの登録 */
	glAttachShader(gl2Program, vertShader);
	glAttachShader(gl2Program, fragShader);

	/* シェーダオブジェクトの削除 */
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	/* シェーダプログラムのリンク */
	glLinkProgram(gl2Program);
	glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
	printProgramInfoLog(gl2Program);
	if (linked == GL_FALSE) {
		fprintf(stderr, "Link error.\n");
		exit(1);
	}

	/* テクスチャユニット０を指定する */
	glUniform1i(glGetUniformLocation(gl2Program, "depth"), 0);


	/* ここからdepthpeel由来 */

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

//peel由来
extern void displayModel(void);

static void display(void)
{
	GLint viewport[4];       /* ビューポートの保存用　　　　 */

	/* 現在のビューポートを保存しておく */
	glGetIntegerv(GL_VIEWPORT, viewport);

	/* ビューポートをテクスチャのサイズに設定する */
	glViewport(0, 0, TEXWIDTH, TEXHEIGHT);

	/* モデルビュー変換行列の設定 */
	glLoadIdentity();

	/* 視点の位置を設定する（物体の方を奥に移動する）*/
	glTranslated(0.0, 0.0, -10.0);

	/* トラックボール式の回転を与える */
	glMultMatrixd(trackballRotation());

	/* フレームバッファオブジェクトへのレンダリング開始 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	/* デプステストを有効にする */
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i <= layer; ++i) {

		/* デプスバッファをデプステクスチャにコピー */
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, TEXWIDTH, TEXHEIGHT);

		/* フレームバッファとデプスバッファをクリアする */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* シーンを描画する */
		scene();

	}

	/* フレームバッファオブジェクトへのレンダリング終了 */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	/* ビューポートを元に戻す */
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	/* デプステストを無効にする */
	glDisable(GL_DEPTH_TEST);

	/* テクスチャオブジェクトを結合する */
	glBindTexture(GL_TEXTURE_2D, tex);

	/* シェーダプログラムを適用する */
	glUseProgram(gl2Program);

	/* 全画面に四角形を描く */
	glBegin(GL_QUADS);
	glVertex2d(0.0, 0.0);
	glVertex2d(1.0, 0.0);
	glVertex2d(1.0, 1.0);
	glVertex2d(0.0, 1.0);
	glEnd();

	/* シェーダプログラムを解除する */
	glUseProgram(0);

	/* テクスチャオブジェクトの結合を解除する */
	glBindTexture(GL_TEXTURE_2D, 0);

	/* ダブルバッファリング */
	glutSwapBuffers();
}

static void resize(int w, int h)
{
	/* トラックボールする範囲 */
	trackballRegion(w, h);

	/* ウィンドウ全体をビューポートにする */
	glViewport(0, 0, w, h);

	/* 透視変換行列の設定 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, (GLdouble)w / (GLdouble)h, 5.0, 20.0);

	/* モデルビュー変換行列に切り替える */
	glMatrixMode(GL_MODELVIEW);
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

static void keyboard(unsigned char key, int x, int y)
{
	if (key >= '0' && key <= '9') {
		/* '0'～'9' のキーで Depth Peeling のレイヤー変更 */
		layer = key - '0';
		glutPostRedisplay();
	}

	switch (key) {
	case 'q':
	case 'Q':
	case '\033':
		/* ESC か q か Q をタイプしたら終了 */
		exit(0);
	default:
		break;
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