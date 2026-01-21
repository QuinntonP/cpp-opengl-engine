# OpenGL Game Engine

A 3D game engine built with C++ and OpenGL, featuring procedural terrain generation, dynamic object placement, and atmospheric rendering.

## Features

### Graphics & Rendering
- **Modern OpenGL 3.3** with GLSL shaders
- **Dynamic Lighting** - Ambient and diffuse lighting with warm sunlight
- **Atmospheric Fog** - Distance-based fog for depth perception
- **Texture Support** - Multi-texture rendering system with STB image loading
- **Smooth Shading** - Interpolated normals for realistic lighting

### Terrain & World Generation
- **Procedural Terrain** - Generated using Perlin noise
- **Configurable Parameters** - Adjustable terrain size, scale, and detail
- **Smart Object Placement** - Randomized placement with constraints:
  - Slope-based filtering
  - Height-based filtering
  - Automatic ground alignment
  - Random rotation and scaling

### Camera System
- **Free-flight Camera** - Full 6DOF movement
- **Mouse Look** - Smooth first-person camera controls
- **Keyboard Controls**:
  - `W/A/S/D` - Move forward/left/backward/right
  - `Space` - Move up
  - `Left Shift` - Move down
  - `Mouse` - Look around
  - `Scroll Wheel` - Zoom in/out
  - `ESC` - Exit
  - `F9` - Take screenshot

### Content Pipeline
- **Multi-format Model Loading** - Import 3D models in STL and OBJ formats
- **Automatic UV Generation** - Spherical, cylindrical, and planar UV mapping for models without texture coordinates
- **Multi-texture Support** - PNG/TGA texture loading
- **Mesh Batching** - Optimized rendering with separate VAOs for terrain, trees, and grass

## Screenshots

![Game Engine Screenshot](screenshot_placeholder.png)

## Prerequisites

- **C++ Compiler** with C++17 support
- **CMake** 3.16 or higher
- **OpenGL** 3.3 or higher
- **GLFW3** - Window and input management
- **GLM** - OpenGL Mathematics library

### Linux (Arch)
```bash
sudo pacman -S cmake glfw-x11 glm
```

### Linux (Ubuntu/Debian)
```bash
sudo apt install cmake libglfw3-dev libglm-dev
```

### macOS
```bash
brew install cmake glfw glm
```

## Building

```bash
# Clone the repository
git clone <repository-url>
cd GameEngine

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build .

# Run the application
./my_app
```

## Project Structure

```
GameEngine/
├── src/                    # Source files
│   ├── main.cpp           # Application entry point
│   ├── shader.cpp         # Shader compilation and linking
│   ├── object.cpp         # 3D object handling (STL/OBJ loading)
│   ├── obj_loader.cpp     # OBJ model loader
│   ├── terrain.cpp        # Terrain generation
│   ├── perlin_noise.cpp   # Noise generation
│   ├── texture.cpp        # Texture loading and management
│   └── stb_image.cpp      # Image loading implementation
├── include/               # Header files
│   ├── camera.h           # Camera controller
│   ├── shader.h           # Shader class
│   ├── object.h           # Object class
│   ├── obj_loader.h       # Model loader
│   ├── terrain.h          # Terrain generator
│   ├── perlin_noise.h     # Noise generator
│   ├── texture.h          # Texture class
│   └── stb_image.h        # Image loading header
├── shaders/               # GLSL shaders
│   ├── vertex.glsl        # Vertex shader
│   └── frag.glsl          # Fragment shader
├── resources/             # Assets
│   ├── models/            # 3D models (.stl, .obj)
│   └── textures/          # Texture images (.png, .tga)
├── external/              # Third-party libraries
│   ├── include/           # GLAD headers
│   └── src/               # GLAD source
└── CMakeLists.txt         # Build configuration
```

## Configuration

Edit the terrain configuration in [src/main.cpp](src/main.cpp):

```cpp
TerrainConfig terrainConfig = {
    100,        // gridSize - Number of vertices per side
    50.0f,      // worldSize - Physical size of terrain
    5.0f,       // heightScale - Maximum terrain height
    0.1f        // noiseScale - Frequency of terrain features
};
```

## Adding Custom Models

### Supported Formats

| Format | Extension | Features |
|--------|-----------|----------|
| **STL** | `.stl` | ASCII STL files, auto-generated spherical UVs |
| **OBJ** | `.obj` | Wavefront OBJ with vertices, normals, and texture coordinates |

### How to Add Models

1. Place your `.stl` or `.obj` model files in `resources/models/`
2. Place corresponding textures in `resources/textures/`
3. Update the object placement code in `main.cpp`:

```cpp
addObjectsRandomly(meshName, terrain, terrainConfig, {
    std::string(PROJECT_ROOT) + "/resources/models/your_model.obj",  // .stl or .obj
    1000,        // count
    1.0f,        // base scale
    0.3f,        // scale variation
    0.0f,        // yOffset
    true,        // placeAtLowestVertex
    0.0f,        // minSlope
    40.0f,       // maxSlope
    0.0f,        // minHeight
    50.0f,       // maxHeight
    -90.0f,      // rotX
    0.0f,        // rotY
    0.0f,        // rotZ
    true         // randomRotationY
}, seed);
```

### UV Mapping

- **OBJ files**: Uses texture coordinates from the file (if present)
- **STL files**: Automatically generates spherical UV mapping for texturing

## Technical Details

### Rendering Pipeline
1. **Terrain Generation** - Perlin noise-based heightmap
2. **Object Placement** - Procedural placement with constraints
3. **Mesh Batching** - Separate VAOs for different object types
4. **Texture Binding** - Multi-texture support per mesh
5. **Lighting Calculation** - Per-fragment lighting in shader
6. **Fog Application** - Distance-based atmospheric effect

### Shader Features
- **Vertex Shader** - World-space position and normal transformation
- **Fragment Shader**:
  - Phong lighting model (ambient + diffuse)
  - Texture sampling with fallback colors
  - Distance-based fog calculation
  - Smooth color blending

### Performance Optimizations
- Static mesh generation (terrain and objects generated once)
- Uniform caching to reduce GL state changes
- Efficient vertex buffer layout
- Batch rendering of similar objects

## Controls Reference

| Key | Action |
|-----|--------|
| `W` | Move Forward |
| `S` | Move Backward |
| `A` | Move Left |
| `D` | Move Right |
| `Space` | Move Up |
| `Left Shift` | Move Down |
| `Mouse Move` | Look Around |
| `Scroll Wheel` | Zoom |
| `F9` | Screenshot |
| `ESC` | Exit |

## Known Limitations

- Limited to static object placement (generated at startup)
- Single light source (sun)
- No shadow rendering

## Future Improvements

- [ ] Add shadow mapping
- [ ] Implement skybox rendering
- [ ] Add support for FBX/glTF model formats
- [ ] Dynamic object spawning/removal
- [ ] Multiple light sources
- [ ] Water rendering
- [ ] Particle system
- [ ] Level of detail (LOD) system

## License

This project is open source and available under the [MIT License](LICENSE).

## Acknowledgments

- **GLAD** - OpenGL loader
- **GLFW** - Window and input management
- **GLM** - Mathematics library
- **stb_image** - Image loading library
- **Perlin Noise** - Terrain generation algorithm

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.
