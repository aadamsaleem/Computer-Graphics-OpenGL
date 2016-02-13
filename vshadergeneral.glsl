/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/
#version 150 

in vec3 vPosition;
in  vec3 vNormal;
in  vec3 vColor;
in vec2 vTexCoord;
out vec4 color;
out vec4 point;
out vec2 TexCoord;
out float sphere_texture;
out vec2 sphere_2D;
out vec2 lattice_pos;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform float Shininess;

uniform vec4 Point_AmbientProduct, Point_DiffuseProduct, Point_SpecularProduct;
uniform vec4 LightDirection;
uniform vec4 GlobalAmbientLight;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation


uniform int lightdestinationFlag;
uniform int lightsourceFlag;

uniform vec4 SpotLight;
uniform float exponent_value;
uniform float cutoff_angle;

uniform int sphere_texture_flag;
uniform int space_flag;
uniform int latticeEffectFlag;


vec4 distant_light_source()
{
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition4).xyz;
	
	vec3 L = normalize((-1)*LightDirection.xyz);
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
      // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    vec3 N = normalize(Normal_Matrix * vNormal);

	/*--- To Do: Compute attenuation ---*/
	float attenuation = 1.0; 

	// Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float d = max( dot(L, N), 0.0 );
    vec4  diffuse = d * DiffuseProduct;

    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = Projection * ModelView * vPosition4;
	return (attenuation * (ambient + diffuse + specular));
}

vec4 light_source(int lightsourceFlag)
{
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
		// Transform vertex  position into eye coordinates
		vec3 pos = (ModelView * vPosition4).xyz;
		
		vec3 L = normalize( LightPosition.xyz - pos );
		vec3 E = normalize( -pos );
		vec3 H = normalize( L + E );

		vec3 N = normalize(Normal_Matrix * vNormal);
		
		float attenuation = 0.0;
		
	if(lightsourceFlag == 1)		//point source light
	{	
		float distance = sqrt(pow((pos.x - LightPosition.x), 2) + pow((pos.y - LightPosition.y), 2) + pow((pos.z - LightPosition.z), 2));
		/*--- To Do: Compute attenuation ---*/
		attenuation = 1.0/(ConstAtt + (LinearAtt * distance) + (QuadAtt * pow(distance,2))); 
	}
	else if (lightsourceFlag ==2)		//spotlight
	{
		vec3 Lf = normalize(SpotLight.xyz - LightPosition.xyz);
		float spotlight_attenuation = 0.0;
		vec3 l = normalize(pos.xyz - LightPosition.xyz);
		
		if(dot(Lf, l) < cos(cutoff_angle))
			spotlight_attenuation = 0;
		else
			spotlight_attenuation = pow(dot(Lf, l),exponent_value);
		
		/*--- To Do: Compute attenuation ---*/
		float distance = sqrt(pow((vPosition4.x - LightPosition.x), 2) + pow((vPosition4.y - LightPosition.y), 2) + pow((vPosition4.z - LightPosition.z), 2));

		attenuation = spotlight_attenuation/(ConstAtt + (LinearAtt * distance) + (QuadAtt * pow(distance,2))); 

	}
	
	// Compute terms in the illumination equation
	vec4 ambient = Point_AmbientProduct;

	float d = max( dot(L, N), 0.0 );
	vec4  diffuse = d * Point_DiffuseProduct;

	float s = pow( max(dot(N, H), 0.0), Shininess );
	vec4  specular = s * Point_SpecularProduct;
		
	if( dot(L, N) < 0.0 )
		specular = vec4(0.0, 0.0, 0.0, 1.0); 

	gl_Position = Projection * ModelView * vPosition4;
	return (attenuation * (ambient + diffuse + specular));
}

void main()
{
	vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	
	vec3 pos = (ModelView * vPosition4).xyz;
	
	if(lightdestinationFlag==1)
			color = GlobalAmbientLight + distant_light_source() + light_source(lightsourceFlag);
	
	if(lightdestinationFlag==0)
	{
		
		vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, 0.65); 

		gl_Position = Projection * ModelView * vPosition4;

		color = vColor4;
	}
	point = Projection * ModelView * vPosition4;
	
	TexCoord.x = vTexCoord.x * 6.0;
	TexCoord.y = vTexCoord.y * 5.0;
	//TexCoord = vTexCoord;        //Texture for ground
	
	if(sphere_texture_flag == 1)    //1D vertical texture on sphere
	{
		if(space_flag == 1)
			sphere_texture =  2.5 * pos.x;
		if(space_flag == 0)
			sphere_texture =  2.5 * vPosition4.x;
	}
	if(sphere_texture_flag == 2)     //1D slanted texture on sphere
	{
		if(space_flag == 1)
			sphere_texture =  1.5 * (pos.x + pos.y + pos.z);
		if(space_flag == 0)
			sphere_texture =  1.5 * (vPosition4.x + vPosition4.y + vPosition4.z);
	}
	
	if(sphere_texture_flag == 3)      //2D vertical texture on sphere
	{
		if(space_flag == 1)
		{
			sphere_2D.x = 0.75*(pos.x + 1);
			sphere_2D.y = 0.75*(pos.y + 1);
		}
		if(space_flag == 0)	
		{
			sphere_2D.x = 0.75*(vPosition4.x + 1);
			sphere_2D.y = 0.75*(vPosition4.y + 1);
		}	
	}
	
	if(sphere_texture_flag == 4)            //1D slanted texture on sphere
	{
		if(space_flag == 1)
		{
			sphere_2D.x = 0.45 * (pos.x + pos.y + pos.z);
			sphere_2D.y = 0.45 * (pos.x - pos.y + pos.z);
		}
		if(space_flag == 0)	
		{
			sphere_2D.x = 0.45 * (vPosition4.x + vPosition4.y + vPosition4.z);
			sphere_2D.y = 0.45 * (vPosition4.x - vPosition4.y + vPosition4.z);
		}	
	}
	
	if(latticeEffectFlag == 1)          //Upright Lattice effect
	{
		lattice_pos.x = 0.5 * (vPosition4.x + 1);
		lattice_pos.y = 0.5 * (vPosition4.y + 1);
	}
	if(latticeEffectFlag == 2)          //Tilted lattice effect
	{
		lattice_pos.x = 0.3 * (vPosition4.x + vPosition4.y + vPosition4.z);
		lattice_pos.y = 0.3 * (vPosition4.x - vPosition4.y + vPosition4.z);
	}
}
