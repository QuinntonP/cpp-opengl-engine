#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "../include/shader.h"
#include "../include/object.h"
#include "../include/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ----------------------------
// Globals / shared state
// ----------------------------
Camera camera;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static bool firstMouse = true;
static float lastX = 400.0f;
static float lastY = 300.0f;

struct Uniforms {
    GLint model = -1;
    GLint view = -1;
    GLint projection = -1;
};

struct AppState {
    GLFWwindow* window = nullptr;

    Shader* shader = nullptr; // owned in main, pointer stored here
    Uniforms u;

    GLuint VAO = 0;
    GLuint VBO = 0;

    GLsizei vertexCount = 0;
};

// ----------------------------
// Callbacks
// ----------------------------
static void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// ----------------------------
// Timing + Input
// ----------------------------
static void updateDeltaTime() {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Forward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Backward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Left, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Right, deltaTime);
}

// ----------------------------
// GL setup helpers
// ----------------------------
static bool initWindowAndGL(AppState& app) {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    app.window = glfwCreateWindow(800, 600, "opengl", nullptr, nullptr);
    if (!app.window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(app.window);
    glfwSetFramebufferSizeCallback(app.window, framebuffer_size_callback);
    glfwSetCursorPosCallback(app.window, mouse_callback);
    glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    return true;
}

static void createMeshBuffers(AppState& app, const std::vector<Vertex>& mesh) {
    app.vertexCount = (GLsizei)mesh.size();

    glGenVertexArrays(1, &app.VAO);
    glGenBuffers(1, &app.VBO);

    glBindVertexArray(app.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, app.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), mesh.data(), GL_STATIC_DRAW);

    // Assumes Vertex begins with 3 floats for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

static void cacheUniformLocations(AppState& app) {
    // Cache once; calling glGetUniformLocation every frame is fine for learning,
    // but caching is cleaner and faster.
    app.u.model      = glGetUniformLocation(app.shader->ID, "model");
    app.u.view       = glGetUniformLocation(app.shader->ID, "view");
    app.u.projection = glGetUniformLocation(app.shader->ID, "projection");
}

// ----------------------------
// Drawing
// ----------------------------
static void drawFrame(AppState& app) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    app.shader->use();

    glm::mat4 model = glm::mat4(1.0f);

    int width, height;
    glfwGetFramebufferSize(app.window, &width, &height);
    if (height == 0) height = 1; // safety on minimize

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom),
        (float)width / (float)height,
        0.1f,
        100.0f
    );

    glm::mat4 view = camera.GetViewMatrix();

    glUniformMatrix4fv(app.u.model, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(app.u.view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(app.u.projection, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(app.VAO);
    glDrawArrays(GL_TRIANGLES, 0, app.vertexCount);
}

// ----------------------------
// Cleanup
// ----------------------------
static void cleanup(AppState& app) {
    if (app.VAO) glDeleteVertexArrays(1, &app.VAO);
    if (app.VBO) glDeleteBuffers(1, &app.VBO);
    glfwTerminate();
}

// ----------------------------
// Main
// ----------------------------
int main() {
    AppState app;

    if (!initWindowAndGL(app)) {
        return -1;
    }

    Object obj("../resources/models/Ranger.stl", 0.1);
    std::vector<Vertex> mesh = obj.getVertices();

    std::cout << "mesh vertices: " << mesh.size() << std::endl;
    if (mesh.empty()) {
        std::cerr << "Mesh is empty\n";
        cleanup(app);
        return -1;
    }

    std::string vertPath = std::string(PROJECT_ROOT) + "/shaders/vertex.glsl";
    std::string fragPath = std::string(PROJECT_ROOT) + "/shaders/frag.glsl";
    Shader shader(vertPath.c_str(), fragPath.c_str());
    app.shader = &shader;

    createMeshBuffers(app, mesh);
    cacheUniformLocations(app);

    while (!glfwWindowShouldClose(app.window)) {
        updateDeltaTime();
        processInput(app.window);
        drawFrame(app);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    cleanup(app);
    return 0;
}
