# GPU RayTracing
## Prérequis
Pour faire fonctionner le ray tracing par shaders (GPU), il est nécessaire d'avoir installé:
- OpenGL 4.3 >=
- GLM
- GLFW3
- Avoir un support des shaders OpenGL (GLSL version 4.30>= )
- GLEW
- libstudxml

Le code a été compilé avec GCC (c++2a) et testé sous Linux. Il se peut qu'il y ait des soucis sous Windows, mais les bibliothèques utilisées sont portables.

## Avant-propos
Le programme a été compilé et exécuté sous Linux (Ubuntu) avec succès. Les bibliothèques utilisées sont portables, mais il n'est pas certain que tout fonctionne sous Windows sans modifications des #include

## Installation
Il est important d'installer les bibliothèques requises pour pouvoir compiler (le linker utilise -lGLEW -lOpenGL -lglfw -lstudxml).

- libstudxml : https://www.codesynthesis.com/projects/libstudxml/#download
- GLFW3 : https://github.com/glfw/glfw
- GLM : https://github.com/g-truc/glm
- GLEW : http://glew.sourceforge.net/

## Commandes:
Pour se déplacer, les touches sur AZERTY sont:
- Z pour avancer
- S pour reculer
- Q pour aller à gauche
- D pour aller à droite
- souris pour bouger la caméra
- ESC pour quitter la fenêtre
- Maintenir SHIFT pour se déplacer plus vite

Sur un clavier QWERTY, les touches sont analogues à la position.

## Charger/Enregistrer une scène:
Les scènes basiques fonctionnent, avec des formes primitives.
- La fonction ExportScene de la classe SceneManager enregistre dans le fichier la configuration de la scène actuelle sous format XML.
- La fonction ImportScene de la classe SceneManager charge depuis le fichier la configuration d'une scène enregistrée au format XML.

## Système de logs:
La fonction Write permet d'écrire avec gestion de l'heure et de la date, et du type de message sur le flux choisi.
Par défaut, les messages d'erreur sont écrits sur std::cerr qui est redirigé vers un fichier dans le dossier log.

## I/O des shaders:
L'algorithme de ray tracing et construction de l'image (texture pour le quad) est contenu dans le shader "shader/ray_tracing.shader".
S'il venait à être nécessaire de modifier les shaders pour avoir un rendu totalement différent, il est nécessaire d'avoir un compute shader.

## Problèmes rencontrés
Sous Linux, faire attention aux pilotes utilisés et être sûr que le programme utilise bien le GPU et non pas uniquement le CPU sinon, l'écran sera noir.

## Remarques
Les matériaux ont beaucoup de paramètres mais ne permettent pas encore de gérer la transparence, réfraction. Cela reste une piste d'amélioration possible [Au niveau du compute shader]