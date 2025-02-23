# Shadow Engine Core

The Shadow Engine Core is the heart of the Shadow Engine project.
It contains the core functionality of the engine, such as the main loop, the window, the renderer, the input system, etc.
# Shadow Engine

## Introduction

Shadow Engine is a 3D game engine written in C++.



## Parts

The Shadow Engine Core is split into multiple parts, each of which is a module in the project.

- Assets
- Core
- Entity
- Math
- Platforms
- Reflection
- Renderer
- Tests
- Utils
## Coding Style

### General

#### Namespaces
The root namespace is `SH` and all other namespaces should be nested within it.

#### Headers
All headers should be placed in the module specific `inc` directory.
The layout of the `inc` directory should start with `shadow/<Module_name>`.
For example, the header for the `core` module should be placed in `inc/shadow/core`.

### Naming Conventions

- **Classes**: `PascalCase` or `pascal_case` if it emulates stdlib style functions
- **Functions**: `PascalCase`
- **Variables**: `camelCase`
- **Constants**: `UPPER_CASE`
- **Private Members**: `m_camelCase`
- **Pointers**: `pCamelCase`
- **References**: `rCamelCase`
- **Static Members**: `s_camelCase`
- **Global Variables**: `g_camelCase`
- **Global Constants**: `G_UPPER_CASE`
- **Macros**: `MACRO_CASE`
- **Enums**: `PascalCase`

### Formatting

- **Indentation**: 4 spaces
- **Brace placement**: K&R style 