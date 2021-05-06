import * as THREE from 'three'
import ReactDOM from 'react-dom'
import React, { useRef, useMemo, useState, Suspense, useEffect } from 'react'
import { Canvas, useFrame, useThree,addTail } from '@react-three/fiber'
import StatsImpl from "stats.js"
import { Environment, ContactShadows } from '@react-three/drei'
import { EffectComposer, SSAO } from '@react-three/postprocessing'
import './styles.css'
import { Physics, useBox, useSphere, usePlane } from '@react-three/cannon'
import throttle from "lodash/throttle"
import { useAsBind } from "use-as-bind";

function w_poly(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * Math.pow(h, 9)) * Math.pow((Math.pow(h, 2) - Math.pow(r, 2)), 3); 
}

function w_poly_derivative6(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * Math.pow(h, 9)) * 3 * Math.pow((Math.pow(h, 2) - Math.pow(r, 2)), 2) * (-2 * r); 
}
function w_poly_laplacian(r, h) {
  if(r > h || r < 0 ) return 0;
  return 315/(64*Math.PI * Math.pow(h, 9)) * 3 * Math.pow((Math.pow(h, 2) - Math.pow(r, 2)), 2) * (-2) + 315/(64*Math.PI * Math.pow(h, 9)) * 6 * Math.pow((Math.pow(h, 2) - Math.pow(r, 2)), 1) * (-2 * r); 
}

function w_viscosity(r, h) {
  if (r > h || r < 0) return 0; 
  return 15/(2 * Math.PI * pow(h, 3)) * (-pow(r, 3)/(2 * pow(h, 3))) * (pow(r, 2)/pow(h,2)) * h/(2*r) - 1; 
}

function laplace_viscosity(r, h) {
  if (r > h || r < 0) return 0; 
  return 45/(Math.PI * Math.pow(h, 6)) * (h-r)
}

function w_spiky(r, h) {
    if (r > h || r < 0) return 0; 
    return 15/(Math.PI * pow(h, 6)) * pow((h - r), 3)
}
function w_spiky_derivative(r, h) {
    if (r > h || r < 0) return 0; 
    return 15/(Math.PI * Math.pow(h, 6)) * 3 * Math.pow((h - r), 2) * (-1)
}

function density_s(particle, particles) {
    // [x, y, z]
    const h = 1;
    var p = 0;
    for(let other_particle of particles) {
        const r = dist(other_particle.pos, particle.pos);
        p += other_particle.mass * w_poly(r, h);
    }    
    particle.density = p;
    return p;
}

function compute_pressure(particle, particles) {
    var f_p_x = 0.0;
    var f_p_y = 0.0;
    var f_p_z = 0.0;
    const k = 1; // speed of sound 
    const h = 1;
    const rho_zero = 1;
    const pi = k * (particle.density - rho_zero);

    // for pressure using w_spiky_derivative kernel
    for(let other_particle of particles) {
        const pj = k * (other_particle.density - rho_zero);
        const r = dist(other_particle.pos, particle.pos)

        const pressure = -other_particle.mass*(pi + pj)/(2*other_particle.density)*w_spiky_derivative(r, h)
        if(r != 0) {
          const n_x = (particle.pos.x - other_particle.pos.x)/r
          const n_y = (particle.pos.y - other_particle.pos.y)/r
          const n_z = (particle.pos.z - other_particle.pos.z)/r
          f_p_x += n_x * pressure
          f_p_y += n_y * pressure
          f_p_z += n_z * pressure
        }

    }

    return new THREE.Vector3(f_p_x, f_p_y, f_p_z)
}

function compute_viscosity(particle, particles) {
    var f_v_x = 0.0;
    var f_v_y = 0.0;
    var f_v_z = 0.0;
    const h = 1;
    const mu =  0.01; // viscosity of water at 20 degrees
    for(let other_particle of particles) {
        const r = dist(other_particle.pos, particle.pos)
        const viscosity_x = -other_particle.mass*(other_particle.vel.x - particle.vel.x)/other_particle.density *laplace_viscosity(r, h)
        const viscosity_y = -other_particle.mass*(other_particle.vel.y - particle.vel.y)/other_particle.density *laplace_viscosity(r, h)
        const viscosity_z = -other_particle.mass*(other_particle.vel.z - particle.vel.z)/other_particle.density *laplace_viscosity(r, h)
        f_v_x += viscosity_x
        f_v_y += viscosity_y
        f_v_z + viscosity_z
    }
    f_v_x *= mu;
    f_v_y *= mu;
    f_v_z *= mu;
    return new THREE.Vector3(f_v_x, f_v_y, f_v_z)
}
function compute_gravity(particle, particles) {
    const volume = 1;
    const mass = particle.density * volume;
    return new THREE.Vector3(0.0, -9.8 * mass, 0.0)
}
function compute_surface_tension(particle, particles) {
    var f_t_x = 0.0;
    var f_t_y = 0.0;
    var f_t_z = 0.0;

    var c_x = 0.0;
    var c_y = 0.0;
    var c_z = 0.0;

    var k = 0.0;

    // https://www.engineeringtoolbox.com/surface-tension-d_962.html
    const sigma = 0.0728 / 10; // signam surface tension of water N/m at 20 degrees
    const h = 1.0;
    for(let other_particle of particles) {
        const r = dist(particle.pos, other_particle.pos)
        const surface_tension = other_particle.mass * 1/(other_particle.density) * w_poly_derivative6(r, h)
        if(r != 0) {
          const n_x = (particle.pos.x - other_particle.pos.x)/r
          const n_y = (particle.pos.y - other_particle.pos.y)/r
          const n_z = (particle.pos.z - other_particle.pos.z)/r
          c_x += surface_tension * n_x;
          c_y += surface_tension * n_y;
          c_z += surface_tension * n_z;
          k +=  other_particle.mass * 1/(other_particle.density) * w_poly_laplacian(r, h)/r
      }
    }

    f_t_x = sigma * c_x * k;
    f_t_y = sigma * c_y * k;
    f_t_z = sigma * c_z * k;
    return new THREE.Vector3(f_t_x, f_t_y, f_t_z)
}

function vvadd(v1, v2) {
  return new THREE.Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z)
}
function vmuls(v1, s) {
  return new THREE.Vector3(v1.x * s, v1.y*s, v1.z*s)
}
function vvadd_multiple(vect) {
  return vect.reduce((acc, item, index) => vvadd(acc, item))
}

function dist(pos1, pos2) {
  return Math.sqrt(Math.pow((pos1.x - pos2.x), 2) + Math.pow((pos1.y - pos2.y), 2) + Math.pow((pos1.z - pos2.z), 2))
}

function Swarm({ count, ...props }) {
  const mesh = useRef()
  const [dummy] = useState(() => new THREE.Object3D())
  // const { loaded, instance, error } = useAsBind("./hello.wasm");

  var plane = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0);
  const { viewport } = useThree();
  const particles = useMemo(() => {
    const temp = []
    for (let i = 0; i < count; i++) {
      const t = Math.random() * 100

      const xFactor = 0 + (Math.random()-.5) * 4
      const yFactor = -1 + (Math.random()-.5) * 1
      const zFactor = 0 + (Math.random()-.5) * 1
      const pos = new THREE.Vector3(xFactor, yFactor, zFactor)
      const force = new THREE.Vector3(0, 0.0, 0);
      // Note using implicit euler over verlet because it is more physically accurate
      const vel = new THREE.Vector3(0.0,0)
      const mass = 1;
      temp.push({ t, pos:pos, force:force, mass:mass, vel: vel})
    }
    return temp
  }, [count])

  useFrame(({state, clock}) => {
    particles.forEach((particle, i) => {
        particle.density = density_s(particle, particles)
    });
    particles.forEach((particle, i) => {
        particle.pressure_force = compute_pressure(particle, particles)
        particle.viscosity_force = compute_viscosity(particle, particles)
        particle.gravity_force = compute_gravity(particle, particles)
        particle.surface_tension = compute_surface_tension(particle, particles)
      });


    particles.forEach((particle, i) => {
        let { t, factor, speed, pos, mass, vel } = particle
        particle.t = t + 1
        const particle_mass = 1 * particle.density
        // t = particle.t += speed / 2
        const delta_t = 1/60 * 20
        const net_force = vvadd_multiple([particle.pressure_force, particle.viscosity_force, particle.gravity_force, particle.surface_tension])
        const next_acc = vmuls(net_force, 1/particle_mass * delta_t * delta_t)
        var new_vel = vvadd(vel, vmuls(next_acc, delta_t))
        var new_pos = vvadd(pos, vmuls(new_vel, delta_t))
        
        // if it hits the bounding box, comes back with opposite velocity
        if(new_pos.y < -2) {
          new_pos.y = -2
          new_vel.y *= -1
        }
        if(new_pos.y > 2) {
          new_pos.y = 2
          new_vel.y *= -1
        }
        if(new_pos.x < -2) {
          new_pos.x = -2
          new_vel.x *= -1
        }
        if(new_pos.x > 2) {
          new_pos.x = 2
          new_vel.x *= -1
        }
        if(new_pos.z < -2) {
          new_pos.z = -2
          new_vel.z *= -1
        }
        if(new_pos.z > 2) {
          new_pos.z = 2
          new_vel.z *= -1
        }
        // Update the dummy object
        particle.pos = new_pos
        particle.vel = new_vel
        // handle hitting the bounds
       

        // temp = <x,y,z>
        dummy.position.set(new_pos.x, new_pos.y, new_pos.z);
        dummy.scale.set(1, 1, 1)
        dummy.updateMatrix()
        // 
        // And apply the matrix to the instanced item
        mesh.current.setMatrixAt(i, dummy.matrix)
      })
    mesh.current.instanceMatrix.needsUpdate = true
  })
  const uniforms = {
    k_a: { value: new THREE.Vector3(0.9, 0.5, 0.3) },
    k_d: { value: new THREE.Vector3(0.9, 0.5, 0.3) },
    k_s: { value: new THREE.Vector3(0.8, 0.8, 0.8) },
    I_a: {value: new THREE.Vector3(1.0, 1.0, 1.0) },
    LightIntensity: { value: new THREE.Vector4(0.5, 0.5, 0.5, 1.0) },
    LightPosition: { value: new THREE.Vector4(0.0, 2000.0, 0.0, 1.0) },
    p: {value: 100.0}
  };
  const vertexShader = `
  varying vec3 Normal;
  varying vec3 Position;
  void main() {
    Normal = normalize(normalMatrix * normal);
    Position = vec3(modelViewMatrix * vec4(position, 1.0));
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
  }
  `;
  const defaultAttributeValues = {
      'color': [ 1, 1, 1 ],
      'uv': [ 0, 0 ],
      'uv2': [ 0, 0 ]
  }
  const fragmentShader = `
  varying vec3 Normal;
  varying vec3 Position;
  uniform vec3 k_a;
  uniform vec3 k_s;
  uniform float p;
  uniform vec3 k_d;
  uniform vec3 I_a;
  uniform vec4 LightPosition;
  uniform vec3 LightIntensity;
  vec3 bp_shading() {
  vec3 n = normalize(Normal);
  vec3 s = normalize(vec3(LightPosition) - Position);
  vec3 v = normalize(vec3(-Position));
  vec3 h = normalize(s) + normalize(v);
    return LightIntensity * (k_a + k_s * pow(max(dot(h, n),0.0), p) + k_d * max(dot(s, Normal), 0.0 ));
}
void main() {
  gl_FragColor = vec4(bp_shading(), 1.0);
}
  `;
  return (
    <instancedMesh ref={mesh} args={[null, null, count]}>
        <sphereBufferGeometry args={[.1, 5, 5]}/>
        <meshStandardMaterial roughness={0} color="royalblue" />
        {/* <shaderMaterial
        name="material"
        uniforms={uniforms}
        vertexShader={vertexShader}
        fragmentShader={fragmentShader}
        defaultAttributeValues={defaultAttributeValues}
      /> */}
    </instancedMesh>
  )
}


function Stats() {
  const [stats] = useState(() => new StatsImpl())
  useEffect(() => {
    stats.showPanel(0)
    document.body.appendChild(stats.dom)
    return () => document.body.removeChild(stats.dom)
  }, [])
  return useFrame(state => {
    stats.begin()
    state.gl.render(state.scene, state.camera)
    stats.end()
  }, 1)
}

// A physical plane without visual representation
function Plane({ color, ...props }) {
  usePlane(() => ({ ...props }));
  return null;
}
function Borders() {
  const { viewport } = useThree();
  return (
    <>
      <Plane
        position={[0, -viewport.height / 2, 0]}
        rotation={[-Math.PI / 2, 0, 0]}
      />
      <Plane
        position={[-viewport.width / 2 - 1, 0, 0]}
        rotation={[0, Math.PI / 2, 0]}
      />
      <Plane
        position={[viewport.width / 2 + 1, 0, 0]}
        rotation={[0, -Math.PI / 2, 0]}
      />
      <Plane position={[0, 0, 0]} rotation={[0, 0, 0]} />
      <Plane position={[0, 0, 12]} rotation={[0, -Math.PI, 0]} />
    </>
  );
}

function RotatingBox() {
    const myMesh = React.useRef();
    const [active, setActive] = useState(false);
    useFrame(({clock}) => {
        const a = clock.getElapsedTime();
        myMesh.current.rotation.x = Math.cos(clock.getElapsedTime());
        myMesh.current.rotation.y = Math.sin(clock.getElapsedTime());
        if(myMesh.current.position.y < -4) {
            myMesh.current.sign_flip = true;
        }  else if(myMesh.current.position.y > 4) {
            myMesh.current.sign_flip = false;
        }

        if(myMesh.current.sign_flip) {
            myMesh.current.position.y += 0.01
        } else {
            myMesh.current.position.y -= 0.01
        }
    });  
    const uniforms = {
      k_a: { value: new THREE.Vector3(0.9, 0.5, 0.3) },
      k_d: { value: new THREE.Vector3(0.9, 0.5, 0.3) },
      k_s: { value: new THREE.Vector3(0.8, 0.8, 0.8) },
      I_a: {value: new THREE.Vector3(1.0, 1.0, 1.0) },
      LightIntensity: { value: new THREE.Vector4(0.5, 0.5, 0.5, 1.0) },
      LightPosition: { value: new THREE.Vector4(0.0, 2000.0, 0.0, 1.0) },
      p: {value: 100.0}
    };
    const vertexShader = `
    varying vec3 Normal;
    varying vec3 Position;
    void main() {
      Normal = normalize(normalMatrix * normal);
      Position = vec3(modelViewMatrix * vec4(position, 1.0));
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
    `;
    const defaultAttributeValues = {
      	'color': [ 1, 1, 1 ],
      	'uv': [ 0, 0 ],
      	'uv2': [ 0, 0 ]
    }
    const fragmentShader = `
    varying vec3 Normal;
    varying vec3 Position;
    uniform vec3 k_a;
    uniform vec3 k_s;
    uniform float p;
    uniform vec3 k_d;
    uniform vec3 I_a;
    uniform vec4 LightPosition;
    uniform vec3 LightIntensity;
    vec3 bp_shading() {
    vec3 n = normalize(Normal);
    vec3 s = normalize(vec3(LightPosition) - Position);
    vec3 v = normalize(vec3(-Position));
    vec3 h = normalize(s) + normalize(v);
      return LightIntensity * (k_a + k_s * pow(max(dot(h, n),0.0), p) + k_d * max(dot(s, Normal), 0.0 ));
  }
  void main() {
    gl_FragColor = vec4(bp_shading(), 1.0);
  }
    `;
    return <mesh
      scale={active ? 1.5 : 1}
      onClick={() => setActive(!active)}
      ref={myMesh}
    >
      <sphereBufferGeometry />
      {/* <meshPhongMaterial color="royalblue" /> */}
      <shaderMaterial
        name="material"
        uniforms={uniforms}
        vertexShader={vertexShader}
        fragmentShader={fragmentShader}
        defaultAttributeValues={defaultAttributeValues}
      />
    </mesh>
}

function App() {
  return (
    <>
      <Canvas >
        <color attach="background" args={['#f0f0f0']} powerPreference={"high-performance"}/>
        <ambientLight intensity={0.5} />

        <directionalLight/>
        {/* <Swarm count={5} position={[0, 10, 0]} /> */}
        {/* <ContactShadows rotation={[Math.PI / 2, 0, 0]} position={[0, -30, 0]} opacity={0.4} width={100} height={100} blur={1} far={40} /> */}
        {/* <EffectComposer multisampling={0}>
          <SSAO samples={31} radius={10} intensity={20} luminanceInfluence={0.1} color="red" />
        </EffectComposer> */}
        {/* <Suspense fallback={null}>
          <Environment preset="city" /> 
        </Suspense>  */}
        <Physics>
          <Borders></Borders>
          <Swarm count={300}/>
        </Physics>
        <Stats />
        {/* <RotatingBox></RotatingBox> */}
        {/* <Swarm /> */}
      </Canvas>
    </>
  )
}

ReactDOM.render(<App />, document.getElementById('root'))
