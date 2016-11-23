#version 130

in vec4 shad_pos;//Position in view from satellite (as if visible).
out vec4 out_colour;

uniform int zen_dist;//Current zenith distance.
uniform sampler2D shadow_sampler;//Depth map from satellite view.
uniform sampler2D map_sampler;//Image from previous (zenith distance) iteration.

void main(){//Only update pixels that are still white, as others have already entered shadow.
	float old_angle = texture(map_sampler, gl_FragCoord.xy/1024).x;
	if(1.0 != old_angle){
		out_colour = vec4(vec3(old_angle),1.0);
		return;
	}
	else{
		vec3 shad_xyz = shad_pos.xyz * 0.5 + vec3(0.5);//Clip space to NDC
		float depth = texture(shadow_sampler, shad_xyz.xy).x;
		if(depth > (shad_xyz.z - 0.001))out_colour = vec4(vec3(old_angle),1.0);//Unshadowed areas should not be modified.
		else out_colour = vec4(vec3(float(zen_dist)/255),1.0);//Better to use integers directly, how?
		return;
	}
}
