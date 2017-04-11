//2017.4.5
//yangshuo
//using WASD to increase velocity of 4 direction

#include "common.h"

#define max(a,b) a>b?a:b

void update();
void updateWithPBD();

GLint uniform_M, uniform_V, uniform_P, uniform_mvp;

mat4 Model, View, Project;

//eye model
vec3 eyePoint = vec3( 0.0, 0.0, -30.0 );
vec3 eyeDirection = vec3(0.0, 0.0, 1.0);
vec3 up = vec3( .0, 1.0, .0 );


//motion model
bool isSim = 1;
float dt = 0.3;
vec3 g = vec3(0.f, -9.8, 0.f);//gravity
float airFrictionConstant = 0.3;

vec3 vel[3][5];//velocity of control hair

float springLength = 8;
float springConstant = 400;

GLFWwindow* window;

void keyCallback(
    GLFWwindow* keyWnd,
    int key, int scancode, int action, int mods
){
    if(action == GLFW_PRESS){
        switch (key) {

            case GLFW_KEY_ESCAPE:{
                glfwSetWindowShouldClose(keyWnd, GLFW_TRUE);
                break;
            }

            case GLFW_KEY_A: {
                for(int i = 0; i < 3; ++i)
                for(int j = 0; j < 5; ++j){
                    vel[i][j]+=vec3(5.f,0.f,0.f);
                }
                break;
            }

            case GLFW_KEY_D: {
                for(int i = 0; i < 3; ++i)
                for(int j = 0; j < 5; ++j){
                    vel[i][j]+=vec3(-5.f,0.f,0.f);
                }
                break;
            }

            case GLFW_KEY_S: {
                for(int i = 0; i < 3; ++i)
                for(int j = 0; j < 5; ++j){
                    vel[i][j]+=vec3(0.f,0.f,-5.f);
                }
                break;
            }

            case GLFW_KEY_W: {
                for(int i = 0; i < 3; ++i)
                for(int j = 0; j < 5; ++j){
                    vel[i][j]+=vec3(0.f,0.f,5.f);
                }
                break;
            }

            case GLFW_KEY_P: {
                isSim = !isSim;
                break;
            }
        }
    }

}

int GLinit(){
    if ( !glfwInit() ){
        std::cerr<<"failed to initialize GLFW"<<std::endl;
    }
    //without setting GLFW_CONTEXT_VERSION_MAJOR and _MINOR，
	//OpenGL 1.x will be used
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    //must be used if OpenGL version >= 3.0
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Open a window and create its OpenGL context
	window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT,
        "hair simulation",
        NULL, NULL
    );

    if( window == NULL ){
        std::cerr<<"failed to open window"<<std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK ){
        std::cerr<<"failed to initialize GLEW"<<std::endl;
        glfwTerminate();
        return -1;

    }

    glEnable( GL_DEPTH_TEST );
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyCallback);

    return 0;
}

//model data
GLuint vbo[5], vbo0n;//contatins id
GLuint vao;

vec3 tri0[3], tri0n[3];//triangle's vertex and normal vector
int numPoint = 5;//number of points on each control hair
vector<vec3> line[3];//3 control hair
vec3 *line_pt[3] = {0};//pointer to each vector

//interpolate model
const int itpnum = 50;//num of itp hairs
GLuint itpid[itpnum];
vec3 itpv[itpnum][5];
float a[itpnum], b[itpnum], c[itpnum];//iteerpolate constant
GLuint curve[itpnum];//id of curve, include control hair and itp hair
vec3 curVtx[itpnum][50];//vertex data

/*
in: a pointer to a series of vec3 vertex
num: id of line

using 3-Bezier, each curve contain 50 points
*/

void calcBezier(vec3 *line, int num)
{
    for(int i = 0; i < 50; i++){
        float t = i/50.0;
        curVtx[num][i] = *line * (float)pow(1-t,3)
                        + *(line+1) * t * (float)pow(1-t,2) * 3.f
                        + *(line+2) * (float)pow(t,2) * (1-t) * 3.f
                        + *(line+4) * (float)pow(t,3);
    }

}

int createModel()
{
    //vertex
    tri0[0] = vec3(-10,15,0);
    tri0[1] = vec3(10,15,0);
    tri0[2] = vec3(0,15,17);
    //normal
    tri0n[0] = vec3(0,-1,0);
    tri0n[1] = vec3(0,-1,0);
    tri0n[2] = vec3(0,-1,0);

    for(int i=0;i<3;++i)
        for(int j=0;j<5;++j){
            vel[i][j] = vec3(0.f,0.f,0.f);
        }

    glGenBuffers( 1, &vbo[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData( GL_ARRAY_BUFFER,
        sizeof( float ) * 3 * 3,
        tri0, GL_STATIC_DRAW
    );

    glGenBuffers( 1, &vbo0n );
    glBindBuffer( GL_ARRAY_BUFFER, vbo0n);
    glBufferData( GL_ARRAY_BUFFER,
        sizeof( float ) * 3 * 3,
        tri0n, GL_STATIC_DRAW
    );
    //calculate 5 points, all along tri0n
    for(float i = 0; i < numPoint; i++){
        srand(i);
        line[0].push_back( tri0[0] + tri0n[0] * float(8.0) * i +vec3(i,0,0));
        line[1].push_back( tri0[1] + tri0n[1] * float(8.0) * i +vec3(i,0,0));
        line[2].push_back( tri0[2] + tri0n[2] * float(8.0) * i +vec3(i,0,0));

    }
    //point to the first element of vector
    line_pt[0] = &(*line[0].begin());
    line_pt[1] = &(*line[1].begin());
    line_pt[2] = &(*line[2].begin());
    //line vbo 1,2,3
    for(int i = 1; i < 4; i++){
        glGenBuffers( 1, &vbo[i] );//vbo 1,2,3
        //
        // glBindBuffer( GL_ARRAY_BUFFER, vbo[i] );
        // glBufferData( GL_ARRAY_BUFFER,
        //     sizeof( float ) * numPoint * 3,
        //     line_pt[i-1], GL_STATIC_DRAW
        // );

    }

    for(int i = 0; i < itpnum; ++i){
        glGenBuffers( 1, &itpid[i]);
        glGenBuffers( 1, &curve[i] );//cureve 0,...,itpnum-1
        //srand(i);
        a[i] = rand()%999/1000.0;
        b[i] = rand()%999/1000.0;
        if (a[i]+b[i] > 1){
            float *t;
            a[i] > b[i] ? t = &a[i] : t = &b[i];
            *t = 1 - *t;
        }
        c[i] = 1 - a[i] - b[i];
    }
    return 0;



}
void countFPS(double &lastTime, int &nbFrames)
{

    // Measure speed
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
        // printf and reset timer
        //printf("%f ms/frame\n", 1000.0/double(nbFrames));
        char title[256];
        snprintf ( title, 255,
                 "hair - [FPS: %3.2f]",
                    1000.0f / (double)nbFrames );

        glfwSetWindowTitle (window, title);

        nbFrames = 0;
        lastTime += 1.0;
    }




}

void shaderInit()
{
    SHADER shader;
    GLuint vs = shader.create( "../vert.glsl", GL_VERTEX_SHADER );
    GLuint fs = shader.create("../frag.glsl", GL_FRAGMENT_SHADER );
    GLuint program = shader.program(vs, fs);

    uniform_M = shader.getLocation(program, "M");
    uniform_V = shader.getLocation(program, "V");
    uniform_P = shader.getLocation(program, "P");


    Model = translate( mat4( 1.f ), vec3( 0.f, 0.f, 0.f ) );
    View = lookAt( eyePoint, eyeDirection, up );
    Project = perspective(
        45.f, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 100.f
    );
    glUniformMatrix4fv( uniform_M, 1, GL_FALSE, value_ptr( Model ) );
    glUniformMatrix4fv( uniform_V, 1, GL_FALSE, value_ptr( View ) );
    glUniformMatrix4fv( uniform_P, 1, GL_FALSE, value_ptr( Project ) );

}

void interpolate()
{
    for(int i = 0; i < itpnum; ++i)
        for(int j = 0; j < 5; j++){
            itpv[i][j] = (*(line_pt[0]+j)) * a[i]
                         +(*(line_pt[1]+j)) * b[i]
                         +(*(line_pt[2]+j)) * c[i];
            //cout << i<<'-'<<j<<':'<<itpv[i][j].x <<' '<<itpv[i][j].y<<endl;

        }

}
int main()
{
    GLinit();
    shaderInit();

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glEnableVertexAttribArray( 0 );

    if ( createModel() == 0 ) cout << "model created sucessfully" <<endl;

    interpolate();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {


        /* Render here */
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //draw triangle
        glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        glDrawArrays(GL_LINE_LOOP, 0, 3);

        //draw control line, not Bezier curve
        for(int i = 0; i < 3; i++){
            //control point
            glBindBuffer( GL_ARRAY_BUFFER, vbo[i+1] );
            glBufferData( GL_ARRAY_BUFFER,
                sizeof( float ) * numPoint * 3,
                line_pt[i], GL_STATIC_DRAW
            );
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            glDrawArrays(GL_LINE_STRIP, 0, 5);

        }

        interpolate();

        //draw itp Bezier curve
        for(int i = 0; i < itpnum; i++){
            calcBezier(itpv[i], i);
            glBindBuffer( GL_ARRAY_BUFFER, curve[i] );
            glBufferData( GL_ARRAY_BUFFER,
                sizeof( float ) * 50 * 3,
                curVtx[i], GL_STATIC_DRAW
            );
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            glDrawArrays(GL_LINE_STRIP, 0, 50);
        }

        //update();
        updateWithPBD();
        //sleep 33ms and fps is 33
        usleep(1000*33);
        //show ms/frame in title
        countFPS(lastTime, nbFrames);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();

    return EXIT_SUCCESS;
}

//simulation
void update()
{
    //just calculate 3 control hair's physics motion
    if( isSim ){
        for(int i = 0; i < 3; ++i)
            for(int j = 0; j < numPoint-1; ++j){
                //distance r between two points
                vec3 springVector = line_pt[i][j] - line_pt[i][j+1];
                //float r = springVector.length();//????????????????????????????????
                float r = length(springVector);
                vec3 force;
                if( r != 0 ){
                    force = (springVector / r) * (r - springLength) * springConstant
                            + g;//g value  maybe is not correct

                }
                force += -(vel[i][j] - vel[i][j+1])*airFrictionConstant;
                //cout<<springVector.x<<' '<<springVector.y<<endl;

                //here assume all the points' mass is 1
                vel[i][j] += force * dt;
                //points on triangle don't move
                if(j != 0)
                    *(line_pt[i]+j) += vec3(vel[i][j].x*dt, 0.f, vel[i][j].z*dt);
                //cout<<vel[i][j].x*dt<<' '<<vel[i][j].y*dt<<endl;
                vel[i][j+1] += -force  * dt;
                //*(line_pt[i]+j+1) += vel[i][j+1]*dt;
                *(line_pt[i]+j+1) += vec3(vel[i][j+1].x*dt,0.f,vel[i][j+1].z*dt);

            }

    }
}

const int solverIter = 10;
void updateWithPBD()
{
    vec3 p[3][5];
    vec3 dp[3][5];
    if(isSim){
        for(int i=0;i<3;++i)
            for(int j=0;j<5;++j){
                vel[i][j] += g*dt;
                if(j==0)
                    p[i][j]=line_pt[i][j];
                else
                    p[i][j] = line_pt[i][j] + vel[i][j] * dt;
            }
        for(int i=0;i<solverIter;++i){
            for(int j=0;j<3;++j)
                for(int k=0;k<4;k++){
                    vec3 dl = p[j][k]-p[j][k+1];
                    float constraint = length(dl) - springLength;
                    //every point has the same mass
                    dp[j][k] = (p[j][k]-p[j][k+1])
                                * -0.5
                                * constraint
                                / length(p[j][k]-p[j][k+1]);

                    dp[j][k+1] = (p[j][k]-p[j][k+1])
                                * 0.5
                                * constraint
                                / length(p[j][k]-p[j][k+1]);

                    if(k != 0)
                        p[j][k] += dp[j][k];
                    p[j][k+1] += dp[j][k+1];
                }
            //display length of hair 0
            
            cout<<"length:"<<i<<"\t"<<length(p[0][0]-p[0][1])
                        +length(p[0][1]-p[0][2])
                        +length(p[0][2]-p[0][3])
                        +length(p[0][3]-p[0][4])<<endl;
        }

        for(int i=0;i<3;++i)
            for(int j=0;j<5;++j){
                vel[i][j] = (p[i][j] - line_pt[i][j]) / dt;
                line_pt[i][j] = p[i][j];
            }

    }
}
