#version 330
layout( location = 0 ) in vec2 vPosition;

out vec3 fragmentColor;

uniform mat4 M, V, P;

void main(){
    //projection plane
    gl_Position = P * V * M * vec4( vPosition, 0.0,  1.0 );




}
