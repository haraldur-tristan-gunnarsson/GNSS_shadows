#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 8) out;
//layout (triangle_strip, max_vertices = 10) out;

//layout(location = 0) uniform mat4 proj_mat;
layout(location = 2) uniform vec4 satellite_vector;

void main(){
////	vec4 N0 = gl_in[0].gl_Position;
////	vec4 N1 = gl_in[1].gl_Position;
////	vec4 N2 = gl_in[2].gl_Position;
////	vec4 F0 = N0 + satellite_vector;
////	vec4 F1 = N1 + satellite_vector;
////	vec4 F2 = N2 + satellite_vector;
////	N0 = proj_mat * N0;
////	N1 = proj_mat * N1;
////	N2 = proj_mat * N2;
////	F0 = proj_mat * F0;
////	F1 = proj_mat * F0;
////	F2 = proj_mat * F0;
	vec4 N0 = gl_in[0].gl_Position;// + vec4(0,0,0.0001,0);
	vec4 N1 = gl_in[1].gl_Position;// + vec4(0,0,0.0001,0);
	vec4 N2 = gl_in[2].gl_Position;// + vec4(0,0,0.0001,0);
	vec4 F0 = N0 + satellite_vector;
	vec4 F1 = N1 + satellite_vector;
	vec4 F2 = N2 + satellite_vector;
//	gl_Position = F0; EmitVertex();//0	GL_CCW
//	gl_Position = N0; EmitVertex();//1
//	gl_Position = F1; EmitVertex();//2
//	gl_Position = N1; EmitVertex();//3
//	gl_Position = F2; EmitVertex();//4
//	gl_Position = N2; EmitVertex();//5
//	gl_Position = F0; EmitVertex();//6
//	gl_Position = N0; EmitVertex();//7
//	EndPrimitive();
	gl_Position = N0; EmitVertex();//0
	gl_Position = N1; EmitVertex();//1
	gl_Position = F0; EmitVertex();//2
	gl_Position = F1; EmitVertex();//3
	gl_Position = F2; EmitVertex();//4
	gl_Position = N1; EmitVertex();//5
	gl_Position = N2; EmitVertex();//6
	gl_Position = N0; EmitVertex();//7
	gl_Position = F2; EmitVertex();//8
	gl_Position = F0; EmitVertex();//9
	EndPrimitive();
}
