#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "sphere.h"		// circle class definition
#include "cube.h"
#include "camera.h"
#include "trackball.h"
#include "obstacle.h"
#include "square.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//*************************************
// global constants
static const char*	window_name = "Team Project";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";

static const char* image_path = "../bin/images/slee_2.jpg";
	// initial tessellation factor of the circle as a polygon

//*************************************
// common structures
camera cam;
trackball tb;
//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280, 720); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

GLuint	texID = 0;

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
bool	b_solid_color = true;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
bool	b_wireframe = false;

struct { bool add=false, sub=false; operator bool() const { return add||sub; } } b; // flags of keys for smooth changes
struct { bool up = false, down = false,right=false,left = false;  operator bool() const { return up || down||right||left; } } d; // flags of keys for smooth changes
int		tc_color_flag = 0;
bool	b_rotating = true;
float	restart_time = 0.0f;
float	actual_moved_time = 0.0f;
int		rotating_type = 0;
bool	game_start;
//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_sphere_vertices, unit_cube_vertices, unit_square_vertices;	// host-side vertices
std::vector<uint> unit_sphere_indices, unit_cube_indices, unit_square_indices;
auto	spheres = std::move(create_sphere());
auto	cubes = std::move(create_cube());
auto	squares = std::move(create_squares());
auto	obstacles = std::list<obstacle_t>();
//*************************************
void update()
{
	cam.aspect = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);

	// update global simulation parameter
	//
	t = float(glfwGetTime())*0.4f;
	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		min(1/aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	mat4 view_projection_matrix = { 0,1,0,0,
		0,0,1,0,
		-1,0,0,1,
		0,0,0,1 };

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	//uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );
	//uloc = glGetUniformLocation(program, "view_projection_matrix");		if(uloc>-1) glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	uloc = glGetUniformLocation(program, "tc_color_flag");		if (uloc > -1) glUniform1i(uloc, tc_color_flag);
	// update vertex buffer by the pressed keys
	void update_tess(); // forward declaration
	if(b) update_tess(); 
	void update_pos();
	if (d) update_pos();
	update_obstacles(obstacles, t);
}

template <typename T>
void render_obj(T& obj, std::vector<uint>& indices, GLuint VAOid) {
	glBindVertexArray(VAOid);
	for (auto& instance : obj) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);

		instance.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, instance.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, instance.model_matrix);

		uloc = glGetUniformLocation(program, "tex");		
		if (uloc > -1) glUniform1i(uloc, 0);	// pointer version
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	}
}

void render()
{
	if (!game_start) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program);
		render_obj(squares, unit_square_indices, 2);

		glfwSwapBuffers(window);
	}
	if (game_start) {
		// clear screen (with background color) and clear depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// notify GL that we use our own program
		glUseProgram(program);
		//printf("%f\n", t);
		render_obj(spheres, unit_sphere_indices, 1);
		render_obj(cubes, unit_cube_indices, 2);
		render_obj(obstacles, unit_cube_indices, 2);
		
		// swap front and back buffers, and display to screen
		glfwSwapBuffers(window);
	}
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle between texture coordinates\n" );
	printf( "- press 'w' to toggle wireframe\n" );
	printf("- press LEFT_MOUSE_BUTTON to rotate the camera\n");
	printf("- press RIGHT_MOUSE_BUTTON or SHIFT+LEFT_MOUSE_BUTTON to zoom the camera\n");
	printf("- press MIDDLE_MOUSE_BUTTON or CTRL+LEFT_MOUSE_BUTTON to pan the camera\n");
	printf("- press 't' to reset the camera\n");
	printf("- press 'r' to start or stop the rotation\n");
	printf("- press 'c' to change the rotation axis\n");
	printf("- press 'z'/'x' to inc/dec the rotation speed\n");
	printf("- press 'a'/'s' to inc/dec the size of the sphere\n");
	printf("- press 'v' to reverse the rotation direction (restarted from center)\n");
	printf("- press 4 arrow keys to move the sphere\n");
	
	printf( "\n" );
}
void update_tess()
{
	if (b.add) {
		//printf("> increasing speed\n");
		for (auto& c : spheres) {
			
			if (c.rotation_speed <= 8.0f) c.rotation_speed += 0.05f;
			else printf("> cannot increase anymore\n");
		}
	}
	else if (b.sub) {
		
		for (auto& c : spheres) {
			if (c.rotation_speed >= 0.3f) {
				c.rotation_speed -= 0.05f;
				//printf("> decreasing speed\n");
			}
			else printf("> cannot decrease anymore\n");
		}
	}
}
void update_pos() {
	vec3 add=vec3();
	if (d.up) add.z = 0.05f;
	else if (d.down) add.z = -0.05f;
	else if (d.right) {
		if(spheres[1].position.y<20.0f) spheres[1].position.y += 0.1f;
	}
	else if (d.left) {
		if(spheres[1].position.y>-20.0f) spheres[1].position.y -= 0.1f;
	}
	
	
}
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		//else if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = true;
		else if (key == GLFW_KEY_Z) {
			b.add = true;
			printf("> increasing speed\n");
		}
		else if (key == GLFW_KEY_X) {
			b.sub = true;
			printf("> decreasing speed\n");
		}
		else if (key == GLFW_KEY_UP) {
			d.up = true;
			printf("> moving up\n");
		}
		else if (key == GLFW_KEY_DOWN) {
			d.down = true;
			printf("> moving down\n");
		}
		else if (key == GLFW_KEY_RIGHT) {
			d.right = true;
			printf("> moving right\n");
		}
		else if (key == GLFW_KEY_LEFT) {
			d.left = true;
			printf("> moving left\n");
		}
		//else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = true;
		else if(key==GLFW_KEY_I)
		{
			/*b_index_buffer = !b_index_buffer;
			update_vertex_buffer( unit_circle_vertices,NUM_TESS );
			printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );*/
		}
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
		else if(key==GLFW_KEY_D)
		{
			/*b_solid_color = !b_solid_color;
			printf( "> using %s\n", b_solid_color ? "solid color" : "texture coordinates as color" );*/
			char* mode[4] = { "solid color", "tc.xy as color", "tc.xxx as color", "tc.yyy as color" }; 
			if (tc_color_flag == 3) tc_color_flag = 0;
			else tc_color_flag++;
			printf("> using %s\n", mode[tc_color_flag]);
		}
		else if (key == GLFW_KEY_R)
		{
			b_rotating = !b_rotating;
			if (!b_rotating) {
				printf("> stopped\n");
				//t = float(glfwGetTime()) * 0.4f;
				actual_moved_time += t-restart_time;
			}
			else {
				printf("> rotating\n");
				restart_time = t;
			}
		}
		else if (key == GLFW_KEY_C)
		{
			rotating_type++;
			rotating_type %= 3;
			char axis[3] = { 'z','x','y' };
			printf("> rotation axis: %c\n", axis[rotating_type]);
		}
		else if (key == GLFW_KEY_V) {
			printf("> direction reversed\n");
			actual_moved_time = 0.0f;
			restart_time = t;
			for (auto& c : spheres) {
				c.rotation_direction *= -1;
			}
		}
		else if (key == GLFW_KEY_A) {
			for (auto& c : spheres) {
				if (c.radius < 0.95f) {
					c.radius += 0.05f;
					printf("> sphere enlarged\n");
				}
				else printf("> cannot make bigger\n");
			}
		}
		else if (key == GLFW_KEY_S) {
			for (auto& c : spheres) {
				if (c.radius > 0.1f) {
					c.radius -= 0.05f;
					printf("> sphere reduced\n");
				}
				else printf("> cannot make smaller\n");
			}
		}
		else if (key == GLFW_KEY_RIGHT) {
			for (auto& c : spheres) {
				vec3 newPos = c.position + vec3(0.0f,0.05f,0.0f);
			}
		}
		else if (key == GLFW_KEY_LEFT) {
			for (auto& c : spheres) {
				vec3 newPos = c.position - vec3(0.0f, 0.05f, 0.0f);
			}
		}
		else if (key == GLFW_KEY_T) {
		cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
		printf("> Camera Reset\n");
		}
	}
	else if(action==GLFW_RELEASE)
	{
		if (key == GLFW_KEY_Z) {
			b.add = false;
			printf("> speed increased: %.3f\n", spheres[0].rotation_speed);
		}
		else if (key == GLFW_KEY_X) {
			b.sub  =false;
			printf("> speed decreased: %.3f\n", spheres[0].rotation_speed);
		}
		else if (key == GLFW_KEY_UP) {
			d.up = false;
		}
		else if (key == GLFW_KEY_DOWN) {
			d.down = false;
		}
		else if (key == GLFW_KEY_RIGHT) {
			d.right = false;
		}
		else if (key == GLFW_KEY_LEFT) {
			d.left = false;
		}

		/*if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT))) {
			b.add = false;
			printf("> speed inreased: %f\n",spheres[0].rotation_speed);
		}
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) {
			b.sub = false;
			printf("> speed decreased: %f\n", spheres[0].rotation_speed);
		}*/
	}
}
void mouse( GLFWwindow* window, int button, int action, int mods )
{
	dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
	tb.mods = mods;
	tb.button = button;
	vec2 npos = cursor_to_ndc(pos, window_size);
	if (action == GLFW_PRESS) {
		tb.begin(cam.view_matrix, npos);
		if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
			if (pos.y > 330 && pos.y < 390 && pos.x>490 && pos.x < 790) {
				if (!game_start) {
					cam.update();
					game_start = true;
				}
				else printf("> rotating camera\n");

				cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);

			}
		}
		else if (tb.button == GLFW_MOUSE_BUTTON_MIDDLE ||
			(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_CONTROL))) printf("> panning camera\n");
		else if (tb.button == GLFW_MOUSE_BUTTON_RIGHT ||
			(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_SHIFT))) printf("> zooming camera\n");
		
	}
	else if (action == GLFW_RELEASE) {
		tb.end();
		printf("> fininshed\n");
	}
	
}
void motion( GLFWwindow* window, double x, double y )
{
	if (!tb.is_tracking()) return;
	vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
	if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
		//printf("<<view_matrix>>\n");
		//for (int i = 0; i < 4; i++) {
		//	for (int j = 0; j < 4; j++) {
		//		printf("%f ", cam.view_matrix[4 * i + j]);
		//	}
		//	puts("");
		//}
		

		if(game_start) cam.view_matrix = tb.update(npos);
	}
	else if (tb.button == GLFW_MOUSE_BUTTON_MIDDLE ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_CONTROL)) ) {
		cam.view_matrix = tb.update_pan(npos);
	}
	else if (tb.button == GLFW_MOUSE_BUTTON_RIGHT ||
		(tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_SHIFT))) {
		cam.view_matrix = tb.update_zoom(npos);
	}
	

}
GLuint create_texture(const char* image_path, bool b_mipmap)
{
	// load the image with vertical flipping
	image* img = cg_load_image(image_path); if (!img) return -1;
	int w = img->width, h = img->height;

	// create a src texture (lena texture)
	GLuint texture; glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img->ptr);
	if (img) delete img; // release image

	// build mipmap
	if (b_mipmap && glGenerateMipmap)
	{
		int mip_levels = 0; for (int k = max(w, h); k; k >>= 1) mip_levels++;
		for (int l = 1; l < mip_levels; l++)
			glTexImage2D(GL_TEXTURE_2D, l, GL_RGB8, max(1, w >> l), max(1, h >> l), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// set up texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, b_mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

	return texture;
}
bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glActiveTexture(GL_TEXTURE0);

	unit_sphere_vertices = std::move(create_sphere_vertices());
	unit_sphere_indices= create_sphere_indices( unit_sphere_vertices, vertex_array);
	unit_cube_vertices = std::move(create_cube_vertices());
	unit_cube_indices = create_cube_indices(unit_cube_vertices, vertex_array);
	unit_square_vertices = std::move(create_square_vertices());
	unit_square_indices = create_square_indices(unit_square_vertices, vertex_array);
	//printf("%d\n", texID);
	texID = create_texture(image_path, true);
	//printf("%d\n", texID);
	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements


	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
		
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
