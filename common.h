#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct vertex_info_t {
    int vertexIndex;
    vec3 vertexCoordinate;
    vec3 vertexNormal;
    vector<int> connectedFaceIndex;
};

struct face_info_t {
    int faceIndex;
    ivec4 containedVertexIndex;
    vec3 faceNormal;
};

struct mesh_info_t {
    vector<vertex_info_t> vertexTable;
    vector<face_info_t> faceTable;
    int vertexNumber;
    vec3 minVertex, maxVertex;
    vector<ivec3> triangleIndex;
};


class SHADER{
public:
    GLuint create(string, GLenum);
    GLint getLocation( GLuint& , string );
    GLuint program( GLuint, GLuint );
private:
    void log( GLuint& object);
    string read_file(const string filename);
};

class OBJ{
public:
    OBJ():vertexNumber(0), faceNormalNumber(0), faceNumber(0){}
    mesh_info_t load( string filename );

private:
    vector<vec3> tempV;//store v
    vector<vec3> tempVn;//store vn
    vector<ivec4> tempFaceContainedVertexIndex;//store v of "v//vn" in f
    vector<int> tempFaceNormalIndex;//store vn of "v//vn" in f
    int vertexNumber, faceNormalNumber, faceNumber;
};

vector<ivec3> quad2tri( mesh_info_t& );
void findAABB( mesh_info_t& );
void drawBox( vec3, vec3 );

GLfloat* getCircle(GLfloat,GLfloat,GLfloat,GLfloat*,int);
