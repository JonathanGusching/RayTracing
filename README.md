# GPU RayTracing
## Prérequis
Pour faire fonctionner le ray tracing par shaders (GPU), il est nécessaire d'avoir installé:
- OpenGL 4.3 >=
- GLM
- GLFW3
- Avoir un support des shaders OpenGL (GLSL version 4.30>= )
- GLEW
- libstudxml

Le code a été compilé et testé sous Linux. Il se peut qu'il y ait des soucis sous Windows, mais les bibliothèques utilisées sont portables.

## Commandes:
Pour se déplacer, les touches sur AZERTY sont:
- Z pour avancer
- S pour reculer
- Q pour aller à gauche
- D pour aller à droite
- souris pour bouger la caméra
- ESC pour quitter la fenêtre

Sur un clavier QWERTY, les touches sont analogues

## Charger/Enregistrer une scène:
Les scènes basiques fonctionnent, avec des formes primitives.
- La fonction ExportScene de la classe SceneManager enregistre dans le fichier la configuration de la scène actuelle sous format XML.
- La fonction ImportScene de la classe SceneManager charge depuis le fichier la configuration d'une scène enregistrée au format XML.

## Système de logs:
La fonction Write permet d'écrire avec gestion de l'heure et de la date, et du type de message sur le flux choisi.
Par défaut, les messages d'erreur sont écrits sur std::cerr qui est redirigé vers un fichier dans le dossier log.

## Problèmes rencontrés
Sous Linux, faire attention aux pilotes utilisés et être sûr que le programme utilise bien le GPU et non pas uniquement le CPU. 