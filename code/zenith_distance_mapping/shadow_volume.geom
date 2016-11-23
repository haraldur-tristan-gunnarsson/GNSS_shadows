#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 8) out;//Need only a triangular tube: 6 triangles so 8 vertices.

layout(location = 2) uniform vec4 satellite_vector;

void main(){//Generate a triangular tube from the input triangle and the satellite vector:
	//Unwrapped triangular tube:
	//
	//N:	0_1_2_0
	//	|\|\|\|
	//F:	0-1-2-0
	//
	//	F0,N0,F1,N1,F2,N2,F0,N0
	vec4 N0 = gl_in[0].gl_Position;
	vec4 N1 = gl_in[1].gl_Position;
	vec4 N2 = gl_in[2].gl_Position;
	vec4 F0 = N0 + satellite_vector;
	vec4 F1 = N1 + satellite_vector;
	vec4 F2 = N2 + satellite_vector;
	gl_Position = F0; EmitVertex();//0	GL_CCW
	gl_Position = N0; EmitVertex();//1
	gl_Position = F1; EmitVertex();//2
	gl_Position = N1; EmitVertex();//3
	gl_Position = F2; EmitVertex();//4
	gl_Position = N2; EmitVertex();//5
	gl_Position = F0; EmitVertex();//6
	gl_Position = N0; EmitVertex();//7
	EndPrimitive();
}
