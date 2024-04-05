
=====

This folder exists to store VLKX renderer objects that are TEMPORARY.
It currently contains:

=====

Model Loader system

The Model Loader is temporarily implemented as a raw file reader and OBJ parser.
It should be removed when the File Format system is able to parse model and texture files.

====

Model Abstraction system

The Model Abstraction allows you to create a model with:
 - A mesh
 - An arbitrary number of textures
 - A push constant
 - An arbitrarily large uniform buffer
 - A fragment and vertex shader

In all, it contains a custom Render Pipeline that will be used to draw the model.
This allows for drastic and visually appealing effects.

It should be maintained and moved into the appropriate Shadow module once ready.

====

Model Builder system

The model Builder allows for simple construction of Models using the Model Abstraction system above.
It consumes the Model Loader and returns a Model Abstraction.

It should be MOVED into the File Format parsing system once it is ready.