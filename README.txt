MOSSY MAZE WALL - RAYLIB ASSET
================================

Files
-----
maze_wall.obj              Render mesh
maze_wall.mtl              OBJ material file
concrete_albedo.png        512x512 diffuse/albedo texture
moss_albedo.png            512x512 diffuse/albedo texture
maze_wall_collision.obj    Simple box collision proxy

Scale / axes
------------
Y-up. Main wall is about 12.0 wide x 14.0 tall x 2.4 deep.
Its base sits at Y=0, so placing the model at {0,0,0} puts it on the ground.

Lighting
--------
There is deliberately NO baked directional lighting in the mesh or textures.
The albedo is noisy/stained but has no top-to-bottom or side-to-side brightness gradient.
Use your raylib lights/shader to light it.

Minimal raylib usage
--------------------
Model wall = LoadModel("maze_wall.obj");
// If your raylib build does not auto-load MTL textures, load them and assign
// them to the model materials manually. OBJ/MTL paths are relative.
DrawModel(wall, (Vector3){0,0,0}, 1.0f, WHITE);

UnloadModel(wall);

Notes
-----
- The wall is modular and symmetrical enough to tile end-to-end.
- The visible moss is separate geometry/material, so you can remove it from the OBJ
  or swap the Moss material without touching the concrete.
- The thin dark panel joints are geometry overlays, not baked shadows.
- Textures are power-of-two and UVs intentionally repeat beyond 0..1. If your
  renderer clamps instead of repeats, set the texture wrap mode to repeat.
