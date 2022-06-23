#version 330

// Input
// TODO(student): Get color value from vertex shader
in vec3 color;
in float noise;


// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Write pixel out color
     out_color = vec4(color, 1);

    // vec3 color = vec3(vUv * ( 1. - 2. * noise ), 0.0);
    // out_color = vec4(color.rgb, 1.0);
}
