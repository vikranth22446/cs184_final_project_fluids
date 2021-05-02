(this["webpackJsonpcs184-final-proj"]=this["webpackJsonpcs184-final-proj"]||[]).push([[0],{25:function(t,e,o){"use strict";o.r(e);var n=o(1),c=o(4),r=o(7),s=o(5),a=o(2),i=o(20),h=o.n(i),u=o(0),p=o(6),f=o(21),j=o.n(f),l=(o(29),o(17)),d=(o(30),o(3));function b(t,e){return t>e||t<0?0:315/(64*Math.PI*Math.pow(e,9))*Math.pow(Math.pow(e,2)-Math.pow(t,2),3)}function w(t,e){return t>e||t<0?0:315/(64*Math.PI*Math.pow(e,9))*3*Math.pow(Math.pow(e,2)-Math.pow(t,2),2)*(-2*t)}function y(t,e){return t>e||t<0?0:315/(64*Math.PI*Math.pow(e,9))*3*Math.pow(Math.pow(e,2)-Math.pow(t,2),2)*-2+315/(64*Math.PI*Math.pow(e,9))*6*Math.pow(Math.pow(e,2)-Math.pow(t,2),1)*(-2*t)}function M(t,e){return t>e||t<0?0:45/(Math.PI*Math.pow(e,6))*(e-t)}function x(t,e){return t>e||t<0?0:15/(Math.PI*Math.pow(e,6))*3*Math.pow(e-t,2)*-1}function O(t,e){return new a.Vector3(t.x+e.x,t.y+e.y,t.z+e.z)}function m(t,e){return new a.Vector3(t.x*e,t.y*e,t.z*e)}function v(t,e){return Math.sqrt(Math.pow(t.x-e.x,2)+Math.pow(t.y-e.y,2)+Math.pow(t.z-e.z,2))}function g(t){let e=t.count;Object(r.a)(t,["count"]);const o=Object(u.useRef)(),n=Object(u.useState)((()=>new a.Object3D)),i=Object(c.a)(n,1)[0];new a.Plane(new a.Vector3(0,1,0),0);const h=Object(p.c)().viewport,f=Object(u.useMemo)((()=>{const t=[];for(let o=0;o<e;o++){const e=100*Math.random(),o=0+4*(Math.random()-.5),n=1*(Math.random()-.5)-1,c=0+1*(Math.random()-.5),r=new a.Vector3(o,n,c),s=new a.Vector3(0,0,0),i=new a.Vector3(0,0),h=.01;t.push({t:e,pos:r,force:s,mass:h,vel:i})}return t}),[e]);Object(p.b)((({state:t,clock:e})=>{f.forEach(((t,e)=>{t.density=function(t,e){var o,n=0,c=Object(s.a)(e);try{for(c.s();!(o=c.n()).done;){let e=o.value;const c=v(e.pos,t.pos);n+=e.mass*b(c,1)}}catch(r){c.e(r)}finally{c.f()}return t.density=n,n}(t,f)})),f.forEach(((t,e)=>{t.pressure_force=function(t,e){var o=0,n=0,c=0;const r=1*(t.density-1);var i,h=Object(s.a)(e);try{for(h.s();!(i=h.n()).done;){let e=i.value;const s=1*(e.density-1),a=v(e.pos,t.pos),h=-e.mass*(r+s)/(2*e.density)*x(a,1);if(0!=a){const r=(t.pos.x-e.pos.x)/a,s=(t.pos.y-e.pos.y)/a,i=(t.pos.z-e.pos.z)/a;o+=r*h,n+=s*h,c+=i*h}}}catch(u){h.e(u)}finally{h.f()}return new a.Vector3(o,n,c)}(t,f)})),f.forEach(((t,e)=>{t.viscosity_force=function(t,e){var o=0,n=0,c=0;const r=.01;var i,h=Object(s.a)(e);try{for(h.s();!(i=h.n()).done;){let e=i.value;const c=v(e.pos,t.pos),r=-e.mass*(e.vel.x-t.vel.x)/e.density*M(c,1),s=-e.mass*(e.vel.y-t.vel.y)/e.density*M(c,1);e.mass,e.vel.z,t.vel.z,e.density,M(c,1),o+=r,n+=s}}catch(u){h.e(u)}finally{h.f()}return o*=r,n*=r,c*=r,new a.Vector3(o,n,c)}(t,f)})),f.forEach(((t,e)=>{t.gravity_force=function(t,e){const o=1*t.density;return new a.Vector3(0,-9.8*o,0)}(t)})),f.forEach(((t,e)=>{t.surface_tension=function(t,e){var o,n,c,r=0,i=0,h=0,u=0;const p=.00728;var f,j=Object(s.a)(e);try{for(j.s();!(f=j.n()).done;){let e=f.value;const o=v(t.pos,e.pos),n=1*e.mass/e.density*w(o,1);if(0!=o){const c=(t.pos.x-e.pos.x)/o,s=(t.pos.y-e.pos.y)/o,a=(t.pos.z-e.pos.z)/o;r+=n*c,i+=n*s,h+=n*a,u+=1*e.mass/e.density*y(o,1)/o}}}catch(l){j.e(l)}finally{j.f()}return o=p*r*u,n=p*i*u,c=p*h*u,new a.Vector3(o,n,c)}(t,f)})),f.forEach(((t,e)=>{let n=t.t,c=(t.factor,t.speed,t.pos),r=(t.mass,t.vel);t.t=n+1;const s=1*t.density,a=1/60*8;const u=m([t.pressure_force,t.viscosity_force,t.gravity_force,t.surface_tension].reduce(((t,e,o)=>O(t,e))),1/s*a*a);var p=O(r,m(u,a)),f=O(c,m(p,a));f.y<-h.height/2+1&&(f.y=-h.height/2+1,p.y*=-1),f.y>2&&(f.y=2,p.y*=-1),f.x<-2&&(f.x=-2,p.x*=-1),f.x>2&&(f.x=2,p.x*=-1),f.z<-2&&(f.z=-2,p.z*=-1),f.z>2&&(f.z=2,p.z*=-1),t.pos=f,t.vel=p,i.position.set(f.x,f.y,f.z),i.scale.set(1,1,1),i.updateMatrix(),o.current.setMatrixAt(e,i.matrix)})),o.current.instanceMatrix.needsUpdate=!0}));new a.Vector3(.9,.5,.3),new a.Vector3(.9,.5,.3),new a.Vector3(.8,.8,.8),new a.Vector3(1,1,1),new a.Vector4(.5,.5,.5,1),new a.Vector4(0,2e3,0,1);return Object(d.jsxs)("instancedMesh",{ref:o,args:[null,null,e],children:[Object(d.jsx)("sphereBufferGeometry",{args:[.1,5,5]}),Object(d.jsx)("meshStandardMaterial",{roughness:0,color:"royalblue"})]})}function z(){const t=Object(u.useState)((()=>new j.a)),e=Object(c.a)(t,1)[0];return Object(u.useEffect)((()=>(e.showPanel(0),document.body.appendChild(e.dom),()=>document.body.removeChild(e.dom))),[]),Object(p.b)((t=>{e.begin(),t.gl.render(t.scene,t.camera),e.end()}),1)}function V(t){t.color;let e=Object(r.a)(t,["color"]);return Object(l.b)((()=>Object(n.a)({},e))),null}function P(){const t=Object(p.c)().viewport;return Object(d.jsxs)(d.Fragment,{children:[Object(d.jsx)(V,{position:[0,-t.height/2,0],rotation:[-Math.PI/2,0,0]}),Object(d.jsx)(V,{position:[-t.width/2-1,0,0],rotation:[0,Math.PI/2,0]}),Object(d.jsx)(V,{position:[t.width/2+1,0,0],rotation:[0,-Math.PI/2,0]}),Object(d.jsx)(V,{position:[0,0,0],rotation:[0,0,0]}),Object(d.jsx)(V,{position:[0,0,12],rotation:[0,-Math.PI,0]})]})}function I(){return Object(d.jsx)(d.Fragment,{children:Object(d.jsxs)(p.a,{children:[Object(d.jsx)("color",{attach:"background",args:["#f0f0f0"],powerPreference:"high-performance"}),Object(d.jsx)("ambientLight",{intensity:.5}),Object(d.jsx)("directionalLight",{}),Object(d.jsxs)(l.a,{children:[Object(d.jsx)(P,{}),Object(d.jsx)(g,{count:1e3})]}),Object(d.jsx)(z,{})]})})}h.a.render(Object(d.jsx)(I,{}),document.getElementById("root"))},29:function(t,e,o){}},[[25,1,2]]]);
//# sourceMappingURL=main.e23dbe8b.chunk.js.map