#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "ROOPCHAND";
const char* password = "SH06DA75";

WebServer server(80);

// Motor Pins
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENA 33
#define ENB 32

// Speed (0-255)
int speedValue = 50;

// Function Prototypes
void stopMotors();
void forward();
void backward();
void left();
void right();
void handleRoot();

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/forward", left);
  server.on("/backward", right);
  server.on("/left", forward);
  server.on("/right", backward);
  server.on("/stop", stopMotors);
  server.begin();
}

void loop() {
  server.handleClient();
}

// ─────────────────────────────────────────────
//  WEB PAGE
// ─────────────────────────────────────────────
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>RC Control</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Rajdhani:wght@400;600;700&family=Share+Tech+Mono&display=swap');

  :root {
    --bg: #0a0a0c;
    --panel: #111116;
    --border: #1e1e28;
    --accent: #00e5ff;
    --accent2: #ff3d71;
    --glow: rgba(0,229,255,0.18);
    --text: #e0e8ff;
    --muted: #4a5070;
  }

  * { margin:0; padding:0; box-sizing:border-box; -webkit-tap-highlight-color:transparent; }

  body {
    background: var(--bg);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    font-family: 'Rajdhani', sans-serif;
    color: var(--text);
    overflow: hidden;
  }

  /* Scanline overlay */
  body::before {
    content: '';
    position: fixed;
    inset: 0;
    background: repeating-linear-gradient(
      0deg,
      transparent,
      transparent 2px,
      rgba(0,0,0,0.08) 2px,
      rgba(0,0,0,0.08) 4px
    );
    pointer-events: none;
    z-index: 100;
  }

  .container {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 20px;
    padding: 24px 16px;
    width: 100%;
    max-width: 400px;
  }

  /* Header */
  .header {
    text-align: center;
    letter-spacing: 6px;
    font-size: 11px;
    color: var(--muted);
    text-transform: uppercase;
    font-family: 'Share Tech Mono', monospace;
  }
  .header span {
    color: var(--accent);
    animation: blink 1.4s step-end infinite;
  }
  @keyframes blink { 50% { opacity: 0; } }

  /* Status bar */
  .statusbar {
    width: 100%;
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 10px 16px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    font-family: 'Share Tech Mono', monospace;
    font-size: 13px;
  }
  .status-label { color: var(--muted); font-size: 10px; letter-spacing: 2px; }
  .status-value {
    color: var(--accent);
    font-size: 14px;
    font-weight: 600;
    transition: all 0.2s;
    letter-spacing: 1px;
  }
  .status-value.danger { color: var(--accent2); }
  .wifi-dot {
    width: 7px; height: 7px;
    border-radius: 50%;
    background: #22c55e;
    box-shadow: 0 0 8px #22c55e;
  }

  /* D-Pad */
  .dpad-wrapper {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 28px;
    position: relative;
  }
  .dpad-wrapper::before {
    content: '';
    position: absolute;
    inset: -1px;
    border-radius: 20px;
    background: linear-gradient(135deg, rgba(0,229,255,0.08), transparent 60%);
    pointer-events: none;
  }

  .dpad {
    display: grid;
    grid-template-columns: repeat(3, 84px);
    grid-template-rows: repeat(3, 84px);
    gap: 8px;
  }

  .btn {
    width: 84px;
    height: 84px;
    border-radius: 16px;
    border: 1px solid var(--border);
    background: #14141a;
    color: var(--text);
    font-size: 13px;
    font-family: 'Rajdhani', sans-serif;
    font-weight: 600;
    letter-spacing: 1px;
    cursor: pointer;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 5px;
    transition: background 0.1s, border-color 0.1s, transform 0.08s, box-shadow 0.1s;
    outline: none;
    position: relative;
    overflow: hidden;
  }

  .btn svg {
    width: 22px;
    height: 22px;
    fill: var(--muted);
    transition: fill 0.1s;
  }

  .btn span {
    font-size: 10px;
    color: var(--muted);
    letter-spacing: 2px;
    transition: color 0.1s;
  }

  .btn:active, .btn.active {
    background: rgba(0,229,255,0.08);
    border-color: var(--accent);
    box-shadow: 0 0 20px var(--glow), inset 0 0 12px rgba(0,229,255,0.05);
    transform: scale(0.94);
  }
  .btn:active svg, .btn.active svg { fill: var(--accent); }
  .btn:active span, .btn.active span { color: var(--accent); }

  .btn.stop-btn:active, .btn.stop-btn.active {
    background: rgba(255,61,113,0.08);
    border-color: var(--accent2);
    box-shadow: 0 0 20px rgba(255,61,113,0.2), inset 0 0 12px rgba(255,61,113,0.05);
  }
  .btn.stop-btn:active svg, .btn.stop-btn.active svg { fill: var(--accent2); }
  .btn.stop-btn:active span, .btn.stop-btn.active span { color: var(--accent2); }

  .btn.stop-btn svg { fill: #3a2030; }

  .cell-empty { visibility: hidden; }

  /* Corner decorations */
  .corner {
    display: flex;
    align-items: center;
    justify-content: center;
    opacity: 0.25;
  }
  .corner svg { width: 20px; height: 20px; fill: var(--accent); }

  /* Speed slider */
  .speed-panel {
    width: 100%;
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 16px 20px;
  }
  .speed-header {
    display: flex;
    justify-content: space-between;
    margin-bottom: 12px;
    font-size: 11px;
    letter-spacing: 2px;
    color: var(--muted);
    font-family: 'Share Tech Mono', monospace;
  }
  .speed-header strong {
    color: var(--accent);
    font-size: 13px;
  }
  input[type=range] {
    -webkit-appearance: none;
    width: 100%;
    height: 4px;
    background: var(--border);
    border-radius: 2px;
    outline: none;
  }
  input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: var(--accent);
    box-shadow: 0 0 10px var(--glow);
    cursor: pointer;
  }
  .speed-labels {
    display: flex;
    justify-content: space-between;
    margin-top: 8px;
    font-size: 10px;
    color: var(--muted);
    font-family: 'Share Tech Mono', monospace;
    letter-spacing: 1px;
  }
</style>
</head>
<body>
<div class="container">

  <div class="header">ESP32 &nbsp;<span>&#9632;</span>&nbsp; RC CONTROL</div>

  <div class="statusbar">
    <div>
      <div class="status-label">STATUS</div>
      <div class="status-value" id="statusVal">STANDBY</div>
    </div>
    <div style="display:flex;align-items:center;gap:8px;">
      <div class="wifi-dot"></div>
      <span style="font-size:11px;color:var(--muted);letter-spacing:1px;">ONLINE</span>
    </div>
  </div>

  <div class="dpad-wrapper">
    <div class="dpad">

      <!-- Row 1 -->
      <div class="cell-empty"></div>
      <button class="btn" id="btn-fwd" ontouchstart="go('/forward',this)" ontouchend="release(this)" onmousedown="go('/forward',this)" onmouseup="release(this)">
        <svg viewBox="0 0 24 24"><path d="M12 4l8 10H4z"/></svg>
        <span>FWD</span>
      </button>
      <div class="cell-empty"></div>

      <!-- Row 2 -->
      <button class="btn" id="btn-left" ontouchstart="go('/left',this)" ontouchend="release(this)" onmousedown="go('/left',this)" onmouseup="release(this)">
        <svg viewBox="0 0 24 24"><path d="M20 12H4l8-8zM4 12l8 8z" transform="rotate(90,12,12)"/><path d="M4 12l8-8v16z"/></svg>
        <span>LEFT</span>
      </button>
      <button class="btn stop-btn" id="btn-stop" ontouchstart="go('/stop',this)" ontouchend="release(this)" onmousedown="go('/stop',this)" onmouseup="release(this)">
        <svg viewBox="0 0 24 24"><rect x="5" y="5" width="14" height="14" rx="2"/></svg>
        <span>STOP</span>
      </button>
      <button class="btn" id="btn-right" ontouchstart="go('/right',this)" ontouchend="release(this)" onmousedown="go('/right',this)" onmouseup="release(this)">
        <svg viewBox="0 0 24 24"><path d="M20 12l-8-8v16z"/></svg>
        <span>RIGHT</span>
      </button>

      <!-- Row 3 -->
      <div class="cell-empty"></div>
      <button class="btn" id="btn-bwd" ontouchstart="go('/backward',this)" ontouchend="release(this)" onmousedown="go('/backward',this)" onmouseup="release(this)">
        <svg viewBox="0 0 24 24"><path d="M12 20l8-10H4z"/></svg>
        <span>REV</span>
      </button>
      <div class="cell-empty"></div>

    </div>
  </div>

  <div class="speed-panel">
    <div class="speed-header">
      <span>THROTTLE</span>
      <strong id="speedLabel">59%</strong>
    </div>
    <input type="range" min="30" max="255" value="150" id="speedSlider" oninput="updateSpeed(this.value)">
    <div class="speed-labels"><span>MIN</span><span>MAX</span></div>
  </div>

</div>

<script>
  const statusEl = document.getElementById('statusVal');
  const labels = {
    '/forward': 'FORWARD',
    '/backward': 'REVERSE',
    '/left': 'LEFT TURN',
    '/right': 'RIGHT TURN',
    '/stop': 'STOPPED'
  };

  let activeBtn = null;

  function go(path, btn) {
    if (activeBtn && activeBtn !== btn) {
      activeBtn.classList.remove('active');
    }
    activeBtn = btn;
    btn.classList.add('active');
    statusEl.textContent = labels[path];
    statusEl.className = 'status-value' + (path === '/stop' ? ' danger' : '');
    fetch(path).catch(() => { statusEl.textContent = 'ERROR'; });
  }

  function release(btn) {
    if (btn.id !== 'btn-stop') {
      btn.classList.remove('active');
      fetch('/stop').catch(() => {});
      statusEl.textContent = 'STANDBY';
      statusEl.className = 'status-value';
    }
  }

  function updateSpeed(val) {
    document.getElementById('speedLabel').textContent = Math.round(val / 255 * 100) + '%';
    fetch('/speed?v=' + val).catch(() => {});
  }

  // Keyboard support
  const keyMap = {
    'ArrowUp': 'btn-fwd',
    'ArrowDown': 'btn-bwd',
    'ArrowLeft': 'btn-left',
    'ArrowRight': 'btn-right',
    ' ': 'btn-stop'
  };
  const held = {};
  document.addEventListener('keydown', e => {
    const id = keyMap[e.key];
    if (!id || held[e.key]) return;
    e.preventDefault();
    held[e.key] = true;
    const btn = document.getElementById(id);
    const path = '/' + { 'btn-fwd':'forward','btn-bwd':'backward','btn-left':'left','btn-right':'right','btn-stop':'stop' }[id];
    go(path, btn);
  });
  document.addEventListener('keyup', e => {
    const id = keyMap[e.key];
    if (!id) return;
    held[e.key] = false;
    release(document.getElementById(id));
  });
</script>
</body></html>
)rawliteral";
  server.send(200, "text/html", html);
}

// ─────────────────────────────────────────────
//  MOTOR FUNCTIONS
// ─────────────────────────────────────────────
void forward() {
  ledcWrite(ENA, speedValue);
  ledcWrite(ENB, speedValue);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  server.send(200, "text/plain", "OK");
}

void backward() {
  ledcWrite(ENA, speedValue);
  ledcWrite(ENB, speedValue);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  server.send(200, "text/plain", "OK");
}

void left() {
  ledcWrite(ENA, speedValue);
  ledcWrite(ENB, speedValue);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  server.send(200, "text/plain", "OK");
}

void right() {
  ledcWrite(ENA, speedValue);
  ledcWrite(ENB, speedValue);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  server.send(200, "text/plain", "OK");
}

void stopMotors() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  server.send(200, "text/plain", "OK");
}
