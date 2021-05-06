function RotatingBox() {
    const myMesh = React.useRef();
    const [active, setActive] = useState(false);
    useFrame(({clock}) => {
        const a = clock.getElapsedTime();
        myMesh.current.rotation.x = Math.cos(clock.getElapsedTime());
        myMesh.current.rotation.y = Math.sin(clock.getElapsedTime());
    });

    return <mesh
      scale={active ? 1.5 : 1}
      onClick={() => setActive(!active)}
      ref={myMesh}
    >
      <boxBufferGeometry />
      <meshPhongMaterial color="royalblue" />
    </mesh>
}
