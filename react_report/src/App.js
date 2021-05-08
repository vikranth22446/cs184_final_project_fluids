import logo from "./logo.png"
import arch_diagram from "./arch.jpg"

import './App.css';
import Navbar from 'react-bootstrap/Navbar'
import Row from 'react-bootstrap/Row'
import Container from 'react-bootstrap/Container'
import React from 'react';
import ReactPlayer from 'react-player/youtube'
var Latex = require('react-latex');

function App() {
  return (
    <div>
  <Navbar bg="dark" variant="dark" className=" rounded">
    <Navbar.Brand><h2>3D Interactive Fluid Simulation Report</h2></Navbar.Brand>

  </Navbar>
  <Container>
  <Row><Navbar.Brand>
      <h4>Vikranth Srivatsa, Bhuvan Basireddy, Kevin Lo, Benjamin Cheung</h4>
    </Navbar.Brand></Row>
    <img src={logo} className="center" alt="logo for fluids"></img>
  </Container>
  <br />
  <Container>
  
    <Row><h2>Abstract</h2></Row>
    <Row><p>Fluid simulations such as those with water, fire, smoke are very important in realtime games and visual effects.
      The many realistic water mods from games such as Skyrim and Minecraft provide a strong case study of their usability. Particle simulation among other approaches is popular method of simulating these effects.
      The study of these simulations particle simulations is usually based around Navier-Stokes fluid dynamics equations. Smoothed Particle Simulation is a method to extend these equations.
      The paper Particle-Based FluidSimulation for Interactive Applications presents an approach to approximate these interactions.
      We extend and attempt to replicate this project, while adding providing an interactive realtime demo for others to extend.
      </p></Row>
    <Row><h2>Motivations</h2></Row>
    <Row><p>There were previous semesters of CS184 that presented fluid simulation. These fluid simulations were usually 2D, which greatly reduced the complexity, and pre-rendered for their demonstration.
      However, for popular games and visual effects, these environment is 3D and close to realtime and not pre-rendered. This greatly increases the complexity and computational requirements to render.
      The project is our attempt at creating this realtime effect with a large number of 3D particles with user interactions.</p>
    </Row>
    <Row><h2>Technical approach</h2></Row>
    <Row><p>The project was implemented C++ and OpenGL for speed and performance.
       The sections below describe the technical approaches and advantages of our approaches</p></Row>
    <Row><h5>Initial Framework: Three.js</h5></Row>
    <Row><p>
      Three.js is a cross-browser library and API that can help developers create and display graphics using WebGL.
      It offers a lot of easy integrations into a scene such as options to specify where to place the camera, directional light, ambient light, etc. It also helps with applying shaders as
      they have several built-in including a Phong shader.

      Up to our project milestone, we had a working physics simulation for our particle system as well as a custom Blinn-Phong shader. During our test runs we noticed that performance was not addequate for realistic
      3d fluid simulation. For a realistic simulation, the goal is to have at least 2,000 particles at a consistent framerate. As a reference, the paper we read uses 5,000 particles.
      Our simulations ran at very low frame rates: 100 particles ran at 45-60 fps and 300 particles ran at 1-3 fps.

      We considered the value of offloading computational logic to Golang using web assembly but after testing with it, web assembly didn't seem to provide the easy integration we sought. Thus, we decided that it may be
      faster to start over using C++.
      </p></Row>
    <Row><h5>Current Framework: C++ and OpenGL</h5></Row>
    <Row><p>
      After implementing in three.js, we wanted something that would perform well. C++ allows for closer native performance and compiler optimizations that would allow us to write real time applications.
      Thus, we transitioned to implementing the algorthim and logic from scratch in C++ and OpenGL. 
      Transitioning to C++ had it's own learning curve in order to setup proper cross compilation and learning the OpenGL abstraction and interfaces. 
      For the cross compilation, we took inspiration from cs184 p4-clothsim, but we had to modify it to use the faster glm vector library, open mp for threading, etc. 
      The OpenGL abstractions were inspired from the tutorial series https://github.com/opengl-tutorials/ogl. 
      </p></Row>
    <Row><p>
      A good overview of framework is provided in the image below.
      </p></Row>
    <Row>    <img src={arch_diagram} className="arch_diagram" alt="Architecture diagram describing flow of logic"></img></Row>
    <Row><b>Rendering Loop and Logic</b></Row>
    <Row><p>
      The general rendering loop followed initializing the window, the shaders, the textures, and repeatedly drawing updates to the rendering logic in a loop. 
      <br></br>
      <br></br>
      To render the sphere, we used the stacks and slices approach to determine the vertices and normals as shown by https://www.songho.ca/opengl/gl_sphere.html. 
      This involes iterates through different angles at <Latex>{`$phi = (\\frac{stack_{iteration}}{stacks} - .5) * .5 * PI$`}</Latex> and different <Latex>{`$theta = 2 * PI * \\frac{slice_{iteration}}{slices}$`}</Latex>.
      Then, given those two angles we can use the spherical coordinates to derive the position of the vertices of the spheres. We compute the normals by normalizing these vector operations.
      By providing a single sphere with vertices to the shader, we can dramatically reduce the computation cost.
      <br></br>
      <br></br>
      In order to further reduce the computational cost, we use the idea of vertex indexing, which prevents repeated vertices from being sent to the shader when rendering the triangles that make up the sphere. 
      We also draw and instantiate all particle positions all at once using glDrawElementsInstanced, which sends the data to the shader in a single mesh instead of multiple meshes, which significantly boosts rendering performance. 
      For our implementation, we use 20 stacks and 20 slices.  
      With some manual openGL binding, we are able to send these to shader and also render the relevant values. 
      <br></br>
      <br></br>
      Another important aspect of rendering is using the correct projection matrix to view the data. In order to setup the camera and projection matrices, we took inspiration from proj4-clothsim management of the camera.
      <br></br>
      <br></br>
      One aspect of rendering that optimized our binding is to manually bind open gl buffer and vertex attributes such as normals. 
      In the proj4-clothsim code, nano-gui is used as an overhead layer for some of these operations, but we were able to use the optimal version of some of these functions in our code. 

    </p></Row>    
    <Row><h5>Simulation and Physics Logic</h5></Row>
    <Row>
    <p> The physics simulation was the most important and expensive part of rendering realistic the fluid. The simulation is based on Smoothed Particle Simulation. 
    </p>
    </Row>
    <Row><p> For each particle, we compute the density for a single particle via <Latex>{`$\\sum_j mass_j * W(r - r_j, h)$`}</Latex>
             where W is a weight kernel and r is the vector distance of particle. We discuss the different kernels later </p>
    </Row>
    <Row><p> Since the local density of each particle is different than the global density, we have to use a density function. 
    For each of the particles, we then compute the forces that are applied to it. The relevant forces are surface tension, pressure, viscosity, gravity, and other external forces.</p></Row>
    
    <Row><p>To compute viscosity, we use the formula  <Latex>{`$\\mu * \\sum_j * mass_j * \\frac{v_j - v_i}{density_j} * laplaceW(r_i - r_j, h)$`}</Latex> 
    </p></Row>
    <Row><p><Latex>{`To compute pressure, $-\\sum_j * mass_j * \\frac{(density_i + density_j)}{(2 * density_j)} * \\nabla W(r_i, r_j)$`}</Latex> </p></Row>
    <Row><p><Latex>{`To compute surface tension, we compute $\\sigma * k * n = \\sigma * \\delta^2 * c_s * \\frac{n}{|n|}$ where c_s is $\\sum_{j} mass_j * \\frac{1}{density_j} * W(r - r_j, h)$ and $k = \\sum_{j} mass_j * \\frac{1}{density_j} * \\nabla^2 W(r - r_j, h)$`}</Latex> 
    </p></Row>
    <Row><p> To compute gravity, we add the force <Latex>$(0, -9.8, 0)$</Latex></p></Row>
    
    <Row><p> The weight kernels we used were the weight kernel with the paramaters r, h which are distance and kernel paramater. 
    <Latex>{`$W_{poly6} = \\frac{315}{64 * pi * h^9} *((h^2 - r^2)^3 \\text{ if } 0 <= r <= h \\text{ else } 0)$`}</Latex> for everything except surface tension and viscosity. 
    Kernel used for surface tension <Latex>{`$W_{spiky} = \\frac{15}{pi * h^6} * ((h-r)^3 \\text{ if } 0 <= r <= h \\text{ else } 0)$`}</Latex>. 
    Kernel used for viscosity <Latex>{"$W_{viscosity} = \\frac{15}{2 * pi * h^3} * (-\\frac{r^3}{2*h^3} + \\frac{r^2}{h^2} + \\frac{h}{2*r} - 1 \\text{ if } 0 <= r <= h \\text{ else } 0)$"}</Latex>
    </p></Row>

    <Row><p> To create user interaction, we took the mouse position and the detla formed with the mouse movement. Then computed the inverse of the camera and projection matrices. 
    Then, we used cosine similairty to determinate which particles on the plane to apply the forces to. With a constant factor, we applied the force in the interaction. We apply the mouse force over multiple iterations.</p></Row>
    <Row><p> Another interesting force we added is the whirlpool effect. This involved adding the force towards the center via <Latex>{`$\\frac{mv^2}{r}$`}</Latex>.</p></Row>
    <Row><p>In order to update the position, we use an implicit euler approach and also use dampening when the water particles hit the walls</p></Row>
    <Row><h5>Optimizations</h5></Row>
    <Row><p
    >We spent a lot of time optimizing our code since we wanted to achieve a large number of particles and similiar results to the paper we are referencing. We first used the valgrind profiling to track areas of the c++ code that required the most optimization. 
    The physics simulation in our case was taking most of the time in serial fashion. We started by optimizing the code to take advantage of cache locality and using threads to run each particles computation in parallel. This provided a great speed up of 3-4x. 
    In order to visualize our performance improvements, we added a FPS counter as part of our rendering logic. The next optimization was to realize that the operation of checking the forces that depend on a particle interaction with every other particle was n^2.
    However, the particles that mattered and applied the most important forces were close together. We needed to find a quick way to compute the neighbors of the particles in 3d space to find which particles for each particle affected it the most. 
    We considered using OctoTrees and KDTrees, but we ended up using nanoflann since it had a fast open source implementation of finding neighbors. Even with the overhead of creating a new KDTree each loop, we were seeing another 3-4x improvement in the performance. 
    Another optimization we did was to hardcode the constants instead of passing them in at runtime. We noticed a large speedup in our code due to the compiler being able to optimize the results. We used the compiler -O3 flat to build a performant release version of the code that does smart loop unrolling and other optimizations.
    After applying these optimizations, we were able to get a realtime simulation of 4000 particles at around 12-15 fps with 40 vertices per sphere. These results were on a Intel i5-4690K with an Nvidia GTX 3060.
    </p>
    </Row>
    <Row><h5>Shaders</h5></Row>
    <Row><p>
      The implement the shaders we planned to apply a water texture to the surface. After adding some opengl bindings to pass the paramaters in, we passed the texture to the fagmeshader. In order to make the texture wrap around the sphere properly, we send proper uv coordinates to map the texture to the sphere. For each coordinate of the sphere we get the uv <Latex>{`$(\\frac{phi}{(2*PI)}, \\frac{theta}{PI} + .5)$`}</Latex>to prevent tearing of the texture. 
      We also wanted the water to be reflective since water is naturally reflective. We used a combination of the mirror shader w0 as camera_pos - vertex_pos. Using the normal, we can compute the reflection as <Latex>{`$w_i = w_0 - 2 *<w_0, n> * n$`}</Latex>. Then we just sample the texture at wi based on that result.

      Another interesting shader we rendered is the colorful shader that changes the color based on the camera angle. We instead first sample the texture then add the mirror direction wi to create the colorful shader. 
      </p></Row>
    <Row><h2>Results</h2></Row>
    <Row><p>Simulation Params that we tweaked to make it more realistic water for our simulation is listed below.
    </p>
    </Row>
    <Row><p>The following are some results that we received from our implementation. We ran at 4000 particles at 12-15fps.
     This is very close to the particle count of 5000 mentioned in the paper with a stable fps</p>
     </Row>
    <Row>
    <table className="data-table">
          <thead>
        <tr>
          <th> Name</th>
          <th> Value </th>
          <th> Description</th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td>h</td>
          <td>.5</td>
          <td>Used in the physics kernel paramaters to determine the properties of water</td>
        </tr>
        <tr>
          <td>surface tension sigma</td>
          <td>.00728</td>
          <td>The surface tension constant applied as a multiplier to surface tension force</td>
        </tr>
        <tr>
          <td>pressure rho zero</td>
          <td>80</td>
          <td>The default water pressure</td>
        </tr>
        <tr>
          <td>The viscosity mu</td>
          <td>1</td>
          <td>Paramater for how viscous the water seems</td>
        </tr>
        <tr>
          <td>Particle Initial Mass</td>
          <td>.0002</td>
          <td>The initial mass of each particle, which drastically affects the simulation</td>
        </tr>
         <tr>
          <td>KD Tree results to query </td>
          <td>1/60</td>
          <td>The number of nearest neighbor particles to query as an optimization</td>
        </tr>
        <tr>
          <td>delta_t per simulation timestamp </td>
          <td>1/60</td>
          <td>Paramter for how viscous the water seems</td>
        </tr>
        <tr>
          <td>Dampening</td>
          <td>.65</td>
          <td>The dampening when a particle hits the wall</td>
        </tr>
        <tr>
          <td>Num external force iterations</td>
          <td>5</td>
          <td>The number of iterations to apply the mouse force over.</td>
        </tr>
        </tbody>
      </table> 
    </Row>
    <Row>
      <ReactPlayer
        width="50%"
        controls={true}
        url="https://www.youtube.com/watch?v=-6NWf9phIio"/>
      <ReactPlayer
        width="50%"
        controls={true}
        url="https://www.youtube.com/watch?v=2OVFJwOe9lU"/>
    </Row>
    <br></br>
    <br></br>
    <Row>
    <ReactPlayer
      width="50%"
      controls={true}
      url="https://www.youtube.com/watch?v=ssmsc0edW8E"/>
    <ReactPlayer
      width="50%"
      controls={true}
      url="https://www.youtube.com/watch?v=Ve3xhCWj_q8"/>
    </Row>
</Container>
  </div>
  );
}

export default App;
