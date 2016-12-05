# Code from a project to predict shadows of GNSS signals for improved positioning with Shadow Mapping

##Code

**This code has only been tested on GNU/Linux, on x86_64 and on nVidia and AMD GPUs, with OpenGL 4.5.**

Within the 'code/zenith_distance_mapping' folder is the original code and a Makefile, which, when run, produces several executables, each made from 'main.c' linked with a 'render_X.c' file after translation. All executables expect the Z-axis to mean "up". Explanations of the generated executables follow:

**shadow_mapped.elf:** This shows shadows generated by shadow mapping (using an oblique projection) or, if toggled (with "S"), the depth map/shadow map, directly. It takes two arguments: firstly, it takes a .obj file from which to generate a depth map and thence shadows; secondly, it (optionally) takes a .obj file upon which to cast shadows (else, the first file will shadow itself).

**zen_dist_map.elf:** This generates .png files showing maps (at a 1 square metre per pixel scale) of the first zenith distance (angle from the zenith, in degrees) at which each point/pixel is first shadowed. It generates this via a variant of shadow mapping, using an oblique projection and alternating "read" and "draw" framebuffers. It takes two arguments: firstly, it takes a .obj file from which to generate depth maps and thence shadows; secondly, it (optionally) takes a .obj file upon which to cast shadows (else, the first file will shadow itself).

**shad_vol_zdmaps.elf:** This generates .png files showing maps (at a 1 square metre per pixel scale) of the first zenith distance (angle from the zenith, in degrees) at which each point/pixel is first shadowed. It generates this via a variant of the stencil shadow volume method, using a geometry shader to generate the shadow volumes. It takes two arguments: firstly, it takes a .obj file from which to generate shadow volumes and thence shadows; secondly, it (optionally) takes a .obj file upon which to cast shadows (else, the first file will shadow itself). It will not work with any file that is only a single flat plane, as it discards the farthest vertex ( or vertices, if equal farthest) from the viewer.

**height_map.elf:** This is the simplest. It takes a single argument on the command-line, the .obj file from which to create a height map, and partly serves as a minimal example of the framework.


##Models

All models provided, as Wavefront .obj files, are derived from models produced by [Zmapping Ltd](http://www.zmapping.com/urban3dmodelling.htm), with the Z-axis being "up" and using the UK National Grid coordinate system (**OSGB36**). **Copyright Zmapping Ltd. 2004.**

**Fenchurch Street City Model.obj:** This is a 3D model of the Fenchurch Street area of the City of London.
**fenchurch_road_and_land_test.obj:** As above, but only the ground level, to be used as a surface to be shadowed.
**fenchurch_road_only_test.obj:** As above, but only the roads, to be used as a surface to be shadowed.
**fenchurch_leadenhall_intersection_17_metre_height_ground_plane.obj:** A ground plane to be used as a surface to be shadowed, to be used for shadowing models including the Fenchurch Street and Leadenhall Street intersection.
**fenchurch_leadenhall_intersection_17_metre_height_ground_plane_2_level.obj:** As above, but with another plane below it in order for it to produce valid (i.e. any) output with **shad_vol_zdmaps.elf**.

___

**For now (and in case anyone cares), this code is licensed under the Affero General Public Licence version 3 or later (AGPL-3.0+).**
