#version 130

in vec4 shad_pos;
out vec4 out_colour;

uniform int zen_dist;
uniform sampler2D shadow_sampler;
//uniform sampler2DShadow shadow_sampler;
//uniform sampler2DRectShadow shadow_sampler;
uniform sampler2D map_sampler;

void main(){
    float old_angle = texture(map_sampler, gl_FragCoord.xy/1024).x;
    if(1.0 != old_angle){
        out_colour = vec4(vec3(old_angle),1.0);
        return;
    }
    else{
        vec3 shad_xyz = shad_pos.xyz * 0.5 + vec3(0.5);//Clip space to NDC
        float depth = texture(shadow_sampler, shad_xyz.xy).x;
        if(depth > (shad_xyz.z - 0.001))out_colour = vec4(vec3(old_angle),1.0);
        else out_colour = vec4(vec3(float(zen_dist)/255),1.0);//Better to use integers directly, how?
        return;
    }
//    vec2 xy = shad_pos.xy * 0.5 + vec2(0.5, 0.5);
//    float z = shad_pos.z * 0.5 + 0.5;
//    float depth = texture(shadow_sampler, xy).x;
//    float new_angle = float(zen_dist)/90;//Better to use integers directly, how?
//    float angle = old_angle;
//    if(1.0 == angle)angle = new_angle;//90 == angle
//    //if(depth > (z + 0.00001))out_colour = vec4(1.0);
//    if(depth > (z - 0.00001))out_colour = vec4(vec3(old_angle),1.0);
//    else out_colour = vec4(vec3(angle),1.0);
}
