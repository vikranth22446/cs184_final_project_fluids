import * as THREE from 'three'
import ReactDOM from 'react-dom'
import React, { useRef, useMemo, useState, Suspense } from 'react'
import { Canvas, useFrame, useThree } from '@react-three/fiber'
import { Environment, ContactShadows } from '@react-three/drei'
import { EffectComposer, SSAO } from '@react-three/postprocessing'
import './styles.css'

function w_poly6(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * pow(h, 9)) * pow((pow(h, 2) - pow(r, 2)), 3); 
}

function w_poly_derivative6(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * pow(h, 9)) * 3 * pow((pow(h, 2) - pow(r, 2)), 2) * (-2 * r); 
}

function w_viscosity(r, h) {
  if (r > h || r < 0) return 0; 
  return 15/(2 * Math.PI * pow(h, 3)) * (-pow(r, 3)/(2 * pow(h, 3))) * (pow(r, 2)/pow(h,2)) * h/(2*r) - 1; 
}

function laplace_viscosity(r, h) {
  if (r > h || r < 0) return 0; 
  return 45/(Math.PI * pow(h, 6)) * (h-r)
}

function w_spiky(r, h) {
    if (r > h || r < 0) return 0; 
    return 15/(Math.PI * pow(h, 6)) * pow((h - r), 3)
}
function w_spiky_derivative(r, h) {
    if (r > h || r < 0) return 0; 
    return 15/(Math.PI * pow(h, 6)) * 3 * pow((h - r), 2) * (-1)
}

function density_s(particle, particles) {
    // [x, y, z]
    const h = 1;
    var p = 0;
    for(other_particle in particles) {
        const delta_r = dist(other_particle.pos, particle.pos);
        p += other_particle.mass * w_poly(delta_r, h);
    }    
    particle.density = p;
    return p;
}

function compute_pressure(particle, particles) {
    const f_p_x = 0.0;
    const f_p_y = 0.0;
    const f_p_z = 0.0;
    const k = 1; // speed of sound 
    const h = 1;
    const rho_zero = 1;
    const pi = k * (particle.density - rho_zero);

    for(other_particle in particles) {
        const pj = k * (other_particle.density - rho_zero);
        const pressure = -other_particle.mass*(pi + pj)/(2*other_particle.density)*w_poly_derivative(dist(other_particle.pos, particle.pos), h)
        const delta_x = particle.x - other_particle.x;
        const delta_y = particle.y - other_particle.y;
        const delta_z = particle.z - other_particle.z;

        f_p_x += delta_x * pressure
        f_p_y += delta_y * pressure
        f_p_z += delta_z * pressure
    }
    return THREE.Vector3(f_p_x, f_p_y, f_p_z)
}
function compute_viscosity(particle, particles) {
    const f_v_x = 0.0;
    const f_v_y = 0.0;
    const f_v_z = 0.0;

    for(other_particle in particles) {
        const viscosity_x = -other_particle.mass*(other_particle.vel.x - particle.vel.x)/other_particle.density *laplace_viscosity(dist(other_particle.pos, particle.pos), h)
        const viscosity_y = -other_particle.mass*(other_particle.vel.y - particle.vel.y)/other_particle.density *laplace_viscosity(dist(other_particle.pos, particle.pos), h)
        const viscosity_z = -other_particle.mass*(other_particle.vel.z - particle.vel.z)/other_particle.density *laplace_viscosity(dist(other_particle.pos, particle.pos), h)
        f_v_x += viscosity_x
        f_v_y += viscosity_y
        f_v_z + viscosity_z
    }
    return THREE.Vector3(f_v_x, f_v_y, f_v_z)
}
function compute_gravity(particle, particles) {
    const volume = 1;
    const mass = particle.density * volume;
    return THREE.Vector3(0.0, -9.8 * mass, 0.0)
}
function compute_surface_tension(particle, particles) {
    const f_t_x = 0.0;
    const f_t_y = 0.0;
    const f_t_z = 0.0;

    const c_x = 0.0;
    const c_y = 0.0;
    const c_z = 0.0;

    const k = 0.0;
    const sigma = 1.0;
    const h = 1.0;
    for(other_particle in particles) {
        const surface_tension = other_particle.mass * 1/(other_particle.density) * w_poly_derivative6(particle, other_particle, h)
        const normal_dist = dist(particle, other_particle)
        const n_x = (particle.pos.x - other_particle.pos.x)/normal_dist
        const n_y = (particle.pos.y - other_particle.pos.y)/normal_dist
        const n_z = (particle.pos.z - other_particle.pos.z)/normal_dist
        c_x += surface_tension * n_x;
        c_y += surface_tension * n_y;
        c_z += surface_tension * n_z;

        k +=  other_particle.mass * 1/(other_particle.density) * laplace_viscosity(particle, other_particle, h)
    }
    f_t_x = sigma * c_x * k;
    f_t_y = sigma * c_y * k;
    f_t_z = sigma * c_z * k;
    return THREE.Vector3(f_t_x, f_t_y, f_t_z)
}


function dist(pos1, pos2) {
  return Math.sqrt(pow((pos1.x - pos2.x), 2) + pow((pos1.y - pos2.y), 2) + pow((pos1.z - pos2.z), 2))
}

function Swarm({ count, ...props }) {
  const mesh = useRef()
  const [dummy] = useState(() => new THREE.Object3D())
  var plane = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0);
  const viewport = useThree((state) => state.viewport) 
  const particles = useMemo(() => {
    const temp = []
    for (let i = 0; i < count; i++) {
      const t = Math.random() * 100
      const factor = 20 + Math.random() * 100
      const speed = 0.01 + (Math.random()-.5) / 200

      const xFactor = 0 + (Math.random()-.5) * 8
      const yFactor = 0 + (Math.random()-.5) * 2
      const zFactor = 0 + (Math.random()-.5) * 2
      const force = new THREE.Vector3(0, -9.8, 0);
      const pos = new THREE.Vector3(xFactor, yFactor, zFactor)
      const vel = new THREE.Vector3(0.0, 0.0, 0.0)
      const mass = .5
      temp.push({ t, factor, speed, pos:pos, mx: 0, my: 0 , force:force, mass:mass, vel:vel})
    }
    return temp
  }, [count])

  useFrame(({state, clock}) => {
    particles.forEach((particle, i) => {
        particle.density = density_s(particle, particles)
    });
    particles.forEach((particle, i) => {
        particle.pressure_force = compute_pressure(particle, particles)
    });
    
    particles.forEach((particle, i) => {
        let { t, factor, speed, pos, force, mass } = particle
        particle.t = t + 1
        // There is no sense or reason to any of this, just messing around with trigonometric functions
        // t = particle.t += speed / 2
        // const a = Math.cos(t) + Math.sin(t * 1) / 10
        // const b = Math.sin(t) + Math.cos(t * 2) / 10
        // // const s = 1
        // if(pos.y < -4) {
        //     particle.sign_flip = true;
        // }  else if(pos.y > 4) {
        //     particle.sign_flip = false;
        // }

        // if(particle.sign_flip) {
        //     pos.y += 0.01
        // } else {
        //     pos.y -= 0.01
        // }

        const delta_t = 1/60
        // dummy.current.rotation.x = Math.cos(clock.getElapsedTime());
        // dummy.current.rotation.y = Math.sin(clock.getElapsedTime());
        // particle.mx += ( - particle.mx) * 0.01
        // particle.my += (- particle.my) * 0.01
        // Update the dummy object
        var new_x = pos.x + force.x/mass*delta_t*delta_t;
        var new_y =  pos.y + force.y/mass*delta_t*delta_t;
        var new_z =  pos.z + force.z/mass*delta_t*delta_t;

        if(new_y < -2.5) {
            new_y = pos.y
        }        

        // temp = <x,y,z>
        dummy.position.set(new_x, new_y, new_z);
        particle.pos.x = new_x;
        particle.pos.y = new_y;
        particle.pos.z = new_z;

        // dummy.setRotationFromAxisAngle(THREE.Vector3(1, 0, 0), clock.getElapsedTime())
        // dummy.position.set(
        //   (particle.mx / 10) * a + xFactor + Math.cos((t / 10) * factor) + (Math.sin(t * 1) * factor) / 10,
        //   (particle.my / 10) * b + yFactor + Math.sin((t / 10) * factor) + (Math.cos(t * 2) * factor) / 10,
        //   (particle.my / 10) * b + zFactor + Math.cos((t / 10) * factor) + (Math.sin(t * 3) * factor) / 10
        // )
        // dummy.position.set(1, particle.yFactor, particle.zFactor)
        dummy.scale.set(1, 1, 1)
        // dummy.rotation.set(s * 5, s * 5, s * 5)
        dummy.updateMatrix()
        // 
        // And apply the matrix to the instanced item
        mesh.current.setMatrixAt(i, dummy.matrix)
      })
    mesh.current.instanceMatrix.needsUpdate = true
  })

  return (
    <instancedMesh ref={mesh} args={[null, null, count]}>
        <sphereBufferGeometry args={[.1, 5, 5]}/>
        <meshStandardMaterial roughness={0} color="royalblue" />
    </instancedMesh>
  )
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
        console.log(myMesh.current.position.x, myMesh.current.position.y, myMesh.current.position.z)
    });

    return <mesh
      scale={active ? 1.5 : 1}
      onClick={() => setActive(!active)}
      ref={myMesh}
    >
      <sphereBufferGeometry />
      <meshPhongMaterial color="royalblue" />
    </mesh>
}

function App() {
  return (
    <>
      <Canvas >
        <color attach="background" args={['#f0f0f0']} />
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
        <Swarm count={10000}/>
        {/* <Swarm /> */}
      </Canvas>
    </>
  )
}

ReactDOM.render(<App />, document.getElementById('root'))
