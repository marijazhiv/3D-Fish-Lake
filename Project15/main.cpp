//Marija Živanoviæ, SV19/2021

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

#include <chrono>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

float fov = 45.0f;
float cameraDistance = 20.0f;

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

// shaders za 3D scene (ribice i jezero)
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

// shaders za 2D prikaz potpisa
const char* vertexShader2DSource = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

const char* fragmentShader2DSource = R"glsl(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main() {
    vec4 sampled = texture(texture1, TexCoord);
    if(sampled.a < 0.1)
        discard;
    FragColor = sampled;
}
)glsl";

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 10.0f) fov = 10.0f;
    if (fov > 90.0f) fov = 90.0f;
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
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
    0, 2, 1,
    0, 3, 2
};


float fishVertices[] = {
    // telo (piramida)
     0.0f,  0.0f,  0.2f,  // vrh
    -0.1f, -0.1f, -0.1f,  // dno
     0.1f, -0.1f, -0.1f,
     0.0f,  0.1f, -0.1f,

     // repiæ (piramida)
      0.0f,  0.0f, -0.2f,  // vrh repa
     -0.05f, -0.05f, -0.1f, // dno repa
      0.05f, -0.05f, -0.1f,
      0.0f,  0.05f, -0.1f
};

unsigned int fishIndices[] = {
    // telo
    0, 1, 2,
    0, 2, 3,
    0, 3, 1,
    1, 2, 3,

    // rep
    4, 5, 6,
    4, 6, 7,
    4, 7, 5,
    5, 6, 7
};

void generateFishes(int count) {
    std::srand((unsigned)std::time(nullptr));
    for (int i = 0; i < count; ++i) {
        Fish fish;
        fish.angle = static_cast<float>(rand() % 360);
        fish.radius = 2.0f + static_cast<float>(rand() % 500) / 100.0f;
        fish.speed = 10.0f + static_cast<float>(rand() % 100) / 10.0f;

        fish.size = (0.3f + static_cast<float>(rand() % 100) / 300.0f) * 2.5f;

        fish.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX);
        fishes.push_back(fish);
    }
}

void processInput(GLFWwindow* window) {
    bool w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (w && !wPressedLastFrame)
        for (auto& f : fishes) f.speed = std::min(f.speed + speedStep, maxSpeed);

    if (s && !sPressedLastFrame)
        for (auto& f : fishes) f.speed = std::max(f.speed - speedStep, minSpeed);

    if (a && !aPressedLastFrame)
        for (auto& f : fishes) f.radius = std::min(f.radius + radiusStep, maxRadius);

    if (d && !dPressedLastFrame)
        for (auto& f : fishes) f.radius = std::max(f.radius - radiusStep, minRadius);

    wPressedLastFrame = w;
    sPressedLastFrame = s;
    aPressedLastFrame = a;
    dPressedLastFrame = d;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Ribice u Jezeru", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);        //depth testing

    glEnable(GL_CULL_FACE);         //face culling
    glCullFace(GL_BACK);           
    glFrontFace(GL_CCW);            

    glfwSetScrollCallback(window, scroll_callback);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //lake setup
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

    //fishes setup
    GLuint fishVAO, fishVBO, fishEBO;
    glGenVertexArrays(1, &fishVAO);
    glGenBuffers(1, &fishVBO);
    glGenBuffers(1, &fishEBO);

    glBindVertexArray(fishVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fishVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fishVertices), fishVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fishEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fishIndices), fishIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    generateFishes(20);

    GLuint vertexShader2D = compileShader(GL_VERTEX_SHADER, vertexShader2DSource);
    GLuint fragmentShader2D = compileShader(GL_FRAGMENT_SHADER, fragmentShader2DSource);
    GLuint shaderProgram2D = glCreateProgram();
    glAttachShader(shaderProgram2D, vertexShader2D);
    glAttachShader(shaderProgram2D, fragmentShader2D);
    glLinkProgram(shaderProgram2D);

    // vertexi za kvadrat (2D quad) za prikaz potpisa u NDC prostoru
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f, -1.0f,  0.0f, 0.0f,   
        -0.5f, -1.0f,  1.0f, 0.0f,   
        -0.5f, -0.9f,  1.0f, 1.0f,   
        -1.0f, -0.9f,  0.0f, 1.0f   
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint quadVAO, quadVBO, quadEBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //loading signature texture
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("signature.png", &texWidth, &texHeight, &texChannels, 4);
    if (!data) {
        std::cout << "Failed to load signature texture" << std::endl;
    }

    GLuint signatureTexture;
    glGenTextures(1, &signatureTexture);
    glBindTexture(GL_TEXTURE_2D, signatureTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    using clock = std::chrono::high_resolution_clock;
    const std::chrono::duration<double, std::milli> frameDuration(1000.0 / 60.0); // ~16.6667ms per frame

    while (!glfwWindowShouldClose(window)) {
        auto frameStart = clock::now();

        processInput(window);
        glClearColor(0.96f, 0.87f, 0.70f, 1.0f); // boja peska
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::vec3 camPos(0.0f, cameraDistance, 0.01f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //lake
        GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 lakeModel = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lakeModel));
        glUniform3f(colorLoc, 0.0f, 0.5f, 1.0f);
        glBindVertexArray(lakeVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //fishes
        glBindVertexArray(fishVAO);
        for (auto& fish : fishes) {
            fish.angle += fish.speed * 0.1f;
            float x = fish.radius * cos(glm::radians(fish.angle));
            float z = fish.radius * sin(glm::radians(fish.angle));

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.1f, z));
            model = glm::rotate(model, glm::radians(-fish.angle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(fish.size));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(colorLoc, 1, glm::value_ptr(fish.color));
            glDrawElements(GL_TRIANGLES, sizeof(fishIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        }

        //signature
        glUseProgram(shaderProgram2D);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);
        glUniform1i(glGetUniformLocation(shaderProgram2D, "texture1"), 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        //frame rate control at 60 fps
        auto frameEnd = clock::now();
        auto elapsed = frameEnd - frameStart;
        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }

    glfwTerminate();
    return 0;
}
