#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "../include/shader.h"
#include "../include/object.h"
#include "../include/terrain.h"
#include "../include/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#include <ctime>

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

static void saveScreenshot(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Allocate buffer for pixel data (RGB, 3 bytes per pixel)
    std::vector<unsigned char> pixels(width * height * 3);

    // Read pixels from framebuffer
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip image vertically (OpenGL has origin at bottom-left)
    std::vector<unsigned char> flipped(width * height * 3);
    for (int y = 0; y < height; y++) {
        memcpy(&flipped[y * width * 3], &pixels[(height - 1 - y) * width * 3], width * 3);
    }

    // Generate filename with timestamp
    time_t now = time(nullptr);
    char filename[64];
    strftime(filename, sizeof(filename), "screenshot_%Y%m%d_%H%M%S.png", localtime(&now));

    // Save as PNG
    if (stbi_write_png(filename, width, height, 3, flipped.data(), width * 3)) {
        std::cout << "Screenshot saved: " << filename << std::endl;
    } else {
        std::cerr << "Failed to save screenshot!" << std::endl;
    }
}

static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // F12 for screenshot (with debounce)
    static bool f9WasPressed = false;
    bool f9IsPressed = (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS);
    if (f9IsPressed && !f9WasPressed) {
        saveScreenshot(window);
    }
    f9WasPressed = f9IsPressed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Forward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Backward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Left, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::Right, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return true;
}

static void createMeshBuffers(AppState& app, const std::vector<Vertex>& mesh) {
    app.vertexCount = (GLsizei)mesh.size();

    glGenVertexArrays(1, &app.VAO);
    glGenBuffers(1, &app.VBO);

    glBindVertexArray(app.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, app.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), mesh.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    // Position camera to view terrain
    camera.Position = glm::vec3(50.0f, 30.0f, 50.0f);
    camera.Pitch = -20.0f;

    // Configure terrain generation - rolling hills with occasional peaks
    TerrainConfig terrainConfig;
    terrainConfig.width = 100;
    terrainConfig.depth = 100;
    terrainConfig.heightScale = 20.0f;      // Base height for hills
    terrainConfig.noiseFrequency = 0.03f;   // Gentle rolling features
    terrainConfig.noiseOctaves = 4;         // Smooth detail
    terrainConfig.noisePersistence = 0.5f;  // Natural looking hills
    terrainConfig.noiseLacunarity = 2.0f;   // Standard frequency steps
    terrainConfig.seed = 2;

    Terrain terrain(terrainConfig);
    std::vector<Vertex> mesh = terrain.getVertices();

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
