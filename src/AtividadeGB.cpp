/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 26/06/2025
 * Aluno: Julia Martins
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h" 

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Bezier.h"
#include "Scene.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

int selectedObjectIndex = 0;

Camera camera;
std::vector<Mesh> meshes;
std::vector<Bezier> bezierCurves;
Scene scene;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void setupWindow(GLFWwindow*& window);
void resetAllRotate();


int main()
{
    GLFWwindow* window;
    setupWindow(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height); 
    glViewport(0, 0, width, height);

    Shader objectShader("C:/Users/bruno/Documents/Julia_Teste/ProjetosCG/shaders/object.vs", "C:/Users/bruno/Documents/Julia_Teste/ProjetosCG/shaders/object.fs");

    cout << "Teste " + __LINE__ << endl;

    Shader curveShader("C:/Users/bruno/Documents/Julia_Teste/ProjetosCG/shaders/curve.vs", "C:/Users/bruno/Documents/Julia_Teste/ProjetosCG/shaders/curve.fs");

    cout << "Teste " + __LINE__ << endl;

    if (!scene.loadConfig("C:/Users/bruno/Documents/Julia_Teste/ProjetosCG/assets/config.json")) {
        std::cerr << "Failed to load scene configuration. Exiting." << std::endl;
        glfwTerminate();
        return -1;
    }
    
    camera.initialize(&objectShader, WINDOW_WIDTH, WINDOW_HEIGHT); 
    scene.setupScene(window, &objectShader, &camera, meshes, bezierCurves);

    camera.setCameraPosInitial(scene.cameraInitialPos);
    camera.setCameraFrontInitial(scene.cameraInitialFront);
    camera.setCameraUpInitial(scene.cameraInitialUp);
    camera.setProjection(scene.cameraFov, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, scene.cameraNearPlane, scene.cameraFarPlane);

    objectShader.Use();
    if (!scene.lightSources.empty()) {
        objectShader.setVec3("light.position", scene.lightSources[0].position);
        objectShader.setVec3("light.ambient", scene.lightSources[0].ambient);
        objectShader.setVec3("light.diffuse", scene.lightSources[0].diffuse); 
        objectShader.setVec3("light.specular", scene.lightSources[0].specular);
    }

    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        objectShader.Use();
        camera.update();

        for (size_t i = 0; i < meshes.size(); ++i) {
            if (i < bezierCurves.size() && bezierCurves[i].getFollowTrajectory()) {
                static std::vector<float> trajectoryProgress(bezierCurves.size(), 0.0f);
                int numCurvePoints = bezierCurves[i].getNbCurvePoints();
                if (numCurvePoints > 0) {
                    trajectoryProgress[i] += bezierCurves[i].getSpeed() * deltaTime * 100.0f; 
                    if (trajectoryProgress[i] >= 1.0f) {
                        trajectoryProgress[i] = 0.0f;
                    }
                    int curveIndex = static_cast<int>(trajectoryProgress[i] * numCurvePoints);
                    curveIndex = glm::min(curveIndex, numCurvePoints - 1); 
                    glm::vec3 newPos = bezierCurves[i].getPointOnCurve(curveIndex);
                    meshes[i].setCurrentPosition(newPos);
                }
            }
        }

        for (size_t i = 0; i < meshes.size(); ++i) {
            bool currentObjectRotationX = (i == selectedObjectIndex) ? rotateX : false;
            bool currentObjectRotationY = (i == selectedObjectIndex) ? rotateY : false;
            bool currentObjectRotationZ = (i == selectedObjectIndex) ? rotateZ : false;

            meshes[i].update(currentObjectRotationX, currentObjectRotationY, currentObjectRotationZ);
            meshes[i].draw();
        }

        for (size_t i = 0; i < bezierCurves.size(); ++i) {
            if (bezierCurves[i].getNbCurvePoints() > 0) {
                bezierCurves[i].setShader(&curveShader);
                bezierCurves[i].drawCurve(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
        
        glfwSwapBuffers(window);
    }

    for (const auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO); 
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float scaleStep = 0.05f;
    float translateStep = 0.1f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {
        int index = key - GLFW_KEY_1;
        if (index < meshes.size()) {
            selectedObjectIndex = index;
            resetAllRotate();
            std::cout << "Selected object: " << scene.objects[selectedObjectIndex].name << std::endl;
        }
    }

    if (selectedObjectIndex < meshes.size()) {
        glm::vec3 currentObjectPos = meshes[selectedObjectIndex].getPosition();
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_LEFT_BRACKET:
                    meshes[selectedObjectIndex].setScale(meshes[selectedObjectIndex].scale_ * (1.0f - scaleStep));
                    break;
                case GLFW_KEY_RIGHT_BRACKET: 
                    meshes[selectedObjectIndex].setScale(meshes[selectedObjectIndex].scale_ * (1.0f + scaleStep));
                    break;
                case GLFW_KEY_X:
                    resetAllRotate();
                    rotateX = true;
                    break;
                case GLFW_KEY_Y:
                    resetAllRotate();
                    rotateY = true;
                    break;
                case GLFW_KEY_Z:
                    resetAllRotate();
                    rotateZ = true;
                    break;
                case GLFW_KEY_P:
                    resetAllRotate();
                    meshes[selectedObjectIndex].setPosition(scene.objects[selectedObjectIndex].initial_transform.position);
                    meshes[selectedObjectIndex].setRotation(scene.objects[selectedObjectIndex].initial_transform.rotation_angle, scene.objects[selectedObjectIndex].initial_transform.rotation_axis);
                    meshes[selectedObjectIndex].setScale(scene.objects[selectedObjectIndex].initial_transform.scale);
                    std::cout << "Object " << scene.objects[selectedObjectIndex].name << " transformations reset." << std::endl;
                    break;
                case GLFW_KEY_UP:
                    currentObjectPos.z -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_DOWN:
                    currentObjectPos.z += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_LEFT:
                    currentObjectPos.x -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_RIGHT:
                    currentObjectPos.x += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_PAGE_UP:
                    currentObjectPos.y += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    currentObjectPos.y -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_V:
                    if (selectedObjectIndex < bezierCurves.size()) {
                        if (selectedObjectIndex < scene.objects.size() && scene.objects[selectedObjectIndex].animation.type == "bezier") {
                            bezierCurves[selectedObjectIndex].setFollowTrajectory(!bezierCurves[selectedObjectIndex].getFollowTrajectory());
                            std::cout << "Trajectory for " << scene.objects[selectedObjectIndex].name << " " 
                                    << (bezierCurves[selectedObjectIndex].getFollowTrajectory() ? "activated." : "deactivated.") << std::endl;
                        } else {
                            std::cout << "No Bezier trajectory defined for selected object." << std::endl;
                        }
                    } else {
                        std::cout << "No trajectory data available for selected object." << std::endl;
                    }
                    break;
            }
        }
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        camera.setCameraPos(key);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.mouseCallback(window, xpos, ypos);
}

void setupWindow(GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Prova GB - Julia Martins", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback); 

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;
}

void resetAllRotate() {
    rotateX = false;
    rotateY = false;
    rotateZ = false;
}