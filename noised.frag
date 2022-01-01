#version 460 core

in vec4 vertexToFragColor;

out vec4 theColor;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    theColor=vec4(rand(vertexToFragColor.xy),rand(vertexToFragColor.yz), rand(vertexToFragColor.zw), rand(vertexToFragColor.wx));
}
