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
    <Navbar.Brand>3D Interactive Fluid Simulation Report</Navbar.Brand>
  
  </Navbar>
  <br />
  <Container>
    <Row><h2>Abstract</h2></Row>
    <Row><p>...</p></Row>
    <Row><h2>Technical approach</h2></Row>
    <Row><h5>Initial Framework: Three.js</h5></Row>
    <Row><h5>Current Framework: C++ and OpenGL</h5></Row>
    <Row><h5>Rendering Loop and Logic</h5></Row>
    Logic about VBO + creating Spheres
    <Row><h5>Simulation and Physics Logic</h5></Row>
    Include info about user interaction
    <Row><h5>Optimizations</h5></Row>
    <Row><h5>Shaders</h5></Row>
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
