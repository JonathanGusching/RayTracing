# GPU RayTracing
## Prérequis
Pour faire fonctionner le ray tracing par shaders (GPU), il est nécessaire d'avoir installé:
- OpenGL 4.0 >=
- GLM
- GLFW3
- Avoir un support des shaders OpenGL (GLSL version 4.30>= )
- GLEW
- libstudxml
Le code a été compilé et testé sous Linux. Il se peut qu'il y ait des soucis sous Windows, mais les bibliothèques utilisées sont portables.

## Problèmes rencontrés
Sous Linux, faire attention aux pilotes utilisés et être sûr que le programme utilise bien le GPU et non pas uniquement le CPU. 