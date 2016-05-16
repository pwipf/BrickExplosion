#ifndef MESH_H
#define MESH_H

#define QOGLVER QOpenGLFunctions_3_3_Core

#define GLM_FORCE_RADIANS
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <glm/glm.hpp>
#include <iostream>


using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::vector;

using std::cout;
using std::endl;

enum MeshType{ARRAY, INDEXED  };

class Material{
    public:
        float shinyness;
        float specular;
        float diffuse;
        float ambient;
        float atten;
        float attenS;
        float glow;
        vec3 specColor;
        GLint alphaLoc,kdloc,ksloc,kaloc,spcolloc,attenLoc,attenSLoc,glowLoc;
};


class Mesh
{
    public:
        Mesh();
        ~Mesh();
        GLuint program;
        GLuint vao;
        mat4 modelMatrix;
        mat4 rotationMatrix;
        mat4 translationMatrix;
        Material material;

    protected:
        GLuint loadShaders(const char* vertf, const char* fragf);

        QOGLVER *gl;
        std::vector<vec3> pts;
        std::vector<vec3> colors;
        std::vector<vec3> normals;
        std::vector<GLuint> idx;



        GLuint buffers[4];
        const char* names[3]={"position","color","normal"};
        //        GLuint ptsBuf;
//        GLuint colorBuf;
//        GLuint normalBuf;
//        GLuint indexBuf;

        GLint index[4];
        int modelMatLoc;



    public:
        void init(QOGLVER *context);
        void initialize(GLuint program, GLint alphaLoc,
                                GLint aLoc, GLint sLoc, GLint dLoc,
                                GLint sColLoc, GLint atLoc, GLint atSLoc, GLint glowLoc);
        void render();
        void renderTest();
        void updateBuffers();

        vec3 ptAt(uint i){ return pts[i];}
        vec3 normAt(uint i){return normals[i];}
        uint getNumVerts(){return pts.size();}

        void addPt(vec3 a){pts.push_back(a);}
        void addColor(vec3 a){colors.push_back(a);}
        void addNorm(vec3 a){normals.push_back(a);}
        void addIdx(GLuint* a,int n){for(int i=0;i<n;i++)idx.push_back(a[i]);}

        void clearVertices();
        void subdivide(int nSubs);
        void makeFlatShade();
        void computeNormals(int fine);
        void reverseNormals();
        void roundEdges(float radius);
        void roughen(float factor, int subdivides);
        void generateCube(const vec3 &color, GLuint xSections);
        void generateRing(const vec3 &color, GLuint sections, float inRad, float outRad);
        void scale(vec3 s);
        void scale(float s);
        void translate(const glm::vec3 &t);
        void transform(mat4 t);
        void copyVertices(vector<vec3> &pts,vector<vec3> &colors,
                          vector<vec3> &normals, vector<GLuint> &idx);
        void copyVertices(vector<vec3> &pts,vector<vec3> &colors);
        void copyVertices(vec3 ptsIn[],vec3 colorsIn[], int count);
        //make round
        void normalizePts(float len){for(uint i=0;i<pts.size();i++) pts[i]=glm::normalize(pts[i])*len;}

};

class InstancedMesh : public Mesh{

    public:
        std::vector<mat4> instanceMats;
        std::vector<vec3> instanceVel;
        std::vector<int> instanceOnGround;

        GLuint instanceMatBuf;
        GLint instanceMatLoc;


        InstancedMesh();

        void initialize(GLuint program, GLint alphaLoc,
                        GLint aLoc, GLint sLoc, GLint dLoc,
                        GLint sColLoc, GLint atLoc, GLint atSLoc,GLint glowLoc);
        void updateInstanceMatBuffers();
        void clearInstances();
        void addInstance(glm::mat4 transform);
        void render();

        int getNumInstances();
        mat4 getInstanceMat(uint i);
};

class LineMesh : public Mesh{
    public:
        void initialize(GLuint program);
        void updateBuffers();
        void render();
};

class SimpleTexMesh :public Mesh{
        GLuint uvBuffer;
        GLint uvIndex;
        std::vector<vec2> uvs;

        GLuint texSlot;

        GLuint loadBMP(const char * imagepath);



    public:
        void initialize(GLuint program, GLuint slot, const char *file, GLint alphaLoc,
                        GLint aLoc, GLint sLoc, GLint dLoc,
                        GLint sColLoc, GLint atLoc, GLint atSLoc,GLint glowLoc, GLint sampLoc, GLint magParam);
        void updateBuffers();
        void render();
        void clearVertices();
        void generateCube(float texScale);
        void addUV(vec2 a){uvs.push_back(a);}
        void scale(vec3 s, int bScaleTexture);
        void scale(float s, int bScaleTexture);
        GLuint texOb;
};


#endif // MESH_H
