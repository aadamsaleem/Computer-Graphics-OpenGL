#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec3  point3;


GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program,programFirework;			/* shader program object id */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint floor_buffer;	/* vertex buffer object id for floor */
GLuint axis_buffer;		/* vertex buffer object id for floor */
GLuint shadow_buffer;	/* vertex buffer object id for shadow */
GLuint firework_buffer; /* vertex buffer object id for firework */

// Projection transformation parameters
GLfloat  fovy = 60.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 15.0;

bool rightmouse = false;
static int shadow_menu, enable_lighting_menu, shading_menu, lightSource_menu, fog_menu, shadow_blending_menu, texture_ground_menu, fireworks_menu, texture_sphere_menu;

vec3 OA = vec3(-4.0, 1.0, 4.0);
vec3 OB = vec3(-1.0, 1.0, -4.0);
vec3 OC = vec3(3.0, 1.0, 5.0);
vec3 OY = vec3(0.0, 1.0, 0.0);
vec3 AB = OB - OA;
vec3 BC = OC - OB;
vec3 CA = OA - OC;

double d = 0.0;
mat4 accum = mat4(		//Identity matrix for accmulated rotaion
	vec4(1, 0, 0, 0),
	vec4(0, 1, 0, 0),
	vec4(0, 0, 1, 0),
	vec4(0, 0, 0, 1));

mat4 shadowMatrix = mat4(
	vec4(12, 14, 0, 0),
	vec4(0, 0, 0, 0),
	vec4(0, 3, 12, 0),
	vec4(0, -1, 0, 12));

vec3 trans, rot;

char start = 'a';

double xtranslate = -4.0;
double ytranslate = 1.0;
double ztranslate = 4.0;

double xrotate = 2.0;
double yrotate = 0.0;
double zrotate = 2.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'
int wireframeSphere = 0;   // 1: wireframe sphere; 0: solid sphere. Toggled by key 's' or 'S'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int shadowFlag = 0; // 1: showshadow; 0: no shadow
int lightingFlag = 0; // 1: enable lighting; 0: disable lighting
int lightsourceFlag = 1; // 1: Point Source; 2: Spot Light
int lightdestinationFlag = 0; // 1: Sphere Lighting ; 2 : Ground Lighting
int shadingFlag = 0; // // 1: enable shading; 0: disable shading
int fogFlag = 1; //1: no fog; 2: linear fog; 3: exponential; 4: eponential square

int shadowBlendFlag = 0;   // 0: Opaque shadow, 1: Semi Transparent shadow. Toggled by menu action on left mouse button
int floorTexureFlag = 0;   // 0: No texture on ground, 1: Texture mapping on ground
int fireworksFlag = 0;  // 0: No fireworks, 1: Enable Fireworks
int sphereTextureFlag = 0;   // 0: No Texture, 1: Contour lines, 2: Checkerboard 
int verticalSlantedFlag = 1;  // 1: Vertical, 2: Slanted
int uprightTiltedFlag = 1;  // 1: Upright, 2: Tilted
int spaceFlag = 0; // 0: Object Space, 1: Eye Space
int latticeFlag = 0;    // 0: No Lattice Effect, 1: Lattice effect 

int sphere_NumVertices = 0;

point3 sphere_points[3073];
color4 sphere_colors[3073];
vec3 sphere_normals[3073];
vec3 sphere_normals_smooth[3073];

int shadow_NumVertices = 0;

point3 shadow_points[3073];
color4 shadow_colors[3073];

point3 **polygons;
int numberPoly = 0;

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 normal_floor[floor_NumVertices];

										//Floor vetices
point3 floorvertices[7] = {
	point3(-5.0, 0.0, -4.0),
	point3(-5.0, 0.0, 8.0),
	point3(5.0,  0.0,  -4.0),
	point3(5.0, 0.0,  8.0)
	
};

// RGBA colors
color4 vertex_colors[10] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.84, 0.0, 1.0),  // golden yellow
	color4(0.25, 0.25, 0.25, 0.65)// shadow
};

//axis varialbles
const int axis_Numvertices = 6;
point3 axis_points[axis_Numvertices];

//axis vertices
point3 axis_vertices[4] =
{
	point3(0.0,0.0,0.0),
	point3(6.0, 0.0, 0.0),
	point3(0.0,10.0,0.0),
	point3(0.0,0.0,8.0)
};
color4 axis_colors[axis_Numvertices];

point4 lightsource = point4(-14.0, 12.0, -3.0, 1.0);

/*Distant Light Source Lighting parameters*/
//lighting parameters
color4 GlobalAmbientLight(1.0, 1.0, 1.0, 1.0);
color4 BlackAmbientLight(0.0, 0.0, 0.0, 1.0);
color4 diffuse_color(0.8, 0.8, 0.8, 1.0);
color4 specular_color(0.2, 0.2, 0.2, 1.0);
point4 direction(0.1, 0.0, -1.0, 0.0);         //direction already given in the eye co-ordinate system

//ground lighting colours
color4 grounddiffuse(0.0, 1.0, 0.0, 1.0);
color4 groundambient(0.2, 0.2, 0.2, 1.0);
color4 groundspecular(0.0, 0.0, 0.0, 1.0);

//ground lighting products
color4 ground_ambient_product = BlackAmbientLight * groundambient;
color4 ground_diffuse_product = diffuse_color * grounddiffuse;
color4 ground_specular_product = specular_color * groundspecular;
color4 ground_global_ambient = GlobalAmbientLight * groundambient;

//sphere lighting colours 
color4 spherediffuse(1.0, 0.84, 0.0, 1.0);
color4 spherespecular(1.0, 0.84, 0.0, 1.0);
color4 sphereambient(0.2, 0.2, 0.2, 1.0);
float shininess_coeffecient = 125.0;

//sphere lighting products
color4 sphere_ambient_product = BlackAmbientLight * sphereambient;
color4 sphere_diffuse_product = diffuse_color * spherediffuse;
color4 sphere_specular_product = specular_color * spherespecular;
color4 sphere_global_ambient = GlobalAmbientLight * sphereambient;

/*Positional lighting source parameters*/
//Lighting Parameters
color4 positional_ambient(0.0, 0.0, 0.0, 1.0);
color4 positional_diffuse(1.0, 1.0, 1.0, 1.0);
color4 positional_specular(1.0, 1.0, 1.0, 1.0);

//ground lighting products
color4 positional_ground_ambient_product = positional_ambient * groundambient;
color4 positional_ground_diffuse_product = positional_diffuse * grounddiffuse;
color4 positional_ground_specular_product = positional_specular * groundspecular;

//Sphere Lighting products
color4 positional_sphere_ambient_product = positional_ambient * sphereambient;
color4 positional_sphere_diffuse_product = positional_diffuse * spherediffuse;
color4 positional_sphere_specular_product = positional_specular * spherespecular;

//Constant, Linear, Quadratic Attenuation
float const_att = 2.0;
float linear_att = 0.01;
float quad_att = 0.001;

//SpotLight parameters
point4 spot_point = point4(-6.0, 0.0, -4.5, 1.0);
float exponent_value = 15.0;
float cutoff_angle = 20.0;
float cutoff_angle_radians = cutoff_angle * M_PI / 180.0;

//fog parameters
color4 fogColor = color4(0.7, 0.7, 0.7, 0.5);
float fogStart = 0.0;
float fogEnd = 18.0;
float fogDensity = 0.09;

/* global definitions for constants and global image arrays */
#define ImageWidth 32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];
static GLuint texName;

//Firework particles definition
const int number_of_particles = 300;
color4 firework_colors[number_of_particles];
point3 firework_vertices[number_of_particles];
point3 firework_velocity[number_of_particles];
point3 initial_point(0.0, 0.1, 0.0);
float  tsub = 0, tmax = 9900.0, t = 0;

//Sphere Mapping Co-ordinates
static GLuint texname_1d, texname_2d;

//Floor Texture co-ordinates
vec2 textCoord_floor[6] = {
	vec2(1.0, 0.0),  // for vertex[0]
	vec2(0.0, 0.0),  // for vertex[1]
	vec2(0.0, 1.0),  // for vertex[3]

	vec2(1.0, 0.0),  // for vertex[0]
	vec2(1.0, 1.0),  // for vertex[2]
	vec2(0.0, 1.0),  // for vertex[3]
};

//----------------------------------------------------------------------------
int Index = 0; // This must be a global variable since quad() is called
			   // multiple times

//----------------------------------------------------------------------------
// quad(): generate two triangles for each face and assign colors to the vertices
void quad(int a, int b, int c, int d)
{
	vec3 u = polygons[a][c] - polygons[a][b];
	vec3 v = polygons[a][d] - polygons[a][b];

	vec3 normal = normalize(cross(u, v));
	
	sphere_colors[Index] = vertex_colors[8];
	sphere_points[Index] = polygons[a][b];
	sphere_normals[Index] = normal;
	sphere_normals_smooth[Index] = polygons[a][b];
	Index++;

	sphere_colors[Index] = vertex_colors[8];
	sphere_points[Index] = polygons[a][c];
	sphere_normals[Index] = normal;
	sphere_normals_smooth[Index] = polygons[a][c];
	Index++;

	sphere_colors[Index] = vertex_colors[8];
	sphere_points[Index] = polygons[a][d];
	sphere_normals[Index] = normal;
	sphere_normals_smooth[Index] = polygons[a][d];
	Index++;
}

//----------------------------------------------------------------------------
void colorsphere()
{
	for (int i = 0; i < numberPoly; i++)
		quad(i, 0, 1, 2);
}

void shadowquad(int a, int b, int c, int d)
{
	shadow_colors[Index] = vertex_colors[9];
	shadow_points[Index] = polygons[a][b]; Index++;

	shadow_colors[Index] = vertex_colors[9];
	shadow_points[Index] = polygons[a][c]; Index++;

	shadow_colors[Index] = vertex_colors[9];
	shadow_points[Index] = polygons[a][d]; Index++;
}

//----------------------------------------------------------------------------
void shadow()
{
	for (int i = 0; i < numberPoly; i++)
		shadowquad(i, 0, 1, 2);
}


//-------------------------------
// generate 2 triangles: 6 vertices
void floor()
{
	vec3 u = floorvertices[1] - floorvertices[0];
	vec3 v = floorvertices[2] - floorvertices[0];

	vec3 normal = normalize(cross(u, v));

	floor_colors[0] = vertex_colors[3]; floor_points[0] = floorvertices[0]; normal_floor[0] = normal;
	floor_colors[1] = vertex_colors[3]; floor_points[1] = floorvertices[1]; normal_floor[1] = normal;
	floor_colors[2] = vertex_colors[3]; floor_points[2] = floorvertices[3]; normal_floor[2] = normal;

	floor_colors[3] = vertex_colors[3]; floor_points[3] = floorvertices[0]; normal_floor[3] = normal;
	floor_colors[4] = vertex_colors[3]; floor_points[4] = floorvertices[2]; normal_floor[4] = normal;
	floor_colors[5] = vertex_colors[3]; floor_points[5] = floorvertices[3]; normal_floor[5] = normal;
}

//-------------------------------
// generate 3 lines : 6 vertices
void axis()
{
	axis_colors[0] = vertex_colors[1]; axis_points[0] = axis_vertices[0];
	axis_colors[1] = vertex_colors[1]; axis_points[1] = axis_vertices[1];

	axis_colors[2] = vertex_colors[5]; axis_points[2] = axis_vertices[0];
	axis_colors[3] = vertex_colors[5]; axis_points[3] = axis_vertices[2];

	axis_colors[4] = vertex_colors[4]; axis_points[4] = axis_vertices[0];
	axis_colors[5] = vertex_colors[4]; axis_points[5] = axis_vertices[3];
}

//Image Set Up function provided
void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* green */
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}
			else  /* white */
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/

	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		*/
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j>4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
	init() (set up texture objects),
	display() (activate the texture object to be used, etc.)
	and in shaders.
	---*/

} /* end function */


  /*Set up Random Particle Colors and Velocity and Initial Position*/
void fireworks()
{
	int i = 0;
	for (i = 0;i < number_of_particles;i++)
	{
		firework_vertices[i].x = initial_point.x;
		firework_vertices[i].y = initial_point.y;
		firework_vertices[i].z = initial_point.z;
		
		firework_colors[i].x = (rand() % 256) / 256.0;
		firework_colors[i].y = (rand() % 256) / 256.0;
		firework_colors[i].z = (rand() % 256) / 256.0;
		firework_colors[i].w = 1.0;

		firework_velocity[i].x = 2.0*((rand() % 256) / 256.0 - 0.5);
		firework_velocity[i].z = 2.0*((rand() % 256) / 256.0 - 0.5);
		firework_velocity[i].y = 1.2*2.0*((rand() % 256) / 256.0);
	}
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{	
	image_set_up();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Create and Initialize a texture object (1D texture for sphere) ---*/
	glGenTextures(1, &texname_1d);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE1);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_1D, texname_1d); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, ImageWidth,	0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	/*--- Create and Initialize a texture object (2D Texture for sphere)---*/
	glGenTextures(1, &texname_2d);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE2);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, texname_2d); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
	
	
	colorsphere();
	// Create and initialize a vertex buffer object for sphere, to be used in display()
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * sphere_NumVertices, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices, sphere_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals_smooth);

	//-----------------------------------------------------------------------------------------------------------------
	Index = 0;		//reset the index
	shadow();
	// Create and initialize a vertex buffer object for shadow, to be used in display()
	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * shadow_NumVertices + sizeof(color4) * shadow_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * shadow_NumVertices, shadow_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * shadow_NumVertices, sizeof(color4) * shadow_NumVertices, shadow_colors);


	//-----------------------------------------------------------------------------------------------------------------

	/*--- Create and Initialize a texture object (2D Texture for ground)---*/
	glGenTextures(1, &texName);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

	floor();
	// Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(normal_floor) + sizeof(textCoord_floor),	NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(normal_floor),	normal_floor);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(normal_floor), sizeof(textCoord_floor), textCoord_floor);

	axis();
	// Create and initialize a vertex buffer object for axis, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);

	fireworks();
	// Create and initialize a vertex buffer object for fireworks, to be used in display()
	glGenBuffers(1, &firework_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * number_of_particles + sizeof(color4) * number_of_particles + sizeof(point3) * number_of_particles, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * number_of_particles, firework_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * number_of_particles, sizeof(color4) * number_of_particles, firework_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * number_of_particles + sizeof(color4) * number_of_particles, sizeof(point3) * number_of_particles,	firework_velocity);

	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshadergeneral.glsl", "fshadergeneral.glsl");
	programFirework = InitShader("vshaderfirework.glsl", "fshaderfirework.glsl");  //Firework Shader

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars_Sphere(mat4 mv):
// Set up lighting parameters for sphere that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------
void SetUp_Lighting_Uniform_Vars_Sphere(mat4 mv)
{
	vec4 light_position_eyeFrame = mv * lightsource;

	glUniform1i(glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, sphere_ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, sphere_diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, sphere_specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightDirection"), 1, direction);
	glUniform4fv(glGetUniformLocation(program, "Point_AmbientProduct"), 1, positional_sphere_ambient_product);
	glUniform4fv(glGetUniformLocation(program, "Point_DiffuseProduct"), 1, positional_sphere_diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "Point_SpecularProduct"), 1, positional_sphere_specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position_eyeFrame);
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess_coeffecient);
	glUniform4fv(glGetUniformLocation(program, "GlobalAmbientLight"), 1, sphere_global_ambient);
	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);

	if (lightsourceFlag == 2)
	{
		vec4 spot_eyeframe = mv * spot_point;

		glUniform1f(glGetUniformLocation(program, "exponent_value"), exponent_value);
		glUniform1f(glGetUniformLocation(program, "cutoff_angle"), cutoff_angle_radians);
		glUniform4fv(glGetUniformLocation(program, "SpotLight"), 1, spot_eyeframe);
	}
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars_Floor(mat4 mv):
// Set up lighting parameters for floor that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------
void SetUp_Lighting_Uniform_Vars_Floor(mat4 mv)
{
	vec4 light_position_eyeFrame = mv * lightsource;


	glUniform1i(glGetUniformLocation(program, "lightsourceFlag"), lightsourceFlag);
	glUniform1i(glGetUniformLocation(program, "lightsourceFlag"), 1);
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ground_ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, ground_diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, ground_specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightDirection"), 1, direction);
	glUniform4fv(glGetUniformLocation(program, "Point_AmbientProduct"), 1, positional_ground_ambient_product);
	glUniform4fv(glGetUniformLocation(program, "Point_DiffuseProduct"), 1, positional_ground_diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "Point_SpecularProduct"), 1, positional_ground_specular_product);
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position_eyeFrame);
	glUniform4fv(glGetUniformLocation(program, "GlobalAmbientLight"), 1, ground_global_ambient);
	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);

	if (lightsourceFlag == 2)
	{
		vec4 spot_eyeframe = mv * spot_point;

		glUniform1f(glGetUniformLocation(program, "exponent_value"), exponent_value);
		glUniform1f(glGetUniformLocation(program, "cutoff_angle"), cutoff_angle_radians);
		glUniform4fv(glGetUniformLocation(program, "SpotLight"), 1, spot_eyeframe);

	}
}

void SetUp_Fog_Uniform_Vars()
{
	glUniform1i(glGetUniformLocation(program, "fogFlag"), fogFlag);
	glUniform4fv(glGetUniformLocation(program, "fogColor"), 1, fogColor);
	glUniform1f(glGetUniformLocation(program, "fogStart"), fogStart);
	glUniform1f(glGetUniformLocation(program, "fogEnd"), fogEnd);
	glUniform1f(glGetUniformLocation(program, "fogDensity"), fogDensity);

}



//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, int type)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor;
	if (lightdestinationFlag == 0)
	{
		vColor = glGetAttribLocation(program, "vColor");
		glEnableVertexAttribArray(vColor);                                  //Draw only when Lighting is disabled
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,	BUFFER_OFFSET(sizeof(point3) * num_vertices));
		// the offset is the (total) size of the previous vertex attribute array(s)
	}

	GLuint vNormal;
	if (lightdestinationFlag == 1)
	{
		if (shadingFlag == 0)
		{
			vNormal = glGetAttribLocation(program, "vNormal");
			glEnableVertexAttribArray(vNormal);                            //Flat Shading for sphere when lighting is enabled
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices));
			// the offset is the (total) size of the previous vertex attribute array(s)
		}
		if (shadingFlag == 1)
		{
			vNormal = glGetAttribLocation(program, "vNormal");
			glEnableVertexAttribArray(vNormal);                           //Smooth shading for sphere when lighting is disabled
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices));
			// the offset is the (total) size of the previous vertex attribute array(s)
		}
	}

	if (lightdestinationFlag == 2)
	{
		vNormal = glGetAttribLocation(program, "vNormal");
		glEnableVertexAttribArray(vNormal);                               //Used for Floor when Lighting is enabled
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices));
		// the offset is the (total) size of the previous vertex attribute array(s)
	}
	GLuint vTexCoord;
	if (floorTexureFlag == 1)
	{
		vTexCoord = glGetAttribLocation(program, "vTexCoord");
		glEnableVertexAttribArray(vTexCoord);                           //Used when Ground Texture is enabled
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(floor_points) + sizeof(floor_colors) + sizeof(normal_floor)));
	}


	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */
	if (type == 1)
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	else if (type == 2)
		glDrawArrays(GL_LINE_STRIP, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	if (lightdestinationFlag == 0)
		glDisableVertexAttribArray(vColor);
	if (lightdestinationFlag == 1 || lightdestinationFlag == 2)
		glDisableVertexAttribArray(vNormal);
	if (floorTexureFlag == 1)
		glDisableVertexAttribArray(vTexCoord);
}

void firework(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(programFirework, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(programFirework, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,	BUFFER_OFFSET(sizeof(point3) * num_vertices));

	GLuint velocity = glGetAttribLocation(programFirework, "velocity");
	glEnableVertexAttribArray(velocity);
	glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	/* Draw a sequence of geometric objs (Lines) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */

	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, num_vertices);
	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(velocity);
}

void drawSphere(GLuint  ModelView, mat4 mv, mat3 normal_matrix)
{
	mat4 temp = mv;
	
	mv = mv * Translate(xtranslate, ytranslate, ztranslate) * Rotate(angle, xrotate, yrotate, zrotate) * accum;
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	normal_matrix = NormalMatrix(mv, 1);
	if (lightingFlag == 0)
	{
		lightdestinationFlag = 0;
		glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
	}
	if (lightingFlag == 1)
	{
		lightdestinationFlag = 1;
		glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 1);
		SetUp_Lighting_Uniform_Vars_Sphere(temp);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
	}
	if (wireframeSphere == 1) // Wireframe sphere
	{
		lightdestinationFlag = 0;
		glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else              // Filled sphere
	{
		if (sphereTextureFlag == 1)                      //1D Texture mapping for sphere
		{
			glUniform1i(glGetUniformLocation(program, "sphere_texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program, "sphere_texture_flag"), verticalSlantedFlag);
			glUniform1i(glGetUniformLocation(program, "space_flag"), spaceFlag);
			glUniform1i(glGetUniformLocation(program, "sphereTexureFlag"), 1);
		}
		if (sphereTextureFlag == 2)                     //2D texture mapping for sphere
		{
			glUniform1i(glGetUniformLocation(program, "sphere_texture_2D"), 2);
			glUniform1i(glGetUniformLocation(program, "sphere_texture_flag"), verticalSlantedFlag + 2);
			glUniform1i(glGetUniformLocation(program, "space_flag"), spaceFlag);
			glUniform1i(glGetUniformLocation(program, "sphereTexureFlag"), 2);
		}
		glUniform1i(glGetUniformLocation(program, "lattice_flag"), latticeFlag);               //Lattice effect for sphere
		glUniform1i(glGetUniformLocation(program, "latticeEffectFlag"), uprightTiltedFlag);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glUniform1i(glGetUniformLocation(program, "Texture_app_flag"), 0);
	drawObj(sphere_buffer, sphere_NumVertices, 1);  // draw the sphere
}

void drawFloor(GLuint  ModelView, mat4 mv, mat3 normal_matrix)
{
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	normal_matrix = NormalMatrix(mv, 1);
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	if (lightingFlag == 0)
	{
		lightdestinationFlag = 0;
		glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
	}
	if (lightingFlag == 1)
	{
		lightdestinationFlag = 2;
		glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
		SetUp_Lighting_Uniform_Vars_Floor(mv);
	}
	if (floorTexureFlag == 1)
	{
		glUniform1i(glGetUniformLocation(program, "floor_texture_2D"), 0);
		glUniform1i(glGetUniformLocation(program, "Texture_app_flag"), 1);
	}
	glUniform1i(glGetUniformLocation(program, "sphere_texture_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "sphereTexureFlag"), 0);
	glUniform1i(glGetUniformLocation(program, "lattice_flag"), 0);

	drawObj(floor_buffer, floor_NumVertices, 1);  // draw the floor
}

void drawShadow(GLuint  ModelView, mat4 mv)
{
	mv = mv * shadowMatrix * Translate(xtranslate, ytranslate, ztranslate) * Rotate(angle, xrotate, yrotate, zrotate)*accum;

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (wireframeSphere == 1) // Wireframe shadow
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform1i(glGetUniformLocation(program, "lattice_flag"), 0);
	}
	else              // Filled  shadow
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUniform1i(glGetUniformLocation(program, "lattice_flag"), latticeFlag);
		glUniform1i(glGetUniformLocation(program, "latticeEffectFlag"), uprightTiltedFlag);
	}

	lightdestinationFlag = 0;
	glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
	glUniform1i(glGetUniformLocation(program, "Texture_app_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "sphere_texture_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "sphereTexureFlag"), 0);
	if (eye[1] > 0.0)
		drawObj(shadow_buffer, shadow_NumVertices, 1);  // draw the shadow

}


void drawAxis(GLuint  ModelView, GLuint  Projection, mat4 mv)
{
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	lightdestinationFlag = 0;
	glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
	drawObj(axis_buffer, axis_Numvertices, 2);  // draw the axis
}

void drawFirework(GLuint  ModelView, GLuint  Projection, mat4 mv, mat4 p)
{
	glUseProgram(programFirework);
	ModelView = glGetUniformLocation(programFirework, "ModelView");
	Projection = glGetUniformLocation(programFirework, "Projection");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
	glUniform1f(glGetUniformLocation(programFirework, "time"), t);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
	firework(firework_buffer, number_of_particles);
}
//----------------------------------------------------------------------------
void display(void)
{
	GLuint  ModelView;  // model-view matrix uniform shader variable location
	GLuint  Projection;  // projection matrix uniform shader variable location
	glClearColor(0.529, 0.807, 0.92, 0.0); //Set background to light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program); // Use the shader program

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4  p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	/*---  Set up and pass on Model-View matrix to the shader ---*/
	// eye is a global variable of vec4 set to init_eye and updated by keyboard()
	vec4    at(0.0, 0.0, 0.0, 1.0);
	vec4    up(0.0, 1.0, 0.0, 0.0);

	mat4  mv;
	mat3 normal_matrix;
	mv = LookAt(eye, at, up);
	SetUp_Fog_Uniform_Vars();
	glUniform1i(glGetUniformLocation(program, "lightdestinationFlag"), 0);
	
	if (shadowBlendFlag == 0)
	{
		glDepthMask(GL_FALSE);		//Disable Z-buffer writting

		mv = LookAt(eye, at, up);
		drawFloor(ModelView, mv, normal_matrix);		//draw floor

		glDepthMask(GL_TRUE);			//Enable Z-buffer writting

		mv = LookAt(eye, at, up);
		if (shadowFlag == 1)
			drawShadow(ModelView, mv);		//draw shadow

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);		//Disable frame buffer writting
		glDepthMask(GL_TRUE);
		mv = LookAt(eye, at, up);
		drawFloor(ModelView, mv, normal_matrix);			//draw floor

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);		//Enable frame buffer writting

	}
	else
	{
	
		glDepthMask(GL_FALSE);		//Disable Z-buffer writting
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		mv = LookAt(eye, at, up);
		drawFloor(ModelView, mv, normal_matrix);		//draw floor

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		mv = LookAt(eye, at, up);
		if (shadowFlag == 1)
			drawShadow(ModelView, mv);		//draw shadow

		glDisable(GL_BLEND);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);		//Disable frame buffer writting
		glDepthMask(GL_TRUE);
		mv = LookAt(eye, at, up);
		drawFloor(ModelView, mv, normal_matrix);			//draw floor

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);		//Enable frame buffer writting
	}
	mv = LookAt(eye, at, up);
	drawSphere(ModelView, mv, normal_matrix);		//draw sphere
	
	mv = LookAt(eye, at, up);
	drawAxis(ModelView, Projection, mv);		//draw axis


	if (fireworksFlag == 1)
	{
		mv = LookAt(eye, at, up);
		drawFirework(ModelView, Projection, mv, p);  //draw fireworks
	}
	glutSwapBuffers();
}


//---------------------------------------------------------------------------
void idle(void)
{
	
	angle += 0.1;
	d = (2 * 3.14 * angle) / 360;
	if (start == 'a')
	{
		trans = OA + (d * AB / length(AB));		//calculating the translation vector
		rot = cross(OY, AB);					//Calculating the rotation vector
		if (trans.x >= -1.0 && trans.z <= -4.0)	//when reached point B
		{
			start = 'b';
			accum = Rotate(angle, rot.x, rot.y, rot.z) * accum;
			angle = 0.0;

		}

	}
	else if (start == 'b')
	{
		trans = OB + (d * BC / length(BC));		//calculating the translation vector
		rot = cross(OY, BC);					//Calculating the rotation vector
		if (trans.x >= 3.0 && trans.z >= 5.0)	//when reached poit C
		{
			start = 'c';
			accum = Rotate(angle, rot.x, rot.y, rot.z) * accum;
			angle = 0.0;
		}
	}
	else
	{
		trans = OC + (d * CA / length(CA));		//calculating the translation vector
		rot = cross(OY, CA);					//Calculating the rotation vector
		if (trans.x <= -4.0 && trans.z <= 4.0)	//When eached point A
		{
			start = 'a';
			accum = Rotate(angle, rot.x, rot.y, rot.z) * accum;
			angle = 0.0;
		}
	}


	xrotate = rot.x;
	yrotate = rot.y;
	zrotate = rot.z;

	xtranslate = trans.x;
	ytranslate = trans.y;
	ztranslate = trans.z;

	
	if (fireworksFlag == 1)
		t = fmod((glutGet(GLUT_ELAPSED_TIME) - tsub), tmax);


	glutPostRedisplay();


}

//----------------------------------------------------------------------------
//Menu action
void menu(int i)
{
	switch (i)
	{
	case 0: eye = init_eye;

		if (animationFlag == 0)
		{
			glutIdleFunc(idle);
			animationFlag = 1 - animationFlag;
		}
		break;

	case 1: exit(EXIT_SUCCESS);
		break;

	case 2: wireframeSphere = 1 - wireframeSphere;
		break;
	}
	glutPostRedisplay();
}

void shadowmenu(int i)
{
	switch (i)
	{
		case 2: shadowFlag = 1; break;
		case 3: shadowFlag = 0; break;
		default:
				break;
	}
	glutPostRedisplay();
}

void enableLightingMenu(int i)
{
	switch (i)
	{
		case 4: lightingFlag = 1; break;
		case 5: lightingFlag = 0; break;
		default:
			break;
	}
	glutPostRedisplay();
}

void shadingMenu(int i)
{
	switch (i)
	{
		case 6: if (lightingFlag == 1) shadingFlag = 0; wireframeSphere = 0; break;
		case 7: if (lightingFlag == 1) shadingFlag = 1; wireframeSphere = 0; break;
		default:
			break;
	}
	glutPostRedisplay();
}

void lightSourceMenu(int i)
{
	switch (i)
	{
		case 8: if (lightingFlag == 1) lightsourceFlag = 1; break;
		case 9: if (lightingFlag == 1) lightsourceFlag = 2; break;
		default:
			break;
	}
	glutPostRedisplay();
}

void fogMenu(int i)
{
	switch (i)
	{
	case 10: fogFlag = 1; break;
	case 11: fogFlag = 2; break;
	case 12: fogFlag = 3; break;
	case 13: fogFlag = 4; break;
	default:
		break;
	}
	glutPostRedisplay();
}

void blendingShadowMenu(int i)
{
	switch (i)
	{
	case 14: shadowBlendFlag = 1; break;
	case 15: shadowBlendFlag = 0; break;
	default:
		break;
	}
	glutPostRedisplay();
}

void textureMappedGroundMenu(int i)
{
	switch (i)
	{
	case 16: floorTexureFlag = 1; break;
	case 17: floorTexureFlag = 0; break;
	default:
		break;
	}
	glutPostRedisplay();
}

void fireworksMenu(int i)
{
	switch (i)
	{
	case 18: fireworksFlag = 1; 
			tsub = (float)glutGet(GLUT_ELAPSED_TIME);    //current time
		break;
	case 19: fireworksFlag = 0; break;
	default:
		break;
	}
	glutPostRedisplay();
}

void textureMappedSphereMenu(int i)
{
	switch (i)
	{
	case 20: sphereTextureFlag = 0; break;
	case 21: sphereTextureFlag = 1; break;
	case 22: sphereTextureFlag = 2; break;

	default:
		break;
	}
	glutPostRedisplay();
}
//----------------------------------------------------------------------------
//Mouse CLick Listener
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && rightmouse == true)
	{
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1)
			glutIdleFunc(idle);
		else
			glutIdleFunc(NULL);
	}

	shadow_menu = glutCreateMenu(shadowmenu);
	glutAddMenuEntry("Yes", 2);
	glutAddMenuEntry("No", 3);

	enable_lighting_menu = glutCreateMenu(enableLightingMenu);
	glutAddMenuEntry("Yes", 4);
	glutAddMenuEntry("No", 5);

	shading_menu = glutCreateMenu(shadingMenu);
	glutAddMenuEntry("Flat Shading", 6);
	glutAddMenuEntry("Smooth Shading", 7);

	lightSource_menu = glutCreateMenu(lightSourceMenu);
	glutAddMenuEntry("Point Light", 8);
	glutAddMenuEntry("Spotlight", 9);

	fog_menu = glutCreateMenu(fogMenu);
	glutAddMenuEntry("No Fog", 10);
	glutAddMenuEntry("Linear", 11);
	glutAddMenuEntry("Exponential", 12);
	glutAddMenuEntry("Exponential Square", 13);

	shadow_blending_menu = glutCreateMenu(blendingShadowMenu);
	glutAddMenuEntry("Yes", 14);
	glutAddMenuEntry("No", 15);

	texture_ground_menu = glutCreateMenu(textureMappedGroundMenu);
	glutAddMenuEntry("Yes", 16);
	glutAddMenuEntry("No", 17);

	fireworks_menu = glutCreateMenu(fireworksMenu);
	glutAddMenuEntry("Yes", 18);
	glutAddMenuEntry("No", 19);

	texture_sphere_menu = glutCreateMenu(textureMappedSphereMenu);
	glutAddMenuEntry("No", 20);
	glutAddMenuEntry("Contour Lines", 21);
	glutAddMenuEntry("Checkerboard", 22);
	
	glutCreateMenu(menu);
	glutAddMenuEntry("Default View Point", 0);
	glutAddSubMenu("Shadow", shadow_menu);
	glutAddSubMenu("Enable Lighting", enable_lighting_menu);
	glutAddMenuEntry("Wireframe Sphere", 2);
	glutAddSubMenu("Shading", shading_menu);
	glutAddSubMenu("Light Source", lightSource_menu);
	glutAddSubMenu("Fog Options", fog_menu);
	glutAddSubMenu("Blending Shadow", shadow_blending_menu);
	glutAddSubMenu("Texture Mapped Ground", texture_ground_menu);
	glutAddSubMenu("Texture Mapped Sphere", texture_sphere_menu);
	glutAddSubMenu("Fireworks", fireworks_menu);
	glutAddMenuEntry("Quit", 1);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key

	case 'q': case 'Q':		//Quit the program
		exit(EXIT_SUCCESS);
		break;

	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;

	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;

	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'b': case 'B': // Toggle between animation and non-animation
		animationFlag = 1 - animationFlag;
		rightmouse = true;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
		break;

	case 'f': case 'F': // Toggle between filled and wireframe floor
		floorFlag = 1 - floorFlag;
		break;

	case ' ':  // reset to initial viewer/eye position
		eye = init_eye;
		break;
	case 'v': case 'V':
		verticalSlantedFlag = 1;                 //Vertical texture on sphere
		break;
	case 's': case 'S':
		verticalSlantedFlag = 2;                 //Slanted texture on sphere
		break;
	case 'o': case 'O':
		spaceFlag = 0;                          //Object space texture
		break;
	case 'e': case 'E':
		spaceFlag = 1;                          //Eye Space texture
		break;
	case 'l': case 'L':
		latticeFlag = 1 - latticeFlag;         //Lattice flag enable, disable
		break;
	case 'u': case 'U':
		uprightTiltedFlag = 1;                 //Upright Lattice effect
		break;
	case 't': case 'T':
		uprightTiltedFlag = 2;                 //Slanted Lattice effect
		break;
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
	glViewport(-30, -30, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
//Read the input file
void readfile()
{
	char filename[20];
	cout << "\nEnter input file name... ";
	cin >> filename;
	ifstream inputFile;
	float polySize = 0, x, y, z;
	inputFile.open(filename);
	if (inputFile)
	{
		inputFile >> numberPoly;

		shadow_NumVertices = sphere_NumVertices = numberPoly * 3;
		polygons = new point3*[(int)numberPoly];
		int row, col;

		for (row = 0; row < numberPoly; row++)
		{
			inputFile >> polySize;
			polygons[row] = new point3[(int)polySize];
			for (col = 0; col < polySize; col++)
			{
				inputFile >> x >> y >> z;
				polygons[row][col] = point3(x, y, z);
			}
		}
	}
	else
	{
		cout << "\nError reading input file\n";
		exit(EXIT_FAILURE);
	}

}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int err;

	readfile();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	// glutInitContextVersion(3, 2);
	// glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Assignment 4 - Aadam Saleem - N19984371 - as9749");

	/* Call glewInit() and error checking */
	err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(NULL);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	init();
	glutMainLoop();
	return 0;
}