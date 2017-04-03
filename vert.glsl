#version 330
layout( location = 0 ) in vec3 vPosition;

out vec3 fragmentColor;

uniform mat4 M, V, P;

void main(){
    //projection plane
    gl_Position = P * V * M * vec4( vPosition,  1.0 );
    //fragmentColor = vPosition/20;




}
