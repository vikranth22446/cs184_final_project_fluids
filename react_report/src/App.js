import logo from './logo.svg';
import './App.css';
import Navbar from 'react-bootstrap/Navbar'
import Row from 'react-bootstrap/Row'
import Container from 'react-bootstrap/Container'
import Col from 'react-bootstrap/Col'
import React, { useState } from 'react';
import ReactPlayer from "react-player"

function App() {
  return (
    <div>
  <Navbar bg="dark" variant="dark" class="shadow p-3 mb-5 bg-body rounded">
    <Navbar.Brand><h2>3D Interactive Fluid Simulation Report</h2></Navbar.Brand>

  </Navbar>
  <Container>
  <Row><Navbar.Brand>
      <h4>Vikranth Srivatsa, Bhuvan Basireddy, Kevin Lo, Benjamin Cheung</h4>
    </Navbar.Brand></Row>
    <img src="logo.png" class="center"></img>
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
      After implementing in three.js, we wanted something that would perform well.
      We transitioned to implementing the algorthim and logic from scratch in C++ and OpenGL.

      </p></Row>
    <Row><h5>Rendering Loop and Logic</h5></Row>
    Logic about VBO + creating Spheres
    <Row><h5>Simulation and Physics Logic</h5></Row>
    Include info about user interaction
    <Row><h5>Optimizations</h5></Row>
    <Row><h5>Shaders</h5></Row>
    <Row><p>
      We applied the Mirror and regular shader implemented in project 4 with a slight variation being that we bind a water texture using uv coordinates. To do this, we create our own sphere mesh, add the necessary
      info for each vertex including details such as the normals for lighting, texture coordinates, vertex positions, etc. Then all this is sent to OpenGL for our particles (spheres) to be rendered.
      </p></Row>
    <Row><h2>Results</h2></Row>
    <Row><ReactPlayer
      url="https://youtu.be/-6NWf9phIio"
    />
     <ReactPlayer
      url="https://youtu.be/ssmsc0edW8E"
    /></Row>
     <Row><ReactPlayer
      url="https://youtu.be/2OVFJwOe9lU"
    /><ReactPlayer
      url="https://www.youtube.com/watch?v=Ve3xhCWj_q8"
    /></Row>
</Container>
  </div>
  );
}

export default App;
