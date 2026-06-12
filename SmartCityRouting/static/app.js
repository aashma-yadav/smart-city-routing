function show(text) {
  const o = document.getElementById("output");
  o.textContent = text;
}
function call(path) {
  fetch(path).then(r=>r.json()).then(j=>{
    show(j.output || JSON.stringify(j,null,2));
  }).catch(e=>show("Error: "+e));
}
function post(path, body) {
  fetch(path, {method:"POST", headers: {'Content-Type':'application/json'}, body: JSON.stringify(body||{})})
    .then(r=>r.json()).then(j=>show(j.output || JSON.stringify(j,null,2)))
    .catch(e=>show("Error: "+e));
}
function moveBus() {
  const busId = document.getElementById("busId").value;
  if (!busId) return alert("Enter bus id");
  post('/move_bus', {busId: busId});
}
function etaBetween() {
  const a = document.getElementById("src").value;
  const b = document.getElementById("dst").value;
  if (!a||!b) return alert("Enter src and dst");
  post('/eta_between', {src: a, dst: b});
}
function etaForBus() {
  const bid = document.getElementById("ebus").value;
  const target = document.getElementById("etarget").value;
  if (!bid||!target) return alert("Enter bus and target");
  post('/eta_for_bus', {busId: bid, target: target});
}
function shortestPath() {
  const a = document.getElementById("sp_src").value;
  const b = document.getElementById("sp_dst").value;
  if (!a||!b) return alert("Enter src and dst");
  post('/shortest_path', {src: a, dst: b});
}
function suggest() {
  const p = document.getElementById("pref").value;
  fetch('/suggest?prefix='+encodeURIComponent(p)).then(r=>r.json()).then(j=>show(j.output));
}

