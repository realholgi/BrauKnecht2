#pragma once

// Local stylesheet and app pages. No external assets are used so the UI works
// on the BrauKnecht access point without internet access.
const char STYLE_CSS[] PROGMEM = R"=====(
:root{--color-canvas:#FFFFFF;--color-surface:#F8F8FC;--color-surface-raised:#FFFFFF;--color-ink:#2C3155;--color-ink-subtle:#686B80;--color-rule:#D9DAE8;--color-rule-strong:#9599B8;--color-disabled:#E9EAF3;--color-action:#3E457A;--color-action-hover:#323866;--color-action-ink:#FFFFFF;--color-action-soft:#E8E9F5;--color-heat:#B95A2A;--color-heat-hover:#9C4920;--color-heat-soft:#F6E3D8;--color-success:#2F7654;--color-success-soft:#E3F1E9;--color-warning:#805B19;--color-warning-soft:#F8EFD3;--color-danger:#A23E4A;--color-danger-soft:#F8E2E5;--color-focus:#294F93;--color-chart-band:rgba(47,118,84,.16);--space-1:4px;--space-2:8px;--space-3:12px;--space-4:16px;--space-5:24px;--space-6:32px;--space-7:48px;--radius-control:8px;--radius-card:12px;--radius-pill:999px;--shadow-card:0 2px 8px rgba(44,49,85,.08);--duration-fast:150ms;--ease-out:cubic-bezier(.22,.61,.36,1)}
*{box-sizing:border-box}html,body{margin:0;min-height:100%}html{-webkit-text-size-adjust:100%}body{font:16px/1.5 "Avenir Next","Segoe UI","Helvetica Neue",Arial,sans-serif;color:var(--color-ink);background:var(--color-canvas);padding:env(safe-area-inset-top,0) env(safe-area-inset-right,0) calc(96px + env(safe-area-inset-bottom,0px)) env(safe-area-inset-left,0)}a{color:inherit}button,input,select{font:inherit}button,a,input,select{transition:color var(--duration-fast) var(--ease-out),background var(--duration-fast) var(--ease-out),border-color var(--duration-fast) var(--ease-out),box-shadow var(--duration-fast) var(--ease-out)}:focus-visible{outline:3px solid var(--color-focus);outline-offset:3px}[hidden]{display:none!important}.app{width:min(1344px,100%);margin:auto;padding:var(--space-5) var(--space-4)}.topbar{display:flex;align-items:center;margin:0 0 var(--space-5)}.brand{font-size:20px;font-weight:500}.page-title{margin:0 0 var(--space-5);font-size:28px;line-height:1.2;font-weight:500}.layout{display:block}.side-nav,.bottom-nav{background:var(--color-surface-raised);border:1px solid var(--color-rule);box-shadow:var(--shadow-card)}.side-nav{display:none}.bottom-nav{position:fixed;z-index:10;inset:auto 0 0;display:grid;grid-template-columns:repeat(4,1fr);border-width:1px 0 0;padding-bottom:env(safe-area-inset-bottom,0px)}.nav-link{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:var(--space-1);min-height:72px;padding:0 var(--space-1);color:var(--color-ink-subtle);text-decoration:none;font-size:12px;font-weight:600}.nav-link svg{width:20px;height:20px;stroke:currentColor;stroke-width:1.8;fill:none;stroke-linecap:round;stroke-linejoin:round}.nav-link .nav-fill{fill:currentColor;stroke:none}.nav-link.active{color:var(--color-action);background:var(--color-action-soft);box-shadow:inset 0 3px var(--color-action)}.grid{display:grid;gap:var(--space-4)}.grid.two{grid-template-columns:1fr}.card,.state-card{background:var(--color-surface-raised);border:1px solid var(--color-rule);border-radius:var(--radius-card);box-shadow:var(--shadow-card);padding:var(--space-4)}.card h1,.card h2{margin:0 0 var(--space-3);font-size:20px;font-weight:500}.state-card{color:var(--color-ink-subtle)}.state-card.success{background:var(--color-success-soft);border-color:var(--color-success);color:var(--color-success)}.state-card.warning{background:var(--color-warning-soft);border-color:var(--color-warning);color:var(--color-warning)}.state-card.danger,.alert{background:var(--color-danger-soft);border-color:var(--color-danger);color:var(--color-danger)}.state-card:empty{display:none}.hero-card,.history-main,.side-stack{display:grid;gap:var(--space-4)}.metrics{display:grid;grid-template-columns:repeat(3,minmax(0,1fr))}.metric{padding:var(--space-3);border-left:1px solid var(--color-rule);text-align:center}.metric:first-child{border-left:0}.metric-label,.kicker{display:block;margin-bottom:var(--space-1);color:var(--color-ink-subtle);font-size:12px;font-weight:600}.metric-value{display:block;font-size:clamp(28px,7vw,40px);font-weight:500;font-variant-numeric:tabular-nums;overflow-wrap:anywhere}.metric-unit,.manual-unit{font-size:14px;font-weight:500}.dashboard-grid{align-items:start}.recipe-panel{min-height:420px}.recipe-head,.history-card-head,.recipe-summary{display:flex;justify-content:space-between;align-items:flex-start;gap:var(--space-3);margin-bottom:var(--space-4)}.recipe-title{margin:0;font-size:24px;line-height:1.2;font-weight:500}.icon-btn{display:grid;place-items:center;width:44px;height:44px;border:1px solid var(--color-rule-strong);border-radius:var(--radius-control);color:var(--color-action);text-decoration:none}.icon-btn svg,.section-title svg{width:22px;height:22px;stroke:currentColor;stroke-width:2;fill:none;stroke-linecap:round;stroke-linejoin:round}.recipe-steps,.recipe-timeline{position:relative;display:grid;gap:0}.recipe-step{position:relative;display:grid;grid-template-columns:42px minmax(0,1fr) auto;gap:var(--space-3);align-items:center;min-height:62px}.recipe-step.active{margin:0 -8px;padding:0 8px;border-left:4px solid var(--color-action);border-radius:var(--radius-control);background:var(--color-action-soft);font-weight:600}.recipe-step::before{content:"";position:absolute;left:20px;top:0;bottom:0;width:2px;background:var(--color-rule)}.recipe-step:first-child::before{top:50%}.recipe-step:last-child::before{bottom:50%}.step-icon,.timeline-icon{position:relative;z-index:1;display:grid;place-items:center;width:42px;height:42px;border:2px solid var(--color-action);border-radius:50%;background:var(--color-surface-raised);color:var(--color-action);font-weight:600}.step-icon svg,.timeline-icon svg,.heat-status svg{width:22px;height:22px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}.step-icon.mash,.timeline-icon.mash{background:var(--color-success);border-color:var(--color-success);color:var(--color-action-ink)}.step-icon.mashout,.timeline-icon.mashout{background:var(--color-warning);border-color:var(--color-warning);color:var(--color-action-ink)}.step-icon.boil,.timeline-icon.boil{background:var(--color-heat);border-color:var(--color-heat);color:var(--color-action-ink)}.step-icon.hop,.timeline-icon.hop{border-color:var(--color-rule-strong);color:var(--color-ink-subtle)}.step-name{display:block;font-size:16px;font-weight:500}.step-sub,.small{display:block;margin-top:var(--space-1);color:var(--color-ink-subtle);font-size:14px}.step-badge,.pill,.chip{display:inline-flex;align-items:center;min-height:32px;padding:0 var(--space-2);border:1px solid var(--color-rule);border-radius:var(--radius-pill);background:var(--color-surface);font-size:14px;font-variant-numeric:tabular-nums;white-space:nowrap}.step-badge.warm{border-color:var(--color-heat);background:var(--color-heat-soft);color:var(--color-heat)}.step-badge.neutral{color:var(--color-ink-subtle)}.live-status-card .metrics{margin-top:var(--space-3)}.heat-status{display:inline-flex;flex-direction:column;align-items:center;gap:var(--space-1);color:var(--color-heat);font-weight:600}.heat-status svg{width:32px;height:32px}.heat-label{font-size:14px}.status-ok{color:var(--color-success)}.status-heat{color:var(--color-heat)}.status-warn{color:var(--color-warning)}.status-neutral{color:var(--color-ink-subtle)}.alert{display:none;padding:var(--space-3);border:1px solid var(--color-danger);border-radius:var(--radius-card);font-weight:600}.alert.visible{display:block}.rows{display:grid;gap:0;margin:0;padding:0;list-style:none}.row,.rows li{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:var(--space-3);align-items:center;padding:var(--space-2) 0;border-top:1px solid var(--color-rule)}.row:first-child,.rows li:first-child{border-top:0}.row-label{color:var(--color-ink-subtle)}.row-value{font-weight:500;text-align:right;font-variant-numeric:tabular-nums;overflow-wrap:anywhere}label{display:block;margin:var(--space-3) 0 var(--space-1);font-size:14px;font-weight:600}input,select{width:100%;min-height:48px;padding:var(--space-2) var(--space-3);border:1px solid var(--color-rule-strong);border-radius:var(--radius-control);background:var(--color-surface);color:var(--color-ink)}input[type=file]{padding:10px}.field-grid,.settings-form,.recipe-editor-fieldset,.recipe-editor-row{display:grid;gap:var(--space-3)}.settings-actions{display:flex}.recipe-editor-actions{position:sticky;bottom:calc(72px + env(safe-area-inset-bottom,0px));z-index:9;display:flex;flex-wrap:wrap;justify-content:flex-end;gap:var(--space-2);margin:0 -16px -16px;padding:var(--space-3) var(--space-4);border-top:1px solid var(--color-rule);background:var(--color-canvas)}.section-title,.import-title{display:flex;align-items:center;gap:var(--space-2);margin:0 0 var(--space-3);font-size:20px;font-weight:500}.inline-form{display:grid;grid-template-columns:minmax(0,1fr) auto;gap:var(--space-2);align-items:end}.btn{display:inline-flex;align-items:center;justify-content:center;min-height:44px;padding:0 22px;border:1px solid var(--color-action);border-radius:var(--radius-control);background:var(--color-action);color:var(--color-action-ink);font-weight:600;text-decoration:none;cursor:pointer}.btn:hover{background:var(--color-action-hover);border-color:var(--color-action-hover)}.btn.heat{background:var(--color-heat);border-color:var(--color-heat)}.btn.heat:hover{background:var(--color-heat-hover);border-color:var(--color-heat-hover)}.btn.outline{background:var(--color-surface);color:var(--color-action);border-color:var(--color-action)}.btn.full{width:100%}.btn:disabled,input:disabled,select:disabled,fieldset:disabled .btn{cursor:not-allowed;background:var(--color-disabled);color:var(--color-ink-subtle);border-color:var(--color-rule);opacity:1}.chip-row,.history-actions{display:flex;flex-wrap:wrap;gap:var(--space-2);margin-top:var(--space-3)}.chip{gap:var(--space-2);min-height:38px;border-radius:var(--radius-control)}.chip svg{width:19px;height:19px;stroke:currentColor;fill:none;stroke-width:2}.recipe-timeline::before{content:"";position:absolute;left:21px;top:22px;bottom:22px;width:2px;background:var(--color-rule)}.timeline-row{position:relative;display:grid;grid-template-columns:44px minmax(0,1fr) 86px 74px;gap:var(--space-3);align-items:center;min-height:64px;border-top:1px solid var(--color-rule)}.timeline-row:first-child{border-top:0}.timeline-icon{width:44px;height:44px;border-width:1px}.timeline-name{font-weight:500;overflow-wrap:anywhere}.timeline-temp,.timeline-time{text-align:right;font-variant-numeric:tabular-nums}.history-session-list{display:grid;gap:var(--space-2)}.history-session{min-height:48px;padding:var(--space-2) var(--space-3);border:1px solid var(--color-rule);border-radius:var(--radius-control);background:var(--color-surface);color:var(--color-ink);text-align:left;font:inherit;cursor:pointer}.history-session.active{border-color:var(--color-action);background:var(--color-action-soft);color:var(--color-action);font-weight:600}.history-meta{display:flex;flex-wrap:wrap;gap:var(--space-2) var(--space-5);color:var(--color-ink-subtle);font-size:14px}.chart-wrap{height:224px}.chart-legend{display:flex;flex-wrap:wrap;gap:var(--space-4);margin-top:var(--space-3);font-size:14px}.legend-line{display:inline-block;width:28px;height:2px;margin-right:var(--space-2);vertical-align:middle;background:var(--color-action)}.legend-line.live{background:var(--color-success)}canvas{width:100%;height:100%;display:block}.settings-info-card .rows{grid-template-columns:1fr}
@media (max-width:519px){.recipe-summary{display:grid}.recipe-summary .btn{width:100%}.timeline-row{grid-template-columns:44px minmax(0,1fr) auto;gap:var(--space-2)}.timeline-time{grid-column:2/4;text-align:left;color:var(--color-ink-subtle)}}
@media (min-width:760px){body{padding-bottom:0}.app{padding:0}.topbar{display:none}.layout{display:grid;grid-template-columns:224px minmax(0,1fr);min-height:100vh}.layout>.grid{width:min(1120px,100%);padding:var(--space-7) var(--space-5)}.side-nav{position:sticky;top:0;display:grid;align-content:start;gap:var(--space-2);min-height:100vh;border-width:0 1px 0 0;box-shadow:none;padding:98px var(--space-3) 0}.side-nav::before{content:"BrauKnecht";position:absolute;top:32px;left:var(--space-5);font-size:20px;font-weight:500}.side-nav .nav-link{flex-direction:row;justify-content:flex-start;min-height:52px;padding:0 var(--space-3);border-radius:var(--radius-control);font-size:14px}.side-nav .nav-link.active{box-shadow:inset 3px 0 var(--color-action)}.bottom-nav{display:none}.grid.two,.field-grid.two,.recipe-editor-row{grid-template-columns:1fr 1fr}.recipe-editor-actions{position:static;z-index:auto;margin:0;padding:0;border:0;background:none}.btn.full{width:auto;min-width:220px}.settings-info-card .rows{grid-template-columns:1fr 1fr;column-gap:var(--space-6)}.settings-info-card .rows li:nth-child(2){border-top:0}.chart-wrap{height:280px}.page-title{font-size:32px}}
@media (min-width:1080px){.dashboard-grid{grid-template-columns:minmax(0,1fr) 360px;grid-template-areas:"recipe live" "recipe manual"}.dashboard-grid>.recipe-panel{grid-area:recipe}.dashboard-grid>.live-status-card{grid-area:live}.dashboard-grid>.manual-card{grid-area:manual}.history-grid{grid-template-columns:280px minmax(0,1fr)}}
@media (prefers-reduced-motion:reduce){:root{--duration-fast:0ms}}
.manual-control{text-align:center}.manual-label{margin:var(--space-1) 0 var(--space-2);color:var(--color-ink-subtle);font-size:14px;font-weight:600}.manual-row{display:grid;grid-template-columns:56px minmax(0,1fr) 56px;gap:var(--space-3);align-items:center;margin:var(--space-2) 0 var(--space-4)}.stepper{width:56px;height:56px;border:1px solid var(--color-rule-strong);border-radius:var(--radius-control);background:var(--color-surface);color:var(--color-action);font-size:32px;font-weight:500;line-height:1;cursor:pointer}.manual-value{font-size:40px;font-weight:500;font-variant-numeric:tabular-nums}.manual-card .btn{width:100%;gap:var(--space-2)}.manual-card .btn svg{stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.brand{display:inline-flex;align-items:center;gap:7px;color:var(--color-action);text-decoration:none}.brand-mark{display:grid;place-items:center;width:40px;height:40px;border:2px solid currentColor;border-radius:10px;flex:none}.brand-mark svg{width:28px;height:28px;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}.brand-name{font-size:18px;font-weight:700;letter-spacing:1px;line-height:1;white-space:nowrap}.brand-name span{color:var(--color-heat)}.brand-sub{margin-left:-4px;color:var(--color-ink-subtle);font-size:9px;font-weight:600;letter-spacing:1.3px;white-space:nowrap}@media (max-width:370px){.brand-mark{width:34px;height:34px}.brand-name{font-size:16px}.brand-sub{font-size:8px;letter-spacing:.8px}}@media (min-width:760px){.side-nav::before{content:"";top:36px;left:24px;width:22px;height:22px;border:2px solid var(--color-action);border-radius:7px;background:linear-gradient(135deg,transparent 46%,var(--color-action) 47% 53%,transparent 54%);transform:rotate(45deg)}.side-nav::after{content:"BRAU KNECHT";position:absolute;top:34px;left:56px;color:var(--color-action);font-size:16px;font-weight:700;letter-spacing:.7px}}
@media (min-width:760px){.side-nav{padding:var(--space-5) var(--space-3) 0}.side-nav::before,.side-nav::after{content:none}.side-nav .brand{margin:0 0 var(--space-5) var(--space-2);gap:6px}.side-nav .brand-mark{width:30px;height:30px;border-radius:8px}.side-nav .brand-mark svg{width:21px;height:21px}.side-nav .brand-name{font-size:14px;letter-spacing:.5px}.side-nav .brand-sub{display:none}}
)=====";

const char PAGE_Dashboard[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <title>BrauKnecht</title>
  <link rel="icon" href="data:,">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
<main class="app">
  <div class="topbar"><a class="brand" href="/" aria-label="BrauKnecht Dashboard"><span class="brand-mark" aria-hidden="true"><svg viewBox="0 0 48 48"><path d="M10 39C10 22 19 10 36 9c0 17-12 29-26 30Z"/><path d="M12 37 35 12"/><path d="M19 29h9"/><path d="M24 22h8"/></svg></span><span class="brand-name">BRAU<span>KNECHT</span></span><span class="brand-sub">STEUERUNG</span></a></div>
  <div class="layout">
    <nav class="side-nav" aria-label="Navigation">
      <a class="brand" href="/" aria-label="BrauKnecht Dashboard"><span class="brand-mark" aria-hidden="true"><svg viewBox="0 0 48 48"><path d="M10 39C10 22 19 10 36 9c0 17-12 29-26 30Z"/><path d="M12 37 35 12"/><path d="M19 29h9"/><path d="M24 22h8"/></svg></span><span class="brand-name">BRAU<span>KNECHT</span></span><span class="brand-sub">STEUERUNG</span></a>
      <a class="nav-link active" aria-current="page" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a>
      <a class="nav-link" href="/history"><svg viewBox="0 0 24 24"><path d="M4 19h16"/><path d="M5 15l5-5 4 3 5-7"/></svg><span>Verlauf</span></a>
      <a class="nav-link" href="/recipe"><svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h5"/><path d="M10 17h5"/></svg><span>Rezept</span></a>
      <a class="nav-link" href="/config"><svg viewBox="0 0 24 24"><path d="M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7Z"/><path d="M19.4 15a1.7 1.7 0 0 0 .34 1.88l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.7 1.7 0 0 0-1.88-.34 1.7 1.7 0 0 0-1.03 1.56V21a2 2 0 1 1-4 0v-.09A1.7 1.7 0 0 0 8.97 19.4a1.7 1.7 0 0 0-1.88.34l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.7 1.7 0 0 0 4.6 15 1.7 1.7 0 0 0 3.04 14H3a2 2 0 1 1 0-4h.09A1.7 1.7 0 0 0 4.6 8.97a1.7 1.7 0 0 0-.34-1.88l-.06-.06A2 2 0 1 1 7.03 4.2l.06.06A1.7 1.7 0 0 0 8.97 4.6 1.7 1.7 0 0 0 10 3.04V3a2 2 0 1 1 4 0v.09a1.7 1.7 0 0 0 1.03 1.51 1.7 1.7 0 0 0 1.88-.34l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.7 1.7 0 0 0-.34 1.88A1.7 1.7 0 0 0 20.96 10H21a2 2 0 1 1 0 4h-.09A1.7 1.7 0 0 0 19.4 15Z"/></svg><span>Einstellungen</span></a>
    </nav>
    <section class="grid">
      <div id="alarm" class="alert" role="alert" aria-live="assertive">RUFALARM</div>
      <div class="grid dashboard-grid">
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
        <article class="card manual-card">
          <h2>Manuelle Steuerung</h2>
          <form id="manualForm" class="manual-control" method="POST" action="/manual">
            <input id="manualSoll" name="soll" type="hidden" value="10">
            <div class="manual-label">Soll</div>
            <div class="manual-row">
              <button id="manualMinus" class="stepper" type="button" aria-label="Soll senken">-</button>
              <div><span id="manualSollValue" class="manual-value" aria-live="polite">--</span> <span class="manual-unit">&deg;C</span></div>
              <button id="manualPlus" class="stepper" type="button" aria-label="Soll erh&ouml;hen">+</button>
            </div>
            <button class="btn heat" type="submit">
              <svg viewBox="0 0 24 24" width="20" height="20" aria-hidden="true"><path d="M9 11V5a2 2 0 0 1 4 0v8"/><path d="M13 9a2 2 0 0 1 4 0v4"/><path d="M17 11a2 2 0 0 1 4 0v4a6 6 0 0 1-6 6h-3a6 6 0 0 1-6-6v-2"/><path d="M7 13l2 2"/></svg>
              Manuell setzen
            </button>
            <p class="small">Sollbereich 10-100 &deg;C</p>
          </form>
        </article>
      </div>
    </section>
  </div>
</main>
<nav class="bottom-nav" aria-label="Navigation">
  <a class="nav-link active" aria-current="page" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a>
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
  const active = Number.isInteger(activeIndex) && index === activeIndex;
  row.className = 'recipe-step' + (active ? ' active' : '');
  if (active) row.setAttribute('aria-current', 'step');
  row.dataset.stepIndex = String(index);
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
  setManualSoll(value, true);
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
<!DOCTYPE html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover"><link rel="icon" href="data:,"><link rel="stylesheet" href="/style.css"><title>BrauKnecht Verlauf</title></head><body><main class="app"><div class="topbar"><div class="brand">BrauKnecht</div></div><div class="layout"><nav class="side-nav" aria-label="Navigation"><a class="nav-link" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a><a class="nav-link active" aria-current="page" href="/history"><svg viewBox="0 0 24 24"><path d="M4 19h16"/><path d="M5 15l5-5 4 3 5-7"/></svg><span>Verlauf</span></a><a class="nav-link" href="/recipe"><svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h5"/><path d="M10 17h5"/></svg><span>Rezept</span></a><a class="nav-link" href="/config"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="8"/><path d="M12 8v8M8 12h8"/></svg><span>Einstellungen</span></a></nav><section class="grid"><h1 class="page-title">Brauverläufe</h1><div class="grid history-grid"><aside><article class="card"><h2>Gespeicherte Läufe</h2><div id="sessions" class="history-session-list"></div></article></aside><div class="history-main"><div id="historyEmpty" class="state-card" role="status"><p id="message">Brauverläufe werden geladen …</p></div><div id="historyDetail" hidden><article class="card"><header class="history-card-head"><div><span class="kicker" id="kind">--</span><h2 id="title">--</h2></div></header><div class="history-meta"><span id="meta">--</span></div><p id="warning" class="state-card warning" role="status"></p><div class="history-actions"><a id="json" class="btn outline" hidden>JSON herunterladen</a><a id="csv" class="btn outline" hidden>CSV herunterladen</a></div><div class="chart-wrap"><canvas id="chart" role="img" aria-label="Temperaturverlauf des ausgewählten Braulaufs"></canvas></div><div class="chart-legend"><span><i class="legend-line"></i>Soll</span><span><i class="legend-line live"></i>Ist (Mittelwert)</span></div></article></div></div></div></section></div></main><nav class="bottom-nav" aria-label="Navigation"><a class="nav-link" href="/"><svg viewBox="0 0 24 24"><path class="nav-fill" d="M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z"/></svg><span>Dashboard</span></a><a class="nav-link active" aria-current="page" href="/history"><svg viewBox="0 0 24 24"><path d="M4 19h16"/><path d="M5 15l5-5 4 3 5-7"/></svg><span>Verlauf</span></a><a class="nav-link" href="/recipe"><svg viewBox="0 0 24 24"><path d="M7 3h7l4 4v14H7z"/><path d="M14 3v5h5"/><path d="M10 13h5"/><path d="M10 17h5"/></svg><span>Rezept</span></a><a class="nav-link" href="/config"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="8"/><path d="M12 8v8M8 12h8"/></svg><span>Einstellungen</span></a></nav>
<script>
'use strict';const $=id=>document.getElementById(id);const rootStyle=getComputedStyle(document.documentElement);const actionColor=rootStyle.getPropertyValue('--color-action').trim(),successColor=rootStyle.getPropertyValue('--color-success').trim(),chartBand=rootStyle.getPropertyValue('--color-chart-band').trim();let records=[];
const brandMarkup='<span class="brand-mark" aria-hidden="true"><svg viewBox="0 0 48 48"><path d="M10 39C10 22 19 10 36 9c0 17-12 29-26 30Z"/><path d="M12 37 35 12"/><path d="M19 29h9"/><path d="M24 22h8"/></svg></span><span class="brand-name">BRAU<span>KNECHT</span></span><span class="brand-sub">STEUERUNG</span>';const brand=document.querySelector('.topbar .brand');if(brand){brand.innerHTML=brandMarkup;brand.setAttribute('role','link');brand.tabIndex=0;brand.addEventListener('click',()=>location.href='/');brand.addEventListener('keydown',event=>{if(event.key==='Enter')location.href='/';});}const sideNav=document.querySelector('.side-nav');if(sideNav){const sideBrand=document.createElement('a');sideBrand.className='brand';sideBrand.href='/';sideBrand.setAttribute('aria-label','BrauKnecht Dashboard');sideBrand.innerHTML=brandMarkup;sideNav.prepend(sideBrand);}
function clear(){records=[];$('title').textContent='--';$('kind').textContent='--';$('meta').textContent='--';$('warning').textContent='';$('json').hidden=true;$('csv').hidden=true;document.querySelectorAll('.history-session').forEach(button=>{button.classList.remove('active');button.setAttribute('aria-pressed','false');});draw();}
function showHistoryState(message,isError){clear();$('message').textContent=message;$('historyEmpty').classList.toggle('danger',isError);$('historyEmpty').hidden=false;$('historyDetail').hidden=true;}
function draw(){const c=$('chart'),r=c.getBoundingClientRect(),d=devicePixelRatio||1;c.width=Math.max(1,r.width*d);c.height=Math.max(1,r.height*d);const x=c.getContext('2d');x.scale(d,d);const w=r.width,h=r.height,p=28;x.clearRect(0,0,w,h);if(!records.length)return;const maxT=Math.max(1,...records.map(v=>v.elapsed_seconds));const min=Math.min(...records.map(v=>v.temp_min_c)),max=Math.max(...records.map(v=>v.temp_max_c));const px=t=>p+(w-2*p)*t/maxT,py=t=>h-p-(h-2*p)*(t-min)/Math.max(1,max-min);x.fillStyle=chartBand;x.beginPath();records.forEach((v,i)=>i?x.lineTo(px(v.elapsed_seconds),py(v.temp_max_c)):x.moveTo(px(v.elapsed_seconds),py(v.temp_max_c)));for(let i=records.length-1;i>=0;i--)x.lineTo(px(records[i].elapsed_seconds),py(records[i].temp_min_c));x.fill();[[actionColor,'target_c'],[successColor,'temp_avg_c']].forEach(s=>{x.strokeStyle=s[0];x.lineWidth=2;x.beginPath();records.forEach((v,i)=>i?x.lineTo(px(v.elapsed_seconds),py(v[s[1]])):x.moveTo(px(v.elapsed_seconds),py(v[s[1]])));x.stroke();});}
async function select(s){try{const response=await fetch('/history/session.json?id='+s.id,{cache:'no-store'});if(!response.ok)throw new Error('detail');const d=await response.json();records=Array.isArray(d.records)?d.records:[];$('title').textContent=s.recipe_name||'Unbenannt';$('kind').textContent=s.kind==='mash'?'Maischen':'Kochen';$('meta').textContent='Lauf #'+s.id+' · '+s.result+' · '+s.duration_seconds+' s · '+s.record_count+' Messfenster';$('warning').textContent=s.truncated?'Aufzeichnung gekürzt':'';$('json').href='/history/session.json?id='+s.id;$('csv').href='/history/session.csv?id='+s.id;$('json').hidden=false;$('csv').hidden=false;document.querySelectorAll('.history-session').forEach(button=>{const active=button.dataset.sessionId===String(s.id);button.classList.toggle('active',active);button.setAttribute('aria-pressed',active?'true':'false');});$('historyEmpty').hidden=true;$('historyDetail').hidden=false;draw();}catch(e){showHistoryState('Brauverlauf konnte nicht geladen werden. Verbindung prüfen und erneut öffnen.',true);}}
async function load(){try{const response=await fetch('/history/sessions.json',{cache:'no-store'});if(!response.ok)throw new Error('sessions');const d=await response.json(),box=$('sessions');box.textContent='';if(!Array.isArray(d.sessions)||!d.sessions.length){showHistoryState('Noch keine Brauverläufe. Ein abgeschlossener Maisch- oder Kochlauf erscheint hier.',false);return;}d.sessions.forEach(s=>{const b=document.createElement('button');b.type='button';b.className='history-session';b.dataset.sessionId=String(s.id);b.setAttribute('aria-pressed','false');b.textContent='#'+s.id+' · '+(s.recipe_name||'Unbenannt');b.addEventListener('click',()=>select(s));box.appendChild(b);});select(d.sessions[0]);}catch(e){showHistoryState('Brauverläufe konnten nicht geladen werden. Verbindung prüfen und erneut öffnen.',true);}}
addEventListener('resize',draw);load();
</script></body></html>
)=====";
