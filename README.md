# Modern Boomer Shooter
Inspired by this video by ModernVintageGamer
https://www.youtube.com/watch?v=SV8uBtUHAkQ

This is an OpenGL Wolfenstein3d like system using raylib.

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/62fb4c547c4b4208a32282a98cbb89b4)](https://app.codacy.com/gh/JeffM2501/ModernBoomerShooter/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

# Features
* Reads levels from LDtk (https://ldtk.io/)
* Data Driven design
* Resource and Texture Management
* Fake lighting and Ambient Occlusion

# Building
This project uses game-premake for raylib (https://github.com/raylib-extras/game-premake) and is configured using premake.

## Building for Windows (Visual Studio)
Run the premake-VisualStudio.bat to generate the .sln, then open it.

## Building for Windows (MinGW-W64)
Run the premake-mingw.bat to generate the makefile, then run make in the folder. If you get shell errors then run make SHELL=cmd

## Building for Linux
Run run ./premake5 gmake2, then run make in the folder

## Building for MacOS
Run run ./premake5.osx gmake2, then run make in the folder

## Building with VSCode
Open the folder in vscode and build


# Code Design

## App
Holds the main app, very lightweight.

## World
Holds the map, game objects, and systems

## Map
Cube based level, read from Tiled TMX files

## Systems
Things that need to be updated in some specific order. Tracked by a hash ID so they can be found by other things.
* Input/Action System
* Player Management System
* Rendering System

## Services
Global thigns that manage stuff, but don't need an update. 
* Texture Manager
* Resource Manager
* Table Manager
* Game Time

## Game Objects
Entities in the game, have components, can register with systems for simple ECS-like operations

## Components
Data attached to components, can register with systems, for simple ECS-like operations

## Resources
Any file read by the game at runtime, managed by resource manager

## Tables
Simple key/value pairs for storing generic resource data. All data is initally read from the bootstrap table and it's dependencies.


# License
Copyright (c) 2020-2024 Jeffery Myers

This software is provided "as-is", without any express or implied warranty. In no event 
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you 
  wrote the original software. If you use this software in a product, an acknowledgment 
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
