# Umbra
Is the name of this monorepo that contains all shadow engine related files including:
documentations, project site, test games etc

# Shadow Engine 
An ambitious game engine with many predecessors and a rewrite filled history.

The goal is to be a "love letter" for the older style game engines like Source 1.
Be as simple to use as possible,
but also allow for extensions and configuration as much as possible.

# Contributing

Mos to f the dependencies are downloaded by the git submodules.
To build the project locally make sure you have them fully synced and cloned.

## External Dependencies
- Compiler
  - Windows: MSVC
  - Linux: GCC
- Vulkan SKD

## Running it locally:

### Running the base engine
To run the base engine either run the shadow-runtime or shadow-editor projects by themselves

### Running Test-Game
To run the test game
``shadow-runtime -game test-game``

To run test-game with editor
``shadow-editor -game test-game``