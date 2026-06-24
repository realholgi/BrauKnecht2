#pragma once

// Shared stylesheet, served locally at /style.css (no external requests so it
// works on the BrauKnecht AP with no internet). Light glassmorphism look,
// matching the WetterStation web UI.
const char STYLE_CSS[] PROGMEM = R"=====(
:root{
  --bg-top:#f3f8fb; --bg-bottom:#dfeaf2;
  --card:rgba(255,255,255,.82); --card-strong:rgba(255,255,255,.94);
  --border:rgba(35,67,95,.12);
  --text:#173042; --muted:#6a7d8d;
  --accent:#0f7f9b; --accent-strong:#0f5f8a;
  --warm:#e17e39; --good:#258b5d; --warn:#cb4b42;
  --shadow:0 24px 60px rgba(16,43,66,.12);
  --radius:24px;
}
*{box-sizing:border-box}
html,body{margin:0;min-height:100%}
html{-webkit-text-size-adjust:100%}
body{
  font-family:"Avenir Next","Segoe UI","Helvetica Neue",Arial,sans-serif;
  color:var(--text);
  background:
    radial-gradient(circle at top left, rgba(15,127,155,.18), transparent 32%),
    radial-gradient(circle at top right, rgba(225,126,57,.16), transparent 28%),
    linear-gradient(180deg,var(--bg-top) 0%,var(--bg-bottom) 100%);
  background-attachment:fixed;
  padding:env(safe-area-inset-top,0) env(safe-area-inset-right,0) env(safe-area-inset-bottom,0) env(safe-area-inset-left,0);
}
.page{width:min(680px,calc(100% - 28px));margin:0 auto;padding:22px 0 34px}
.hero,.card{background:var(--card);border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow);backdrop-filter:blur(18px)}
.hero{position:relative;overflow:hidden;padding:24px 24px 20px;margin-bottom:18px}
.hero::after{content:"";position:absolute;inset:auto -40px -70px auto;width:220px;height:220px;border-radius:50%;
  background:radial-gradient(circle,rgba(15,127,155,.18),transparent 65%);pointer-events:none}
.hero-top{display:flex;justify-content:space-between;gap:18px;align-items:flex-start;margin-bottom:24px}
.eyebrow{margin:0 0 8px;color:var(--accent-strong);font-size:.78rem;font-weight:700;letter-spacing:.18em;text-transform:uppercase}
h1{margin:0;font-size:clamp(2rem,5vw,3rem);line-height:.95;letter-spacing:-.04em}
.hero-copy{margin:12px 0 0;max-width:34rem;color:var(--muted);font-size:.98rem;line-height:1.5}
.hero-side{display:flex;flex-direction:column;align-items:flex-end;gap:12px}
.live-pill{display:inline-flex;align-items:center;gap:10px;padding:10px 14px;border-radius:999px;
  border:1px solid rgba(15,95,138,.15);background:rgba(255,255,255,.7);color:var(--accent-strong);
  font-size:.9rem;font-weight:700;white-space:nowrap}
.live-dot{width:10px;height:10px;border-radius:50%;background:var(--good);box-shadow:0 0 0 6px rgba(37,139,93,.14);animation:pulse 2.2s infinite}
.hero-metrics{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}
.hero-stat{padding:14px 16px;border-radius:18px;background:var(--card-strong);border:1px solid rgba(35,67,95,.08)}
.hero-stat-label{display:block;margin-bottom:7px;color:var(--muted);font-size:.8rem;text-transform:uppercase;letter-spacing:.08em}
.hero-stat-value{font-size:clamp(1.2rem,3vw,1.8rem);font-weight:800;letter-spacing:-.03em;font-variant-numeric:tabular-nums}
.status-good{color:var(--good)} .status-warn{color:var(--warn)} .status-warm{color:var(--warm)} .status-neutral{color:var(--accent-strong)}
.alert{display:none;margin-bottom:18px;padding:13px 16px;border-radius:18px;border:1px solid rgba(203,75,66,.18);
  background:rgba(255,240,238,.88);color:var(--warn);font-weight:800;text-align:center;letter-spacing:.06em;
  box-shadow:0 12px 28px rgba(203,75,66,.08)}
.grid{display:grid;gap:18px}
.card{padding:20px;margin-bottom:18px}
.card h2{margin:0 0 12px;font-size:1rem;letter-spacing:.04em;text-transform:uppercase;color:var(--accent-strong)}
.sensor-head{display:flex;justify-content:space-between;align-items:center;gap:12px}
.sensor-title{margin:0;font-size:1rem;letter-spacing:.04em;text-transform:uppercase}
.sensor-badge{padding:7px 10px;border-radius:999px;background:rgba(15,127,155,.1);color:var(--accent-strong);font-size:.78rem;font-weight:700}
.sensor-temp{margin:18px 0 4px;font-size:clamp(2.4rem,6vw,4rem);font-weight:800;letter-spacing:-.05em;line-height:.92;font-variant-numeric:tabular-nums}
.sensor-temp-sub{margin:0;color:var(--muted);font-size:.92rem}
.rows{display:grid;gap:0;margin:14px 0 0;padding:0;list-style:none}
.row,.rows li{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:14px;align-items:center;padding:12px 0;border-top:1px solid rgba(35,67,95,.08)}
.row:first-child,.rows li:first-child{border-top:0;padding-top:0}
.row-label{color:var(--muted);font-size:.94rem}
.row-value{font-weight:700;font-variant-numeric:tabular-nums;text-align:right}
.tl{position:relative;margin:6px 0 4px;padding-left:22px}
.tl::before{content:"";position:absolute;left:6px;top:10px;bottom:10px;width:2px;background:rgba(15,127,155,.3)}
.tl-row{position:relative;display:flex;justify-content:space-between;align-items:center;gap:12px;padding:8px 0}
.tl-row::before{content:"";position:absolute;left:-21px;top:50%;transform:translateY(-50%);width:11px;height:11px;border-radius:50%;background:var(--accent);border:2px solid var(--card-strong);box-shadow:0 0 0 2px rgba(15,127,155,.25)}
.tl-node::before{background:var(--accent-strong)}
.tl-boil::before{background:var(--warm);box-shadow:0 0 0 2px rgba(225,126,57,.25)}
.tl-label{color:var(--text);font-weight:600}
.tl-badge{padding:4px 11px;border-radius:999px;background:rgba(15,127,155,.1);color:var(--accent-strong);font-size:.82rem;font-weight:700;font-variant-numeric:tabular-nums;white-space:nowrap}
.tl-hops{display:flex;flex-wrap:wrap;gap:8px;padding:6px 0 2px 22px}
.tl-hop{padding:4px 10px;border-radius:999px;background:rgba(225,126,57,.14);color:var(--warm);font-size:.8rem;font-weight:700;font-variant-numeric:tabular-nums}
.links{text-align:center;margin:8px 0 0;color:var(--muted)}
a{color:var(--accent-strong);font-weight:600}
label{display:block;font-size:.8rem;color:var(--muted);text-transform:uppercase;letter-spacing:.08em;margin:16px 0 6px}
input,select{width:100%;padding:12px 14px;font-size:1rem;color:var(--text);background:var(--card-strong);border:1px solid var(--border);border-radius:14px}
input[type=file]{padding:9px 12px}
.btn{display:block;width:100%;margin-top:22px;padding:14px;font-size:1rem;font-weight:800;letter-spacing:.02em;
  color:#f4fbff;background:linear-gradient(135deg,var(--accent),var(--accent-strong));border:0;border-radius:16px;cursor:pointer}
.fade{animation:fade-up .5s ease both}
@keyframes pulse{0%,100%{transform:scale(1);opacity:1}50%{transform:scale(1.08);opacity:.8}}
@keyframes fade-up{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}
@media (max-width:520px){
  .page{width:min(100%,calc(100% - 18px));padding:12px 0 22px}
  .hero,.card{border-radius:20px}
  .hero{padding:18px 16px 16px;margin-bottom:14px}
  .hero-top{flex-direction:column;align-items:flex-start;gap:14px;margin-bottom:18px}
  .hero-side{align-items:flex-start;width:100%}
  .live-pill{width:100%;justify-content:center}
  .card{padding:18px 16px}
  .sensor-temp{font-size:clamp(2.1rem,11vw,3rem)}
}
)=====";

const char PAGE_Kochen[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="mobile-web-app-capable" content="yes">
  <title>BrauKnecht</title>
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
<main class="page">
  <section class="hero fade">
    <div class="hero-top">
      <div>
        <p class="eyebrow">Lokal &middot; Live &middot; Sud</p>
        <h1 id="title">BrauKnecht</h1>
        <p class="hero-copy">Live-Status der Maische- und Kochsteuerung. Die Seite aktualisiert sich jede Sekunde.</p>
      </div>
      <div class="hero-side">
        <div class="live-pill"><span class="live-dot"></span><span id="conn">Aktualisiert</span></div>
      </div>
    </div>
    <div class="hero-metrics">
      <div class="hero-stat"><span class="hero-stat-label">Ist</span><span id="m_ist" class="hero-stat-value">--</span></div>
      <div class="hero-stat"><span class="hero-stat-label">Soll</span><span id="m_soll" class="hero-stat-value">--</span></div>
      <div class="hero-stat"><span class="hero-stat-label">Heizung</span><span id="m_heiz" class="hero-stat-value status-neutral">--</span></div>
    </div>
  </section>

  <div id="alarm" class="alert fade">RUFALARM</div>

  <section class="grid">
    <article class="card fade">
      <div class="sensor-head">
        <h2 class="sensor-title">Temperatur</h2>
        <span class="sensor-badge">Ist / Soll</span>
      </div>
      <div id="big_ist" class="sensor-temp">--</div>
      <p class="sensor-temp-sub">Soll <span id="big_soll">--</span></p>
      <div class="rows">
        <div class="row"><span class="row-label">Heizung</span><span id="r_heiz" class="row-value">--</span></div>
      </div>
    </article>

    <article id="detailCard" class="card fade" style="display:none">
      <div class="sensor-head"><h2 id="title2" class="sensor-title">Details</h2></div>
      <ul id="data" class="rows"></ul>
    </article>
  </section>

  <p class="links"><a href="/config">Einstellungen</a> &middot; <a href="/recipe">Rezept</a></p>
</main>

<script>
const $ = id => document.getElementById(id);
async function tick(){
  try{
    const d = await (await fetch('data.json', {cache:'no-store'})).json();
    $('title').textContent = d.title;

    const ist  = d.regelung == 0 ? '--' : Number(d.temp_ist).toFixed(1);
    const soll = Number(d.temp_soll).toFixed(0);
    $('m_ist').textContent  = ist + ' °C';
    $('m_soll').textContent = soll + ' °C';
    $('big_ist').textContent  = ist + ' °C';
    $('big_soll').textContent = soll + ' °C';

    const on = d.heizung === 'an';
    $('m_heiz').textContent = d.heizung;
    $('m_heiz').className = 'hero-stat-value ' + (on ? 'status-warm' : 'status-neutral');
    $('r_heiz').textContent = d.heizung;
    $('r_heiz').className = 'row-value ' + (on ? 'status-warm' : '');

    if (d.data && d.data.length > 0){
      $('title2').textContent = d.title2;
      $('data').innerHTML = d.data;
      $('detailCard').style.display = '';
    } else {
      $('detailCard').style.display = 'none';
    }

    const alarm = d.rufmodus == 28 || d.modus == 18;
    $('alarm').style.display = alarm ? 'block' : 'none';
    document.title = alarm ? '⚠ RUFALARM' : 'BrauKnecht';
    $('conn').textContent = 'Aktualisiert';
  } catch(e){
    $('conn').textContent = 'Keine Verbindung';
  } finally {
    setTimeout(tick, 1000);  // chain, not setInterval: ESP8266 serves one client at a time
  }
}
tick();
</script>
</body></html>
)=====";
