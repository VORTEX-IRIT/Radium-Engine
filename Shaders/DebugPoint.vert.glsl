layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inCol;

uniform mat4 view;
uniform mat4 proj;

out vec3 vCol;

void main()
{

    gl_Position = proj * view * vec4(inPos, 1.0);
    vCol = inCol;

    gl_PointSize = 40 / gl_Position.w;
}
