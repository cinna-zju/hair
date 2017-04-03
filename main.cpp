#include "common.h"

#define max(a,b) a>b?a:b
void update();

GLint uniform_M, uniform_V, uniform_P, uniform_mvp;

mat4 Model, View, Project;

//eye model
vec3 eyePoint = vec3( 0.0, 0.0, -30.0 );
vec3 eyeDirection = vec3(0.0, 0.0, 1.0);
vec3 up = vec3( .0, 1.0, .0 );


//motion model
float dt = 0.01;
vec3 g = vec3(0.f, -9.8, 0.f);
float airFrictionConstant = 0.3;

vec3 vel[3][5];

float springLength = 6;
float springConstant = 200;



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
        "glsl",
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

//moadel data
GLuint vbo[5], vbo0n;
GLuint vao;
GLuint curve[3], color[3];

vec3 tri0[3], tri0n[3];
vec3 curVtx[3][50];
int numPoint = 5;
vector<vec3> line[3];
vec3 *line_pt[3] = {0};


void calcBezier(vector< vec3 > &line, int num)
{
    vector<vec3 >::iterator iter = line.begin();
    for(int i = 0; i < 50; i++){
        float t = i/50.0;
        curVtx[num][i] = *iter * (float)pow(1-t,3)
                        + *(iter+1) * t * (float)pow(1-t,2) * 3.f
                        + *(iter+2) * (float)pow(t,2) * (1-t) * 3.f
                        + *(iter+4) * (float)pow(t,3);
    }

}

int createModel()
{
    //verte
    tri0[0] = vec3(-10,10,0);
    tri0[1] = vec3(10,10,0);
    tri0[2] = vec3(0,15,17);
    //normal
    tri0n[0] = vec3(0,-1,0);
    tri0n[1] = vec3(0,-1,0);
    tri0n[2] = vec3(0,-1,0);
    //Bezier


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

    for(float i = 0; i < numPoint; i++){
        srand(i);
        line[0].push_back( tri0[0] + tri0n[0] * float(5.0) * i );
        line[1].push_back( tri0[1] + tri0n[1] * float(5.0) * i );
        line[2].push_back( tri0[2] + tri0n[2] * float(5.0) * i );

    }

    line_pt[0] = &(*line[0].begin());
    line_pt[1] = &(*line[1].begin());
    line_pt[2] = &(*line[2].begin());
    //line vbo 1,2,3
    for(int i = 1; i < 4; i++){
        glGenBuffers( 1, &vbo[i] );//vbo 1,2,3
        glGenBuffers( 1, &curve[i-1] );//cureve 0,1,2
        glBindBuffer( GL_ARRAY_BUFFER, vbo[i] );
        glBufferData( GL_ARRAY_BUFFER,
            sizeof( float ) * numPoint * 3,
            line_pt[i-1], GL_STATIC_DRAW
        );

    }
    return 0;



}
void shaderInit()
{
    SHADER shader;
    GLuint vs = shader.create( "vert.glsl", GL_VERTEX_SHADER );
    GLuint fs = shader.create("frag.glsl", GL_FRAGMENT_SHADER );
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

int main()
{
    GLinit();
    shaderInit();

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glEnableVertexAttribArray( 0 );

    if ( createModel() == 0 ) cout << "model created sucessfully" <<endl;


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        glDrawArrays(GL_LINE_LOOP, 0, 3);


        for(int i = 0; i < 3; i++){
            //control point
            glBindBuffer( GL_ARRAY_BUFFER, vbo[i+1] );
            glBufferData( GL_ARRAY_BUFFER,
                sizeof( float ) * numPoint * 3,
                line_pt[i], GL_STATIC_DRAW
            );
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            glDrawArrays(GL_LINE_STRIP, 0, 5);

            calcBezier(line[i], i);
            glBindBuffer( GL_ARRAY_BUFFER, curve[i] );
            glBufferData( GL_ARRAY_BUFFER,
                sizeof( float ) * 50 * 3,
                curVtx[i], GL_STATIC_DRAW
            );
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            glDrawArrays(GL_LINE_STRIP, 0, 50);


        }

        update();
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();

    return EXIT_SUCCESS;
}

void update()
{
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < numPoint-1; ++j){
            vec3 springVector = line_pt[i][j] - line_pt[i][j+1];
            float r = springVector.length();

            vec3 force;
            if( r != 0 ){
                force = (springVector / r) * (r - springLength) * springConstant
                        +g;

            }
            force += -(vel[i][j] - vel[i][j+1])*airFrictionConstant;
            //cout<<springVector.x<<' '<<springVector.y<<endl;
            vel[i][j] += force * dt;
            if(j!=0)
                *(line_pt[i]+j) += vec3(vel[i][j].x * dt,0.f,0.f);
            //cout<<vel[i][j].x*dt<<' '<<vel[i][j].y*dt<<endl;
            vel[i][j+1] += -force  * dt;
            //*(line_pt[i]+j+1) += vel[i][j+1]*dt;
            *(line_pt[i]+j+1) += vec3(vel[i][j+1].x * dt,0.f,0.f);

        }


}
