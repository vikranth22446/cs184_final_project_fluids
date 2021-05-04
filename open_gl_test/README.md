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