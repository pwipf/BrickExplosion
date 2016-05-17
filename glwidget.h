#ifndef GLWIDGET_H
#define GLWIDGET_H

#define GLM_FORCE_RADIANS

#define MAJORVNUM 3
#define MINORVNUM 3
#define QOGLVER QOpenGLFunctions_3_3_Core

#define NSPINAXES 7
#define NSPINSPDS 13

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QTimer>
#include <glm/glm.hpp>
#include <iostream>
#include <mesh.h>


using glm::mat4;
using glm::vec3;
using std::vector;
using std::cout;
using std::endl;

class GLWidget : public QOpenGLWidget, protected QOGLVER {
    Q_OBJECT

    public:
        GLWidget(QWidget *parent=0);

    private slots:
        void onCheckLines(int b);
        void onCheckGrid(int b);
        void onCheckNormals(int b);
        void onCheckMortar(int b);
        void onCheckBrickFlat(int b);
        void onCheckLight(int b);
        void onChangeSubdivides(int val);
        void onChangeBrickWidth(double val);
        void onChangeBrickDepth(double val);
        void onChangeBrickHeight(double val);
        void onChangeBrickRadius(double val);
        void onChangeBrickSpace(double val);
        void onChangeBrickRough(double val);
        void onChangeBricksPerRow(int val);
        void onChangeRows(int val);
        void onChangeWallShape(int val);

        void onChangeAlpha(int val);
        void onChangeDiffuse(int val);
        void onChangeSpecular(int val);
        void onChangeAmbient(int val);
        void onChangeLightX(int val);
        void onChangeLightY(int val);
        void onChangeLightZ(int val);
        void onClickLightColor();

    public slots:
        void animate();

    private:
        int checkLines;
        int checkGrid;
        int checkNormals;
        int renderMortar=1;
        int renderRoof=1;
        int renderFloor=1;
        int checkLight;
        int subdivides;

        float brickWidth;
        float brickDepth;
        float brickHeight;
        float brickSpace;
        float brickRough;
        float brickRadius;
        int brickFlatShade;
        int bricksPerRow;
        int rows;

        float inset;
        int mac=0;

        vec3 brickColor;
        vec3 mortarColor;

        enum WallShape{SINGLE,CIRCLE,SQUARE};
        WallShape wallShape;

        void rebuildBrick(vec3 col);
        void buildMortar();
        void rebuildNormalMarks(Mesh mesh);
        void initAxes();
        void initializeGrid();
        glm::vec2 buildWall(float xs, float zs, float xf, float zf, int isStart, int isFinish, int startHeight, int height, int extMort);
        void buildHouse();
        void generateRing(Mesh &r, float inRad, float outRad);
        void generateGround();
        void generateFloor();
        void rebuildGeometry();
        void generateSimpleMortar(float angle, float x, float z, int startRow, int rows, int ext);
        void generateCircularMortar(float radius);
        void scaleBrick();
        void unScaleBrick();
        void updateWallBuffers();
        void insertBrick(mat4 t);
        void updateLight();

        void updateViewMat();
        void generateLight();
        void animateRing();
        void brickExplosion();


        vec3 spinAxes[NSPINAXES];
        float spinSpeeds[NSPINSPDS];


        void testForStart();

        void loadBox();
        void initSky();
        void renderSky();
        GLuint skyVbo,skyVao;
        GLuint skyTex;
        void create_cube_map_1file(const char* boxbmp,GLuint* tex_cube);
        void create_cube_map(const char* front,const char* back,const char* top,
                              const char* bottom,const char* left,const char* right,GLuint* tex_cube);
        bool load_cube_map(GLuint texture, const char* file_name);
        bool load_cube_map_side(GLuint texture, GLenum side_target, const char* file_name);

    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

        glm::vec2 w2dcSquare(const glm::vec2 &pt);

    private:
        GLuint loadShaders(const char* vertf, const char* fragf);
        unsigned char* loadImg(const char * path, int &x, int &y);
        void initMeshes();

        GLuint programU,programI,programS,programT,programBox;
        GLint projMatrixLocU,projMatrixLocI,projMatrixLocS,projMatrixLocT;
        GLint viewMatrixLocU,viewMatrixLocI,viewMatrixLocS,viewMatrixLocT;
        GLint modelMatrixLocU,modelMatrixLocI,modelMatrixLocS,modelMatrixLocT;
        GLint rotMatLocBox;
        mat4 projMatrix;
        mat4 viewMatrix;
        mat4 lightModelMatrix;
        mat4 modelMatrix;

        int width;
        int height;

        glm::vec3 lastVPt;
        glm::vec3 pointOnVirtualTrackball(const glm::vec2 &pt);

        GLuint restart;

        GLint lightLocU;
        GLint lightCLocU;
        GLint spotOnLocU;
        GLint spotDirLocU;
        GLint gattenLocU;

        GLint lightLocI;
        GLint lightCLocI;
        GLint spotOnLocI;
        GLint spotDirLocI;
        GLint gattenLocI;

        GLint lightLocT;
        GLint lightCLocT;
        GLint alphaLocT;
        GLint spotOnLocT;
        GLint spotDirLocT;
        GLint gattenLocT;

        float gatten;//global light attenuation

        GLint spotOn;
        vec3 spotDir;

        float alpha,Kd,Ks,Ka;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
        float zoom;

        glm::vec3 lightCenter;

        vec3 lookAngle;

        InstancedMesh brick;
        InstancedMesh mortar;
        Mesh light;

        Mesh ring1;
        Mesh ring2;
        Mesh ring3;
        Mesh ring4;
        Mesh ring5;
        vec3 ringLoc;
        vec3 ringColor;

        float ringSpeed;
        int ringStart=0,ringStop=0,ringArmed=0,startDay=0,finishDarken=0,finishedBrighten=1;
        int darkenSky=0,brickExplode=0,lightFollow=0,finishedRebuild=0;

        SimpleTexMesh ground;
        SimpleTexMesh floor;
        SimpleTexMesh roof;

        float skyBrightness=0;
        GLint skyBrightLoc;

        //BoxMesh sky;

        LineMesh grid;
        LineMesh normalMarks;
        LineMesh axes;

        QBasicTimer keyTimer;
        int actionKeys[4]={Qt::Key_W,Qt::Key_S,Qt::Key_A,Qt::Key_D};
        int keyTimerID;

        int time;

        QBasicTimer animTimer;
        int animTimerInt;
        int animTimerID;

        QTimer* timer;
        std::map<int,bool> keys;
        vec3 eyePos;
        float eyeHeight;
        vec3 eyeVel;
        bool flyMode;

        mat4 matPitch; //eye pitch
        mat4 matYaw; //eye yaw
        mat4 matTrans; //eye translation
        vec3 right;
        vec3 forward;
        float angX;
        float angY;

        float mouseRateX;
        float mouseRateY;
        float moveSpeed;
        glm::vec2 lastPt;

        std::vector<vec2> wallplan;
        int flashlight;
        float flashlightHeight=-.2f;
        int finished=0;

        int timerSpeed;
        int pause=0;

        vec3 roofVel;
        vec3 roofPos;
        int roofOnGround=0;
};

#endif // GLWIDGET_H
