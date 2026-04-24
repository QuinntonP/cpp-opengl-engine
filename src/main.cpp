#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/shader.h"
#include "../include/object.h"
#include "../include/terrain.h"
#include "../include/camera.h"
#include "../include/texture.h"
#include "../include/obj_placer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#include <ctime>
#include <random>

// ----------------------------
// Globals / shared state
// ----------------------------
Camera camera;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

// FPS counter
static int frameCount = 0;
static float fpsTimer = 0.0f;
static float currentFPS = 0.0f;

static bool firstMouse = true;
static float lastX = 800;
static float lastY = 800;

static bool enableEffects = true;  // Set to false for normal rendering

struct Uniforms {
    GLint model = -1;
    GLint view = -1;
    GLint projection = -1;
    GLint useTexture = -1;
    GLint viewPos = -1;
    GLint enableEffects = -1;
};

struct AppState {
    GLFWwindow* window = nullptr;

    Shader* shader = nullptr; // owned in main, pointer stored here
    Uniforms u;

    // Terrain buffers
    GLuint terrainVAO = 0;
    GLuint terrainVBO = 0;
    GLsizei terrainVertexCount = 0;
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
static void updateDeltaTime(GLFWwindow* /*window*/) {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Update FPS counter
    frameCount++;
    fpsTimer += deltaTime;

    // Update FPS display every 0.5 seconds
    if (fpsTimer >= 0.5f) {
        currentFPS = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;

        // Print FPS to console
        std::cout << "FPS: " << (int)currentFPS << std::endl;
    }
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

    // F9 for screenshot (with debounce)
    static bool f9WasPressed = false;
    bool f9IsPressed = (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS);
    if (f9IsPressed && !f9WasPressed) {
        saveScreenshot(window);
    }
    f9WasPressed = f9IsPressed;

    // E key to toggle effects (with debounce)
    static bool eWasPressed = false;
    bool eIsPressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    if (eIsPressed && !eWasPressed) {
        enableEffects = !enableEffects;
        std::cout << "Effects " << (enableEffects ? "ENABLED" : "DISABLED")
                  << " (PS1-style: " << (enableEffects ? "ON" : "OFF") << ")" << std::endl;
    }
    eWasPressed = eIsPressed;

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

    app.window = glfwCreateWindow(1920, 1080, "opengl", nullptr, nullptr);
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

static void createMeshBuffers(GLuint& VAO, GLuint& VBO, GLsizei& vertexCount, const std::vector<Vertex>& mesh) {
    vertexCount = (GLsizei)mesh.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), mesh.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

static void cacheUniformLocations(AppState& app) {
    // Cache once; calling glGetUniformLocation every frame is fine for learning,
    // but caching is cleaner and faster.
    app.u.model         = glGetUniformLocation(app.shader->ID, "model");
    app.u.view          = glGetUniformLocation(app.shader->ID, "view");
    app.u.projection    = glGetUniformLocation(app.shader->ID, "projection");
    app.u.useTexture    = glGetUniformLocation(app.shader->ID, "useTexture");
    app.u.viewPos       = glGetUniformLocation(app.shader->ID, "viewPos");
    app.u.enableEffects = glGetUniformLocation(app.shader->ID, "enableEffects");
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
    glUniform3fv(app.u.viewPos, 1, glm::value_ptr(camera.Position));
    glUniform1i(app.u.enableEffects, enableEffects ? 1 : 0);

    // Draw terrain without texture
    glUniform1i(app.u.useTexture, 0);
    glBindVertexArray(app.terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, app.terrainVertexCount);
}

// ----------------------------
// Object Placement Helper
// ----------------------------
struct PlacedObject {
    std::string modelPath;
    float x, z;           // Position on terrain (X and Z coords)
    float scale;
    float yOffset;        // Additional height offset above terrain
    bool placeAtLowestVertex; // If true, places model so lowest vertex sits on terrain
    float rotX = 0.0f;    // Rotation around X axis in degrees
    float rotY = 0.0f;    // Rotation around Y axis in degrees
    float rotZ = 0.0f;    // Rotation around Z axis in degrees
};

static void addObjectsToMesh(std::vector<Vertex>& mesh, const Terrain& terrain, const std::vector<PlacedObject>& objects) {
    std::unordered_map<std::string, Object> loadedObjects;

    for (const auto& obj : objects) {
        // Load the model with rotation
        std::vector<Vertex> modelVerts;
        if (loadedObjects.find(obj.modelPath) == loadedObjects.end()){
            Object model(obj.modelPath, obj.scale, obj.rotX, obj.rotY, obj.rotZ);
            loadedObjects.emplace(obj.modelPath, model);
        }

        modelVerts = loadedObjects.find(obj.modelPath)->second.getVertices();

        if (modelVerts.empty()) {
            std::cerr << "Warning: Model has no vertices: " << obj.modelPath << std::endl;
            continue;
        }

        // Get terrain height at this position
        float terrainHeight = terrain.getHeightAt(obj.x, obj.z);

        // Calculate placement heightPv
        float placementHeight = terrainHeight + obj.yOffset;

        if (obj.placeAtLowestVertex) {
            // Find the lowest Y coordinate in the model
            float lowestY = modelVerts[0].y;
            for (const auto& vert : modelVerts) {
                if (vert.y < lowestY) {
                    lowestY = vert.y;
                }
            }
            // Adjust placement so the lowest vertex sits on the terrain
            placementHeight -= lowestY;
        }

        // Offset all vertices to position on terrain
        for (auto& vert : modelVerts) {
            vert.x += obj.x;
            vert.y += placementHeight;
            vert.z += obj.z;
        }

        // Add to combined mesh
        mesh.insert(mesh.end(), modelVerts.begin(), modelVerts.end());

        // std::cout << "Placed " << obj.modelPath << " at ("
        //           << obj.x << ", " << placementHeight << ", " << obj.z
        //           << ") - " << modelVerts.size() << " vertices"
        //           << (obj.placeAtLowestVertex ? " [grounded]" : "") << std::endl;
    }
}


// ----------------------------
// Cleanup
// ----------------------------
static void cleanup(AppState& app) {
    if (app.terrainVAO) glDeleteVertexArrays(1, &app.terrainVAO);
    if (app.terrainVBO) glDeleteBuffers(1, &app.terrainVBO);
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

    // Position camera to view terrain and model
    camera.Position = glm::vec3(100.0f, 40.0f, 100.0f);
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
    std::vector<Vertex> terrainMesh = terrain.getVertices();

    std::cout << "Terrain vertices: " << terrainMesh.size() << std::endl;

    createMeshBuffers(
        app.terrainVAO,
        app.terrainVBO,
        app.terrainVertexCount,
        terrainMesh
    );

    // Define all objects to place on the terrain
    std::vector<PlacedObject> objectsToPlace = {
        // {modelPath, x, z, scale, yOffset, placeAtLowestVertex}
    };

    std::string vertPath = std::string(PROJECT_ROOT) + "/shaders/vertex.glsl";
    std::string fragPath = std::string(PROJECT_ROOT) + "/shaders/frag.glsl";
    Shader shader(vertPath.c_str(), fragPath.c_str());
    app.shader = &shader;

    cacheUniformLocations(app);

    // Set texture uniform
    app.shader->use();
    glUniform1i(glGetUniformLocation(app.shader->ID, "texture1"), 0);

    while (!glfwWindowShouldClose(app.window)) {
        updateDeltaTime(app.window);
        processInput(app.window);

        // Draw with tree and grass textures
        drawFrame(app);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    cleanup(app);
    return 0;
}
