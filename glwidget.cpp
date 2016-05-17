/***************************************************************************************
 *
 * Copyright © Philip Wipf
 * Please use this source code any way you like.
 *
 * (This software is designed to dynamically links to Qt which is
 * licensed under the LGPL license.
 * The source code for Qt SDK can be found at www.qt.io)
 *
 **************************************************************************************/

// glwidget.cpp
// This file has the main program logic, starting with the constructor for the GLWidget
// class, which initializes some instance variables.
//
// The main functions are the initializeGL(), resizeGL(), and paintGL() callbacks which
// are called at the appropriate times by the framework.
//
// Also included are the "slot" functions called when user interface controls are adjusted,
// "onCheckLines()" is an example, which is called if the "Lines Only" checkbox is clicked.
//
// There are several functions which generate various geometries, called from the initializeGL()
// function.
//
// The keyboard and mouse response are taken care of in the functions mouseMoveEvent(),
// keyPressEvent(), etc.
//
// The animation is handled by the animate() function which is called by a timer on a regular
// interval, 16ms.  The timer is started in the constructor.
//
// There are a few functions that are not used, like pointOnVirtualTrackball(), which were
// waiting for more features or left over from old features.


#include "glwidget.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include <QTextStream>
#include <QColorDialog>

#include <iostream>


using glm::inverse;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::perspective;
using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;
using glm::rotate;
using glm::rotateX;
using glm::rotateZ;
using glm::translate;
using glm::scale;
using glm::value_ptr;
using glm::lookAt;
using std::cout;
using std::endl;
using std::vector;


// constructor for GLWidget class, which fills the main "widget" of the window.
GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {

    mac=0;

    restart = 0xFFFFFFFF;
    checkLines=0;
    checkGrid=1;
    checkNormals=0;
    renderMortar=1;
    subdivides=4;

    brickWidth=2;
    brickHeight=0.7f;
    brickDepth=1;
    brickSpace=.2f;
    brickRadius=.0f;

    brickSpace=0.1f;
    brickRough=.5f;
    inset=0.05f;
    bricksPerRow=9;
    rows=9;

    wallShape=SINGLE;

    brickColor=vec3(255/255.0f,100/255.0f,15/255.0f);
    mortarColor=vec3(1.0f,1.0f,1.0f);
    ringColor=vec3(255/255.0f,196/255.0f,0);

    flashlight=1;

    eyeHeight=2; // eye height when feet on ground!
    eyePos=vec3(18,eyeHeight,-11);
    viewMatrix = mat4(1.0f);
    angX=M_PI;
    angY=0;

    matPitch=rotate(mat4(1.0f),angY,vec3(1,0,0));
    matYaw=rotate(mat4(1.0f),angX,vec3(0,1,0));
    matTrans=translate(mat4(1.0f),eyePos);

    lightPosition=eyePos;//vec3(0,7,-5);
    lightCenter=lightPosition;
    lightColor=vec3(1,.7f,.7f);
    alpha=100;
    Kd=0.8f;
    Ks=0.7f;
    Ka=0.2f;

    spotOn=1;
    spotDir=vec3(0,0,1);


    gatten=1;


    brickFlatShade=0;
    modelMatrix = mat4(1.0f);

    timerSpeed=16;


    mouseRateX=.002f;
    mouseRateY=.002f;
    moveSpeed=0.2f;
    timer=new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer,SIGNAL(timeout()),this,SLOT(animate()));
    timer->start(16);
    flyMode=false;

    ringLoc=vec3(0,2.4f,0);
    ringSpeed=0;
    ringStart=0;
    ringStop=0;

    for(int i=0;i<NSPINAXES;i++){
        spinAxes[i]=glm::sphericalRand(1.0f);
    }
    for(int i=0;i<NSPINSPDS;i++){
        spinSpeeds[i]=glm::linearRand(.05f,.1f);
    }

    roofVel=vec3(.1,.4,.02);

}

// slots for various user interface controls, many non-existent, but there to help me
// remember how to "do that type of control" again.
void GLWidget::onChangeAlpha(int val){
    ring1.material.shinyness = val/10.0f;
    cout<<"alph "<<val/10.0f<<endl;
    update();
}

void GLWidget::onChangeDiffuse(int val){
    ring1.material.diffuse=val/10.0f;
    cout<<"dif "<<val/10.0f<<endl;
    update();
}
void GLWidget::onChangeSpecular(int val){
    ring1.material.specular=val/10.0f;
    cout<<"spec "<<val/10.0f<<endl;
    update();
}
void GLWidget::onChangeAmbient(int val){
    ring1.material.ambient=val/10.0f;
    cout<<"amb "<<val/10.0f<<endl;
    update();
}
void GLWidget::onChangeLightX(int val){

    lightPosition.x=-val/10.0f;
    light.modelMatrix=translate(mat4(1.0f),lightPosition);
    updateLight();
    update();
}
void GLWidget::onChangeLightY(int val){
    lightPosition.y=val/10.0f;
    light.modelMatrix=translate(mat4(1.0f),lightPosition);
    updateLight();
    update();
}
void GLWidget::onChangeLightZ(int val){
    lightPosition.z=val/10.0f;;
    light.modelMatrix=translate(mat4(1.0f),lightPosition);
    updateLight();
    update();
}
void GLWidget::onClickLightColor(){
    QColor c = QColorDialog::getColor(QColor(lightColor.x*255,lightColor.y*255,lightColor.z*255),this,"Select Specular Color");
    if(c.isValid()){
        lightColor=vec3(c.redF(),c.greenF(),c.blueF());
        generateLight();
        updateLight();
        update();
    }
}

void GLWidget::onCheckLines(int b){
    checkLines=b;
    update();
}
void GLWidget::onCheckGrid(int b){
    checkGrid=b;
    update();
}
void GLWidget::onCheckLight(int b){
    checkLight=b;
    update();
}
void GLWidget::onCheckNormals(int b){
    checkNormals=b;
    //rebuildNormalMarks(ring);
    update();
}
void GLWidget::onCheckMortar(int b){
    renderMortar=b;
    buildHouse();
}
void GLWidget::onCheckBrickFlat(int b){
    brickFlatShade=b;
    rebuildGeometry();
}
void GLWidget::onChangeSubdivides(int val){
    subdivides=val;
    rebuildGeometry();
}
void GLWidget::onChangeBrickRadius(double val){
    brickRadius=val;
    rebuildGeometry();
}

void GLWidget::onChangeBrickWidth(double val){
    unScaleBrick();
    brickWidth=val;
    scaleBrick();
    brick.updateBuffers();
    buildHouse();
}
void GLWidget::onChangeBrickDepth(double val){
    unScaleBrick();
    brickDepth=val;
    scaleBrick();
    brick.updateBuffers();
    buildHouse();
}
void GLWidget::onChangeBrickHeight(double val){
    unScaleBrick();
    brickHeight=val;
    scaleBrick();
    brick.updateBuffers();
    buildHouse();
}
void GLWidget::onChangeBrickSpace(double val){
    brickSpace=val;
    buildHouse();
}
void GLWidget::onChangeBrickRough(double val){
    brickRough=val;
    if(val>=.8f)
        inset=.05f+.1f*(val-.8f);
    rebuildGeometry();
}
void GLWidget::onChangeBricksPerRow(int val){
    bricksPerRow=val;
    buildHouse();
}
void GLWidget::onChangeRows(int val){
    rows=val;
    buildHouse();
}
void GLWidget::onChangeWallShape(int val){
    wallShape=(WallShape)val;
    buildHouse();
}


// rebuildGeometry()
// central function to call when rebuilding geometry after changes to bricks, etc.
void GLWidget::rebuildGeometry(){

    rebuildBrick(brickColor);
    buildMortar();
    //    if(checkNormals){
    //        rebuildNormalMarks();
    //    }

    //    for(uint bp=0;bp<brickPts.size();bp++){
    //        pts.push_back(brickPts[bp]);
    //        colors.push_back(brickColors[bp]);
    //        normals.push_back(brickNormals[bp]);
    //    }

    //    for(uint bi=0;bi<brickIdx.size();bi++){
    //        idx.push_back(brickIdx[bi]);
    //    }
    //nBrickIdx=brickIdx.size();

    //brickInstancePos.push_back(mat4(1.0f));

    //    updateWallBuffers();

     buildHouse();
    //if(checkNormals)
    //    rebuildNormalMarks(ring);
}


// geometry generation functions.
// most (all?) objects are members of my Mesh class, which has built-in generation functions.

void GLWidget::generateRing(Mesh &r, float inRad, float outRad){
    r.clearVertices();
    r.generateRing(ringColor,60,inRad,outRad);
    r.computeNormals(1);
    r.scale(vec3(1,.1f,1));

    r.transform(glm::rotate(mat4(),(float)M_PI/2,vec3(1,0,0)));

    r.updateBuffers();

    r.translationMatrix=glm::translate(mat4(),ringLoc);
    r.modelMatrix=r.translationMatrix;

//    rebuildNormalMarks(ring);
//    normalMarks.modelMatrix=glm::translate(normalMarks.modelMatrix,vec3(0,2,-7));
//    normalMarks.modelMatrix= glm::rotate(normalMarks.modelMatrix,(float)M_PI/2,vec3(1,0,0));

}

// very manual method to create the ground which is just two triangles with texture coordinates
// and normals.
void GLWidget::generateGround(){
    ground.clearVertices();

    ground.addPt(vec3(10,0,10));
    ground.addPt(vec3(10,0,-10));
    ground.addPt(vec3(-10,0,-10));
    ground.addPt(vec3(-10,0,10));
    ground.addNorm(vec3(0,1,0));
    ground.addNorm(vec3(0,1,0));
    ground.addNorm(vec3(0,1,0));
    ground.addNorm(vec3(0,1,0));
    ground.addColor(vec3(0,.6f,0));
    ground.addColor(vec3(0,.6f,0));
    ground.addColor(vec3(0,.6f,0));
    ground.addColor(vec3(0,.6f,0));
    GLuint idx[]={0,1,3,3,1,2};
    ground.addIdx(idx,6);
    ground.addUV(vec2(0,500));
    ground.addUV(vec2(0,0));
    ground.addUV(vec2(500,0));
    ground.addUV(vec2(500,500));
    ground.scale(50,0);
    ground.updateBuffers();

    ground.material.specular=0;
}

// generate floor (and roof!)
void GLWidget::generateFloor(){
    floor.clearVertices();

    floor.generateCube(.5f);
    floor.computeNormals(1);
    floor.scale(.5f,0);
    floor.translate(vec3(0,.5f,0));
    floor.scale(vec3(19,.1f,13),1);
    floor.translate(vec3(3,0,0));
    floor.updateBuffers();

    //floor.modelMatrix=scale(mat4(),vec3(22,.1f,16));

    floor.material.specular=.4f;

    float height = rows*(brickHeight+brickSpace)+(brickHeight+brickSpace)/2;
    roof.clearVertices();
    roof.generateCube(.3f);
    roof.computeNormals(1);
    roof.scale(.5f,0);
    roof.modelMatrix=translate(mat4(),vec3(3,height,0));
    //roof.translate(vec3(0,height,0));
    roof.scale(vec3(22,1,16),1);
    //roof.translate(vec3(3,0,0));
    roof.updateBuffers();
    roof.material.specular=.5f;
}

void GLWidget::generateLight(){
    light.clearVertices();
    light.generateCube(lightColor,1);
    light.subdivide(3);
    light.normalizePts(0.3f);
    light.computeNormals(0);
    light.updateBuffers();

    lightModelMatrix=translate(modelMatrix,lightPosition);
}


// build the house. calls the buildWall function for each section of wall, using the
// return coordinate to start the next wall section.
void GLWidget::buildHouse(){

    generateFloor();
    brick.clearInstances();
    mortar.clearInstances();
    vec2 fin=buildWall(6,-2,6,8,1,0,0,rows-2,1);
    fin = buildWall(fin.x,fin.y,-8,fin.y,0,0,0,rows-2,1);
    fin = buildWall(fin.x,fin.y,fin.x,-8,0,0,0,rows-2,1);
    fin = buildWall(fin.x,fin.y,14,fin.y,0,0,0,rows-2,1);
    fin = buildWall(fin.x,fin.y,fin.x,8,0,0,0,rows-2,1);
    fin = buildWall(fin.x,fin.y,10,fin.y,0,1,0,rows-2,1);

    fin=buildWall(6,-2,6,8,1,0,rows-2,rows,0);
    fin = buildWall(fin.x,fin.y,-8,fin.y,0,0,rows-2,rows,0);
    fin = buildWall(fin.x,fin.y,fin.x,-8,0,0,rows-2,rows,0);
    fin = buildWall(fin.x,fin.y,14,fin.y,0,0,rows-2,rows,0);
    fin = buildWall(fin.x,fin.y,fin.x,8,0,0,rows-2,rows,0);
    fin = buildWall(fin.x,fin.y,0,fin.y,0,0,rows-2,rows,0);

    fin = buildWall(22,-14,22,14,0,0,0,rows-5,0);
    fin = buildWall(fin.x,fin.y,-14,fin.y,0,0,0,rows-5,0);
    fin = buildWall(fin.x,fin.y,fin.x,-15,0,0,0,rows-5,0);
    fin = buildWall(fin.x,fin.y,22,fin.y,0,0,0,rows-5,0);
}

// buildWall takes various parameters to build a section of brick wall by adding instances
// of bricks, and instances of mortar.
// The return value is the point that the end of the wall ended up at, which is generally
// different than the end point which was asked for, xf and zf.  The return value can then
// be used for the start of the next wall section so that it will line up properly (or not
// so properly)
// xs, zs, xf, zf are the start and finish x and z coordinates of the wall section.
// isStart and isFinish determine whether to put little half bricks in the offsets, to make
// a straight (vertical) wall end, or leave the rows offset to join to the next section.
// height is the number of rows of bricks to add.
// startHeight is the row to start with, allowing wall sections that start above a door or window.
// extMort is a flag to make the mortar inset at the bottom as well as the sides and top. A bit
// of a hack.
vec2 GLWidget::buildWall(float xs, float zs, float xf, float zf,
                         int isStart, int isFinish,int startHeight, int height, int extMort){
    int rows=height;
    float len=glm::sqrt((xf-xs)*(xf-xs)+(zf-zs)*(zf-zs));
    if(len<brickWidth)
        cout<<"too short wall"<<endl;

    //brick.clearInstances();

    float wallAngle=-glm::atan((zf-zs),(xf-xs));


    float bricklen=brickWidth+brickSpace;
    bricksPerRow=(len/bricklen);

    vec4 f(bricksPerRow*bricklen,0,0,0);
    f=glm::rotateY(f,wallAngle);
    vec2 finish(f.x+xs,f.z+zs);

    mat4 transform;
    mat4 temp,temp2;

    int isRowOffset=0;
    float rowOffset=(brickWidth+brickSpace)/2;
    float halfBrick=.5f*(brickWidth/(brickWidth+brickSpace));

    switch(wallShape){
        case SQUARE:
        case SINGLE:
            for(int y=startHeight;y<rows;y++){
                isRowOffset=y%2;

                for(int x=0;x<bricksPerRow;x++){
                    transform = translate(mat4(), vec3(x*bricklen+bricklen/4,y*(brickHeight+brickSpace)+brickHeight/2,0));

                    if(isRowOffset && x==0 && isStart){
                        temp=scale(translate(transform,vec3(-0.5f*(brickWidth-(brickWidth*halfBrick)),0,0)),vec3(halfBrick,1,1));
                        temp2=translate(mat4(),vec3(xs,0,zs));
                        temp2=rotate(temp2,wallAngle,vec3(0,1,0));
                        temp=temp2*temp;
                        insertBrick(temp);
                    }

                    if(y%2)
                        transform=translate(transform,vec3(rowOffset,0,0));

                    temp2=translate(mat4(),vec3(xs,0,zs));
                    temp2=rotate(temp2,wallAngle,vec3(0,1,0));
                    transform=temp2*transform;
                    //transform=translate(transform,vec3(xs,0,zs));
                    insertBrick(transform);

                    if(!isRowOffset && x==bricksPerRow-1 && isFinish){
                        mat4 temp=scale(translate(transform,vec3(0.5f*(brickWidth-(brickWidth*halfBrick))+rowOffset,0,0)),vec3(halfBrick,1,1));
                        insertBrick(temp);
                    }
                }
            }
            generateSimpleMortar(wallAngle,xs,zs,startHeight,rows,extMort);
            break;

        case CIRCLE:
            brickSpace/=3.0f;
            float circum=bricksPerRow*(brickWidth+brickSpace);
            float radius= circum/(2*(float)M_PI)+brickDepth/2;
            for(int y=0;y<rows;y++){
                isRowOffset=y%2;
                float angle = 0;
                float da=2*M_PI/bricksPerRow;
                for(int x=0;x<bricksPerRow;x++){
                    transform = rotate(mat4(),angle+da*x+isRowOffset*da/2,vec3(0,1,0));
                    transform = translate(transform,
                                          vec3( radius, y*(brickHeight+brickSpace)+brickHeight/2 , 0));

                    insertBrick(rotate(transform,(float)M_PI/2,vec3(0,1,0)));
                }
            }

            generateCircularMortar(radius);
            brickSpace*=3.0f;
            break;
    }

    return finish;
}


void GLWidget::insertBrick(mat4 t){
    brick.addInstance(t);
}

void GLWidget::generateCircularMortar(float radius){

    float outRadius=radius+brickDepth/2.3f;
    float inRadius =radius-brickDepth/4;
    float height = rows*brickHeight+(rows-1)*brickSpace-brickDepth*inset;

    mortar.clearVertices();
    mortar.generateRing(mortarColor,60,inRadius,outRadius);
    mortar.scale(vec3(1,height,1));
    mortar.translate(vec3(0,height/2,0));

    mortar.updateBuffers();
}

void GLWidget::generateSimpleMortar(float angle,float x,float z, int startRow, int rows, int ext){
    float totalWidth=bricksPerRow*brickWidth + (bricksPerRow-1)*brickSpace
            +(brickWidth+brickSpace)/2 - brickDepth*inset-inset;
    float totalDepth=brickDepth-(brickDepth*inset)*2;
    float fullHeight =rows*brickHeight+(rows-1)*brickSpace-brickDepth*inset;
    float totalHeight=(rows-startRow)*brickHeight+((rows-startRow)-1)*brickSpace-brickDepth*inset;
    float halfBrick=.5f*(brickWidth/(brickWidth+brickSpace));

    if(ext){
        totalHeight+=brickHeight;
        fullHeight+=brickHeight;
    }

    float f=0;
    if(startRow !=0){
        f=inset;
    }

    mat4 trans1=translate(mat4(),vec3(1,1,0));
    mat4 scl=scale(mat4(),.5f*vec3(totalWidth,totalHeight,totalDepth));
    mat4 trans2=translate(mat4(),vec3(-halfBrick+inset,fullHeight-totalHeight+f,0));
    mat4 rot=rotate(mat4(),angle,vec3(0,1,0));
    mat4 trans3=translate(mat4(),vec3(x,0,z));

    mortar.addInstance(trans3*rot*trans2*scl*trans1);
}

void GLWidget::buildMortar(){
    mortar.clearVertices();
    mortar.generateCube(mortarColor,1);
    mortar.makeFlatShade();
    mortar.computeNormals(1);
    mortar.updateBuffers();
}

void GLWidget::rebuildBrick(vec3 col){
    brick.clearVertices();
    brick.generateCube(col,2);
    brick.subdivide(subdivides);
    brick.roundEdges(brickRadius);
    //compute normals for to classify points as which side they are on
    brick.computeNormals(1);
    brick.roughen(brickRough,subdivides);

    // compute normals AGAIN
    // makeFlatShade also computes normals as it goes through the vertices
    if(brickFlatShade)
        brick.makeFlatShade();
    else
        brick.computeNormals(1);

    // rescale to 1 unit
    // should have made the cube function create a 1x1 instead but changing it
    // messes up the roughness calculations
    brick.scale(0.5f);

    //make the brick the right size according to the user inputs
    scaleBrick();
    brick.updateBuffers();

    brick.material.specular=.3f;
    brick.material.shinyness=25;
}


// at some point i rendered little lines showing the normals at vertices, for
// troubleshooting/figureing-out.
void GLWidget::rebuildNormalMarks(Mesh mesh){
    normalMarks.clearVertices();
    vector<vec3> pts,colors;
    vec3 c(1,1,1);
    for(uint i=0;i<mesh.getNumVerts();i++){
        pts.push_back(mesh.ptAt(i));
        pts.push_back(mesh.ptAt(i)+mesh.normAt(i)*.2f);
    }
    //mat4 t=brick.getInstanceMat(0);
    for(uint i=0;i<pts.size();i++){
        //pts[i]=vec3(t*vec4(pts[i],1));
        colors.push_back(c);
    }

    normalMarks.copyVertices(pts,colors);
    normalMarks.updateBuffers();


}

void GLWidget::unScaleBrick(){
    vec3 s(brickWidth,brickHeight,brickDepth);
    brick.scale(1.0f/s);
}

void GLWidget::scaleBrick(){
    vec3 s(brickWidth,brickHeight,brickDepth);
    brick.scale(s);
}

void GLWidget::updateLight(){

    if(flashlight){
        lightPosition=eyePos;
        lightPosition.y+=flashlightHeight;//.2f;

    }else if(lightFollow)
        lightPosition=eyePos+forward+vec3(0,3,0);

    vec3 fwd=vec3(viewMatrix*(-(matYaw*matPitch)[2]));

    light.modelMatrix=translate(mat4(1.0f),lightPosition);

    glUseProgram(programU);
    glUniform3fv(lightLocU,1,value_ptr(vec3(viewMatrix*vec4(lightPosition,1))));
    glUniform3fv(lightCLocU,1,value_ptr(lightColor));
    glUniform1i(spotOnLocU,spotOn);
    glUniform3fv(spotDirLocU,1,value_ptr(fwd));
    glUniform1f(gattenLocU,gatten);


    glUseProgram(programI);
    glUniform3fv(lightLocI,1,value_ptr(vec3(viewMatrix*vec4(lightPosition,1))));
    glUniform3fv(lightCLocI,1,value_ptr(lightColor));
    glUniform1i(spotOnLocI,spotOn);
    glUniform3fv(spotDirLocI,1,value_ptr(fwd));
    glUniform1f(gattenLocI,gatten);

    glUseProgram(programT);
    glUniform3fv(lightLocT,1,value_ptr(vec3(viewMatrix*vec4(lightPosition,1))));
    glUniform3fv(lightCLocT,1,value_ptr(lightColor));
    glUniform1i(spotOnLocT,spotOn);
    glUniform3fv(spotDirLocT,1,value_ptr(fwd));
    glUniform1f(gattenLocT,gatten);
}

// axes not used, just makes a little object with green red and blue axes to let me know
// which way is which, for troubleshooting/figureing-out.
void GLWidget::initAxes(){
    vector<vec3>pts,colors;
    vec3 o(0,1,0);
    vec3 cx(1,.3f,.3f);
    vec3 cy(.3f,1,.3f);
    vec3 cz(.3f,.3f,1);

    pts.push_back(o);
    pts.push_back(o+vec3(1,0,0));
    colors.push_back(cx);
    colors.push_back(cx);

    pts.push_back(o);
    pts.push_back(o+vec3(0,1,0));
    colors.push_back(cy);
    colors.push_back(cy);

    pts.push_back(o);
    pts.push_back(o+vec3(0,0,1));
    colors.push_back(cz);
    colors.push_back(cz);

    axes.clearVertices();
    axes.copyVertices(pts,colors);
    axes.updateBuffers();

}


// the main initialization function to initialize and generate the objects. Also loads shader
// programs.
//
// The Mesh classes need a pointer to the openGL context, which is passed to each of them with
// the .init() function.  The QOGLVER macro makes it a little easier to change the version
// of openGL being used.
//
// The meshes are also .initialized() with shader programs and locations of various uniforms in
// those programs.  It's probably an awful way to do this but is what I have done so far.
void GLWidget::initMeshes() {
    brick.init((QOGLVER*)this);
    mortar.init((QOGLVER*)this);
    light.init((QOGLVER*)this);
    grid.init((QOGLVER*)this);
    normalMarks.init((QOGLVER*)this);
    axes.init((QOGLVER*)this);

    ring1.init((QOGLVER*)this);
    ring2.init((QOGLVER*)this);
    ring3.init((QOGLVER*)this);
    ring4.init((QOGLVER*)this);
    ring5.init((QOGLVER*)this);

    ground.init((QOGLVER*)this);
    floor.init((QOGLVER*)this);
    roof.init((QOGLVER*)this);

    programU=loadShaders(":/vert_uninstanced.glsl", ":/frag.glsl");
    glUseProgram(programU);

    projMatrixLocU = glGetUniformLocation(programU, "projection");
    viewMatrixLocU = glGetUniformLocation(programU, "view");
    modelMatrixLocU = glGetUniformLocation(programU, "model");

    lightLocU = glGetUniformLocation(programU,"lightP");
    lightCLocU = glGetUniformLocation(programU,"lightC");
    GLint alphaLocU=glGetUniformLocation(programU,"alpha");
    GLint kdLocU=glGetUniformLocation(programU,"Kd");
    GLint ksLocU=glGetUniformLocation(programU,"Ks");
    GLint kaLocU=glGetUniformLocation(programU,"Ka");
    GLint sColLocU=glGetUniformLocation(programU,"specColor");
    GLint attLocU=glGetUniformLocation(programU,"attenuation");
    GLint attSLocU=glGetUniformLocation(programU,"attenuationS");
    GLint glowLocU=glGetUniformLocation(programU,"glow");
    spotOnLocU=glGetUniformLocation(programU,"spot");
    spotDirLocU=glGetUniformLocation(programU,"spotDir");
    gattenLocU=glGetUniformLocation(programU,"gatten");


    programI=loadShaders(":/vert_instanced.glsl", ":/frag.glsl");
    glUseProgram(programI);
    projMatrixLocI = glGetUniformLocation(programI, "projection");
    viewMatrixLocI = glGetUniformLocation(programI, "view");
    modelMatrixLocI = glGetUniformLocation(programI, "model");

    lightLocI = glGetUniformLocation(programI,"lightP");
    lightCLocI = glGetUniformLocation(programI,"lightC");
    GLint alphaLocI=glGetUniformLocation(programI,"alpha");
    GLint kdLocI=glGetUniformLocation(programI,"Kd");
    GLint ksLocI=glGetUniformLocation(programI,"Ks");
    GLint kaLocI=glGetUniformLocation(programI,"Ka");
    GLint sColLocI=glGetUniformLocation(programI,"specColor");
    GLint attLocI=glGetUniformLocation(programI,"attenuation");
    GLint attSLocI=glGetUniformLocation(programI,"attenuationS");
    GLint glowLocI=glGetUniformLocation(programI,"glow");
    spotOnLocI=glGetUniformLocation(programI,"spot");
    spotDirLocI=glGetUniformLocation(programI,"spotDir");
    gattenLocI=glGetUniformLocation(programI,"gatten");


    programT=loadShaders(":/vert_texture.glsl", ":/frag_texture.glsl");
    glUseProgram(programT);
    projMatrixLocT = glGetUniformLocation(programT, "projection");
    viewMatrixLocT = glGetUniformLocation(programT, "view");
    modelMatrixLocT = glGetUniformLocation(programT, "model");

    lightLocT = glGetUniformLocation(programT,"lightP");
    lightCLocT = glGetUniformLocation(programT,"lightC");
    GLint alphaLocT=glGetUniformLocation(programT,"alpha");
    GLint kdLocT=glGetUniformLocation(programT,"Kd");
    GLint ksLocT=glGetUniformLocation(programT,"Ks");
    GLint kaLocT=glGetUniformLocation(programT,"Ka");
    GLint sColLocT=glGetUniformLocation(programT,"specColor");
    GLint attLocT=glGetUniformLocation(programT,"attenuation");
    GLint attSLocT=glGetUniformLocation(programT,"attenuationS");
    GLint sampLocT=glGetUniformLocation(programT,"samp");
    GLint glowLocT=glGetUniformLocation(programT,"glow");
    spotOnLocT=glGetUniformLocation(programT,"spot");
    spotDirLocT=glGetUniformLocation(programT,"spotDir");
    gattenLocT=glGetUniformLocation(programT,"gatten");


    brick.initialize(programI,alphaLocI,kaLocI,ksLocI,kdLocI,
                     sColLocI,attLocI,attSLocI,glowLocI);
    mortar.initialize(programI,alphaLocI,kaLocI,ksLocI,kdLocI,
                      sColLocI,attLocI,attSLocI,glowLocI);

    ring1.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);
    ring2.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);
    ring3.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);
    ring4.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);
    ring5.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);

    light.initialize(programU,alphaLocU,kaLocU,ksLocU,kdLocU,
                     sColLocU,attLocU,attSLocU,glowLocU);
    grid.initialize(programS);
    normalMarks.initialize(programS);
    axes.initialize(programS);

    ground.initialize(programT,1,":/grasstex.bmp",alphaLocT,kaLocT,ksLocT,kdLocT
                      ,sColLocT,attLocT,attSLocT,glowLocT,sampLocT,GL_NEAREST);
    floor.initialize(programT,0,":/wood.bmp",alphaLocT,kaLocT,ksLocT,kdLocT
                     ,sColLocT,attLocT,attSLocT,glowLocT,sampLocT,GL_LINEAR);
    roof.initialize(programT,0,":/wood.bmp",alphaLocT,kaLocT,ksLocT,kdLocT
                     ,sColLocT,attLocT,attSLocT,glowLocT,sampLocT,GL_LINEAR);
    generateGround();


    initSky();

    initializeGrid();
    initAxes();
    generateLight();

    float ringattenS=.01f;
    float ringatten=.2f;
    float ringalpha=300;
    float ringspec=2.0f;
    generateRing(ring1, 1.6f,2);
    ring1.material.specColor=vec3(1,0,0);
    ring1.material.atten=ringatten;
    ring1.material.attenS=ringattenS;
    ring1.material.shinyness=ringalpha;
    ring1.material.specular=ringspec;

    generateRing(ring2, 1.2f,1.6f);
    ring2.material.specColor=vec3(0,1,0);
    ring2.material.atten=ringatten;
    ring2.material.attenS=ringattenS;
    ring2.material.shinyness=ringalpha;
    ring2.material.specular=ringspec;

    generateRing(ring3, 0.8f,1.2f);
    ring3.material.specColor=vec3(0,0,1);
    ring3.material.atten=ringatten;
    ring3.material.attenS=ringattenS;
    ring3.material.shinyness=ringalpha;
    ring3.material.specular=ringspec;

    generateRing(ring4, 0.4f,.8f);
    ring4.material.specColor=vec3(1,0,1);
    ring4.material.atten=ringatten;
    ring4.material.attenS=ringattenS;
    ring4.material.shinyness=ringalpha;
    ring4.material.specular=ringspec;

    generateRing(ring5, 0.001f,.4f);
    ring5.material.specColor=vec3(0,1,1);
    ring5.material.atten=ringatten;
    ring5.material.attenS=ringattenS;
    ring5.material.shinyness=ringalpha;
    ring5.material.specular=ringspec;
}




//the grid is not used.  This is left over from various labs and programs that used it.
void GLWidget::initializeGrid() {
    vec3 pts[84];
    for(int i = -10; i <= 10; i++) {

        pts[2*(i+10)] = vec3(i, 0, 10);
        pts[2*(i+10)+1] = vec3(i, 0, -10);

        pts[2*(i+10)+42] = vec3(10,0, i);
        pts[2*(i+10)+43] = vec3(-10,0, i);
    }
    vec3 colors[84];
    for(int i=0;i<84;i++){
        colors[i].y=1.0f-glm::abs(pts[i].x)/20.0f;
        colors[i].x=0.0f;
        colors[i].z=1.0f-glm::abs(pts[i].z)/20.0f;
    }

    grid.copyVertices(pts,colors,84);
    grid.updateBuffers();
}


void GLWidget::initializeGL() {
    initializeOpenGLFunctions();


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glPointSize(4.0f);
    glLineWidth(2);

    glEnable(GL_DEPTH_TEST);
    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);



    initMeshes();

    rebuildGeometry();
}

void GLWidget::resizeGL(int w, int h) {
    width = w;
    height = h;
    float aspect = (float)w/h;
    projMatrix = perspective(45.0f, aspect, 0.01f, 100.0f);


    glUseProgram(programI);
    glUniformMatrix4fv(projMatrixLocI, 1, false, value_ptr(projMatrix));
    glUniformMatrix4fv(viewMatrixLocI, 1, false, value_ptr(viewMatrix));

    glUseProgram(programU);
    glUniformMatrix4fv(projMatrixLocU, 1, false, value_ptr(projMatrix));
    glUniformMatrix4fv(viewMatrixLocU, 1, false, value_ptr(viewMatrix));

    glUseProgram(programS);
    glUniformMatrix4fv(projMatrixLocS, 1, false, value_ptr(projMatrix));
    glUniformMatrix4fv(viewMatrixLocS, 1, false, value_ptr(viewMatrix));

    glUseProgram(programT);
    glUniformMatrix4fv(projMatrixLocT, 1, false, value_ptr(projMatrix));
    glUniformMatrix4fv(viewMatrixLocT, 1, false, value_ptr(viewMatrix));

    mat4 rot=projMatrix*viewMatrix;
    glUseProgram(programBox);
    glUniformMatrix4fv(rotMatLocBox, 1, false, value_ptr(rot));

    updateLight();
    updateViewMat();

}

// render the sky-box or cube-map whatever you call it
void GLWidget::renderSky(){
    glDepthMask(GL_FALSE);
    glUseProgram(programBox);
    glBindVertexArray(skyVao);
    glUniform1f(skyBrightLoc,skyBrightness);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyTex);
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, checkLines? GL_LINE : GL_FILL);

    renderSky();

    brick.render();
    ring1.render();
    ring2.render();
    ring3.render();
    ring4.render();
    ring5.render();
    ground.render();

    if(renderFloor && !mac)
        floor.render();

    if(renderRoof && !mac)
        roof.render();

    if(renderMortar){
        mortar.render();
    }

    if(checkLight){
        glUseProgram(light.program);
        glBindVertexArray(light.vao);
        glUniformMatrix4fv(modelMatrixLocU,1,0,value_ptr(lightModelMatrix));

        light.render();

        glUniformMatrix4fv(modelMatrixLocU,1,0,value_ptr(modelMatrix));
    }


    if(checkNormals){
        normalMarks.render();
    }


    //axes.render();
}


// brickExplosion() called in animateRing when time to do the exploding (called each frame
// of course)
void GLWidget::brickExplosion(){
    renderMortar=0;
    //renderRoof=0;
    spotOn=0;

    for(uint i=0;i<brick.instanceMats.size();i++){
        if(brick.instanceOnGround[i])
            continue;
        brick.instanceMats[i]=translate(mat4(),brick.instanceVel[i])*brick.instanceMats[i];
        brick.instanceMats[i]=rotate(brick.instanceMats[i],
                                     spinSpeeds[i%NSPINSPDS],spinAxes[i%NSPINAXES]);

        brick.instanceVel[i].y-=0.0015f;
        vec3 pos(brick.instanceMats[i][3]);


        if(pos.y<0)
            brick.instanceOnGround[i]=1;
    }

    if(!roofOnGround){
        roof.modelMatrix=translate(mat4(),roofVel)*roof.modelMatrix;
        roof.modelMatrix=rotate(roof.modelMatrix,.01f,vec3(.2f,.3f,-0.4f));
        roofVel.y-=.0015f;
        vec3 pos(roof.modelMatrix[3]);
        if(pos.y<0)
            roofOnGround=1;
        roof.updateBuffers();
    }
    brick.updateInstanceMatBuffers();
}


// animateRing does the ring spinning (few lines at the end)
// the rest with all the if statements is a sort of story-line that goes through
// some stages to speed up the ring, suck the light, start glowing, get really bright,
// explode the house, fade the brightness back to normal, etc.
void GLWidget::animateRing(){
    if(!finished){
        if(ringStart){
            ringSpeed+=.001f;
        }


        if(ringStop){
            ringSpeed-=.005f;
            ring1.material.glow=ringSpeed-1.0f;
            ring2.material.glow=ringSpeed-1.0f;
            ring3.material.glow=ringSpeed-1.0f;
            ring4.material.glow=ringSpeed-1.0f;
            ring5.material.glow=ringSpeed-1.0f;
            if(ringSpeed<=1){
                ringSpeed=1;
                ringStop=0;
            }
        }


        if(!finishDarken && ringSpeed>.1f){
            gatten-=.001f;

            if(gatten<=0){
                finishDarken=1;
                gatten=0;
                spotOn=0;
                lightColor=ringColor;
                lightPosition=ringLoc;
            }
            updateLight();
        }

        if(finishDarken&&ringSpeed>1.0f){
            ring1.material.glow+=.001f;
            ring2.material.glow+=.001f;
            ring3.material.glow+=.001f;
            ring4.material.glow+=.001f;
            ring5.material.glow+=.001f;
        }


        if(ring1.material.glow>.2f){
            gatten+=.001f;
            if(gatten>.2f)
                gatten+=.006f;

            if(gatten>=3){
                finishedBrighten=1;
                ringStop=1;
                ringStart=0;
                darkenSky=1;
                brickExplode=1;
                skyBrightness=3;
                //lightFollow=1;
                flashlight=0;
                lightPosition=vec3(2,10,-5);
                lightColor=vec3(1,1,1);
            }
            updateLight();
        }

        if(brickExplode){
            brickExplosion();
        }

        if(darkenSky){
            gatten-=.03f;
            skyBrightness-=.0035f;
            if(skyBrightness<=1){
                skyBrightness=1;
                finished=1;
            }
            if(gatten<=1){
                gatten=1;
            }
            updateLight();
        }
    }



    ring1.rotationMatrix=glm::rotate(ring1.rotationMatrix,.007f+.05f*ringSpeed,vec3(0,1,0));
    ring1.modelMatrix=ring1.translationMatrix*ring1.rotationMatrix;

    ring2.rotationMatrix=glm::rotate(ring2.rotationMatrix,.047f*ringSpeed,vec3(1,0,0));
    ring2.modelMatrix=ring2.translationMatrix*ring1.rotationMatrix*ring2.rotationMatrix;

    ring3.rotationMatrix=glm::rotate(ring3.rotationMatrix,.041f*ringSpeed,vec3(0,1,0));
    ring3.modelMatrix=ring3.translationMatrix*ring1.rotationMatrix*ring2.rotationMatrix*ring3.rotationMatrix;

    ring4.rotationMatrix=glm::rotate(ring4.rotationMatrix,.053f*ringSpeed,vec3(1,1,0));
    ring4.modelMatrix=ring4.translationMatrix*ring1.rotationMatrix*ring2.rotationMatrix*ring3.rotationMatrix*ring4.rotationMatrix;

    ring5.rotationMatrix=glm::rotate(ring5.rotationMatrix,.087f*ringSpeed,vec3(.5,1,0));
    ring5.modelMatrix=ring5.translationMatrix*ring1.rotationMatrix*ring2.rotationMatrix*ring3.rotationMatrix*ring4.rotationMatrix*ring5.rotationMatrix;


    if(!finishedRebuild)
        testForStart();
}

void GLWidget::testForStart(){
    //get close to ring and stop
    if(length(ringLoc-eyePos)<5 && length(eyeVel)<=.0001){
        ringArmed=1;
    }
}

// length function but only in 2D (ignore the y coordinate)
float lengthXZ(vec3 v){
    return glm::sqrt(v.x*v.x+v.z*v.z);
}

// animate() calls the animateRing() function and then does the physics of the first-person
// controls, complete with gravity and momentum, and friction which is different depending
// if the person is on the ground or not (jumping).
void GLWidget::animate(){
    animateRing();
    //temp force from keys
    vec3 keyForce(0,0,0);
    if(keys[Qt::Key_W]){
        keyForce+=forward;
    }
    if(keys[Qt::Key_A]){
        keyForce-=right;
    }
    if(keys[Qt::Key_S]){
        keyForce-=forward;
    }
    if(keys[Qt::Key_D]){
        keyForce+=right;
    }
    if(flyMode){
        if(keys[Qt::Key_Shift]){//down
            keyForce-=vec3(0,1,0);
        }
        if(keys[Qt::Key_Space]){//up
            keyForce+=vec3(0,1,0);
        }
    }

    if(length(keyForce)>0.0001){
        keyForce=normalize(keyForce);
        keyForce*=moveSpeed;

        //integrate force from keys. lower factor when in the air (and not fly mode)
        float keyFactor=0.15f;
        if(!flyMode && eyePos.y>eyeHeight)
            keyFactor=.03f;
        eyeVel+=keyForce*keyFactor;

        //limit max velocity
        if(flyMode){
            float l=length(eyeVel);
            if(l>moveSpeed){
                eyeVel/=l;//normalize
            }

        }else{
            float l=lengthXZ(eyeVel);
            if(l>moveSpeed){
                eyeVel.x=eyeVel.x/l*moveSpeed;//normalize x
                eyeVel.z=eyeVel.z/l*moveSpeed;//normalize z
            }
        }
    }



    //gravity
    if(!flyMode && eyePos.y>eyeHeight){
        //integrate gravity
        eyeVel.y-=0.0015;
    }

    //friction
    vec3 fricForce;
    float fFactor=.15f; //friction coefficient, greater when on the ground
    if(flyMode){
        fricForce=-eyeVel;
    }else{
        fricForce=vec3(-eyeVel.x,0,-eyeVel.z);
        if(eyePos.y>eyeHeight)
            fFactor=.01f;
    }
    //integrate friction
    eyeVel+= fricForce*fFactor;

    //account for the ground
    if(eyePos.y<eyeHeight){
        eyePos.y=eyeHeight;
        eyeVel.y=0;
    }

    //if vel is small, set to zero and return
    //(this is like infinite coef of static fric!)
    if(length(eyeVel)<0.0002){
        eyeVel=vec3(0,0,0);
    }else{

        //integrate velocity and update
        eyePos+=eyeVel;
        matTrans=translate(mat4(1.0f),eyePos);
        updateViewMat();
    }

    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    if(ringArmed){
        if(!finished){
            ringArmed=0;
            ringStart=1;
        }else{
            if(!finishedRebuild){
                rebuildGeometry();
                renderRoof=1;
                renderMortar=1;
                update();
                finishedRebuild=1;
                lightFollow=1;
                lightColor=vec3(1,1,1);
                updateLight();
            }
        }
    }

    vec2 pt(event->x(), event->y());
    lastPt = pt;
    setCursor(Qt::BlankCursor);
//    mouseBegin=QCursor::pos();
}
void GLWidget::mouseReleaseEvent(QMouseEvent *) {
    setCursor(Qt::ArrowCursor);
//    QCursor::setPos(mouseBegin);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    vec2 d = pt-lastPt;

    angX-=d.x*mouseRateX;
    angY-=d.y*mouseRateY;
    if(angY>M_PI/2) angY=M_PI/2;
    if(angY<-M_PI/2) angY=-M_PI/2;

    matPitch=rotate(mat4(1.0f),angY,vec3(1,0,0));
    matYaw=rotate(mat4(1.0f),angX,vec3(0,1,0));
    matTrans=translate(mat4(1.0f),eyePos);

    right = vec3(matYaw[0]);
    forward = flyMode? vec3(-(matYaw*matPitch)[2]) : vec3(-matYaw[2]);

    updateViewMat();
    update();

    lastPt = pt;

}

// updateViewMat() calculates the viewMatrix from the translation, yaw, and pitch
// of the first-person, then "uploads" it to the shader programs.
void GLWidget::updateViewMat(){
    viewMatrix=inverse(matTrans*matYaw*matPitch);

    glUseProgram(programI);
    glUniformMatrix4fv(viewMatrixLocI, 1, false, value_ptr(viewMatrix));
    glUniform3fv(lightLocI,1,value_ptr(vec3(viewMatrix*vec4(lightPosition,1))));

    glUseProgram(programU);
    glUniformMatrix4fv(viewMatrixLocU, 1, false, value_ptr(viewMatrix));
    glUniform3fv(lightLocU,1,value_ptr(vec3(viewMatrix*vec4(lightPosition,1))));

    glUseProgram(programS);
    glUniformMatrix4fv(viewMatrixLocS, 1, false, value_ptr(viewMatrix));

    glUseProgram(programT);
    glUniformMatrix4fv(viewMatrixLocT, 1, false, value_ptr(viewMatrix));

    // this is the sky-box or cube-map, needs to "rotate" along with view in order to
    // stay stationary.
    mat4 rot = projMatrix*inverse(matYaw*matPitch);
    glUseProgram(programBox);
    glUniformMatrix4fv(rotMatLocBox, 1, false, value_ptr(rot));

    updateLight();
}

// keypress and keyrelease mainly use the "keys" variable which is a std::map.
void GLWidget::keyReleaseEvent(QKeyEvent *event){
    if(event->isAutoRepeat())
        return;

    keys[event->key()]=false;
}

void GLWidget::keyPressEvent(QKeyEvent *event){
    if(event->isAutoRepeat())
        return;

    keys[event->key()]=true;

    switch(event->key()) {
        case Qt::Key_F:
            if(!pause){
                pause=1;
                timer->stop();
            }
            animate();\
            break;
        case Qt::Key_P:
            pause=!pause;
            if(pause)
                timer->stop();
            else
                timer->start(timerSpeed);
            break;
        case Qt::Key_E:
            brickExplode=1;
            skyBrightness=1;
            gatten=1;
            updateLight();
            break;
        case Qt::Key_Tab:
            // toggle fly mode
            flyMode=!flyMode;
            break;
        case Qt::Key_Space:
            // jump
            if(!flyMode && eyePos.y<=eyeHeight+0.1f)
                eyeVel.y=0.04f;
            break;
    }

}


// convert a 2d point, for example from a mouse click, to device coordinates.
// the returned device coordinates have x in range -1 to 1, and y coordinates
// also centered on 0 but with range set to have the same "scale" as the x coordinates.
// (for example, using the virtual trackball function below, regardless what the window aspect
// ratio has been resized to be, the x and y "sensitivity" when moving the mouse will be equal)
vec2 GLWidget::w2dcSquare(const vec2 &pt){
    vec2 p;
    p.x=-1+pt.x*(2.0f/width);
    p.y=-(float)height/width*(1-pt.y*(2.0f/height));
    return p;
}

// this is used to make a trackball user interface, very useful for rotating
// meshes with a mouse interface.
vec3 GLWidget::pointOnVirtualTrackball(const vec2 &pt) {
    float r = .5f;
    float rr = r*r;
    vec3 p=vec3(w2dcSquare(pt),0);

    float xx = p.x*p.x;
    float yy = p.y*p.y;

    if(xx+yy <= rr*.5) {
        p.z = sqrt(rr-(xx+yy));
    } else {
        p.z = rr*.5/sqrt(xx+yy);
    }

    //    std::cout << p.x << ", " << p.y << ", " << p.z << std::endl;

    return p;
}



// initialization of cube-map sky-box.
void GLWidget::initSky(){
    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };
    for(int i=0;i<24;i++)
        pts[i].y+=0;

    GLuint indices[] = {
        0,3,2,1, restart,
        4,7,6,5, restart,
        8,11,10,9, restart,
        12,15,14,13, restart,
        16,19,18,17, restart,
        20,23,22,21
    };


    glGenVertexArrays (1, &skyVao);
    glBindVertexArray (skyVao);
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    GLuint program = loadShaders(":/vert_sky.glsl", ":/frag_sky.glsl");
    glUseProgram(program);
    programBox = program;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    skyBrightLoc = glGetUniformLocation(program, "brightness");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    rotMatLocBox=glGetUniformLocation(program,"rotation");

    create_cube_map_1file(":/grass.bmp",&skyTex);

}

///////////////////////////////////////////////////////////////////////////////////////////
//// loading textures functions

// subImage() gets a subsection of image and puts it in sub.
void getSubImage(uchar* sub, int sx,int sy, uchar* image, int posx, int posy){
    int x=sx*4;

    for(int j=sy*posy,sj=0; j<sy*(posy+1); j++,sj++){
        for(int i=sx*posx,si=0; i<sx*(posx+1); i++,si++){
            sub[(sj*sx+si)*3]=image[(j*x+i)*3];
            sub[(sj*sx+si)*3+1]=image[(j*x+i)*3+1];
            sub[(sj*sx+si)*3+2]=image[(j*x+i)*3+2];
        }
    }
}

// load the six faces of a cube-map texture with a single .bmp file that has all 6 faces
// jammed into one.
bool GLWidget::load_cube_map(GLuint texture, const char* file_name){

  glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

  int x, y;

  uchar * image_data=loadImg(file_name,x,y);

  if (!image_data) {
    cout<<"ERROR: could not load "<<file_name<<"\n";
    return false;
  }

  uint subx=x/4;
  uint suby=y/3;
  if(subx!=suby){
      cout<<"not a skybox bmp"<<endl;
      return false;
  }

  uchar* subimage[6];
  for(uint i=0;i<6;i++){
      subimage[i]=new uchar[subx*suby*3];

  }

  int xpos[]={0,2,1,1,1,3};
  int ypos[]={1,1,2,0,1,1};

  for(int pos=0;pos<6;pos++){
      getSubImage(subimage[pos],subx,suby,image_data,xpos[pos],ypos[pos]);
      uchar* temp=new uchar[subx*suby*3];
      int size=subx*suby*3;
      for(int i=0;i<size/3;i++){
          temp[i*3]=subimage[pos][size-(i*3)];
          temp[i*3+1]=subimage[pos][size-(i*3)+1];
          temp[i*3+2]=subimage[pos][size-(i*3)+2];
      }


      glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+pos,
                    0, GL_RGB, subx,suby,0, GL_BGR,
                    GL_UNSIGNED_BYTE,temp);

      delete temp;
  }

  delete image_data;
  for(int i=0;i<6;i++){
      delete subimage[i];
  }

  return true;
}

// open and load a 24-bit .bmp file. Returns the data as an array of uchars.
// there are many libraries to do this but more trouble than it's worth to figure
// out how to use the library, so I just made my own (very basic) function.
// depending on what is commented out, it may or may not reverse the order of the lines
// (.bmp files store the pixels sort of backwards compared to how you might expect and
// how you want to use the data)
unsigned char* GLWidget::loadImg(const char * path, int &x, int &y){

    char header[54]; // Each BMP file begins by a 54-bytes header

    unsigned int width, height;
    unsigned int imageSize;

    // Open the file
    QFile vertFile(path);
    if(!vertFile.open(QFile::ReadOnly)){
        cout<<"could not open image\n";
        return 0;
    }


    if(vertFile.read((char*)header,54) != 54){
        cout<<"Not a correct BMP file\n";
        return 0;
    }

    if ( header[0]!='B' || header[1]!='M' ){
        cout<<"Not a correct BMP file\n"<<endl;
        return 0;
    }

    // Read ints from the byte array
    imageSize = header[0x22]  | ( (int)header[0x23] << 8 )
                        | ( (int)header[0x24] << 16 )
                        | ( (int)header[0x25] << 24 );
    width = header[0x12]  | ( (int)header[0x13] << 8 )
                        | ( (int)header[0x14] << 16 )
                        | ( (int)header[0x15] << 24 );
    height = header[0x16]  | ( (int)header[0x17] << 8 )
                        | ( (int)header[0x18] << 16 )
                        | ( (int)header[0x19] << 24 );


    imageSize=width*height*3;
    x=width;y=height;

    uchar* data=new uchar[imageSize];


    vertFile.read((char *)data,imageSize);


    vertFile.close();

//    uchar *revdata = new uchar[imageSize];
//    for(int i=0;i<imageSize/3;i++){
//        revdata[i*3]=data[imageSize-i*3];
//        revdata[i*3+1]=data[imageSize-i*3+1];
//        revdata[i*3+2]=data[imageSize-i*3+2];
//    }


//    for(int j=0;j<y;j++){
//        for(int i=0;i<x;i++){
//            revdata[(j*x+i)*3]= data[((y-j)*x+i)*3];
//            revdata[(j*x+i)*3+1]= data[((y-j)*x+i)*3+1];
//            revdata[(j*x+i)*3+2]= data[((y-j)*x+i)*3+2];
//        }
//    }
//    delete data;

    return data;
}

void GLWidget::create_cube_map_1file(const char* boxbmp,
                              GLuint* tex_cube){

  // generate a cube-map texture to hold all the sides
  glActiveTexture (GL_TEXTURE0);
  glGenTextures (1, tex_cube);

  //load each image and copy into a side of the cube-map texture
  load_cube_map(*tex_cube, boxbmp);

  // format cube map texture
  glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


// load a vertex and fragment shader from a file.
// I did not write this but I truly can't remember where I found it.
GLuint GLWidget::loadShaders(const char* vertf, const char* fragf) {
    GLuint program = glCreateProgram();
    // read vertex shader from Qt resource file
    QFile vertFile(vertf);
    vertFile.open(QFile::ReadOnly | QFile::Text);
    QString vertString;
    QTextStream vertStream(&vertFile);
    vertString.append(vertStream.readAll());
    std::string vertSTLString = vertString.toStdString();

    const GLchar* vertSource = vertSTLString.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);
    {
        GLint compiled;
        glGetShaderiv( vertShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( vertShader, len, &len, log );
            std::cout << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, vertShader);
    // read fragment shader from Qt resource file
    QFile fragFile(fragf);
    fragFile.open(QFile::ReadOnly | QFile::Text);
    QString fragString;
    QTextStream fragStream(&fragFile);
    fragString.append(fragStream.readAll());
    std::string fragSTLString = fragString.toStdString();

    const GLchar* fragSource = fragSTLString.c_str();

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    {
        GLint compiled;
        glGetShaderiv( fragShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( fragShader, len, &len, log );
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    return program;
}
