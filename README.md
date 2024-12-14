# R3D - 3D Rendering Library for raylib

<img align="left" style="width:260px" src="https://github.com/Bigfoot71/r3d/blob/master/logo.png" width="288px">

R3D is a 3D rendering library designed to work with [raylib](https://www.raylib.com/). It offers features for lighting, shadows, materials, post effects, and more.

Primarily written in C++, the **API is C compatible**, making it easy to integrate into projects using C or C++.

R3D is ideal for developers who want to add 3D rendering to their raylib projects without building a full rendering engine from scratch.

---

## Features

- **Material System**: Allows the creation of materials with multiple diffuse and specular rendering modes, including a toon shading mode. Features include IBL, normal maps, ambient occlusion, and more, as well as sorting surfaces by material to reduce state changes during rendering.
- **Lighting**: Support for multiple light types, including directional, point, and spotlights, with customizable properties.
- **Shadow Mapping**: Real-time shadow rendering with configurable resolution and support for different light types.
- **Post-processing Effects**: Easily integrate post-processing effects like fog, bloom, tonemapping, color correction, etc.
- **CPU Particle System**: Includes built-in support for CPU-side simulated particles with dynamic interpolation curve support.
- **Optional Frustum Culling**: Support for frustum culling by bounding box, can be disabled if you are already using your own system.
- **Optional Depth Sorting**: Support for depth sorting, near-to-far, far-to-near, or disabled by default.
- **Blit Management**: Renders at a base resolution and blits the result (with applied post-processing effects), either maintaining the window aspect ratio or the internal resolution aspect ratio with an auto letterbox effect.

---

## Getting Started

To use R3D, you must have [raylib](https://www.raylib.com/) installed. Follow the steps below to get started with R3D in your project.

### Prerequisites

- **raylib**: The library is provided as a submodule, which is optional. It is only required to build the examples that come with R3D. However, if **raylib 5.5** is already installed on your system, you can use R3D without cloning the submodule.
- **CMake**: For building the library.
- **C++ Compiler**: A C++20 or higher compatible compiler.

### Installation

1. **Clone the repository**:

   ```bash
   git clone https://github.com/Bigfoot71/r3d
   cd R3D
   ```

2. **Optional: Clone raylib submodule** (only needed for building the examples):

   ```bash
   git submodule update --init --recursive
   ```

   **Note**: raylib is provided as a submodule but you can skip cloning it if you don't intend to build the examples.

3. **Build the library**:

   Use CMake to configure and build the library.

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **Link the library to your project**:

   - R3D is a CMake project, and you can include it in your own CMake-based project via `add_subdirectory()` or by linking directly to the built library.
   - If you're using it as the main project, you can build the examples using the option `R3D_BUILD_EXAMPLES` in CMake.

---

## Usage

### Initialize R3D

To initialize the R3D renderer, there are two options depending on your desired settings:

1. **R3D_Init()**: This function initializes the renderer with the current resolution defined by raylib and no additional flags. It’s the simplest method for basic rendering.

2. **R3D_InitEx(int internalWidth, int internalHeight, int flags)**: This function allows you to specify a custom internal resolution for the renderer and set additional initialization flags.

Here’s an example using `R3D_Init()`:

```c
#include <r3d.h>

int main() {
    // Initialize raylib window
    InitWindow(800, 600, "R3D Example");

    // Initialize R3D Renderer with default settings
    R3D_Init();

    // Load a model to render
    R3D_Model model = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Create a directional light
    R3D_Light light = R3D_CreateLight(R3D_DIRLIGHT, 0);
    R3D_SetLightDirection(light, Vector3Normalize((vector3) { -1, -1, -1 }));

    // Init a Camera3D
    Camera3D camera = {
        .position = (Vector3) { 0, 0, 5 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // Main rendering loop
    while (!WindowShouldClose()) {
        BeginDrawing();
            R3D_Begin(camera);
                R3D_DrawModel(&model); // Draw the model in the scene
            R3D_End();
        EndDrawing();
    }

    // Close R3D renderer and raylib
    R3D_Close();
    CloseWindow();

    return 0;
}
```

This example demonstrates how to set up a basic 3D scene using R3D:

1. It initializes a raylib window (800x600 pixels).
2. The `R3D_Init()` function sets up the R3D renderer with the default internal resolution (matching raylib’s window size).
3. A simple cube model is loaded and rendered using `R3D_LoadModelFromMesh()`.
4. A directional light is created to illuminate the scene from an angle.
5. A `Camera3D` is initialized to view the scene from a set position.
6. The main loop continuously renders the cube and light using R3D, updating the display until the window is closed.
7. Finally, the R3D renderer and raylib window are closed gracefully.

### Adding Lights

R3D supports several types of lights, each with its own behavior and characteristics. You can create and manage lights as shown below:

```c
R3D_Light light = R3D_CreateLight(R3D_SPOTLIGHT, 0);                    // Adds a spotlight, without shadows (zero parameter)
R3D_SetLightPositionTarget(light, (Vector3){0, 10, 0}, (Vector3){0});   // Set light position and target
```

The function `R3D_CreateLight()` takes two parameters: the light type and the shadow map resolution. It is recommended to provide a power of 2 for optimal memory usage. Alternatively, you can specify a value `<= 0` to indicate that this light should not produce shadows.

R3D supports the following light types:

1. **R3D_DIRLIGHT (Directional Light)**:
   - Directional lights simulate sunlight, lighting the scene from a specific direction.
   - These lights do not have a position _(unless shadows are enabled)_. Instead, they have a direction and affect the entire scene equally, regardless of the light's position.
   - However, if a directional light is supposed to cast shadows, you will need to assign it a position in order to determine from which point the objects it illuminates should be rendered in its shadow map.

2. **R3D_SPOTLIGHT (Spotlight)**:
   - Spotlights emit light in a cone-shaped area and are positioned at a specific location.
   - You must define both the position and the target for a spotlight. The target specifies where the light is pointing.
   - Spotlights have additional parameters:
     - **Inner Cutoff**: The angle within which the light is fully bright.
     - **Outer Cutoff**: The angle at which the light starts to fade from full brightness to full darkness.
     - **Attenuation**: Defines how the light intensity decreases with distance from the light source. This can be adjusted for more realistic lighting effects.

3. **R3D_OMNILIGHT (Omni Light)**:
   - Omni lights are point lights that emit light in all directions, like a light bulb.
   - The position is always required for omni lights, but the direction is **never used**.
   - Similar to spotlights, omni lights also support **attenuation** to control how the light intensity falls off over distance.

### Drawing a Model

To draw a model in the scene, use the `R3D_DrawModel()` methods. There are three possible variants:

- `void R3D_DrawModel(const R3D_Model* model)`
- `void R3D_DrawModelEx(const R3D_Model* model, Vector3 position, float scale)`
- `void R3D_DrawModelPro(const R3D_Model* model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale)`

The basic function, which only takes the model as a parameter, will apply the transformation stored in the `R3D_Model`, as well as any transformations applied via `rlgl.h` if there are any. The other variants will apply the transformations passed as parameters in addition to those previously mentioned.

Here’s an example:

```c
R3D_Model model = R3D_LoadModel("model.obj");
R3D_DrawModelPro(&model, (Vector3){0, 0, 0}, (Vector3){0, 1, 0}, 45.0f, (Vector3){1, 1, 1});
```

---

## Additional Notes

- **Shadow Mapping**: Supports shadows for point, spot, and directional lights. When creating a light, you can specify a shadow map resolution to enable shadow casting, or set it to `0` to disable shadows. Shadows can still be enabled later using the `R3D_EnableLightShadow` function:

```c
R3D_EnableLightShadow(light, 2048);  // Enable shadow mapping with a 2048x2048 shadow map resolution
```

- **Material System**: The material system allows you to configure shaders and material properties for your models. Use the `R3D_CreateMaterialConfig` function to load a material configuration:

```c
R3D_MaterialConfig config = R3D_CreateMaterialConfig(
    R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
    R3D_BLEND_ALPHA, R3D_CULL_BACK,
    R3D_MATERIAL_FLAG_MAP_NORMAL |
    R3D_MATERIAL_FLAG_MAP_AO
);

R3D_SetMaterialConfig(&model, surfaceIndex, config);
```

- **Post-processing**: Post-processing effects like fog, bloom, tonemapping or color correction can be added at the end of the rendering pipeline using R3D's built-in shaders.

---

## Contributing

If you'd like to contribute, feel free to open an issue or submit a pull request.

---

## License

This project is licensed under the **Zlib License** - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgements

Thanks to [raylib](https://www.raylib.com/) for providing an easy-to-use framework for 3D development!

## Screenshots

![](examples/screenshots/robot.png)
![](examples/screenshots/skybox.png)
![](examples/screenshots/pbr.png)
![](examples/screenshots/bloom.png)
