
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#include "Light.hpp"
#include "Camera.hpp"
#include "FBTool.hpp"
#include "Head.hpp"


#define MAX_VERTEX_BUFFER   512 * 1024
#define MAX_ELEMENT_BUFFER  128 * 1024
const float RADIAN = 3.141592f / 180.0f;
const int shadowW = 4096;
const int shadowH = 4096;


void                init(void);
void                nuklearInit(GLFWwindow* window);
void                render(GLFWwindow* window);
void                UIRender(void);
bool                checkUIClicked(double x, double y);
void                handle_dropped_file(const char* path);
void                mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void                cursorMotionCallback(GLFWwindow* window, double xpos, double ypos);
void                keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void                debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
void                drop_callback(GLFWwindow* window, int count, const char** paths);
void                loadFile(const std::string& fn);


// hair
Shader              shader;
HairModel           hairModel(0.2f, glm::vec4(0,0,0,1));
nk_colorf           hairColor = {0,0,0,1};

Shader              longitudinalShader;
Shader              azimuthalShader;
Framebuffer         longitudinalFB;
Framebuffer         azimuthalRFB;
Framebuffer         azimuthalTTFB;
Framebuffer         azimuthalTRTFB;
Model               quadModel;

glm::vec3           absorption = glm::vec3(0.2f, 0.2f, 0.2f);
float               absorptionFactor = 1.;
float               alphaR = -7.5f;
float               betaR = 7.5f;


// head
// Head                head("D:\\source\\repos\\HairRenderer\\hairstyles\\head_model.obj");
Shader              headShader;
nk_colorf           headColor = { 0.98, 0.8, 0.69, 1. };


// shadow
Shader              depthShader;
Framebuffer         depthFB;
Framebuffer         headDepthFB;
// Framebuffer         hairDepthFB;

Shader              opacityShader;
Framebuffer         opacityFB;
// Framebuffer         hairOpacityFB;

float               layerSize = 0.002;
float               opacityValue = 0.01;

Camera              camera;
double              lastX = 0;
double              lastY = 0;

Light               light;



struct nk_context*  ctx;

int                 keyInput = 0;

float               roughness = 1.;

float               type = 0.;

float               sampling = 0.;

float               shadowBias = 0.0001f;

glm::mat4           shadowProjMat;
glm::mat4           shadowViewMat;


int main( void )
{
    if (!glfwInit()) {
        fprintf(stderr, "Failed to setup GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_SAMPLES, 32);
    GLFWwindow* window = glfwCreateWindow(800, 600, "HairTest", 0, 0);
    if (window == NULL) {
        cout << "Failed to create the window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(EXIT_FAILURE);
    }

    // Callback functions
    { glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorMotionCallback);
    glfwSetKeyCallback(window, keyInputCallback);
    glfwSetDropCallback(window, drop_callback);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE); }

    nuklearInit(window);
    init();

    handle_dropped_file("..\\hairstyles\\strands00003.data");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        render(window);
        UIRender();

        glfwSwapBuffers(window);
    }

    // Shutdown
    { nk_glfw3_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate(); }

    return 0;
}


void nuklearInit(GLFWwindow* win) {
    ctx = nk_glfw3_init(win, NK_GLFW3_DEFAULT);

    { struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end(); }
}

void init(void) {
    shader.loadShaders("..\\Shaders\\render.vert", "..\\Shaders\\render.frag");
    longitudinalShader.loadShaders("..\\Shaders\\temp.vert", "..\\Shaders\\longitudinal.frag");
    azimuthalShader.loadShaders("..\\Shaders\\temp.vert", "..\\Shaders\\azimuthal.frag");
    headShader.loadShaders("..\\Shaders\\head.vert", "..\\Shaders\\head.frag");
    depthShader.loadShaders("..\\Shaders\\depth.vert", "..\\Shaders\\depth.frag");
    opacityShader.loadShaders("..\\Shaders\\opacity.vert", "..\\Shaders\\opacity.frag");
    // head.create();

    // depthFB.createMap(1024, 1024);
    // opacityFB.createOpacityMap(1024, 1024);

    light.setPos(glm::vec3(1.1f, 3.0f, 1.1f));
}


void createLongitudinalTex(Framebuffer& fbo, int w, int h) {
    fbo.create(w, h);
    fbo.use();
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    longitudinalShader.use();
    longitudinalShader.setFloat("alphaR", (alphaR * RADIAN));
    longitudinalShader.setFloat("betaR", (betaR * RADIAN));
    longitudinalShader.setFloat("eta", 1.55f);
    quadModel.drawQuad();
    fbo.unuse();
}


// pass 0 : R
// pass 1 : TT
// pass 2 : TRT
void createAzimuthalTex(Framebuffer& fbo, int w, int h, int pass) {
    fbo.create(w, h);
    fbo.use();
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    azimuthalShader.use();
    azimuthalShader.setInt("Pass", pass);
    if(pass > 0)
        azimuthalShader.setVec3("absorption", absorption * absorptionFactor);
    azimuthalShader.setFloat("eta", 1.55f);
    longitudinalFB.bindTex(0);
    quadModel.drawQuad();
    fbo.unuse();
}


// mode 0 : head
// mode 1 : hair
// mode 2 : head + hair
void createDepthMap(Framebuffer& fbo, int shadowW, int shadowH, int oldW, int oldH, int mode) {

    fbo.create(shadowW, shadowH, true, GL_FLOAT, GL_RGB32F, GL_RGB);
    fbo.use();
    glViewport(0, 0, shadowW, shadowH);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    depthShader.use();
    depthShader.setMat4("projMat", shadowProjMat);
    depthShader.setMat4("viewMat", shadowViewMat);

    // if (mode == 0)
    //     head.render();
    // else if (mode == 1)
    //     hairModel.render();
    // else {
    //     head.render();
    //     hairModel.render();
    // }
    hairModel.render();

    fbo.unuse();
    glViewport(0, 0, oldW, oldH);
}


void createOpacityMap(Framebuffer& fbo, Framebuffer& depthFbo, int mapW, int mapH, int oldW, int oldH) {
//  if (sampling == 0) {
//      glfwDefaultWindowHints();
//  }
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    fbo.create(shadowW, shadowH);
    fbo.use();
    glViewport(0, 0, shadowW, shadowH);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    opacityShader.use();
    depthFbo.bindTex(0, opacityShader, "depthMap", true);
    opacityShader.setMat4("shadowViewMat", shadowViewMat);
    opacityShader.setMat4("shadowProjMat", shadowProjMat);
    opacityShader.setMat4("projMat", camera.getProjMat());
    opacityShader.setMat4("viewMat", camera.getViewMat());
    opacityShader.setFloat("layerSize", layerSize);
    opacityShader.setFloat("opacityValue", 1);
    // head.render();
    opacityShader.setFloat("opacityValue", opacityValue);
    hairModel.render();
    fbo.unuse();

    // hairOpacityFB.unuse();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, oldW, oldH);
//  if (sampling == 0) {
//      glfwWindowHint(GLFW_SAMPLES, 32);
//  }
}


void renderHead() {
    headShader.use();
    camera.setToProgram(headShader);
    camera.setPosToProgram(headShader);
    light.setPosToProgram(headShader);
    light.setFactorToProgram(headShader);
    headShader.setInt("Pass", 0);
    // headDepthFB.bindTex(0, headShader, "depthMap", true);
    opacityFB.bindTex(1, headShader, "opacityMap");
    // depthFB.bindTex(2, headShader, "hairDepthMap", true);
    depthFB.bindTex(2, headShader, "depthMap", true);
    headDepthFB.bindTex(3, headShader, "headDepthMap", true);
    headShader.setMat4("shadowProjMat", shadowProjMat);
    headShader.setMat4("shadowViewMat", shadowViewMat);
    headShader.setFloat("layerSize", layerSize);
    headShader.setFloat("shadowBias", shadowBias);
    headShader.setFloat("lightSize", light.lightSize);
    headShader.setVec3("lightColor", light.color);
    headShader.setVec4("albedo", glm::vec4(headColor.r, headColor.g, headColor.b, 1));
    headShader.setFloat("roughness", roughness);

    // headShader.setFloat("type", type);
    // head.render();
}


void renderHair() {
    shader.use();
    camera.setToProgram(shader);
    camera.setPosToProgram(shader);
    light.setPosToProgram(shader);
    light.setFactorToProgram(shader);
    longitudinalFB.bindTex(0, shader, "longitudinalTex");
    azimuthalRFB.bindTex(1, shader, "azimuthalRTex");
    azimuthalTTFB.bindTex(2, shader, "azimuthalTTTex");
    azimuthalTRTFB.bindTex(3, shader, "azimuthalTRTTex");
    depthFB.bindTex(4, shader, "depthMap", true);
    opacityFB.bindTex(5, shader, "opacityMap");
    // headDepthFB.bindTex(6, shader, "headDepthMap", true);
    shader.setMat4("shadowViewMat", shadowViewMat);
    shader.setMat4("shadowProjMat", shadowProjMat);
    shader.setMat4("projMat", camera.getProjMat());
    shader.setMat4("viewMat", camera.getViewMat());
    shader.setFloat("layerSize", layerSize);
    shader.setFloat("shadowBias", shadowBias);
    shader.setFloat("lightSize", light.lightSize);
    shader.setVec3("lightColor", light.getColor());
    shader.setVec4("albedo", glm::vec4(hairColor.r, hairColor.g, hairColor.b, 1));
    shader.setInt("Pass", 0);
    hairModel.render();
}


void render(GLFWwindow* window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    camera.setFrameSize(w, h);
    glViewport(0, 0, w, h);

    glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);


    // glm::mat4 shadowProjMat = glm::ortho(-5.f, 5.f, -5.f, 5.f, 0.f, 100.f);
    // glm::mat4 shadowProjMat = glm::perspective(camera.fov, 1.f, camera.zNear, camera.zFar);
    shadowProjMat = glm::perspective(camera.getFov(), 1.f, 0.35f, 100.f);
    shadowViewMat = glm::lookAt(light.position, glm::vec3(0, 1.7f, 0), glm::vec3(0, 1, 0));
    

    // createDepthMap(headDepthFB, shadowW, shadowH, w, h, 0);
    // createDepthMap(hairDepthFB, shadowW, shadowH, w, h, 1);
    createDepthMap(depthFB, shadowW, shadowH, w, h, 2);
    createOpacityMap(opacityFB, depthFB, shadowW, shadowH, w, h);
    
    
    createLongitudinalTex(longitudinalFB, w, h);
    createAzimuthalTex(azimuthalRFB, w, h, 0);
    createAzimuthalTex(azimuthalTTFB, w, h, 1);
    createAzimuthalTex(azimuthalTRTFB, w, h, 2);
    
    // renderHead();
    renderHair();
    

    shader.setInt("Pass", 1);
    drawPlane();

    glDisable(GL_CULL_FACE);
    drawPlane(light.getPos());
    glEnable(GL_CULL_FACE);
}



void UIRender(void) {
    nk_glfw3_new_frame();


    if (nk_begin(ctx, "Option", nk_rect(0, 0, 330, 350),
        NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE))
    {
        //nk_layout_row_dynamic(ctx, 25, 2);
        //nk_labelf(ctx, NK_TEXT_LEFT, "type: %.1f", type);
        //nk_slider_float(ctx, 0, &type, 1., 1.);

        //nk_layout_row_dynamic(ctx, 25, 2);
        //nk_labelf(ctx, NK_TEXT_LEFT, "sampling: %.1f", sampling);
        //nk_slider_float(ctx, 0, &sampling, 32., 32.);

        nk_layout_row_dynamic(ctx, 25, 2);
        nk_labelf(ctx, NK_TEXT_LEFT, "shadow bias: %.4f", shadowBias);
        nk_slider_float(ctx, 0, &shadowBias, 0.01, 0.0001);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Light", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 25, 2);
            nk_labelf(ctx, NK_TEXT_LEFT, "light factor: %.1f", light.lightFactor);
            nk_slider_float(ctx, 0, &light.lightFactor, 10., 0.1);

            nk_labelf(ctx, NK_TEXT_LEFT, "light size: %.1f", light.lightSize);
            nk_slider_float(ctx, 0, &light.lightSize, 30., 0.1);

            glm::vec3 lightPos = light.getPos();
            nk_labelf(ctx, NK_TEXT_LEFT, "lightPos x: %.1f", lightPos.x);
            nk_slider_float(ctx, -10., &lightPos.x, 10., 0.01);
            nk_labelf(ctx, NK_TEXT_LEFT, "lightPos y: %.1f", lightPos.y);
            nk_slider_float(ctx, -10., &lightPos.y, 10., 0.01);
            nk_labelf(ctx, NK_TEXT_LEFT, "lightPos z: %.1f", lightPos.z);
            nk_slider_float(ctx, -10., &lightPos.z, 10., 0.01);
            light.setPos(lightPos);

            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Hair", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 25, 2);
            nk_labelf(ctx, NK_TEXT_LEFT, "alphaR: %.1f", alphaR);
            nk_slider_float(ctx, -10.0f, &alphaR, 0.0f, 0.1f);

            nk_labelf(ctx, NK_TEXT_LEFT, "betaR: %.1f", betaR);
            nk_slider_float(ctx, 5.0f, &betaR, 30.0f, 0.1f);

            nk_labelf(ctx, NK_TEXT_LEFT, "absorption Factor: % .1f", absorptionFactor);
            nk_slider_float(ctx, 0, &absorptionFactor, 1., 0.1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "absorption r: % .1f", absorption.x);
            nk_slider_float(ctx, 0, &absorption.x, 1., 0.1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "absorption g: % .1f", absorption.y);
            nk_slider_float(ctx, 0, &absorption.y, 1., 0.1f);
            nk_labelf(ctx, NK_TEXT_LEFT, "absorption b: % .1f", absorption.z);
            nk_slider_float(ctx, 0, &absorption.z, 1., 0.1f);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "hair color:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(hairColor), nk_vec2(nk_widget_width(ctx), 400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                hairColor = nk_color_picker(ctx, hairColor, NK_RGB);
                nk_layout_row_dynamic(ctx, 25, 1);
                hairColor.r = nk_propertyf(ctx, "#R:", 0, hairColor.r, 1.0f, 0.01f, 0.005f);
                hairColor.g = nk_propertyf(ctx, "#G:", 0, hairColor.g, 1.0f, 0.01f, 0.005f);
                hairColor.b = nk_propertyf(ctx, "#B:", 0, hairColor.b, 1.0f, 0.01f, 0.005f);
                nk_combo_end(ctx);
            }

            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Head", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "head color:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(headColor), nk_vec2(nk_widget_width(ctx), 400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                headColor = nk_color_picker(ctx, headColor, NK_RGB);
                nk_layout_row_dynamic(ctx, 25, 1);
                headColor.r = nk_propertyf(ctx, "#R:", 0, headColor.r, 1.0f, 0.01f, 0.005f);
                headColor.g = nk_propertyf(ctx, "#G:", 0, headColor.g, 1.0f, 0.01f, 0.005f);
                headColor.b = nk_propertyf(ctx, "#B:", 0, headColor.b, 1.0f, 0.01f, 0.005f);
                nk_combo_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_labelf(ctx, NK_TEXT_LEFT, "roughness: %.1f", roughness);
            nk_slider_float(ctx, 0, &roughness, 1., 0.1f);

            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Shadow", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 25, 2);

            // nk_labelf(ctx, NK_TEXT_LEFT, "Opacity Layers: %d", layers);
            // nk_slider_int(ctx, 4, &layers, 28, 1);

            nk_labelf(ctx, NK_TEXT_LEFT, "Layer Size: %.4f", layerSize);
            nk_slider_float(ctx, 0.0001f, &layerSize, .01f, 0.0001f);

            nk_labelf(ctx, NK_TEXT_LEFT, "Opacity Value: %.2f", opacityValue);
            nk_slider_float(ctx, 0, &opacityValue, 0.5f, 0.01f);

            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void handle_dropped_file(const char* path) {
    cout << "Reading file...." << endl;
    hairModel.readHairFile(path);
    hairModel.create();

    hairColor.r = hairModel.getColor().r;
    hairColor.g = hairModel.getColor().g;
    hairColor.b = hairModel.getColor().b;
    hairColor.a = hairModel.getColor().a;

    cout << "Reading complete !" << endl;
}

void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    for (int i = 0; i < count; i++)
        handle_dropped_file(paths[i]);
}

bool checkUIClicked(double x, double y) {
    float leftX, leftY, rightX, rightY;
    int name_len = (int)nk_strlen("Option");
    nk_hash name_hash = nk_murmur_hash("Option", (int)name_len, NK_WINDOW_TITLE);
    nk_window* win = nk_find_window(ctx, name_hash, "Option");
    if (win) {
        leftX = win->bounds.x;
        leftY = win->bounds.y;
        rightX = leftX + win->bounds.w;
        rightY = leftY + win->bounds.h;
        if (x >= leftX && x <= rightX && y >= leftY && y <= rightY) {
            return true;
        }
        return false;
    }
    else {
        leftX = 0; leftY = 0; rightX = 0; rightY = 0;
        cout << "Not found nuklear window" << endl;
        return false;
    }
}

int UIClicked = nk_false;
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double x, y;

    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(window, &x, &y);

    if (action == GLFW_PRESS) {
        if (checkUIClicked(x, y)) { UIClicked = nk_true; }
        else { UIClicked = nk_false; }

        lastX = int(x);
        lastY = int(y);
    }
    else glfw.is_double_click_down = nk_false;
}

void cursorMotionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !UIClicked) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) { camera.scrolled(0, (float)(ypos - lastY)); }
        else { camera.mouseMoved((float)(xpos - lastX), (float)(ypos - lastY)); }

        lastX = int(xpos);
        lastY = int(ypos);
    }
}

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) keyInput = 0;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) keyInput = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) keyInput = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) keyInput = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) keyInput = 4;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    // ignore non-significant error/warning codes 
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return; std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other"; break;
    }
    std::cout << std::endl;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other"; break;
    }
    std::cout << std::endl;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification"; break;
    }
    std::cout << std::endl; std::cout << std::endl;
}