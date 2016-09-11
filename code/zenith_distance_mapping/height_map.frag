#version 130

out vec4 out_colour;
//uniform int win_x_x2;
//uniform float zmin;
//uniform float inverse_z_range;

void main(){
    //out_colour = vec4(1.0);
    //out_colour = vec4(gl_FragCoord.z);
    float zval = 1 - (gl_FragCoord.z);// * 512 - 512 * zmin) * 1 * inverse_z_range;// / 512.0;
    //float zval = (gl_FragCoord.z + 8 * zmin) * -1 * inverse_z_range / 8.0;
    //float zval = (gl_FragCoord.z * win_x_x2 - zmin) * inverse_z_range;
    out_colour = vec4(zval);
}
