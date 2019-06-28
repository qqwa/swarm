#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aWorldPos;
layout (location = 2) in vec3 aDirection;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 uu = normalize(cross(aDirection, vec3(0,1,0)));
    vec3 vv = normalize(cross(uu, aDirection));
    mat4 model = mat4(
        vec4(uu.xyz, 0),
        vec4(vv.xyz, 0),
        vec4(aDirection.xyz, 0),
        vec4(aWorldPos.xyz, 1)
    );
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}