# Crescent
<img src="https://github.com/xRiveria/Spatium/blob/master/Documentation/StanfordDragon.png" width="45%"></img> <img src="https://github.com/xRiveria/Spatium/blob/master/Documentation/StanfordBunny.png" width="45%"></img>

Crescent is the culmination of my very first foray into the world of custom engines. I tried out anything and everything I could before discovering an interest in graphics and editor tooling, 
which is why you will find OpenGL, Vulkan, DirectX12 and a half-baked RHI (Render Hardware Interface) all stuffed into this project.

While I've since moved on to more structured and advanced engines, Crescent serves as a personal nostalgic trip down memory lane. 

## Compilation

To build the project, simply navigate to the `Scripts` folder and run `CrescentBuildWindows.bat`. This will leverage Premake and automatically generate a C++17 solution in the project's root directory.

# Features

- Core OpenGL Backend, with Vulkan and DX12 Sample Projects
- Deferred Rendering
- PBR
- Screen Space Ambient Occlusion
- Directional Shadow Mapping
- Tranparency Sorting
- Material System and Instancing
- Animations
- Custom ECS
- An Editor with a Hierarchy, Inspector and Debug Information
- Numerous Supported Files Formats: 10+ for Fonts, 20+ for Audio, 30+ for Images and 40+ for Models
