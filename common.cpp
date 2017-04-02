#include "common.h"


string SHADER::read_file(const string filename){
    ifstream in;
    in.open( filename );//c_str?
    stringstream ss;
    ss << in.rdbuf();
    string so = ss.str();
    in.close();

    return so;

}

GLuint SHADER::create(string filename, GLenum type){
    string s_temp = read_file( filename );
    string info;
    const GLchar* source = s_temp.c_str();

    switch( type ){
        case GL_VERTEX_SHADER:
            info = "Vertex";
            break;
        case GL_FRAGMENT_SHADER:
            info = "Fragment";
            break;
    }

    if( source == NULL ){
        std::cout << info << " Shader : Can't read shader source file." << std::endl;
        return 0;
    }

    const GLchar* sources[] = {
        source
    };
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, sources, NULL );
    //void glShaderSource( GLuint shader,
    //GLsizei count,
    //const GLchar **string,
    //const GLint *length);
    glCompileShader( shader );

    GLint compile_ok;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compile_ok );
    if( compile_ok == GL_FALSE ){
        std::cout << info << " Shader : Fail to compile." << std::endl;
		log( shader );
        glDeleteShader(shader);
        return 0;
    }

    return shader;

}

void SHADER::log( GLuint& object ){
    GLint log_length = 0;
    if ( glIsShader( object ) ) {
		glGetShaderiv( object, GL_INFO_LOG_LENGTH, &log_length );
	} else if ( glIsProgram( object ) ) {
		glGetProgramiv( object, GL_INFO_LOG_LENGTH, &log_length );
	} else {
		cerr << "printlog: Not a shader or a program" << endl;
		return;
	}

	char* text = ( char* )malloc( log_length );

	if ( glIsShader( object ) )
		glGetShaderInfoLog( object, log_length, NULL, text );
	else if ( glIsProgram( object ) )
		glGetProgramInfoLog( object, log_length, NULL, text );

	cerr << text << endl;
	free( text );
}

GLint SHADER::getLocation( GLuint& prog, string name){
    GLint loc;
    loc = glGetUniformLocation( prog, name.c_str() );
    if ( loc == -1 ){
        cerr << "Could not bind uniform : " << name << ". "
            << "Did you set the right name? "
            << "Or is " << name << " not used?" << endl;
    }
    return loc;
}

GLuint SHADER::program(GLuint vs, GLuint fs){
    GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	glLinkProgram(prog);
	GLint link_ok;
	glGetProgramiv(prog, GL_LINK_STATUS, &link_ok);

	if(link_ok == GL_FALSE){
		std::cout << "Link failed." << std::endl;
	}

    glUseProgram(prog);
    return prog;
}

mesh_info_t OBJ::load( string filename ){
    ifstream fin;
    fin.open( filename.c_str() );

    if( !( fin.good() ) ){
        cerr << "failed to open file : " << filename << endl;
    }

    int temp_vNum = 0; //number of vertices
    while( fin.peek() != EOF ){//read obj loop
        string s;
        fin >> s;

        //vertex coordinate
        if( "v" == s ){
            float x, y, z, w;
            fin >> x;
            fin >> y;
            fin >> z;
            tempV.push_back( vec3( x, y, z ) );
            vertexNumber++;
        }
        //face normal (recorded as vn in obj file)
        else if( "vn" == s ){
            float x, y, z;
            fin >> x;
            fin >> y;
            fin >> z;
            tempVn.push_back( vec3( x, y, z ) );
            faceNormalNumber++;
        }
        //vertices contained in face, and face normal
        else if( "f" == s ){
            ivec4 v4i;//for v in "v//vn"
            int v1i;//for vn in "v//vn"

            fin >> v4i[0];//first "v//vn"
            fin.ignore( 2 );//remove "//"
            fin >> v1i;//all vn in "v//vn" are same

            fin >> v4i[1];//second "v//vn"
            fin.ignore( 2 );
            fin >> v1i;

            fin >> v4i[2];//third "v//vn"
            fin.ignore( 2 );
            fin >> v1i;

            //in case we are dealing with triangle face
            if( fin.peek() != '\n' ){
                fin >> v4i[3];//fourth "v//vn"
                fin.ignore( 2 );
                fin >> v1i;
            }
            else{
                v4i[3] = -1;//means "no value"
            }

            //CAUTION:
            //  v and vn in "v//vn" start from 1,
            //  but indices of std::vector start from 0,
            //  so we need (v4i - 1), (v1i - 1)
            tempFaceContainedVertexIndex.push_back( v4i - 1 );
            tempFaceNormalIndex.push_back( v1i - 1 );

            faceNumber++;
        }
    }//end read obj loop

    fin.close();

    /* Computing mesh information from those temp*s */
    mesh_info_t meshInfo;

    for (size_t i = 0; i < vertexNumber; i++) {
        vertex_info_t vertexInfo;

        vertexInfo.vertexIndex = i;
        vertexInfo.vertexCoordinate = tempV[i];

        meshInfo.vertexTable.push_back( vertexInfo );
    }

    for (size_t i = 0; i < faceNumber; i++) {
        face_info_t faceInfo;

        faceInfo.faceIndex = i;
        faceInfo.containedVertexIndex = tempFaceContainedVertexIndex[i];
        faceInfo.faceNormal = tempVn[ tempFaceNormalIndex[i] ];

        meshInfo.faceTable.push_back( faceInfo );
    }

    /* compute "connected face index" */
    //check "contained vertex index" in face table,
    //write "face index" to "connected face index"
    for (size_t i = 0; i < faceNumber; i++) {
        //for convenience
        ivec4& cvIdx = meshInfo.faceTable[i].containedVertexIndex;

        for (size_t j = 0; j < 4; j++) {//ivec4 has 4 elements
            //for convenience
            int vtxIdx = cvIdx[j];
            vector<int>& cfIdx = meshInfo.vertexTable[ vtxIdx ].connectedFaceIndex;

            if( -1 != vtxIdx ){//in case we are dealing with a triangle face
                cfIdx.push_back( i );//"i" equals to face index
            }
        }
    }

    /* compute "vertex normal" */
    //check "connected face index" in vertex table,
    //extract "face normal" corresponding to "connected face index",
    //summerize them and compute their mean
    for (size_t i = 0; i < vertexNumber; i++) {
        vector<int>& cfIdx = meshInfo.vertexTable[i].connectedFaceIndex;
        vec3 vtxNormal( 0.f );

        //summerize
        for (size_t j = 0; j < cfIdx.size(); j++) {
            int faceIdx = cfIdx[j];
            face_info_t faceInfo = meshInfo.faceTable[ faceIdx ];
            vtxNormal += faceInfo.faceNormal;
        }
        vtxNormal /= cfIdx.size();//means

        //always normalize
        meshInfo.vertexTable[i].vertexNormal = normalize( vtxNormal );
    }

	//other information
	meshInfo.vertexNumber = vertexNumber;

	//convert quadrangle to triangle
    for (size_t i = 0; i < faceNumber; i++) {
        ivec4 vtxidx = meshInfo.faceTable[i].containedVertexIndex;

        //triangle one
        meshInfo.triangleIndex.push_back(
            ivec3( vtxidx[0], vtxidx[1], vtxidx[2] )
        );

        //if there is triangle two
        if( -1 != vtxidx[3]){
            meshInfo.triangleIndex.push_back(
                ivec3( vtxidx[2], vtxidx[3], vtxidx[0] )
            );
        }
    }

    return meshInfo;
}

GLfloat* getCircle(GLfloat r, GLfloat x, GLfloat y, GLfloat* data, int num)
{
    for(int i = 0; i < num*2-1 ; i += 2){
        data[i] = x + r * cos( i*3.14/num );
        data[i+1] = y + r * sin( i * 3.14 / num);
    }
    return data;


}
