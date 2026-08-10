#pragma once

// Local stylesheet and app pages. No external assets are used so the UI works
// on the BrauKnecht access point without internet access.
const char STYLE_CSS[] PROGMEM = R"=====(
:root{
  --bg:#ffffff; --surface:#ffffff; --surface-2:#f8fafc;
  --border:#d8dee8; --text:#111827; --muted:#5f6673;
  --accent:#0b57d0; --accent-soft:#eef5ff; --heat:#ff6b00; --ok:#2f9e44; --warn:#b94038;
  --shadow:0 3px 12px rgba(17,24,39,.06); --radius:8px;
}
*{box-sizing:border-box}
html,body{margin:0;min-height:100%}
html{-webkit-text-size-adjust:100%}
body{
  font-family:"Avenir Next","Segoe UI","Helvetica Neue",Arial,sans-serif;
  color:var(--text);background:var(--bg);
  padding:env(safe-area-inset-top,0) env(safe-area-inset-right,0) calc(env(safe-area-inset-bottom,0) + 72px) env(safe-area-inset-left,0);
}
a{color:inherit}
.app{width:min(1180px,100%);margin:0 auto;padding:22px 14px}
.topbar{display:flex;align-items:center;margin:0 0 18px}
.brand{font-size:1.65rem;font-weight:500;letter-spacing:0}
.page-title{margin:0 0 18px;font-size:1.75rem;line-height:1.1;font-weight:500;letter-spacing:0}
.layout{display:block}
.side-nav,.bottom-nav{background:rgba(255,255,255,.96);border:1px solid var(--border)}
.side-nav{display:none}
.bottom-nav{position:fixed;left:0;right:0;bottom:0;z-index:10;display:grid;grid-template-columns:repeat(4,1fr);border-width:1px 0 0;border-radius:0;overflow:hidden;padding-bottom:env(safe-area-inset-bottom,0)}
.nav-link{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:4px;min-height:62px;text-decoration:none;color:#20242a;font-size:.72rem;font-weight:400}
.nav-link svg{width:20px;height:20px;stroke:currentColor;stroke-width:1.8;fill:none;stroke-linecap:round;stroke-linejoin:round}
.nav-link .nav-fill{fill:currentColor;stroke:none}
.nav-link.active{color:var(--text);background:var(--accent-soft)}
.nav-link.active svg{color:var(--accent)}
.nav-link.active span{color:var(--text)}
.grid{display:grid;gap:14px}
.grid.two{grid-template-columns:1fr}
.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow);padding:16px}
.card h1,.card h2{margin:0 0 14px;font-size:1.05rem;letter-spacing:0;color:var(--text)}
.hero-card{display:grid;gap:14px}
.hero-title{margin:0;font-size:1.7rem;line-height:1.1;letter-spacing:0;color:var(--text);font-weight:500}
.hero-copy{margin:0;color:var(--muted);line-height:1.45}
.metrics{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:0}
.metric{padding:12px;border-left:1px solid var(--border);background:transparent;text-align:center}
.metric:first-child{border-left:0}
.metric-label{display:block;margin-bottom:6px;color:var(--text);font-size:.86rem}
.metric-value{display:block;font-size:2rem;font-weight:500;font-variant-numeric:tabular-nums;overflow-wrap:anywhere}
.metric-unit{font-size:.9rem;font-weight:500}
.dashboard-grid{align-items:start}
.recipe-panel{min-height:420px}
.recipe-head{display:flex;justify-content:space-between;align-items:flex-start;gap:14px;margin-bottom:18px}
.kicker{display:block;margin-bottom:4px;color:var(--text);font-size:.9rem}
.recipe-title{margin:0;font-size:1.65rem;line-height:1.16;font-weight:500}
.icon-btn{display:grid;place-items:center;width:42px;height:42px;border:1px solid var(--border);border-radius:6px;color:#28313d;text-decoration:none}
.icon-btn svg{width:22px;height:22px;stroke:currentColor;stroke-width:2;fill:none;stroke-linecap:round;stroke-linejoin:round}
.recipe-steps{position:relative;display:grid;gap:0;padding:2px 0}
.recipe-step{position:relative;display:grid;grid-template-columns:42px minmax(0,1fr) auto;gap:12px;align-items:center;min-height:62px}
.recipe-step.active{margin:0 -8px;padding:0 8px;border-radius:var(--radius);background:var(--accent-soft)}
.recipe-step::before{content:"";position:absolute;left:20px;top:0;bottom:0;width:2px;background:#d8dee8}
.recipe-step:first-child::before{top:50%}
.recipe-step:last-child::before{bottom:50%}
.step-icon{position:relative;z-index:1;display:grid;place-items:center;width:42px;height:42px;border-radius:50%;border:2px solid var(--accent);background:#fff;color:var(--accent);font-weight:600;font-size:.9rem}
.step-icon.mash{background:#35a344;border-color:#35a344;color:#fff}
.step-icon.mashout{background:#ffc928;border-color:#ffc928;color:#fff}
.step-icon.boil{background:var(--heat);border-color:var(--heat);color:#fff}
.step-icon.hop{background:#6b7280;border-color:#6b7280;color:#fff}
.step-icon svg,.timeline-icon svg,.mini-icon svg,.heat-status svg{width:22px;height:22px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.step-icon.mash svg,.step-icon.mashout svg,.step-icon.boil svg,.step-icon.hop svg,.timeline-icon svg,.mini-icon svg{stroke-width:1.9}
.step-name{display:block;font-size:1.1rem;font-weight:500}
.step-sub{display:block;margin-top:2px;color:var(--text);font-size:.95rem}
.step-badge{padding:5px 8px;border:1px solid #b9d0ff;border-radius:6px;background:#eef5ff;color:var(--accent);font-weight:500;white-space:nowrap}
.step-badge.warm{border-color:#ffd5bd;background:#fff0e8;color:var(--heat)}
.step-badge.neutral{border-color:#d8dee8;background:#f7f8fa;color:var(--text)}
.side-stack{display:grid;gap:14px}
.live-status-card .metrics{margin-top:12px}
.live-status-card .metric-value{font-size:2.45rem}
.heat-status{display:inline-flex;flex-direction:column;align-items:center;gap:3px;color:var(--heat);font-weight:500}
.heat-status svg{width:38px;height:38px;stroke-width:1.8}
.heat-label{display:block;color:var(--heat);font-size:1rem}
.manual-control{text-align:center}
.manual-label{margin:4px 0 8px;color:var(--text)}
.manual-row{display:grid;grid-template-columns:56px minmax(0,1fr) 56px;gap:18px;align-items:center;margin:8px 0 16px}
.stepper{width:56px;height:56px;border:1px solid var(--border);border-radius:6px;background:#fff;color:var(--text);font-size:2rem;font-weight:500;line-height:1;cursor:pointer}
.manual-value{font-size:3rem;font-weight:500;font-variant-numeric:tabular-nums}
.manual-unit{font-size:1.1rem;font-weight:500}
.manual-card .btn{display:inline-flex;align-items:center;justify-content:center;gap:10px;width:100%;font-size:1rem}
.manual-card .btn svg{stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.status-ok{color:var(--ok)} .status-heat{color:var(--heat)} .status-warn{color:var(--warn)} .status-neutral{color:var(--accent)}
.alert{display:none;padding:12px;border:1px solid rgba(185,64,56,.25);border-radius:var(--radius);background:#fff0ee;color:var(--warn);font-weight:600;text-align:center}
.rows{display:grid;gap:0;margin:0;padding:0;list-style:none}
.row,.rows li{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:12px;align-items:center;padding:10px 0;border-top:1px solid var(--border)}
.row:first-child,.rows li:first-child{border-top:0}
.row-label{color:var(--muted)}
.row-value{font-weight:500;text-align:right;font-variant-numeric:tabular-nums}
label{display:block;margin:12px 0 6px;color:var(--text);font-size:.88rem;font-weight:500;letter-spacing:0}
input,select{width:100%;min-height:44px;padding:10px 12px;border:1px solid var(--border);border-radius:var(--radius);background:#fff;color:var(--text);font:inherit}
input[type=file]{padding:8px 10px}
.field-grid{display:grid;gap:10px}
.settings-form{display:grid;gap:14px}
.settings-actions{display:flex;justify-content:flex-start}
.section-title{display:flex;align-items:center;gap:10px;margin:0 0 14px;color:var(--text);font-size:1.2rem;font-weight:500}
.section-title svg{width:22px;height:22px;stroke:currentColor;stroke-width:2;fill:none;stroke-linecap:round;stroke-linejoin:round}
.inline-form{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:end}
.btn{min-height:44px;padding:0 22px;border:0;border-radius:6px;background:linear-gradient(180deg,#1d66df,#0b57d0);color:#fff;font-weight:600;cursor:pointer}
.btn.full{width:100%;margin-top:0}
.btn.outline{background:#fff;color:var(--accent);border:1px solid #8ab4f8}
.pill{display:inline-flex;align-items:center;min-height:26px;padding:0 9px;border-radius:999px;border:1px solid #9fd4aa;background:#effaf1;color:#1f7a35;font-size:.82rem;font-weight:500}
.chip-row{display:flex;flex-wrap:wrap;gap:8px;margin-top:14px}
.chip{display:inline-flex;align-items:center;gap:8px;min-height:38px;padding:0 12px;border:1px solid var(--border);border-radius:6px;background:#fff;font-size:.9rem}
.chip svg,.import-title svg{width:19px;height:19px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.recipe-summary{display:flex;justify-content:space-between;align-items:flex-start;gap:14px}
.recipe-summary h2{margin:0;color:var(--text);font-size:1.35rem;font-weight:500}
.recipe-timeline{position:relative;display:grid;gap:0}
.recipe-timeline::before{content:"";position:absolute;left:21px;top:22px;bottom:22px;width:2px;background:#d8dee8}
.timeline-row{position:relative;display:grid;grid-template-columns:44px minmax(0,1fr) 86px 74px;gap:14px;align-items:center;min-height:64px;border-top:1px solid #edf0f4}
.timeline-row:first-child{border-top:0}
.timeline-icon{position:relative;z-index:1;display:grid;place-items:center;width:44px;height:44px;border-radius:50%;border:1px solid #d8dee8;background:#fff;color:var(--accent);font-size:1.05rem;font-weight:500}
.timeline-icon.mash{background:#35a344;border-color:#35a344;color:#fff}
.timeline-icon.mashout{background:#ffc928;border-color:#ffc928;color:#fff}
.timeline-icon.boil,.timeline-icon.hop{color:var(--heat);border-color:#ffc59f;background:#fff7ed}
.timeline-icon.hop{color:#6b7280;border-color:#c8ced7;background:#f7f8fa}
.timeline-name{font-size:1.02rem;font-weight:500}
.timeline-temp,.timeline-time{text-align:right;font-variant-numeric:tabular-nums}
.import-title{display:flex;align-items:center;gap:12px;margin:0 0 12px;color:var(--text);font-size:1.25rem;font-weight:500}
.history-grid{align-items:start}
.history-main{display:grid;gap:14px}
.history-card-head{display:flex;align-items:flex-start;justify-content:space-between;gap:12px;margin-bottom:12px}
.history-card-head h2{margin:0;color:var(--text);font-size:1.45rem;font-weight:500}
.history-meta{display:flex;flex-wrap:wrap;gap:10px 24px;color:var(--muted);font-size:.86rem}
.chart-wrap{height:280px}
.chart-legend{display:flex;flex-wrap:wrap;gap:20px;margin-top:12px;color:var(--text);font-size:.86rem}
.legend-line{display:inline-block;width:28px;height:2px;margin-right:8px;vertical-align:middle;background:var(--accent)}
.legend-line.live{background:#259b24}
.legend-heat{display:inline-flex;align-items:center;gap:6px;color:var(--heat);font-weight:500}
.legend-heat svg{width:20px;height:20px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.chart-legend .legend-heat{font-size:.86rem}
.chart-legend .legend-heat svg{width:22px;height:22px;stroke-width:1.8}
.stat-strip{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:0}
.stat-strip .metric{padding:8px}
.stat-strip .metric-value{font-size:1.45rem}
.progress{height:7px;border-radius:999px;background:#e8ecf1;overflow:hidden}
.progress span{display:block;height:100%;width:45%;background:var(--accent);border-radius:inherit}
.mini-recipe{display:grid;gap:12px}
.mini-step{display:grid;grid-template-columns:30px minmax(0,1fr) auto auto;gap:10px;align-items:center;font-size:.9rem}
.mini-icon{display:grid;place-items:center;width:30px;height:30px;border-radius:50%;border:2px solid var(--accent);color:var(--accent);font-weight:500}
.mini-icon.mash{background:#45ad4b;border-color:#45ad4b;color:#fff}
.mini-icon.mashout{background:#ffc928;border-color:#ffc928;color:#fff}
.mini-icon.boil{background:var(--heat);border-color:var(--heat);color:#fff}
.mini-icon.hop{background:#6b7280;border-color:#6b7280;color:#fff}
.settings-info-card .rows{grid-template-columns:1fr}
canvas{width:100%;height:100%;display:block}
.small{color:var(--muted);font-size:.9rem;line-height:1.45}
@media (min-width:760px){
  body{padding-bottom:0}
  .app{padding:0}
  .topbar{display:none}
  .layout{display:grid;grid-template-columns:190px minmax(0,1fr);min-height:100vh;align-items:start}
  .layout>.grid{padding:42px 28px}
  .side-nav{position:sticky;top:0;display:grid;align-content:start;gap:8px;min-height:100vh;border-width:0 1px 0 0;border-radius:0;box-shadow:none;padding:98px 14px 0}
  .side-nav::before{content:"BrauKnecht";position:absolute;top:32px;left:20px;color:var(--text);font-size:1.5rem;font-weight:500}
  .side-nav .nav-link{flex-direction:row;justify-content:flex-start;min-height:52px;padding:0 14px;border-radius:6px;font-size:.95rem;font-weight:400}
  .bottom-nav{display:none}
  .grid.two{grid-template-columns:1.1fr .9fr}
  .dashboard-grid{grid-template-columns:minmax(0,1fr) 360px}
  .history-grid{grid-template-columns:minmax(0,1fr) 280px}
  .field-grid.two{grid-template-columns:1fr 1fr}
  .field-grid.three{grid-template-columns:1.2fr .45fr .7fr}
  .hero-card{grid-template-columns:minmax(0,1fr) 420px;align-items:end}
  .hero-title{font-size:2.2rem}
  .btn.full{width:auto;min-width:220px}
  .settings-info-card .rows{grid-template-columns:1fr 1fr;column-gap:36px}
  .settings-info-card .rows li:nth-child(2){border-top:0}
}
@media (max-width:520px){
  .timeline-row{grid-template-columns:44px minmax(0,1fr) auto;gap:10px}
  .timeline-time{grid-column:2 / 4;text-align:left;color:var(--muted);font-size:.9rem}
  .stat-strip{grid-template-columns:repeat(2,minmax(0,1fr))}
  .history-meta{display:grid;gap:4px}
}
)=====";

const char PAGE_Dashboard[] PROGMEM = R"=====(
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
<main class="app">
  <div class="topbar"><div class="brand">BrauKnecht</div></div>
  <div class="layout">
    <nav class="side-nav" aria-label="Navigation">
      <a class="nav-link active" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a>
      <a class="nav-link" href="/history"><svg viewBox="0 0 24 24"><path d="M4 19h16"/><path d="M5 15l5-5 4 3 5-7"/></svg><span>Verlauf</span></a>
      <a class="nav-link" href="/recipe"><svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h5"/><path d="M10 17h5"/></svg><span>Rezept</span></a>
      <a class="nav-link" href="/config"><svg viewBox="0 0 24 24"><path d="M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7Z"/><path d="M19.4 15a1.7 1.7 0 0 0 .34 1.88l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.7 1.7 0 0 0-1.88-.34 1.7 1.7 0 0 0-1.03 1.56V21a2 2 0 1 1-4 0v-.09A1.7 1.7 0 0 0 8.97 19.4a1.7 1.7 0 0 0-1.88.34l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.7 1.7 0 0 0 4.6 15 1.7 1.7 0 0 0 3.04 14H3a2 2 0 1 1 0-4h.09A1.7 1.7 0 0 0 4.6 8.97a1.7 1.7 0 0 0-.34-1.88l-.06-.06A2 2 0 1 1 7.03 4.2l.06.06A1.7 1.7 0 0 0 8.97 4.6 1.7 1.7 0 0 0 10 3.04V3a2 2 0 1 1 4 0v.09a1.7 1.7 0 0 0 1.03 1.51 1.7 1.7 0 0 0 1.88-.34l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.7 1.7 0 0 0-.34 1.88A1.7 1.7 0 0 0 20.96 10H21a2 2 0 1 1 0 4h-.09A1.7 1.7 0 0 0 19.4 15Z"/></svg><span>Einstellungen</span></a>
    </nav>
    <section class="grid">
      <div id="alarm" class="alert">RUFALARM</div>
      <div class="grid dashboard-grid">
        <article class="card recipe-panel">
          <header class="recipe-head">
            <div>
              <span class="kicker">Rezept</span>
              <h1 id="recipeTitle" class="recipe-title">--</h1>
            </div>
            <a class="icon-btn" href="/recipe" aria-label="Rezept">
              <svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h6"/><path d="M10 17h6"/></svg>
            </a>
          </header>
          <div id="recipeSteps" class="recipe-steps"></div>
        </article>
        <div class="side-stack">
          <article class="card live-status-card">
            <h2>Live-Status</h2>
            <div class="metrics">
              <div class="metric"><span class="metric-label">Ist</span><span id="m_ist" class="metric-value">--</span></div>
              <div class="metric"><span class="metric-label">Soll</span><span id="m_soll" class="metric-value">--</span></div>
              <div class="metric"><span class="metric-label">Heizung</span><span id="m_heiz" class="metric-value status-neutral">--</span></div>
            </div>
            <ul class="rows">
              <li><span class="row-label">Schritt</span><span id="activeStep" class="row-value">--</span></li>
              <li><span class="row-label">Verstrichen</span><span id="activeElapsed" class="row-value">--</span></li>
              <li><span class="row-label">Gesamt</span><span id="activeTotal" class="row-value">--</span></li>
              <li><span class="row-label">Verbleibend</span><span id="activeRemaining" class="row-value">--</span></li>
            </ul>
          </article>
          <article class="card manual-card">
            <h2>Manuelle Steuerung</h2>
            <form id="manualForm" class="manual-control" method="POST" action="/manual">
              <input id="manualSoll" name="soll" type="hidden" value="10">
              <div class="manual-label">Soll</div>
              <div class="manual-row">
                <button id="manualMinus" class="stepper" type="button" aria-label="Soll senken">-</button>
                <div><span id="manualSollValue" class="manual-value">--</span> <span class="manual-unit">&deg;C</span></div>
                <button id="manualPlus" class="stepper" type="button" aria-label="Soll erh&ouml;hen">+</button>
              </div>
              <button class="btn" type="submit">
                <svg viewBox="0 0 24 24" width="20" height="20" aria-hidden="true"><path d="M9 11V5a2 2 0 0 1 4 0v8"/><path d="M13 9a2 2 0 0 1 4 0v4"/><path d="M17 11a2 2 0 0 1 4 0v4a6 6 0 0 1-6 6h-3a6 6 0 0 1-6-6v-2"/><path d="M7 13l2 2"/></svg>
                Manuell setzen
              </button>
              <p class="small">Sollbereich 10-100 &deg;C</p>
            </form>
          </article>
        </div>
      </div>
    </section>
  </div>
</main>
<nav class="bottom-nav" aria-label="Navigation">
  <a class="nav-link active" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a>
  <a class="nav-link" href="/history"><svg viewBox="0 0 24 24"><path d="M4 19h16"/><path d="M5 15l5-5 4 3 5-7"/></svg><span>Verlauf</span></a>
  <a class="nav-link" href="/recipe"><svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h5"/><path d="M10 17h5"/></svg><span>Rezept</span></a>
  <a class="nav-link" href="/config"><svg viewBox="0 0 24 24"><path d="M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7Z"/><path d="M19.4 15a1.7 1.7 0 0 0 .34 1.88l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.7 1.7 0 0 0-1.88-.34 1.7 1.7 0 0 0-1.03 1.56V21a2 2 0 1 1-4 0v-.09A1.7 1.7 0 0 0 8.97 19.4a1.7 1.7 0 0 0-1.88.34l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.7 1.7 0 0 0 4.6 15 1.7 1.7 0 0 0 3.04 14H3a2 2 0 1 1 0-4h.09A1.7 1.7 0 0 0 4.6 8.97a1.7 1.7 0 0 0-.34-1.88l-.06-.06A2 2 0 1 1 7.03 4.2l.06.06A1.7 1.7 0 0 0 8.97 4.6 1.7 1.7 0 0 0 10 3.04V3a2 2 0 1 1 4 0v.09a1.7 1.7 0 0 0 1.03 1.51 1.7 1.7 0 0 0 1.88-.34l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.7 1.7 0 0 0-.34 1.88A1.7 1.7 0 0 0 20.96 10H21a2 2 0 1 1 0 4h-.09A1.7 1.7 0 0 0 19.4 15Z"/></svg><span>Einstellungen</span></a>
</nav>
<script>
'use strict';
const $ = id => document.getElementById(id);
let manualDirty = false;
function fmtTemp(v, digits){ const n = Number(v); return Number.isFinite(n) ? n.toFixed(digits) + ' °C' : '--'; }
function clampSoll(v){ return Math.max(10, Math.min(100, Number(v) || 10)); }
function setManualSoll(v, dirty){
  const value = clampSoll(v);
  $('manualSoll').value = value;
  $('manualSollValue').textContent = String(value);
  manualDirty = dirty === true;
}
function stepSvg(kind){
  if (kind === 'mash') return '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M5 19c8 0 14-6 14-14C11 5 5 11 5 19Z"/><path d="M5 19c4-5 8-8 14-14"/></svg>';
  if (kind === 'mashout') return '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 3s6 7 6 11a6 6 0 0 1-12 0c0-4 6-11 6-11Z"/><path d="M9.5 15a2.5 2.5 0 0 0 5 0"/></svg>';
  if (kind === 'boil') return '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M8 9h8v9a2 2 0 0 1-2 2h-4a2 2 0 0 1-2-2Z"/><path d="M6 12h12"/><path d="M9 5c-1 1.2 1 2.2 0 3.4"/><path d="M13 4c-1 1.2 1 2.2 0 3.4"/><path d="M17 5c-1 1.2 1 2.2 0 3.4"/></svg>';
  if (kind === 'hop') return '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 3c4 3 6 7 6 11a6 6 0 0 1-12 0c0-4 2-8 6-11Z"/><path d="M12 3v18"/><path d="M8 10l4 3 4-3"/><path d="M7 14l5 3 5-3"/></svg>';
  return '';
}
function heatSvg(){
  return '<span class="heat-status"><svg viewBox="0 0 24 24" aria-hidden="true"><path d="M7 20c-1.5-2.5 1.5-3.5 0-6"/><path d="M12 20c-1.5-2.5 1.5-3.5 0-6"/><path d="M17 20c-1.5-2.5 1.5-3.5 0-6"/></svg><span class="heat-label">an</span></span>';
}
function addStep(kind, iconText, name, sub, badge, badgeClass, index, activeIndex){
  const row = document.createElement('div');
  row.className = 'recipe-step' + (Number.isInteger(activeIndex) && index === activeIndex ? ' active' : '');
  row.dataset.stepIndex = String(index);
  const icon = document.createElement('span');
  icon.className = 'step-icon ' + kind;
  const svg = stepSvg(kind);
  if (svg) icon.innerHTML = svg; else icon.textContent = iconText;
  const main = document.createElement('span');
  const stepName = document.createElement('span');
  stepName.className = 'step-name';
  stepName.textContent = name;
  main.appendChild(stepName);
  if (sub) {
    const stepSub = document.createElement('span');
    stepSub.className = 'step-sub';
    stepSub.textContent = sub;
    main.appendChild(stepSub);
  }
  const stepBadge = document.createElement('span');
  stepBadge.className = 'step-badge ' + (badgeClass || '');
  stepBadge.textContent = badge || '';
  row.appendChild(icon);
  row.appendChild(main);
  row.appendChild(stepBadge);
  $('recipeSteps').appendChild(row);
}
function renderRecipe(d){
  $('recipeTitle').textContent = d.recipe_name || 'Standardrezept';
  $('recipeSteps').textContent = '';
  const activeIndex = d.active_step_index === null || d.active_step_index === undefined
    ? null : Number.isInteger(Number(d.active_step_index)) ? Number(d.active_step_index) : null;
  const rests = Array.isArray(d.recipe_rasten) ? d.recipe_rasten : [];
  const restCount = rests.length;
  addStep('mash', '', 'Einmaischen', fmtTemp(d.recipe_maischtemp, 0), '-', '', 0, activeIndex);
  rests.forEach((r, i) => addStep('', String(i + 1), (i + 1) + '. Rast', fmtTemp(r.temp, 0), Number(r.zeit || 0) + ' min', '', i + 1, activeIndex));
  addStep('mashout', '', 'Abmaischen', fmtTemp(d.recipe_endtemp, 0), '-', '', restCount + 1, activeIndex);
  addStep('boil', '', 'Kochen', '', Number(d.recipe_kochzeit || 0) + ' min', 'warm', restCount + 2, activeIndex);
  const hops = Array.isArray(d.recipe_hopfen) ? d.recipe_hopfen : [];
  hops.forEach((t, i) => addStep('hop', '', (i + 1) + '. Hopfengabe', '', Number(t || 0) + ' min', 'neutral', restCount + 3 + i, activeIndex));
  addStep('boil', '', 'Kochende', '', Number(d.recipe_kochzeit || 0) + ' min', 'warm', restCount + 3 + hops.length, activeIndex);
}
function fmtSeconds(v){
  const seconds = Number(v);
  if (!Number.isFinite(seconds) || seconds < 0) return '--';
  return Math.floor(seconds / 60) + ':' + String(Math.floor(seconds % 60)).padStart(2, '0');
}
function renderDashboard(d){
  const ist = fmtTemp(d.temp_ist, 1);
  const hasSoll = d.temp_soll !== null && Number.isFinite(Number(d.temp_soll));
  const sollValue = hasSoll ? clampSoll(d.temp_soll) : clampSoll(Math.round(Number(d.temp_ist)));
  $('m_ist').textContent = ist;
  $('m_soll').textContent = hasSoll ? fmtTemp(sollValue, 0) : '--';
  if (!manualDirty) setManualSoll(sollValue, false);
  const heat = d.heizung === 'an';
  $('m_heiz').innerHTML = heat ? heatSvg() : 'aus';
  $('m_heiz').className = 'metric-value ' + (heat ? 'status-heat' : 'status-neutral');
  renderRecipe(d);
  $('activeStep').textContent = d.active_step_label || '--';
  $('activeElapsed').textContent = fmtSeconds(d.active_step_elapsed_seconds);
  $('activeTotal').textContent = fmtSeconds(d.active_step_total_seconds);
  $('activeRemaining').textContent = fmtSeconds(d.active_step_remaining_seconds);
  const alarm = d.alarm === true;
  $('alarm').textContent = alarm ? 'RUFALARM: ' + (d.alarm_reason || 'none') + ' — ' + (d.alarm_action || 'acknowledge_at_controller') : '';
  $('alarm').style.display = alarm ? 'block' : 'none';
  document.title = alarm ? 'RUFALARM' : 'BrauKnecht';
}
async function poll(){
  try{
    const d = await (await fetch('/data.json', {cache:'no-store'})).json();
    renderDashboard(d);
  } catch(e){
  } finally {
    setTimeout(poll, 1000);
  }
}
$('manualForm').addEventListener('submit', async ev => {
  ev.preventDefault();
  const value = clampSoll($('manualSoll').value);
  setManualSoll(value, false);
  try{
    await fetch('/manual', {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded', 'X-BrauKnecht-Action':'manual'}, body:'soll=' + encodeURIComponent(value)});
  } catch(e) {
    manualDirty = true;
  }
});
$('manualMinus').addEventListener('click', () => setManualSoll(Number($('manualSoll').value) - 1, true));
$('manualPlus').addEventListener('click', () => setManualSoll(Number($('manualSoll').value) + 1, true));
poll();
</script>
</body></html>
)=====";

const char PAGE_History[] PROGMEM = R"=====(
<!DOCTYPE html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>BrauKnecht Verlauf</title><link rel="stylesheet" href="/style.css"></head><body><main class="app"><div class="topbar"><div class="brand">BrauKnecht</div></div><div class="layout"><nav class="side-nav"><a class="nav-link" href="/">Dashboard</a><a class="nav-link active" href="/history">Verlauf</a><a class="nav-link" href="/recipe">Rezept</a><a class="nav-link" href="/config">Einstellungen</a></nav><section class="grid"><h1 class="page-title">Brauverläufe</h1><div class="grid history-grid"><aside class="side-stack"><article class="card"><h2>Gespeicherte Läufe</h2><div id="sessions"></div><p id="message" class="small"></p></article></aside><div class="history-main"><article class="card"><header class="history-card-head"><div><span class="kicker" id="kind">--</span><h2 id="title">--</h2></div></header><div class="history-meta"><span id="meta">--</span><span id="warning"></span></div><p><a id="json" hidden>JSON herunterladen</a> · <a id="csv" hidden>CSV herunterladen</a></p><div class="chart-wrap"><canvas id="chart"></canvas></div><div class="chart-legend"><span><i class="legend-line"></i>Soll</span><span><i class="legend-line live"></i>Ist (Mittelwert)</span></div></article><article class="card"><h2>Rezeptübersicht</h2><div id="recipe" class="mini-recipe"></div></article></div></div></section></div></main>
<script>
'use strict';const $=id=>document.getElementById(id);let records=[];
function clear(){records=[];$('title').textContent='--';$('kind').textContent='--';$('meta').textContent='--';$('warning').textContent='';$('recipe').textContent='';$('json').hidden=true;$('csv').hidden=true;draw();}
function draw(){const c=$('chart'),r=c.getBoundingClientRect(),d=devicePixelRatio||1;c.width=Math.max(1,r.width*d);c.height=Math.max(1,r.height*d);const x=c.getContext('2d');x.scale(d,d);const w=r.width,h=r.height,p=28;x.clearRect(0,0,w,h);if(!records.length)return;const maxT=Math.max(1,...records.map(v=>v.elapsed_seconds));const min=Math.min(...records.map(v=>v.temp_min_c)),max=Math.max(...records.map(v=>v.temp_max_c));const px=t=>p+(w-2*p)*t/maxT,py=t=>h-p-(h-2*p)*(t-min)/Math.max(1,max-min);x.fillStyle='rgba(37,155,36,.18)';x.beginPath();records.forEach((v,i)=>i?x.lineTo(px(v.elapsed_seconds),py(v.temp_max_c)):x.moveTo(px(v.elapsed_seconds),py(v.temp_max_c)));for(let i=records.length-1;i>=0;i--)x.lineTo(px(records[i].elapsed_seconds),py(records[i].temp_min_c));x.fill();[['#0b57d0','target_c'],['#259b24','temp_avg_c']].forEach(s=>{x.strokeStyle=s[0];x.lineWidth=2;x.beginPath();records.forEach((v,i)=>i?x.lineTo(px(v.elapsed_seconds),py(v[s[1]])):x.moveTo(px(v.elapsed_seconds),py(v[s[1]])));x.stroke();});}
function recipe(d){const r=d.recipe||{};$('recipe').textContent='Einmaischen: '+(r.maischtemp??'--')+' °C · Rasten: '+(r.rasten||[]).map(v=>v.temp+' °C / '+v.zeit+' min').join(', ')+' · Abmaischen: '+(r.endtemp??'--')+' °C · Kochen: '+(r.kochzeit??'--')+' min';}
async function select(s){try{const d=await (await fetch('/history/session.json?id='+s.id,{cache:'no-store'})).json();records=Array.isArray(d.records)?d.records:[];$('title').textContent=s.recipe_name||'Unbenannt';$('kind').textContent=s.kind==='mash'?'Maischen':'Kochen';$('meta').textContent='Lauf #'+s.id+' · '+s.result+' · '+s.duration_seconds+' s · '+s.record_count+' Messfenster';$('warning').textContent=s.truncated?'Aufzeichnung gekürzt':'';$('json').href='/history/session.json?id='+s.id;$('csv').href='/history/session.csv?id='+s.id;$('json').hidden=false;$('csv').hidden=false;recipe(d);draw();}catch(e){clear();$('message').textContent='Brauverlauf konnte nicht geladen werden.';}}
async function load(){try{const d=await (await fetch('/history/sessions.json',{cache:'no-store'})).json(),box=$('sessions');box.textContent='';if(!Array.isArray(d.sessions)||!d.sessions.length){clear();$('message').textContent='Noch keine Brauverläufe gespeichert';return;}d.sessions.forEach(s=>{const b=document.createElement('button');b.type='button';b.textContent='#'+s.id+' · '+s.recipe_name;b.onclick=()=>select(s);box.appendChild(b);});select(d.sessions[0]);}catch(e){clear();$('message').textContent='Brauverläufe konnten nicht geladen werden.';}}
addEventListener('resize',draw);load();
</script></body></html>
)=====";
