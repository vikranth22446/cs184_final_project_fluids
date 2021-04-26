import * as THREE from 'three'
import ReactDOM from 'react-dom'
import React, { useRef, useMemo, useState, Suspense } from 'react'
import { Canvas, useFrame, useThree } from '@react-three/fiber'
import { Environment, ContactShadows } from '@react-three/drei'
import { EffectComposer, SSAO } from '@react-three/postprocessing'
import './styles.css'
function w_poly(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * pow(h, 9)) * pow((pow(h, 2) - pow(r, 2)), 3)
}

function w_poly_derivative(r, h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*Math.PI * pow(h, 9)) * 3 * pow((pow(h, 2) - pow(r, 2)), 2) * (-2 * r)
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
    const force_pressure = 0;
    const h = 1;
    for(other_particle in particles) {
        force_pressure -= other_particle.mass*1/(2*other_particle.density)*w_poly_derivative(dist(other_particle.pos, particle.pos), h)
    }

}

function dist(pos1, pos2) {
//   return Math.sqrt(pow((pos1.x - pos2.x), 2) + (pos1.y - pos2.y), 2) + (pos1.z - pos2.z), 2))
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
      const mass = .5
      temp.push({ t, factor, speed, pos:pos, mx: 0, my: 0 , force:force, mass:mass})
    }
    return temp
  }, [count])

  useFrame(({state, clock}) => {
    
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
