const $ = (id) => document.getElementById(id);

async function api(path, options = {}) {
  const response = await fetch(path, options);
  if (!response.ok) throw new Error(await response.text());
  return response.json();
}

function populateRouters(routers) {
  for (const id of ["source", "destination"]) {
    const select = $(id);
    select.innerHTML = routers.map(r => `<option value="${r.id}">${r.id}</option>`).join("");
  }
  if (routers.length > 1) $("destination").selectedIndex = 1;
}

function renderTopology(state) {
  $("topology").innerHTML = state.routers.map(r =>
    `<div class="node"><strong>${r.id}</strong><span>${r.ip}</span></div>`
  ).join("");
}

function renderLinks(state) {
  $("linkCount").textContent = `${state.links.length} links`;
  $("links").innerHTML = state.links.map(link => {
    const action = link.active ? "fail" : "recover";
    const label = link.active ? "Fail link" : "Recover";
    return `
      <div class="link ${link.active ? "" : "failed"}">
        <div>
          <div class="link-name">${link.id}: ${link.source} ↔ ${link.destination}</div>
          <div class="link-meta">${link.latency} ms · ${link.bandwidth} Mbps · loss ${link.packetLoss}% · ${link.active ? "ACTIVE" : "FAILED"}</div>
        </div>
        <button onclick="changeLink('${link.id}', '${action}')">${label}</button>
      </div>`;
  }).join("");
}

async function loadState() {
  const state = await api("/api/state");
  populateRouters(state.routers);
  renderTopology(state);
  renderLinks(state);
}

async function findRoute() {
  const source = $("source").value;
  const destination = $("destination").value;
  const route = await api(`/api/route?source=${encodeURIComponent(source)}&destination=${encodeURIComponent(destination)}`);

  if (!route.reachable) {
    $("routePath").textContent = "No route available";
    for (const id of ["latency", "bandwidth", "cost", "packetLoss"]) $(id).textContent = "—";
    return;
  }

  $("routePath").textContent = route.routers.join(" → ");
  $("latency").textContent = `${route.latency} ms`;
  $("bandwidth").textContent = `${route.bandwidth} Mbps`;
  $("cost").textContent = route.cost;
  $("packetLoss").textContent = `${route.packetLoss}%`;
}

async function changeLink(id, action) {
  await api(`/api/links/${id}/${action}`, { method: "POST" });
  await loadState();
  await findRoute();
}

$("routeBtn").addEventListener("click", findRoute);

loadState()
  .then(findRoute)
  .then(() => $("status").textContent = "C++ backend connected")
  .catch(error => {
    $("status").textContent = "Backend error";
    console.error(error);
  });
