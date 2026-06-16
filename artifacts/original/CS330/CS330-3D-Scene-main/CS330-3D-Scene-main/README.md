# CS 330 – 3D Scene Project

This repository contains my CS 330: Computational Graphics and Visualization 3D scene project. It includes the core source files that manage the scene, view, shaders, and textures, along with supporting libraries and utilities needed to build and run the project.

## Approach to Designing Software

I approach software design by first breaking the problem into logical components and giving each part a clear responsibility. For this project, I organized the scene logic into classes such as `SceneManager`, `ViewManager`, and `ShaderManager`, so that scene composition, camera/view control, and shader handling were separated instead of mixed together. This helped keep the design easier to understand and change as the scene became more complex.

Working on this project helped me grow my design skills in several ways. I had to think in three dimensions when arranging objects, choosing textures, and planning how the camera would move through the scene. I also learned to design around the graphics pipeline—deciding what data each part of the program needed, how to structure transformations, and how shaders would be applied—so the overall design supported both performance and visual quality.

## Design Process and Future Use

My design process followed an iterative cycle of planning, implementing, and refining. I started with a very simple version of the scene to make sure the basic rendering, camera, and shaders worked, then gradually added more detail. As I added textures like the fire, walnut, and wicker images, and refined my shaders and lighting, I kept testing from the camera’s perspective and making small adjustments to object placement and materials.

The tactics I used in this project—breaking the design into focused classes, starting with a minimal working version, and refining in small steps—are strategies I can apply to future work. Whether I’m designing a user interface, a game level, or another visualization, I can reuse this approach of separating responsibilities, building a simple prototype first, and then iterating based on what I see and learn.

## Approach to Developing Programs

When I develop programs, I try to move from core functionality to polish, testing frequently along the way. In this project, I focused first on getting the core managers (`SceneManager`, `ViewManager`, `ShaderManager`) working together so that the scene would render correctly and the camera could move. After that, I added and adjusted textures, materials, and lighting using files like `fragmentShader.glsl`, the image textures, and libraries such as `stb_image.h` and `linmath.h` to support image loading and math operations.

I relied on incremental development and frequent compilation and run‑throughs. Each time I changed the fragment shader, added a new texture, or adjusted a transformation, I built and ran the program to see the effect immediately. Over the course of the milestones, my approach evolved from just “getting output on the screen” to paying more attention to code organization, reusability of managers, and consistent naming and structure. By the time I finished, I had a cleaner separation between rendering logic, camera logic, and resource management.

## Iteration in the Development Process

Iteration played a major role in this 3D scene. I often tweaked the positions, scales, and rotations of objects based on what I saw from the active camera view. I also iterated on texture choices and shader parameters to get better visual results—for example, adjusting how materials reacted to light or how a texture like the fire or wood grain appeared on a surface. Each small iteration revealed visual issues or design improvements that I couldn’t see just by reading the code.

This iterative process made the final scene feel more intentional. Instead of trying to design everything perfectly up front, I accepted that I would need to see the scene rendered, identify what looked off, and then adjust the code or assets. That mindset is something I can bring to other programming projects, where early versions may be rough but provide valuable feedback for the next iteration.

## Computational Graphics, Visualization, and My Goals

Computational graphics and visualizations have given me a new way to connect programming concepts with visual results. Building this 3D scene helped me practice transformations, vectors, matrices, lighting, and texture mapping in a hands‑on way, and it showed me how small code changes can dramatically change what the user experiences. These skills support my educational goals by deepening my understanding of math and computer science through visual, interactive projects.

Professionally, these skills are valuable in any role that involves interactive applications, simulations, games, or data visualizations. Knowing how to structure a graphics project, manage resources like shaders and textures, and think about user viewpoint and scene composition makes me better prepared for work in areas such as game development, real‑time 3D applications, or visually rich user interfaces. This project is an example I can show in my portfolio to demonstrate that I can design and implement a functioning 3D scene from the ground up.
