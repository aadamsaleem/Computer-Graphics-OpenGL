/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader for particle system
 *****************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
out vec4 fColor;
in float position;

void main() 
{ 
	if(position > 0.1)
		fColor = color;
	else
		discard;
} 

