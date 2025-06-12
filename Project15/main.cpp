//Marija Živanoviæ, SV19/2021
// Opis: Testiranje dubine, Uklanjanje lica, Transformacije, Prostori i Projekcije

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

float fov = 45.0f;
float cameraDistance = 20.0f;

// Granice za brzinu i radijus
const float minSpeed = 1.0f;
const float maxSpeed = 50.0f;
const float speedStep = 1.0f;

const float minRadius = 1.0f;
const float maxRadius = 10.0f;
const float radiusStep = 0.2f;

bool wPressedLastFrame = false;
bool sPressedLastFrame = false;


bool aPressedLastFrame = false;
bool dPressedLastFrame = false;

struct Fish {
    float angle;
    float radius;
    float speed;
    glm::vec3 color;
    float size;
};

std::vector<Fish> fishes;

const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)glsl";

// Kontrola zooma
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 10.0f) fov = 10.0f;
    if (fov > 90.0f) fov = 90.0f;
}

// Kompajliranje shadera
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    // Provera gresaka
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader Compilation Error:\n" << infoLog << std::endl;
    }
    return shader;
}

float lakeVertices[] = {
    -10.0f, 0.0f, -10.0f,
     10.0f, 0.0f, -10.0f,
     10.0f, 0.0f,  10.0f,
    -10.0f, 0.0f,  10.0f
};

unsigned int lakeIndices[] = {
    0, 1, 2,
    2, 3, 0
};

float fishVertices[] = {
    0.0f,  0.0f, 0.0f,
    -0.2f, 0.0f, -0.1f,
    -0.2f, 0.0f,  0.1f
};

void generateFishes(int count) {
    std::srand((unsigned)std::time(nullptr));
    for (int i = 0; i < count; ++i) {
        Fish fish;
        fish.angle = static_cast<float>(rand() % 360);
        fish.radius = 2.0f + static_cast<float>(rand() % 500) / 100.0f; // 2.0 - 7.0
        fish.speed = 10.0f + static_cast<float>(rand() % 100) / 10.0f; // 10.0 - 20.0
        fish.size = 0.3f + static_cast<float>(rand() % 100) / 300.0f;
        fish.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX);
        fishes.push_back(fish);
    }
}

// Obrada tastera W, S, A, D za kontrolu brzine i radijusa ribica
void processInput(GLFWwindow* window) {
    bool wPressedNow = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    bool sPressedNow = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    bool aPressedNow = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    bool dPressedNow = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);

    // Poveæaj brzinu samo ako je W upravo pritisnut
    if (wPressedNow && !wPressedLastFrame) {
        for (auto& fish : fishes) {
            fish.speed += speedStep;
            if (fish.speed > maxSpeed) fish.speed = maxSpeed;
        }
    }

    // Smanji brzinu samo ako je S upravo pritisnut
    if (sPressedNow && !sPressedLastFrame) {
        for (auto& fish : fishes) {
            fish.speed -= speedStep;
            if (fish.speed < minSpeed) fish.speed = minSpeed;
        }
    }

    // Poveæaj radijus samo ako je A upravo pritisnut
    if (aPressedNow && !aPressedLastFrame) {
        for (auto& fish : fishes) {
            fish.radius += radiusStep;
            if (fish.radius > maxRadius) fish.radius = maxRadius;
        }
    }

    // Smanji radijus samo ako je D upravo pritisnut
    if (dPressedNow && !dPressedLastFrame) {
        for (auto& fish : fishes) {
            fish.radius -= radiusStep;
            if (fish.radius < minRadius) fish.radius = minRadius;
        }
    }

    // Update stanja za sledeæi frame
    wPressedLastFrame = wPressedNow;
    sPressedLastFrame = sPressedNow;
    aPressedLastFrame = aPressedNow;
    dPressedLastFrame = dPressedNow;
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Jezero s ribicama - Kontrola", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    glfwSetScrollCallback(window, scroll_callback);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLuint lakeVAO, lakeVBO, lakeEBO;
    glGenVertexArrays(1, &lakeVAO);
    glGenBuffers(1, &lakeVBO);
    glGenBuffers(1, &lakeEBO);

    glBindVertexArray(lakeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lakeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lakeVertices), lakeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lakeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lakeIndices), lakeIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint fishVAO, fishVBO;
    glGenVertexArrays(1, &fishVAO);
    glGenBuffers(1, &fishVBO);
    glBindVertexArray(fishVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fishVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fishVertices), fishVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    generateFishes(20);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::vec3 cameraPos = glm::vec3(0.0f, cameraDistance, 0.01f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, target, up);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Jezero
        glBindVertexArray(lakeVAO);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(colorLoc, 0.2f, 0.4f, 0.9f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Ribice
        glBindVertexArray(fishVAO);
        float time = glfwGetTime();
        for (auto& fish : fishes) {
            float x = fish.radius * cos(glm::radians(fish.angle + time * fish.speed));
            float z = fish.radius * sin(glm::radians(fish.angle + time * fish.speed));

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.1f, z));
            model = glm::scale(model, glm::vec3(fish.size));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(colorLoc, fish.color.r, fish.color.g, fish.color.b);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &lakeVAO);
    glDeleteBuffers(1, &lakeVBO);
    glDeleteBuffers(1, &lakeEBO);
    glDeleteVertexArrays(1, &fishVAO);
    glDeleteBuffers(1, &fishVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}