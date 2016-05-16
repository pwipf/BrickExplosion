
#include "mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include <glm/glm.hpp>


#define M_PI 3.14159265358979323846


using glm::inverse;
using glm::vec2;
using glm::vec3;
using glm::vec4;
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


Mesh::Mesh(){
    modelMatrix=mat4(1.0f);
    material.shinyness=100;
    material.diffuse=.8f;
    material.specular=.7f;
    material.ambient=.2f;
    material.specColor=vec3(1,1,1);
    material.atten=.2f;
    material.attenS=.2f;
    material.glow=0;
}
Mesh::~Mesh(){

}

void Mesh::clearVertices(){
    pts.clear();
    colors.clear();
    normals.clear();
    idx.clear();
}

void Mesh::updateBuffers(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    gl->glBufferData(GL_ARRAY_BUFFER, pts.size()*sizeof(vec3),&pts[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    gl->glBufferData(GL_ARRAY_BUFFER, colors.size()*sizeof(vec3),&colors[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    gl->glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(vec3),&normals[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(GLuint),&idx[0],GL_DYNAMIC_DRAW);
}

void Mesh::init(QOGLVER* context){
    gl=context;
}

void Mesh::initialize(GLuint program, GLint alphaLoc,
                      GLint aLoc, GLint sLoc, GLint dLoc,
                      GLint sColLoc, GLint atLoc, GLint atSLoc, GLint glowLoc){
    material.alphaLoc=alphaLoc;
    material.kaloc=aLoc;
    material.ksloc=sLoc;
    material.kdloc=dLoc;
    material.spcolloc=sColLoc;
    material.attenLoc=atLoc;
    material.attenSLoc=atSLoc;
    material.glowLoc=glowLoc;
    gl->glGenVertexArrays(1,&vao);
    this->program=program;
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glGenBuffers(4,buffers);
    for(int i=0;i<3;i++){
        gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[i]);
        index[i]=gl->glGetAttribLocation(program,names[i]);
        gl->glVertexAttribPointer(index[i], 3, GL_FLOAT, GL_FALSE, 0,0);
        gl->glEnableVertexAttribArray(index[i]);
    }
    modelMatLoc=gl->glGetUniformLocation(program,"model");
}

void Mesh::render(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glUniformMatrix4fv(modelMatLoc,1,false,value_ptr(modelMatrix));

    gl->glUniform1f(material.alphaLoc,material.shinyness);
    gl->glUniform1f(material.kdloc,material.diffuse);
    gl->glUniform1f(material.ksloc,material.specular);
    gl->glUniform1f(material.kaloc,material.ambient);
    gl->glUniform3fv(material.spcolloc,1,value_ptr(material.specColor));
    gl->glUniform1f(material.attenLoc,material.atten);
    gl->glUniform1f(material.attenSLoc,material.attenS);
    gl->glUniform1f(material.glowLoc,material.glow);

    gl->glDrawElements(GL_TRIANGLES,idx.size(),GL_UNSIGNED_INT,0);
}
void Mesh::renderTest(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glUniformMatrix4fv(modelMatLoc,1,false,value_ptr(modelMatrix));
//    gl->glDrawElements(GL_TRIANGLES,idx.size(),GL_UNSIGNED_INT,0);
}

class Edge{
    public:
        uint s;
        uint e;

        uint midpoint;
        int midfound;

        Edge(uint start, uint end): s(start), e(end), midfound(0){
        }

        uint getMid(vector<vec3> &pts, vector<vec3> &colors, vector<vec3> &normals, vector<uint> &idx){
            if(!midfound){
                vec3 newv = (pts[s] + pts[e])*0.5f;
                midpoint = pts.size();
                idx.push_back(midpoint);
                pts.push_back(newv);
                normals.push_back(vec3(0,0,0));
                colors.push_back(colors[s]);
                midfound=1;
            }
            return midpoint;
        }

};

class Triangle{
    public:
        uint v0,v1,v2;
        Triangle(uint a,uint b,uint c): v0(a),v1(b),v2(c){
        }
        Triangle insideTriangle(vector<vec3> &pts, vector<vec3> &colors, vector<vec3> &normals, vector<uint> &idx, vector<Edge> &edges){

            // find Edge in edge list and get midpoint vertex
            uint e0 = findEdge(edges,v0,v1);
            uint e1 = findEdge(edges,v1,v2);
            uint e2 = findEdge(edges,v2,v0);

            Triangle t(edges[e0].getMid(pts,colors,normals,idx),edges[e1].getMid(pts,colors,normals,idx),edges[e2].getMid(pts,colors,normals,idx));

            return t;
        }

        vec3 mid(vec3 a,vec3 b){
            return (a+b)*0.5f;
        }
        void appendTo(vector<uint> &idx){
            idx.push_back(v0);
            idx.push_back(v1);
            idx.push_back(v2);
        }

        uint findEdge(vector<Edge> &edges, uint a, uint b){
            for(uint i=0;i<edges.size();i++){
                uint s=edges[i].s, e=edges[i].e;
                if((s==a && e==b) || (s==b && e==a))
                    return i;
            }
            std::cout<<"Edge not found in insideTriangle() "<<a<<"-"<<b<<" edges.size:"<<edges.size()<<std::endl;
            return 0;
        }

        int edgeExists(vector<Edge> &edges, uint a, uint b){
            for(uint i=0;i<edges.size();i++){
                uint s=edges[i].s, e=edges[i].e;
                if((s==a && e==b) || (s==b && e==a)){
                    return 1;
                }
            }
            return 0;
        }

        void addToEdges(vector<Edge> &edges){
            if(!edgeExists(edges,v0,v1))
                edges.push_back(Edge(v0,v1));
            if(!edgeExists(edges,v1,v2))
                edges.push_back(Edge(v1,v2));
            if(!edgeExists(edges,v2,v0))
                edges.push_back(Edge(v2,v0));
        }
};

void Mesh::subdivide(int nSubs){
    for(int n=0;n<nSubs;n++){
        std::vector<Edge> edges;
        std::vector<Triangle> triangles;
        for(uint i=0;i<idx.size()/3;i++){
            Triangle t(Triangle(idx[i*3],idx[i*3+1],idx[i*3+2]));
            triangles.push_back(t);
            t.addToEdges(edges);
        }

        vector<Triangle> newTri;
        for(uint i=0;i<triangles.size();i++){
            Triangle t=triangles[i];
            Triangle in=t.insideTriangle(pts,colors,normals,idx,edges);
            newTri.push_back(in);
            newTri.push_back(Triangle(t.v0,in.v0,in.v2));
            newTri.push_back(Triangle(t.v1,in.v1,in.v0));
            newTri.push_back(Triangle(t.v2,in.v2,in.v1));

        }
        triangles.clear();
        triangles.insert(triangles.end(),newTri.begin(),newTri.end());

        idx.clear();
        for(uint i=0;i<triangles.size();i++){
            triangles[i].appendTo(idx);
        }

    }
}
// generate 2x2x2 cube with multiple sections in the x direction (ends are still just 2 triangles each)
void Mesh::generateCube(const vec3 &color, GLuint xSections){
    vec3 v(1,1,1);
    vec3 n(0,0,0);
    int neg=0;
    uint end1[]={3,2,0,0,2,1};
    uint end2[]={3,0,2,2,0,1};
    uint middle[]={0,1,4,4,1,5, 1,2,5,5,2,6, 2,3,6,6,3,7, 3,0,7,7,0,4};

    clearVertices();
    idx.insert(idx.end(), &end1[0], &end1[6]);
    int num=4+xSections*4;
    for(int i=0;i<num/4;i++){

        for(int k=0;k<4;k++){
            pts.push_back(v);
            normals.push_back(n);
            colors.push_back(color);
            neg? v.y=-v.y : v.z=-v.z;
            neg=!neg;
        }
        v.x-=2.0f/xSections;

        if(i<num/4 - 1){
            idx.insert(idx.end(), &middle[0], &middle[24]);
            for(int k=0;k<24;k++)
                middle[k]+=4;
            for(int k=0;k<6;k++)
                end2[k]+=4;
        }
    }
    idx.insert(idx.end(), &end2[0], &end2[6]);
}

//generate ring lying flat with
//inRadius, outRadius in x-z direction, 2 units in y direction
void Mesh::generateRing(const vec3 &color, GLuint sections,
                        float inRad, float outRad){

    float da=2*M_PI/sections;
    uint x[]={0,1,8,8,1,9, 2,10,3,3,10,11, 4,12,5,5,12,13, 6,7,14,14,7,15};

    for(uint i=0;i<sections;i++){
        vec3 p0=glm::rotateY(vec3(outRad,.5f,0),da*i);
        vec3 p1(p0.x,-.5f,p0.z);
        vec3 p2=glm::rotateY(vec3(inRad,.5f,0),da*i);
        vec3 p3(p2.x,-.5f,p2.z);
        pts.push_back(p0);
        pts.push_back(p1);
        pts.push_back(p2);
        pts.push_back(p3);

        pts.push_back(p0);
        pts.push_back(p2);

        pts.push_back(p1);
        pts.push_back(p3);

        normals.push_back(p0);
        normals.push_back(p1);
        normals.push_back(-p2);
        normals.push_back(-p3);

        normals.push_back(vec3(0,1,0));
        normals.push_back(vec3(0,1,0));
        normals.push_back(vec3(0,-1,0));
        normals.push_back(vec3(0,-1,0));

        for(int j=0;j<24;j++)
            idx.push_back((x[j]+i*8) % (sections*8));

    }

    for(uint i=0;i<pts.size();i++){
        colors.push_back(color);
        normals[i]=normalize(normals[i]);
    }
}

void Mesh::scale(vec3 s){
    for(uint i=0;i<pts.size();i++){
        pts[i]*=s;
    }
}
void Mesh::scale(float s){
    for(uint i=0;i<pts.size();i++){
        pts[i]*=s;
    }
}
void Mesh::translate(const vec3 &t){
    for(uint i=0;i<pts.size();i++){
        pts[i]+=t;
    }
}
void Mesh::transform(mat4 t){
    for(uint i=0;i<pts.size();i++){
        pts[i]=vec3(t*vec4(pts[i],1));
        normals[i]=vec3(t*vec4(normals[i],0));
    }
}

void Mesh::roughen(float factor, int subdivides){

    float rough=0.2f*factor;
    for(uint i=0;i<pts.size();i++){

        float r=glm::gaussRand(0.0f,rough);

        // front and back
        if(normals[i].z > 0.5f || normals[i].z < -0.5f){
            float old=pts[i].z;
            float q=std::cos(pts[i].x*(float)M_PI*std::pow(2.0f,(int)(subdivides-1)));
            //q=q<0?-1:1;
            q*=.04f*(subdivides==6? factor*.8f : factor);
            pts[i].z+=q;
            pts[i].z+=r;

            if(normals[i].z<0.5f)
                colors[i]=colors[i]+(vec3(1,1,1)*((old-pts[i].z)*5));
            else
                colors[i]=colors[i]+(vec3(1,1,1)*((pts[i].z-old)*5));

        }

        // ends
        else if(normals[i].x > 0.5f || normals[i].x < -0.5f){
            float old=pts[i].x;
            pts[i].x+= std::cos(pts[i].z*(float)M_PI*std::pow(2.0f,(int)(subdivides-2)))*.02f*factor;
            pts[i].x+=r*.5f;

            if(normals[i].x<0.5f)
                colors[i]=colors[i]+(vec3(1,1,1)*((old-pts[i].x)*5));
            else
                colors[i]=colors[i]+(vec3(1,1,1)*((pts[i].x-old)*5));
        }

        // top and bottom
        else if(normals[i].y > 0.5f || normals[i].y < -0.5f){
            pts[i].y+=r*.1;
        }
    }
}

float lenX(vec3 p){
    return sqrt(p.z*p.z+p.y*p.y);
}
float lenY(vec3 p){
    return sqrt(p.z*p.z+p.x*p.x);
}
float lenZ(vec3 p){
    return sqrt(p.x*p.x+p.y*p.y);
}

void Mesh::roundEdges(float radius){
    int neg=0;
    for(uint i=0;i<pts.size();i++){

        vec3 axpos(1-radius,1-radius,0);
        for(int j=0;j<4;j++){

            if(neg) axpos.x=-axpos.x;
            else axpos.y=-axpos.y;

            if(((axpos.x>0)? pts[i].x>axpos.x : pts[i].x<axpos.x) && ((axpos.y>0)? pts[i].y>axpos.y : pts[i].y<axpos.y)){

                vec3 np = pts[i]-axpos;
                float len = lenZ(np);
                if(len>radius){
                    float rat=radius/len;
                    np.x*=rat;
                    np.y*=rat;
                    pts[i]=np+axpos;
                }
            }
            neg=!neg;
        }

        axpos=vec3(0,1-radius,1-radius);
        for(int j=0;j<4;j++){

            if(neg) axpos.z=-axpos.z;
            else axpos.y=-axpos.y;

            if(((axpos.z>0)? pts[i].z>axpos.z : pts[i].z<axpos.z) && ((axpos.y>0)? pts[i].y>axpos.y : pts[i].y<axpos.y)){

                vec3 np = pts[i]-axpos;
                float len = lenX(np);
                if(len>radius){
                    float rat=radius/len;
                    np.z*=rat;
                    np.y*=rat;
                    pts[i]=np+axpos;
                }
            }
            neg=!neg;
        }

        axpos=vec3(1-radius,0,1-radius);
        for(int j=0;j<4;j++){

            if(neg) axpos.z=-axpos.z;
            else axpos.x=-axpos.x;

            if(((axpos.z>0)? pts[i].z>axpos.z : pts[i].z<axpos.z) && ((axpos.x>0)? pts[i].x>axpos.x : pts[i].x<axpos.x)){

                vec3 np = pts[i]-axpos;
                float len = lenY(np);
                if(len>radius){
                    float rat=radius/len;
                    np.z*=rat;
                    np.x*=rat;
                    pts[i]=np+axpos;
                }
            }
            neg=!neg;
        }
    }
}
void Mesh::computeNormals(int fine){
    // should have the right number of normals already in vector<vec3> normals
    for(uint i=0;i<idx.size()/3;i++){
        uint ind0=idx[3*i];
        uint ind1=idx[3*i+1];
        uint ind2=idx[3*i+2];
        vec3 p0= pts[ind0];
        vec3 p1= pts[ind1];
        vec3 p2= pts[ind2];

        vec3 n=normalize(cross(p0-p1,p0-p2));

        if(fine){
            // use the angle of the triangle to weight it's contribution to that total normal
            float angle0=glm::acos(dot(normalize(p1-p0), normalize(p2-p0)));
            float angle1=glm::acos(dot(normalize(p0-p1), normalize(p2-p1)));
            float angle2=glm::acos(dot(normalize(p0-p2), normalize(p1-p2)));

            normals[ind0]+=n*angle0;
            normals[ind1]+=n*angle1;
            normals[ind2]+=n*angle2;
        }else{
            normals[ind0]+=n;
            normals[ind1]+=n;
            normals[ind2]+=n;
        }

    }
    //normalize the normals
    for(uint i=0;i<normals.size();i++){
        normals[i]=normalize(normals[i]);
    }
}

void Mesh::reverseNormals(){
    for(uint i=0;i<normals.size();i++){
        normals[i]=-normals[i];
    }
}

//duplicate vertices for each triangle.
void Mesh::makeFlatShade(){

    vector<vec3> ptsTemp, colorsTemp, normalsTemp;
    vector<GLuint> idxTemp;

    // (1) fill temporary std::vectors with new data
    int k=0;
    for(uint i=0;i<idx.size()/3;i++){
        uint ind0=idx[3*i];
        uint ind1=idx[3*i+1];
        uint ind2=idx[3*i+2];
        vec3 p0= pts[ind0];
        vec3 p1= pts[ind1];
        vec3 p2= pts[ind2];

        vec3 n=normalize(cross(p0-p1,p0-p2));

        ptsTemp.push_back(pts[ind0]);
        ptsTemp.push_back(pts[ind1]);
        ptsTemp.push_back(pts[ind2]);
        colorsTemp.push_back(colors[ind0]);
        colorsTemp.push_back(colors[ind1]);
        colorsTemp.push_back(colors[ind2]);
        normalsTemp.push_back(n);
        normalsTemp.push_back(n);
        normalsTemp.push_back(n);
        idxTemp.push_back(k++);
        idxTemp.push_back(k++);
        idxTemp.push_back(k++);
    }

    // (2) copy temporary to parameter std::vectors
    clearVertices();
    idx.insert(idx.end(),idxTemp.begin(),idxTemp.end());
    pts.insert(pts.end(),ptsTemp.begin(),ptsTemp.end());
    colors.insert(colors.end(),colorsTemp.begin(),colorsTemp.end());
    normals.insert(normals.end(),normalsTemp.begin(),normalsTemp.end());
}
void Mesh::copyVertices(vector<glm::vec3> &ptsIn, vector<glm::vec3> &colorsIn,
                  vector<glm::vec3> &normalsIn, vector<GLuint> &idxIn){
    pts.insert(pts.end(),ptsIn.begin(),ptsIn.end());
    colors.insert(colors.end(),colorsIn.begin(),colorsIn.end());
    normals.insert(normals.end(),normalsIn.begin(),normalsIn.end());
    idx.insert(idx.end(),idxIn.begin(),idxIn.end());
}
void Mesh::copyVertices(vector<vec3> &ptsIn,vector<vec3> &colorsIn){
    pts.insert(pts.end(),ptsIn.begin(),ptsIn.end());
    colors.insert(colors.end(),colorsIn.begin(),colorsIn.end());
}
void Mesh::copyVertices(vec3 ptsIn[],vec3 colorsIn[], int count){
    pts.insert(pts.end(), &ptsIn[0], &ptsIn[count]);
    colors.insert(colors.end(), &colorsIn[0], &colorsIn[count]);
}

//////////////////////////////////////////////////////////////////////////

InstancedMesh::InstancedMesh(){
    material.shinyness=100;
    material.diffuse=.8f;
    material.specular=.7f;
    material.ambient=.2f;
    material.specColor=vec3(1,1,1);
}

void InstancedMesh::initialize(GLuint program, GLint alphaLoc,
                               GLint aLoc, GLint sLoc, GLint dLoc,
                               GLint sColLoc, GLint atLoc, GLint atSLoc, GLint glowLoc){
    material.alphaLoc=alphaLoc;
    material.kaloc=aLoc;
    material.ksloc=sLoc;
    material.kdloc=dLoc;
    material.spcolloc=sColLoc;
    material.attenLoc=atLoc;
    material.attenSLoc=atSLoc;
    material.glowLoc=glowLoc;
    gl->glGenVertexArrays(1,&vao);
    this->program=program;
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glGenBuffers(4,buffers);
    for(int i=0;i<3;i++){
        gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[i]);
        index[i]=gl->glGetAttribLocation(program,names[i]);
        gl->glVertexAttribPointer(index[i], 3, GL_FLOAT, GL_FALSE, 0,0);
        gl->glEnableVertexAttribArray(index[i]);
    }

    gl->glGenBuffers(1,&instanceMatBuf);
    gl->glBindBuffer(GL_ARRAY_BUFFER, instanceMatBuf);
    instanceMatLoc=gl->glGetAttribLocation(program,"instanceMat");
    //cout<<"instanceMat"<<" index:"<<instanceMatLoc<<endl;

    for(int i=0;i<4;i++){
        gl->glVertexAttribPointer(instanceMatLoc+i, 4, GL_FLOAT,GL_FALSE,sizeof(mat4),(void*)(sizeof(vec4)*i));
        gl->glEnableVertexAttribArray(instanceMatLoc+i);
        gl->glVertexAttribDivisor(instanceMatLoc+i,1);
    }

    modelMatLoc=gl->glGetUniformLocation(program,"model");
}

void InstancedMesh::addInstance(mat4 transform){
    instanceMats.push_back(transform);
    updateInstanceMatBuffers(); //TODO: for efficiency this probably shouldn't be in each call to addInstance

    vec3 pos=vec3(transform[3]);
    glm::normalize(pos);
    pos*=.01;
    pos.y+=glm::linearRand(.1f,.3f);
    pos.x+=glm::gaussRand(0.0f,.001f);
    pos.z+=glm::gaussRand(0.0f,.001f);
    instanceVel.push_back(pos);
    instanceOnGround.push_back(0);
}
void InstancedMesh::render(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);

    gl->glUniform1f(material.alphaLoc,material.shinyness);
    gl->glUniform1f(material.kdloc,material.diffuse);
    gl->glUniform1f(material.ksloc,material.specular);
    gl->glUniform1f(material.kaloc,material.ambient);
    gl->glUniform1f(material.attenLoc,material.atten);
    gl->glUniform1f(material.attenSLoc,material.attenS);
    gl->glUniform1f(material.glowLoc,material.glow);
    gl->glUniform3fv(material.spcolloc,1,value_ptr(material.specColor));

    gl->glUniformMatrix4fv(modelMatLoc,1,false,value_ptr(modelMatrix));
    gl->glDrawElementsInstanced(GL_TRIANGLES,idx.size(),GL_UNSIGNED_INT,
                                          0,instanceMats.size());
}
void InstancedMesh::clearInstances(){
    instanceMats.clear();
}

void InstancedMesh::updateInstanceMatBuffers(){
    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, instanceMatBuf);
    gl->glBufferData(GL_ARRAY_BUFFER, instanceMats.size()*sizeof(mat4),&instanceMats[0] ,GL_DYNAMIC_DRAW);
}
int InstancedMesh::getNumInstances(){
    return instanceMats.size();
}

mat4 InstancedMesh::getInstanceMat(uint i){
    if(instanceMats.size()==0 || i>instanceMats.size()-1){
        cout<<"no instances in getInstanceMat"<<endl;
        return mat4(1.0f);
    }
    return instanceMats[i];
}

//////////////////////////////////////////////////////////////////////////


void LineMesh::initialize(GLuint program){
    gl->glGenVertexArrays(1,&vao);
    this->program=program;
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glGenBuffers(2,buffers);
    for(int i=0;i<2;i++){
        gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[i]);
        index[i]=gl->glGetAttribLocation(program,names[i]);
        gl->glEnableVertexAttribArray(index[i]);
        gl->glVertexAttribPointer(index[i], 3, GL_FLOAT, GL_FALSE, 0,0);
    }
    modelMatLoc=gl->glGetUniformLocation(program,"model");
}
void LineMesh::updateBuffers(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
    gl->glBufferData(GL_ARRAY_BUFFER, pts.size()*sizeof(vec3), &pts[0], GL_STATIC_DRAW);
    gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[1]);
    gl->glBufferData(GL_ARRAY_BUFFER, colors.size()*sizeof(vec3), &colors[0], GL_STATIC_DRAW);
}
void LineMesh::render(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glUniformMatrix4fv(modelMatLoc,1,false,value_ptr(modelMatrix));
    gl->glDrawArrays(GL_LINES,0,pts.size());
}


//////////////////////////////////////////////////////////////////////////


void SimpleTexMesh::initialize(GLuint program, GLuint slot, const char* file, GLint alphaLoc,
                               GLint aLoc, GLint sLoc, GLint dLoc,
                               GLint sColLoc, GLint atLoc, GLint atSLoc, GLint glowLoc, GLint sampLoc, GLint magParam){
    texSlot=slot;

    material.alphaLoc=alphaLoc;
    material.kaloc=aLoc;
    material.ksloc=sLoc;
    material.kdloc=dLoc;
    material.spcolloc=sColLoc;
    material.attenLoc=atLoc;
    material.attenSLoc=atSLoc;
    material.glowLoc=glowLoc;


    gl->glGenTextures(1,&texOb);
    gl->glActiveTexture(GL_TEXTURE0+texSlot);
    gl->glBindTexture(GL_TEXTURE_2D,texOb);
    gl->glEnable(GL_TEXTURE_2D);
    gl->glUniform1i(sampLoc,texSlot);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magParam);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gl->glGenVertexArrays(1,&vao);
    this->program=program;
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glGenBuffers(4,buffers);
    for(int i=0;i<3;i++){
        gl->glBindBuffer(GL_ARRAY_BUFFER,buffers[i]);
        index[i]=gl->glGetAttribLocation(program,names[i]);
        gl->glVertexAttribPointer(index[i], 3, GL_FLOAT, GL_FALSE, 0,0);
        gl->glEnableVertexAttribArray(index[i]);
    }
    modelMatLoc=gl->glGetUniformLocation(program,"model");


    gl->glGenBuffers(1,&uvBuffer);
    gl->glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    uvIndex=gl->glGetAttribLocation(program, "uv");
    gl->glEnableVertexAttribArray(uvIndex);
    gl->glVertexAttribPointer(uvIndex,2,GL_FLOAT,GL_FALSE,0,0);


    loadBMP(file);
}

void SimpleTexMesh::updateBuffers(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    gl->glBufferData(GL_ARRAY_BUFFER, pts.size()*sizeof(vec3),&pts[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    gl->glBufferData(GL_ARRAY_BUFFER, colors.size()*sizeof(vec3),&colors[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    gl->glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(vec3),&normals[0],GL_DYNAMIC_DRAW);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(GLuint),&idx[0],GL_DYNAMIC_DRAW);

    gl->glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    gl->glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

}
void SimpleTexMesh::render(){
    gl->glUseProgram(program);
    gl->glBindVertexArray(vao);

    gl->glBindTexture(GL_TEXTURE_2D,texOb);

    gl->glUniformMatrix4fv(modelMatLoc,1,false,value_ptr(modelMatrix));

    gl->glUniform1f(material.alphaLoc,material.shinyness);
    gl->glUniform1f(material.kdloc,material.diffuse);
    gl->glUniform1f(material.ksloc,material.specular);
    gl->glUniform1f(material.kaloc,material.ambient);
    gl->glUniform3fv(material.spcolloc,1,value_ptr(material.specColor));
    gl->glUniform1f(material.attenLoc,material.atten);
    gl->glUniform1f(material.attenSLoc,material.attenS);
    gl->glUniform1f(material.glowLoc,material.glow);

    gl->glDrawElements(GL_TRIANGLES,idx.size(),GL_UNSIGNED_INT,0);
}

void SimpleTexMesh::clearVertices(){
    pts.clear();
    colors.clear();
    normals.clear();
    idx.clear();
    uvs.clear();
}

void SimpleTexMesh::generateCube(float texScale){
    vec3 parray[] = {
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
    vec2 uvarray[]={
        vec2(0,1),
        vec2(0,0),
        vec2(1,0),
        vec2(1,1),

        vec2(0,1),
        vec2(0,0),
        vec2(1,0),
        vec2(1,1),

        vec2(1,1),
        vec2(1,0),
        vec2(0,0),
        vec2(0,1),

        vec2(0,1),
        vec2(0,0),
        vec2(1,0),
        vec2(1,1),

        vec2(1,1),
        vec2(1,0),
        vec2(0,0),
        vec2(0,1),

        vec2(0,1),
        vec2(0,0),
        vec2(1,0),
        vec2(1,1)};
    for(int i=0;i<24;i++)
        uvarray[i]*=texScale;


    GLuint indices[] = {0,1,3,3,1,2,
                        4,5,7,7,5,6,
                        8,9,11,11,9,10,
                        12,13,15,15,13,14,
                        16,17,19,19,17,18,
                        20,21,23,23,21,22};

    pts.insert(pts.end(),&parray[0],&parray[24]);
    uvs.insert(uvs.end(),&uvarray[0],&uvarray[24]);
    idx.insert(idx.end(),&indices[0],&indices[36]);
    normals.insert(normals.end(),&parray[0],&parray[24]);
    computeNormals(1);
}

void SimpleTexMesh::scale(vec3 s, int bScaleTexture){
    for(uint i=0;i<pts.size();i++){
        pts[i]*=s;
    }
    if(bScaleTexture)
        for(uint i=0;i<uvs.size();i++){
            if(glm::abs(normals[i].y)>.3f)
                uvs[i]*=vec2(s.x,s.z);
            else if(glm::abs(normals[i].z)>.3f)
                uvs[i]*=vec2(s.x,s.y);
            else if(glm::abs(normals[i].x)>.3f)
                uvs[i]*=vec2(s.y,s.z);
        }
}
void SimpleTexMesh::scale(float s, int bScaleTexture){
    for(uint i=0;i<pts.size();i++){
        pts[i]*=s;
    }
    if(bScaleTexture){
        for(uint i=0;i<uvs.size();i++){
            uvs[i].x*=s;
        }
    }
}

GLuint SimpleTexMesh::loadBMP(const char * imagepath){
    // Data read from the header of the BMP file
    char header[54];
    unsigned int width, height;
    unsigned int imageSize;



    // Open the file
    QFile vertFile(imagepath);
    if(!vertFile.open(QFile::ReadOnly)){
        cout<<"could not open image\n";
        return 0;
    }


    if(vertFile.read(header,54) != 54){
        cout<<"Not a correct BMP file\n";
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

    // Create a buffer
    char* data = new char [imageSize];

    vertFile.read(data,imageSize);

    vertFile.close();

    gl->glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    gl->glGenerateMipmap(GL_TEXTURE_2D);

    delete data;
    return texOb;
}


