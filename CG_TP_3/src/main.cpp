#include "cgmath.h"      // slee's simple math library
#include "cgut.h"      // slee's OpenGL utility
#include "trackball.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh_generator.h"
#include "object.h"
#include "constant.h"
#define TT puts("TEST");

//*******************************************************************
// forward declarations for freetype text
bool init_text();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);

//*************************************
// global constants
static const char* window_name = "Team Project";
static const char* vert_shader_path = "../bin/shaders/circ.vert";
static const char* frag_shader_path = "../bin/shaders/circ.frag";
static const char* image_path = "../bin/images/slee_1.jpg";
// initial tessellation factor of the circle as a polygon


struct light_t
{
    vec4	position = vec4(100.0f, 0.0f, 10.0f, 0.0f);   // directional light
    vec4	ambient = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    vec4	diffuse = vec4(2.0f, 2.0f, 2.0f, 1.0f);
    vec4	specular = vec4(100.0f, 100.0f, 100.0f, 1.0f);
};

struct material_t
{
    vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
    vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    vec4	specular = vec4(10.0f, 10.0f, 10.0f, 1.0f);
    float	shininess = 1000.0f;
};

//*************************************
// common structures
camera cam;
trackball tb;
game_state gs;
light_t light;
material_t material;

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2      window_size = ivec2(1280, 720); // initial window size

//*************************************
// OpenGL objects
GLuint   program = 0;      // ID holder for GPU program
GLuint   vertex_array = 0;   // ID holder for vertex array object

GLuint   texID = 0;
std::vector<Mesh*> meshes;

//*************************************
// global variables
int      frame = 0;                  // index of rendering frames
float   t = 0.0f;                  // current simulation parameter
bool   b_solid_color = true;         // use circle's color?
bool   b_index_buffer = true;         // use index buffering?
bool   b_wireframe = false;

struct { bool add = false, sub = false; operator bool() const { return add || sub; } } b; // flags of keys for smooth changes
struct { bool up = false, down = false, right = false, left = false;  operator bool() const { return up || down || right || left; } } d; // flags of keys for smooth changes
int      tc_color_flag = 0;
bool    executing = true;
float   restart_time = 0.0f;
float   actual_moved_time = 0.0f;
int      rotating_type = 0;

//*************************************
// holder of vertices and indices of a unit circle
std::vector<object_t> Map, Student, Professor, 
Title_buttons, Main_menu_button, Tmp_walls, Option_buttons;
std::list<obstacle_t> Obstacles;
std::list<object_t> Pen;

class physics_engine {
public:
    template<typename T1, typename T2>
    bool collision_exists(const T1& obj, const T2& obs, float correction_distance=0.0f) {
        if ((obj.position.x - obj.size.x - correction_distance <= obs.position.x + obs.size.x) && (obj.position.x + obj.size.x >= obs.position.x - obs.size.x)) {
            if (obj.lane == obs.lane) return true;
            return false;
        }
        return false;
    }
};

physics_engine PEngine;

//*************************************
void restart() { //수정필요!
    gs.user_life = 3;
    gs.user_score = 0;
    gs.game_stage = 1;
    executing = true;
}



void update()
{
    cam.aspect = window_size.x / float(window_size.y);
    cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);
    // update global simulation parameter
    //
    if(gs.game_start&&executing) t = float(glfwGetTime()) * 0.4f;
    // tricky aspect correction matrix for non-square window
    float aspect = window_size.x / float(window_size.y);
    mat4 aspect_matrix =
    {
       min(1 / aspect,1.0f), 0, 0, 0,
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
    glUseProgram(program);
    uloc = glGetUniformLocation(program, "b_solid_color");   if (uloc > -1) glUniform1i(uloc, b_solid_color);
    //uloc = glGetUniformLocation( program, "aspect_matrix" );   if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );
    //uloc = glGetUniformLocation(program, "view_projection_matrix");      if(uloc>-1) glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);
    uloc = glGetUniformLocation(program, "view_matrix");         if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
    uloc = glGetUniformLocation(program, "projection_matrix");   if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
    uloc = glGetUniformLocation(program, "tc_color_flag");      if (uloc > -1) glUniform1i(uloc, tc_color_flag);
    uloc = glGetUniformLocation(program, "game_start");      if (uloc > -1) glUniform1i(uloc, gs.game_start);

    glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
    glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
    glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
    glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

    // setup material properties
    glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
    glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
    glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
    glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

    // update vertex buffer by the pressed keys
    void update_tess(); // forward declaration
    if (b) update_tess();
    void update_pos();
    if (d) update_pos();
   //update_obstacles(obstacles, t);
    if (gs.game_start) {
        if (gs.user_life <= 0) {
            cam.init();
            gs.game_over = true;
            gs.game_start = false;
            executing = false;
        }
        for (size_t i = 0; i < Professor.size(); i++) Professor[i].update_to_random_lane(frame);
        for (auto k = Pen.begin(); k != Pen.end();) {
            if (k->position.x < -250.0f) {
                puts("PEN gone");
                k = Pen.erase(k);
            }
            else if (PEngine.collision_exists(*k, Professor[0], 50.0f)) {
                puts("Professor Hit");
                cam.init();
                gs.game_over = true;
                gs.game_start = false;
                executing = false;
                k = Pen.erase(k);
            }
            else k++;
        }
        update_obstacles(Obstacles, Professor[0].lane, frame, gs.game_stage);
        for (auto p = Obstacles.begin(); p != Obstacles.end();) {
            bool temp = false;
            for (auto k = Pen.begin(); k != Pen.end();) {
                if (PEngine.collision_exists(*k, *p, 20.0f)) {
                    k = Pen.erase(k);
                    if (p->type == 2) {
                        puts("good kill 점수 + 10");
                        p = Obstacles.erase(p);
                        temp = true;

                        gs.user_score+=10;

                        break;
                    }
                    else {
                        puts("wrong kill 점수 -10");
                        gs.user_score-=10;
                    }
                }
                else k++;
            }

            if (!temp) p++;

        }
        for (auto p = Obstacles.begin(); p != Obstacles.end();) {

            if (PEngine.collision_exists(Student[0], *p)) {
                if (p->type == 0) { // 시험: 맞는 색깔로
                    if (p->subject_id == Student[0].subject_id) {
                        puts("GREAT 점수 +10");
                        gs.user_score+=10;
                    }
                    else {
                        puts("FAIL life -10"); //틀린 색깔로 충돌
                        gs.user_life-=10;
                    }
                }
                else if (p->type == 1) {
                    puts("life -1"); // 과제 못피함
                    gs.user_life--;
                }
                else {
                    puts("life -1"); //강의 제거못해서 맞음
                    gs.user_life--;
                }
                p = Obstacles.erase(p);
            }
            else {
                if (p->position.x > 70.0f) {
                    if (p->type == 1) {
                        puts("GREAT 점수 +10"); // 과제 피해서 피한 과제가 끝에 도달
                        gs.user_score+=10;
                    }
                    else {
                        puts("life -1"); // 과제가 아닌데 끝에 도달 -> 망함
                        gs.user_life--;
                    }
                    p = Obstacles.erase(p);
                }
                else p++;
            }
        }
    }
}
void update_pos() {
    if (d.left) {

        
    }
    else if (d.right) {
        //for (size_t i = 0; i < Student.size(); i++) Student[i].position.y += 1.0f;
        
    }
}
template <typename T>
void render_obj(T& obj, std::vector<uint>& indices, GLuint VAOid) {
    glDisable(GL_BLEND);
    glBindVertexArray(VAOid);
    for (auto& instance : obj) {
       // glActiveTexture(GL_TEXTURE0);
       // glBindTexture(GL_TEXTURE_2D, texID);
        instance.update(t);
        GLint uloc;
        uloc = glGetUniformLocation(program, "solid_color");     
            glUniform4fv(uloc, 1, instance.color);   // pointer version
        
        uloc = glGetUniformLocation(program, "model_matrix");  glUniformMatrix4fv(uloc, 1, GL_TRUE, instance.model_matrix);

        uloc = glGetUniformLocation(program, "tex");
         glUniform1i(uloc, 0);   // pointer version

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    glEnable(GL_BLEND);
}

void render()
{
    if (!gs.game_start&&gs.title){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        render_obj(Title_buttons, meshes[0]->indices, meshes[0]->VAO);
        // render texts
        float dpi_scale = cg_get_dpi_scale();
        render_text("Life in Sung-Kil-Kwan University", 100, 150, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("START", 467, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("HOW TO", 459, 372, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("OPTION", 459, 444, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("EXIT", 473, 516, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        glfwSwapBuffers(window);
    }
    if (!gs.game_start && !gs.title && gs.how_to) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        
        //render_obj(buttons, unit_square_indices, 2);
        render_obj(Tmp_walls, meshes[0]->indices, meshes[0]->VAO);

        float dpi_scale = cg_get_dpi_scale();
        render_text("Back", 971, 37, 0.4f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("HOW TO", 400, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("You are one of the students in SKKU(Sung-Kil-Kwan University).", 60, 100, 0.4f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("As game starts, the professor Lee starts to give you three kind of walls on random lane.", 60, 120, 0.4f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("To successfully finish your semester, you should do following instructions.", 60, 140, 0.4f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("- Lecture Wall : Change your color to the same color as the lectures!", 10, 200, 0.5f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text("Press Z to change your color.", 40, 230, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Lectures' Color is Red, Green, Blue, White.", 40, 255, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Easy mode is two colors(RB), Medium is three(RGB),", 40, 280, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Hard mode is four colors(RGBW).", 40, 305, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("- Assignment Wall : Just avoid the assignments!", 10, 350, 0.5f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text("Press <-, -> to move.", 40, 380, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Assignments' Color is Yellow.", 40, 405, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);  //펜 추가하기

        render_text("- Exam Wall : Throw the pen and break the exams!", 10, 450, 0.5f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text("Press X to throw your pen.", 40, 480, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Exams' Color is Black.", 40, 505, 0.45f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        glfwSwapBuffers(window);
    }
    if (!gs.game_start && !gs.title && gs.option) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

       // render_obj(option_button, unit_square_indices, 2);
        render_obj(Option_buttons, meshes[0]->indices, meshes[0]->VAO);

        float dpi_scale = cg_get_dpi_scale();
        render_text("Back", 971, 37, 0.4f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("OPTION", 430, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("There are three difficulty levels.(Easy, Normal, Hard)", 150, 130, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Default difficulty level is Easy mode.", 250, 160, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("You can adjust these levels to add more fun to your play!", 150, 190, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("EASY", 235, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("NORMAL", 510, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("HARD", 815, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text(" Lecture Walls", 10, 350, 0.4f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text(" 2 Kinds(RB)          3 Kinds(RGB)          4 Kinds(RGBW)", 195, 350, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text(" Item Frequency", 10, 400, 0.4f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text(" High                  Often                 Seldom", 230, 400, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
      //  render_text(" Speed of Walls", 10, 450, 0.4f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
       // render_text(" Fast                  Normal                 Slow", 230, 450, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("Your Difficulty Level : ", 400, 530, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        char* level;
        if (gs.game_level == 0) level = "EASY";
        else if (gs.game_level == 1) level = "NORMAL";
        else level = "HARD";

        render_text(level, 850, 530, 0.7f, vec4(1.0f, 0.5f, 0.5f, 1.0f), dpi_scale);

        glfwSwapBuffers(window);
    }
    if (gs.game_start) {
        // clear screen (with background color) and clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // notify GL that we use our own program
        glUseProgram(program);

        render_obj(Map, meshes[0]->indices, meshes[0]->VAO);
        render_obj(Student, meshes[1]->indices, meshes[1]->VAO);
        render_obj(Professor, meshes[1]->indices, meshes[1]->VAO);
        render_obj(Obstacles, meshes[0]->indices, meshes[0]->VAO);
        render_obj(Pen, meshes[2]->indices, meshes[2]->VAO);
        //render_obj(Face, meshes[0]->indices, meshes[0]->VAO);
        
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texID);
        //glUniform1i(glGetUniformLocation(program, "tex"), 1);

        //// bind vertex array object
        //glBindVertexArray(vertex_array);

        //// render quad vertices
        //glDrawArrays(GL_TRIANGLES, 0, 6);


        float dpi_scale = cg_get_dpi_scale();
        render_text("Life  : ", 20, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text(std::to_string(gs.user_life), 210, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("Stage : ", 20, 100, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text(std::to_string(gs.game_stage), 210, 100, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("Level : ", 20, 150, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        char* level;
        if (gs.game_level == 0) level = "EASY";
        else if (gs.game_level == 1) level = "NORMAL";
        else level = "HARD";
        render_text(level, 210, 150, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);

        render_text("Score : ", 700, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text(std::to_string(gs.user_score), 900, 50, 1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);


        glfwSwapBuffers(window);
    }
    if (gs.game_over) {
        // clear screen (with background color) and clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // notify GL that we use our own program
        glUseProgram(program);


        //render_obj(main_menu_button, unit_square_indices, 2);
        Main_menu_button[0].color = vec3(1.0f, 0.0f, 0.0f);
        render_obj(Main_menu_button, meshes[0]->indices, meshes[0]->VAO);

        float dpi_scale = cg_get_dpi_scale();
        render_text("GAME OVER", 300, 200, 2.0f, vec4(1.0f, 0.0f, 0.0f, 1.0f), dpi_scale);
        render_text("Your Grade is F.", 400, 250, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("You Should Do JaeSuGang. Go Back to Main Menu.", 100, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Main Menu", 455, 390, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        // swap front and back buffers, and display to screen
        glfwSwapBuffers(window);
    }
    if (gs.game_clear) {
        // clear screen (with background color) and clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // notify GL that we use our own program
        glUseProgram(program);

        Main_menu_button[0].color = vec3(0.0f, 0.5f, 0.5f);
        render_obj(Main_menu_button, meshes[0]->indices, meshes[0]->VAO);

        float dpi_scale = cg_get_dpi_scale();
        render_text("GAME CLEAR", 280, 200, 2.0f, vec4(0.0f, 0.5f, 0.5f, 1.0f), dpi_scale);
        render_text("Your Grade is C+.", 390, 250, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Congratulations! You successfully finish your semester!", 20, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Main Menu", 455, 390, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        // swap front and back buffers, and display to screen
        glfwSwapBuffers(window);
    }
    if (gs.hid_game_clear) {
        // clear screen (with background color) and clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // notify GL that we use our own program
        glUseProgram(program);

        Main_menu_button[0].color = vec3(0.0f, 0, 1.0f);
        render_obj(Main_menu_button, meshes[0]->indices, meshes[0]->VAO);

        float dpi_scale = cg_get_dpi_scale();
        render_text("GAME CLEAR", 260, 150, 2.0f, vec4(0, 0, 1.0f, 1.0f), dpi_scale);
        render_text("Your Grade is", 370, 250, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);  render_text("A+.", 625, 253, 1.0f, vec4(0, 0, 1.0f, 1.0f), dpi_scale);
        render_text("Congratulations! You successfully finish your semester!", 20, 300, 0.7f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        render_text("Main Menu", 455, 390, 0.5f, vec4(1.0f, 1.0f, 1.0f, 1.0f), dpi_scale);
        // swap front and back buffers, and display to screen
        glfwSwapBuffers(window);
    }
}

void reshape(GLFWwindow* window, int width, int height)
{
    // set current viewport in pixels (win_x, win_y, win_width, win_height)
    // viewport: the window area that are affected by rendering 
    window_size = ivec2(width, height);
    glViewport(0, 0, width, height);
}

void print_help()
{
    printf("[help]\n");
    printf("- press ESC or 'q' to terminate the program\n");
    printf("- press F1 or 'h' to see help\n");
    printf("- press 'd' to toggle between texture coordinates\n");
    //printf( "- press '+/-' to increase/decrease tessellation factor (min=%d, max=%d)\n", MIN_TESS, MAX_TESS );
    //printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
    printf("- press 'w' to toggle wireframe\n");

    printf("\n");
}




void update_tess()
{
    if (b.add) {
        //printf("> increasing speed\n");
       
    }
    else if (b.sub) {

      
    }
}


void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)   glfwSetWindowShouldClose(window, GL_TRUE);
        else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)   print_help();
        //else if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))   b.add = true;
        else if (key == GLFW_KEY_Z) {
            b.add = true;
            insert_new_pen(Pen, Student[0].lane);
            
        }
        else if (key == GLFW_KEY_X) {
            b.sub = true;
            printf("> decreasing speed\n");
        }
        else if (key == GLFW_KEY_UP) {
            if (gs.game_start) {
                cam.init();
                gs.game_clear = true;
                gs.game_start = false;
            }
            b.add = true;
            d.up = true;
            printf("> moving up\n");
        }
        else if (key == GLFW_KEY_DOWN) {
            if (gs.game_start) {
                cam.init();
                gs.game_over = true;
                gs.game_start = false;
            }
            d.down = true;
            printf("> moving down\n");
        }
        else if (key == GLFW_KEY_RIGHT) {
            d.right = true;
            for (size_t i = 0; i < Student.size(); i++) {
                if (Student[i].position.y < 20.0f) {
                    Student[i].position.y += 20.0f;
                    Student[i].lane++;
                }
            }
        }
        else if (key == GLFW_KEY_LEFT) {
            d.left = true;
           // for (size_t i = 0; i < Student.size(); i++) Student[i].position.y -= 20.0f;
            for (size_t i = 0; i < Student.size(); i++) {
                if (Student[i].position.y > -20.0f) {
                    Student[i].position.y -= 20.0f;
                    Student[i].lane--;
                }
            }
        }
        //else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = true;
        else if (key == GLFW_KEY_I)
        {
            if (gs.game_start) {
                cam.init();
                gs.hid_game_clear = true;
                gs.game_start = false;
            }
            /*b_index_buffer = !b_index_buffer;
            update_vertex_buffer( unit_circle_vertices,NUM_TESS );
            printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );*/
        }
        else if (key == GLFW_KEY_W)
        {
            b_wireframe = !b_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
            printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
        }
       
        else if (key == GLFW_KEY_P)
        {
            executing = !executing;
            if (!executing) {
                printf("> stopped\n");
                actual_moved_time += t - restart_time;
            }
            else {
                printf("> continue\n");
                restart_time = t;
            }
        }
        else if (key == GLFW_KEY_C)
        {
            Student[0].subject_id = (Student[0].subject_id + 1) % (subject_count);
            Student[0].color = subject_color[Student[0].subject_id];
        }
        else if (key == GLFW_KEY_V) {
            printf("> direction reversed\n");
            actual_moved_time = 0.0f;
            restart_time = t;
            
        }
        else if (key == GLFW_KEY_A) {
            if (gs.game_stage < 5) gs.game_stage++;
            printf("stage: %d\n", gs.game_stage);
        }
        else if (key == GLFW_KEY_S) {
            if (gs.game_stage > 1) gs.game_stage--;
            printf("stage: %d\n", gs.game_stage);
        }
        else if (key == GLFW_KEY_D)
        {
        if (gs.game_level < 3) {
            gs.game_level++;
            subject_count++;
        }
        printf("level: %d\n", gs.game_level);
        }
        else if (key == GLFW_KEY_F)
        {
        if (gs.game_level >1) {
            gs.game_level--;
            subject_count--;
        }
        printf("level: %d\n", gs.game_level);
        }
        
        else if (key == GLFW_KEY_T) {
            cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
            printf("> Camera Reset\n");
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_Z) {
            b.add = false;
        }
        else if (key == GLFW_KEY_X) {
            b.sub = false;
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

    }
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
    dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
    tb.mods = mods;
    tb.button = button;
    vec2 npos = cursor_to_ndc(pos, window_size);
    if (action == GLFW_PRESS) {
        tb.begin(cam.view_matrix, npos);
        if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
            if (gs.title) {
                if (pos.y > 420 && pos.y < 480 && pos.x>490 && pos.x < 790) {  //how to button
                    gs.how_to = true;
                    gs.title = false;
                }
                else if (pos.y > 510 && pos.y < 570 && pos.x>490 && pos.x < 790) {  //option button
                    gs.option = true;
                    gs.title = false;
                }
                else if (pos.y > 600 && pos.y < 660 && pos.x>490 && pos.x < 790) {  //exit button
                    glfwSetWindowShouldClose(window, GL_TRUE);
                }
            }
            else if (gs.how_to) {
                if (pos.y > 15 && pos.y < 75 && pos.x>1210 && pos.x < 1270) {
                    gs.how_to = false;
                    gs.title = true;
                }
            }
            else if (gs.option) {
                if (pos.y > 15 && pos.y < 75 && pos.x>1210 && pos.x < 1270) {
                    gs.option = false;
                    gs.title = true;
                }
                if (pos.y > 330 && pos.y < 390) {
                    if (pos.x > 250 && pos.x < 430) gs.game_level = 0;
                    else if (pos.x > 610 && pos.x < 790) gs.game_level = 1;
                    else if (pos.x > 970 && pos.x < 1150) gs.game_level = 2;
                }
            }
            else if (gs.game_over || gs.game_clear || gs.hid_game_clear) {
                if (pos.x > 520 && pos.x < 760 && pos.y>450 && pos.y < 510) {
                    if (gs.game_over) gs.game_over = false;
                    else if (gs.game_clear) gs.game_clear = false;
                    else if (gs.hid_game_clear) gs.hid_game_clear = false;
                    gs.title = true;

                }
            }
            else if (gs.game_start) {
                printf("> rotating camera\n");
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
        if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
            if (gs.title) {
                if (pos.y > 330 && pos.y < 390 && pos.x>490 && pos.x < 790) {  //start button
                    if (!gs.game_start) {
                        cam.update();
                        restart();
                        gs.title = false;
                        gs.game_start = true;
                        
                    }
                }
            }
        }
    }

}

void motion(GLFWwindow* window, double x, double y)
{
    if (!tb.is_tracking()) return;
    vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
    if (tb.button == GLFW_MOUSE_BUTTON_LEFT && tb.mods == 0) {
        if (gs.game_start) cam.view_matrix = tb.update(npos);
    }
    else if (tb.button == GLFW_MOUSE_BUTTON_MIDDLE ||
        (tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_CONTROL))) {
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
    glLineWidth(1.0f);
    glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);   // set clear color
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);                        // turn on backface culling
    glEnable(GL_DEPTH_TEST);                        // turn on depth tests
    glActiveTexture(GL_TEXTURE0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    texID = create_texture(image_path, true);
    meshes = generate_mesh(vertex_array);
    Map = create_map();
    Student = create_student();
    Professor = create_professor();
    Title_buttons = create_title_buttons();
    Tmp_walls = create_tmp_walls();
    Main_menu_button = create_main_menu_buttons();
    Option_buttons = create_option_buttons();
    //Face = create_face();

    vertex corners[4];
    corners[0].pos = vec3(-1.0f, -1.0f, 0.0f);	corners[0].tex = vec2(0.0f, 0.0f);
    corners[1].pos = vec3(+1.0f, -1.0f, 0.0f);	corners[1].tex = vec2(1.0f, 0.0f);
    corners[2].pos = vec3(+1.0f, +1.0f, 0.0f);	corners[2].tex = vec2(1.0f, 1.0f);
    corners[3].pos = vec3(-1.0f, +1.0f, 0.0f);	corners[3].tex = vec2(0.0f, 1.0f);
    vertex vertices[6] = { corners[0], corners[1], corners[2], corners[0], corners[2], corners[3] };

    // generation of vertex buffer is the same, but use vertices instead of corners
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // generate vertex array object, which is mandatory for OpenGL 3.3 and higher
    if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
    vertex_array = cg_create_vertex_array(vertex_buffer);
    if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return false; }
    texID = create_texture(image_path, true);
    if (!init_text()) return false;
    return true;
}

void user_finalize()
{
}

int main(int argc, char* argv[])
{
    srand(unsigned(time(NULL)));
    // create window and initialize OpenGL extensions
    if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
    if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }   // init OpenGL extensions

    // initializations and validations of GLSL program
    if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }   // create and compile shaders/program
    if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }               // user initialization

    // register event callbacks
    glfwSetWindowSizeCallback(window, reshape);   // callback for window resizing events
    glfwSetKeyCallback(window, keyboard);         // callback for keyboard events
    glfwSetMouseButtonCallback(window, mouse);   // callback for mouse click inputs
    glfwSetCursorPosCallback(window, motion);      // callback for mouse movements


    // enters rendering/event loop
    frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();   // polling and processing of events
        update();         // per-frame update
        if (executing||!gs.game_start) {
            render();         // per-frame render
            frame++;
        }
    }
    //for (frame = 0; !glfwWindowShouldClose(window); frame++)
    //{
    //    glfwPollEvents();   // polling and processing of events
    //    update();         // per-frame update
    //    render();         // per-frame render

    //}

    // normal termination
    user_finalize();
    cg_destroy_window(window);

    return 0;
}
