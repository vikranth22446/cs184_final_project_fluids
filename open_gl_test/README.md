# 3D Fluid simulation based on Smoothed-particle hydrodynamics(SPH)

Core fluid simulation logic: <TODO add paper link>

Building the code
```
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
```
On Linux:
sudo apt install libglm-dev
sudo apt-get install libglfw3-dev

On Mac:
brew install glm glfw glew 

On Windows:


References:
- nano-gui: used to render text and buttons on the screen easily
    - We choose not to let shaders be managed through it and instead use a custom solution since we found it required less boilerplate
- https://github.com/opengl-tutorials/ogl tutorials on how basic open gl works
    - helpful in understanding how array instantiated, vertex indexing works
- Proj4 clothsim: helpful for setting up camera code + views and mouse callbacks
- Rendiner a sphere: 
    - https://www.songho.ca/opengl/gl_sphere.html
    - https://github.com/Erkaman/cute-deferred-shading also helpful in looking at how vbo shaders worked
    - gluSphere is old so choose to manually setup with slices and stacks
    - we used both indexed vbo for performance


# Building the project in Visual Studio for Windows

We had some compatibility issues building the project for Windows as the project was created using Linux. You can try using Ubuntu for Windows and setting up a display to access a GUI. That did not work for us. Below is our solution:
<br>
Install vcpkg (CS184 students should already have this) and cd into the directory.
Run the following commands to download and install the packages needed for OpenGL:
<br>
	./vcpkg install glm
	./vcpkg install glew
	./vcpkg install glfw3
	./vcpkg integrate install
<br>
Next open the project (open_gl_test should be the root folder) and right click on CMakeLists.txt to open the CMake Settings. Add x64-Release and your CMake toolchain as usual. Navigate down to "CMake variables and cache" where you will see "GLEW_DIR", "GLFW3_INCLUDE_DIR", "GLFW3_LIBRARY", and "GLM_INCLUDE_DIR". Find the paths in your vcpkg directory (vcpkg/packages) and set the paths for the corresponding CMake variables. They will look something like the following:
	vcpkg/packages/glew_x86-windows
	vcpkg/packages/glfw3_x86-windows
	vcpkg/packages/glfw3_x86-windows/lib/glfw3dll.lib
	vcpkg/packages/glm_x86-windows/include
<br>

If there is a build error with 'misc/getopt.h', go into main.h and comment out "#include 'misc/getopt.h'" in the if statement for WIN32.

<br>

If you get the error "'M_PI':undeclared identifier' in the physics.cpp, it means math.h does not define it. Declare a definition like so:
	#ifndef M_PI
	#define M_PI 3.14159265358979323846
	#endif
<br>
Finally, if there are external reference errors concerning stb_image, head over to utils/stb_image.h and add the following line to the top of the file:
	 "#define STB_IMAGE_IMPLEMENTATION"
<br>
Hopefully, this guide helped you get the project building for Windows. Quite a nuiscance!`

