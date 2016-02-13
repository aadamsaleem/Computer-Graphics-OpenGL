/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150 


in  vec4 color;
out vec4 fColor;
in vec4 point;
in vec2 TexCoord;
in float sphere_texture;
in vec2 sphere_2D;
in vec2 lattice_pos;

uniform int fogFlag;
uniform vec4 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;
uniform int sphereTexureFlag;
uniform int lattice_flag;

uniform int Texture_app_flag;

uniform sampler1D sphere_texture_1D;
uniform sampler2D floor_texture_2D;
uniform sampler2D sphere_texture_2D;

void main() 
{ 
    float f, clamp;
	vec4 temp = color;

	if(Texture_app_flag == 1)
		temp = color * texture(floor_texture_2D, TexCoord);

	if(sphereTexureFlag == 1)
		temp = color * texture(sphere_texture_1D,sphere_texture);

	if(sphereTexureFlag == 2)
	{
		temp = texture(sphere_texture_2D,sphere_2D);
		if(temp.x < 0.5)
		{
			temp.x = 0.9;
			temp.y = 0.1;
			temp.z = 0.1;
			temp.w = 1.0;
		}
		temp = color * temp;
	}
	if(lattice_flag == 1)
	{
		if(fract(4 * lattice_pos.x) < 0.35 && fract(4 * lattice_pos.y) < 0.35)
			discard;
	}
	
	if(fogFlag == 1)
	{
		fColor = temp;
	}
	else if (fogFlag ==2)
	{
		f = ((fogEnd - point.z)/(fogEnd - fogStart));
		clamp = clamp(f,0.0,1.0);
		fColor = mix(fogColor, temp, clamp);
	}
	else if (fogFlag ==3)
	{
		f = exp(-(fogDensity * point.z));
		clamp = clamp(f,0.0,1.0);
		fColor = mix(fogColor, temp, clamp);
	}
	else if (fogFlag ==4)
	{
		f= exp(-pow((fogDensity * point.z),2));
		clamp = clamp(f,0.0,1.0);
		fColor = mix(fogColor, temp, clamp);
	}
} 

