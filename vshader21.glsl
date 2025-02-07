#version 150

in vec2 vPosition;
in vec3 vColor;       // New attribute for color
out vec3 color;       // Pass to fragment shader

void main() {
    gl_Position = vec4(vPosition, 0.0, 1.0);
    color = vColor;
}