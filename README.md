# 3D Fluid simulation based on Smoothed-particle hydrodynamics(SPH)
View the website at https://vikranth22446.github.io/cs184_final_project_fluids/


Core fluid simulation logic based on https://matthias-research.github.io/pages/publications/sca03.pdf

## Building the code
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
