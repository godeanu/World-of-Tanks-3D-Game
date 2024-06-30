# World-of-Tanks-3D-Game
OpenGL C++ project inspired by the World of Tanks game

## Description

This project is a 3D game inspired by World of Tanks. The tank models (cannon, body, turret and tracks) are obj files created with Blender. 

There is a simple map with different buildings spawning on random locations at each game run. The enemies are similar tanks with different colors also spawning at random locations. The tanks move randomly and shoot bullets at the player when within range. We use WASD to move the tank and the mouse to rotate the turret. The player can shoot bullets with the left mouse button. 

On each hit, the enemy tank will lose health points and deform its body based on the remaining health. The deformation is done in the vertex shader by moving the vertices based on a mathematical function that takes the degree of damage and the local coordinates of the vertex as parameters. Also, the enemy tanks will change their color by making it darker based on the degree of damage. The color change will be done in the fragment shader.

Made sure the collision detection is done properly between the tanks, the tanks and the bullets, and the tanks and the buildings. When aproaching the map limits, the tanks will turn around and move in the opposite direction.

There is also a mini-map in the screen corner made using an orthographic projection.

The game ends in a win when we destroy all the enemy tanks and in a loss when we lose all our health points or the time expires.

For the framework used, check this [repository](https://github.com/UPB-Graphics/gfx-framework).