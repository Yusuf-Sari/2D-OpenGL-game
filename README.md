# 2D-OpenGL-game

Overview:

This is a simple 2D game built with modern OpenGL, GLUT, and GLEW. The game displays a scene with lanes, sidewalks, a player-controlled agent, moving vehicles, and collectible coins.


What It Does:

1. Background:
   - The game scene includes lanes and sidewalks.
2. Agent:
   - The player controls a red triangle (the agent) using the arrow keys.
   - Can only move in the direction that the triangle points.
   - The point of the triangle changes when the triangle reaches the top or down of the screen
3. Vehicles:
   - Vehicles (cars and trucks) appear in random lanes and move across the screen.
   - Cars are drawn as squares; trucks are drawn as wider rectangles.
   - If a vehicle collides with the agent, the game exits.
4. Coins:
   - Yellow round coins are spawned at random positions.
   - Each coin lasts 5 seconds.
   - When the agent touches a coin, 5 points are added and the coin is removed.


How It Works:

1. Initialization:
   - Geometry for lanes, sidewalks, the agent, vehicles, and coins is set up.
   - Shaders (from vshader21.glsl and fshader21.glsl) are loaded.
   - VAOs and VBOs are created for each type of object.

2. Rendering:
   - The display function clears the screen and draws:
      The lanes and sidewalks (using a combined VAO).
      The agent (red triangle).
      Vehicles (drawn as rectangles).
      Coins (drawn as yellow circles using a triangle fan).
     
3. Game Updates:
   - A timer function (timeOut) runs every 20 ms.
   - Updates positions of vehicles and coins, spawns new vehicles and coins randomly, and checks for collisions.
   - If a collision occurs between the agent and a vehicle, the game exits.
   - If the agent collects a coin, 5 points are added and that coin is removed.

4. Input Handling:
   - Arrow keys move the agent.
   - If the user moves in the wrong direction, the game exits.
   - The 'q' key quits the game.
   - The 'p' key pauses or resumes the game.
   - The 's' key prints the current game state (score, agent position, etc.) to the console.


How to Run:

   - Compile and run this on windows.

1. File List:
   - Angel.h
   - CheckError.h
   - vec.h
   - chickenGame.cpp
   - initshader.cpp
   - fshader21.glsl
   - vshader21.glsl

2. Requirements:
   - Install Visual Studio with C++ development tools.
   - Ensure GLUT, GLEW, and OpenGL are installed.
   - The shader files (vshader21.glsl and fshader21.glsl) must be in the same directory as the exe file.

3. Building the Project:
   - Open the Visual Studio solution file.
   - Ensure all necessary files are included in the project.
   - Build the project.

4. Running the Game:
   - Press F5 to run the project in Debug mode.
   - Press Ctrl + F5 to run without debugging.

