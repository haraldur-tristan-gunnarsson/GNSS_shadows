#version 130

in vec4 shad_pos;
out vec4 out_colour;

uniform sampler2D shadow_sampler;
//uniform sampler2DShadow shadow_sampler;
//uniform sampler2DRectShadow shadow_sampler;

void main(){
    out_colour = vec4(vec3(gl_FragCoord.z),1.0);
    //out_colour = gl_FragCoord;
    //if(depth > (z + 0.00001))out_colour = vec4(1.0);
    //if(depth < (z + 0.001))out_colour = vec4(1.0);
    //if(depth > (z - 0.00001))out_colour = vec4(1.0);
    //else out_colour = vec4(vec3(0.0),1.0);
}
