/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader for particle system.
 ***************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec3 vPosition;
in  vec3 vColor;
out vec4 color;
in vec3 velocity;
out float position;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform float time;


void main() 
{
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, 1.0); 

	float a = -0.00000049;

	float x = vPosition4.x + (0.001 * velocity.x * time );
	float y = vPosition4.y + (0.001 * velocity.y * time ) + (0.5 * a * pow(time,2));
	float z = vPosition4.z + (0.001 * velocity.z * time );

	vec4 vPosition5 = vec4(x,y,z,1.0); 

    gl_Position = Projection * ModelView * vPosition5;
	position = y;
    color = vColor4;
} 
