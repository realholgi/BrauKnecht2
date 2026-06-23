#pragma once

// Shared stylesheet, served locally at /style.css (no external requests so it
// works on the BrauKnecht AP with no internet). Auto light/dark, iPhone-friendly.
const char STYLE_CSS[] PROGMEM = R"=====(
:root{
  --bg:#f2f2f7; --card:#fff; --fg:#1c1c1e; --muted:#8a8a8e; --line:#e2e2e7;
  --accent:#0a84ff; --on:#34c759; --off:#8a8a8e; --danger:#ff3b30;
}
@media (prefers-color-scheme:dark){
  :root{ --bg:#000; --card:#1c1c1e; --fg:#fff; --muted:#8d8d93; --line:#2c2c2e; }
}
*{box-sizing:border-box}
body{
  margin:0; background:var(--bg); color:var(--fg);
  font:16px/1.4 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;
  padding:env(safe-area-inset-top) env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left);
}
.wrap{max-width:560px; margin:0 auto; padding:16px}
h1{font-size:1.5rem; margin:8px 4px 16px}
.card{background:var(--card); border:1px solid var(--line); border-radius:14px; padding:16px; margin-bottom:14px}
.card h2{font-size:.8rem; text-transform:uppercase; letter-spacing:.04em; color:var(--muted); margin:0 0 10px}
.temps{display:flex; gap:16px; align-items:baseline}
.temps .big{font-size:3rem; font-weight:600; line-height:1}
.temps .sub{color:var(--muted)}
.row{display:flex; justify-content:space-between; align-items:center; padding:6px 0}
.pill{padding:4px 12px; border-radius:999px; color:#fff; font-size:.85rem; font-weight:600}
.pill.on{background:var(--on)} .pill.off{background:var(--off)}
ul{margin:0; padding-left:1.1rem} li{padding:3px 0}
.alarm{background:var(--danger); color:#fff; font-weight:700; text-align:center;
  padding:14px; border-radius:14px; margin-bottom:14px}
.err{color:var(--danger); font-size:.9rem; margin-bottom:10px}
label{display:block; font-size:.85rem; color:var(--muted); margin:10px 0 4px}
input,select{width:100%; padding:12px; font-size:1rem; border:1px solid var(--line);
  border-radius:10px; background:var(--bg); color:var(--fg)}
input[type=file]{padding:8px}
.btn{display:block; width:100%; margin-top:18px; padding:14px; font-size:1rem; font-weight:600;
  color:#fff; background:var(--accent); border:0; border-radius:12px; cursor:pointer}
a{color:var(--accent)}
.hide{display:none}
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
<div class="wrap">
  <h1>BrauKnecht</h1>

  <div id="err" class="err"></div>
  <div id="alarm" class="alarm hide">RUFALARM</div>

  <div class="card">
    <h2 id="title">&hellip;</h2>
    <div class="temps">
      <div><div class="big"><span id="temp_ist">--</span>&deg;</div><div class="sub">Ist</div></div>
      <div><div class="big" style="font-size:2rem"><span id="temp_soll">--</span>&deg;</div><div class="sub">Soll</div></div>
    </div>
    <div class="row"><span>Heizung</span><span id="heizung" class="pill off">aus</span></div>
  </div>

  <div class="card hide" id="zweites">
    <h2 id="title2"></h2>
    <ul id="data"></ul>
  </div>

  <p style="text-align:center"><a href="/config">Einstellungen</a> &middot; <a href="/recipe">Rezept</a></p>
</div>

<script>
const $ = id => document.getElementById(id);
async function tick(){
  try{
    const d = await (await fetch('data.json')).json();
    $('title').textContent = d.title;
    $('temp_soll').textContent = Number(d.temp_soll).toFixed(0);
    $('temp_ist').textContent = d.regelung == 0 ? '--' : Number(d.temp_ist).toFixed(1);

    const h = $('heizung');
    h.textContent = d.heizung;
    h.className = 'pill ' + (d.heizung === 'an' ? 'on' : 'off');

    if (d.data && d.data.length > 0){
      $('title2').textContent = d.title2;
      $('data').innerHTML = d.data;
      $('zweites').classList.remove('hide');
    } else {
      $('zweites').classList.add('hide');
    }

    const alarm = d.rufmodus == 28 || d.modus == 18;
    $('alarm').classList.toggle('hide', !alarm);
    document.title = alarm ? '⚠ RUFALARM' : 'BrauKnecht';
    $('err').textContent = '';
  } catch(e){
    $('err').textContent = 'Keine Verbindung zum BrauKnecht';
  } finally {
    setTimeout(tick, 1000);  // chain, not setInterval: ESP8266 serves one client at a time
  }
}
tick();
</script>
</body></html>
)=====";
