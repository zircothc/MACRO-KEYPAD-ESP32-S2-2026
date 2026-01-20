// ==========================================
// LOLIN S2 MINI MACRO KEYPAD
// ZCARLOS 2026
// ==========================================

// ==========================================
// LIBRERIAS
// ==========================================
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <Keypad.h>
#include <DNSServer.h>

// ==========================================
// CONFIGURACIÓN HARDWARE
// ==========================================
#define NUM_LEDS 3 
#define LED_PIN 16

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ==========================================
// TECLADO USB Y KEYPAD
// ==========================================
USBHIDKeyboard Keyboard;

const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[COLS] = {34, 36, 38, 40};
byte colPins[ROWS] = {1, 2, 4, 6};

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
const char keycodes[16] = {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};

// ==========================================
// VARIABLES GLOBALES
// ==========================================
bool modoConfig = true; 
bool solicitarCambioUSB = false; 
unsigned long tiempoCambio = 0;
String wifiPass = "12345678"; 

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

String macros[16]; 

// ==========================================
// HELPER LEDS
// ==========================================
void ponerColor(uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0; i<NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// ==========================================
// HTML (RESTAURADO CON TODOS LOS COMANDOS)
// ==========================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Lolin Config</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; background-color: #222; color: #fff; padding-bottom: 50px; }
    h2 { margin-top: 10px; color: #04AA6D; }
    input[type=text], input[type=password] { width: 75%; padding: 4px; margin: 1px; border-radius: 4px; border: 1px solid #555; background: #333; color: #fff; font-size: 16px; }
    .btn { background-color: #04AA6D; color: white; padding: 15px 30px; margin: 20px 0; border: none; cursor: pointer; width: 90%; font-size: 18px; border-radius: 5px; font-weight: bold; }
    .btn:hover { opacity: 0.8; }
    .row { display: flex; align-items: center; justify-content: center; }
    label { width: 30px; font-weight: bold; font-size: 20px; color: #ddd; }
    #status { display: none; padding: 15px; margin: 10px auto; width: 85%; border-radius: 5px; font-weight: bold; font-size: 18px; }
    .help-box { background: #333; margin: 10px auto; width: 90%; padding: 10px; border-radius: 5px; font-size: 12px; text-align: left; }
    .help-box h3 { margin: 0 0 5px 0; font-size: 14px; color: #aaa; border-bottom: 1px solid #555; padding-bottom: 5px; }
    .tag { color: #f39c12; font-weight: bold; font-family: monospace; }
    .desc { color: #ccc; margin-left: 5px; }
    .help-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 5px; }
    .wifi-box { border-bottom: 1px solid #555; padding-bottom: 5px; margin-bottom: 5px; }
    .wifi-label { width: auto; font-size: 16px; margin-right: 10px; color: #04AA6D; }
    .show-pass { font-size: 14px; color: #aaa; margin-bottom: 15px; display: flex; justify-content: center; align-items: center; }
    .show-pass input { width: auto; margin-right: 5px; }
  </style>
</head>
<body>
  <h2>Configuraci&oacute;n</h2>
  <div id="status"></div>
  
  <div class="row wifi-box">
    <label class="wifi-label">Pass WiFi:</label>
    <input type="password" id="wifi_pass" placeholder="M&iacute;nimo 8 caracteres">
  </div>
  <div class="show-pass">
    <input type="checkbox" onclick="togglePass()"> Mostrar contrase&ntilde;a
  </div>

  <div class="help-box">
    <h3>Comandos:</h3>
    <div class="help-grid">
      <div><span class="tag">[ENTER]</span><span class="desc">Intro</span></div>
      <div><span class="tag">[TAB]</span><span class="desc">Tab</span></div>
      <div><span class="tag">[ESC]</span><span class="desc">Esc</span></div>
      <div><span class="tag">[BACKSPACE]</span><span class="desc">Borrar</span></div>
      <div><span class="tag">[WIN]</span><span class="desc">Windows</span></div>
      <div><span class="tag">[CTRL]</span><span class="desc">Control</span></div>
      <div><span class="tag">[ALT]</span><span class="desc">Alt</span></div>
      <div><span class="tag">[SHIFT]</span><span class="desc">Mayus</span></div>
      <div><span class="tag">[DELAY5]</span><span class="desc">0.5s</span></div>
    </div>
  </div>
  <div id="container"></div>
  <button onclick="guardarDatos()" class="btn">GUARDAR TODO</button>
  <script>
    const keys = ['1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'];
    let html = "";
    for(let i=0; i<16; i++) {
      html += '<div class="row"><label>' + keys[i] + '</label>';
      html += '<input type="text" id="key' + i + '" placeholder="Vac&iacute;o"></div>';
    }
    document.getElementById("container").innerHTML = html;
    
    function togglePass() {
      var x = document.getElementById("wifi_pass");
      if (x.type === "password") { x.type = "text"; } else { x.type = "password"; }
    }

    fetch('/getValues').then(res => res.json()).then(data => {
      for(let i=0; i<16; i++) {
        if(data["key"+i]) document.getElementById("key"+i).value = data["key"+i];
      }
      if(data["wifi_pass"]) document.getElementById("wifi_pass").value = data["wifi_pass"];
    });

    function guardarDatos() {
      const st = document.getElementById("status");
      st.style.display = "block";
      st.style.backgroundColor = "#e67e22";
      st.innerHTML = "Guardando...";
      let fd = new FormData();
      for(let i=0; i<16; i++) fd.append("key"+i, document.getElementById("key"+i).value);
      fd.append("wifi_pass", document.getElementById("wifi_pass").value);
      fetch('/save', { method: 'POST', body: fd })
      .then(res => {
        if (res.ok) { st.style.backgroundColor = "#04AA6D"; st.innerHTML = "GUARDADO. Reiniciando..."; } 
        else { st.style.backgroundColor = "#f44336"; st.innerHTML = "Error al guardar."; }
      }).catch(e => { st.style.backgroundColor = "#04AA6D"; st.innerHTML = "ENVIADO. Reiniciando..."; });
    }
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// FUNCIÓN CRÍTICA: ENVÍO RAW HID
// ==========================================
void enviarRaw(uint8_t tecla, uint8_t modifiers) {
    KeyReport report = {0};
    report.modifiers = modifiers; // 8=Win, 1=Ctrl, 2=Shift, 4=Alt
    report.keys[0] = tecla;
    Keyboard.sendReport(&report);
    delay(20); 
    Keyboard.releaseAll();
    delay(20);
}

// Convierte ASCII a HID Usage ID
uint8_t getHIDCode(char c) {
  if (c >= 'a' && c <= 'z') return c - 93; 
  if (c >= 'A' && c <= 'Z') return c - 61; 
  if (c >= '1' && c <= '9') return c - 19; 
  if (c == '0') return 39;
  return 0;
}

void escribirSimboloSimple(char c) {
    if (c == '#') { Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('3'); Keyboard.releaseAll(); return; }
    if (c == '"') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('2'); Keyboard.releaseAll(); return; }
    if (c == '*') { enviarRaw(48, 2); return; } 
    if (c == '&') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('6'); Keyboard.releaseAll(); return; }
    if (c == '/') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('7'); Keyboard.releaseAll(); return; }
    if (c == '(') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('8'); Keyboard.releaseAll(); return; }
    if (c == ')') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('9'); Keyboard.releaseAll(); return; }
    if (c == '=') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('0'); Keyboard.releaseAll(); return; }
    if (c == '?') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write(45); Keyboard.releaseAll(); return; }
    if (c == '-') { Keyboard.write('/'); return; } 
    if (c == '_') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('/'); Keyboard.releaseAll(); return; }
    if (c == '@') { Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('2'); Keyboard.releaseAll(); return; }
    if (c == '\\') { Keyboard.press(KEY_RIGHT_ALT); Keyboard.write('`'); Keyboard.releaseAll(); return; } 
    if (c == ':') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('.'); Keyboard.releaseAll(); return; }
    if (c == ';') { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write(','); Keyboard.releaseAll(); return; } 
    if (c == '\'') { Keyboard.write(45); return; } 
    Keyboard.print(c);
}

// ==========================================
// EJECUTAR MACRO (MODIFICADORES RESTAURADOS)
// ==========================================
void ejecutarMacro(String texto) {
  Keyboard.releaseAll();
  delay(50);

  int len = texto.length();

  for (int i = 0; i < len; i++) {
    char c = texto.charAt(i);
    
    // 1. COMANDOS [XXX]
    if (c == '[') {
      String tag = "";
      i++; 
      while (i < len && texto.charAt(i) != ']') { tag += texto.charAt(i); i++; }
      tag.toUpperCase();

      if (tag == "ENTER") Keyboard.write(KEY_RETURN);
      else if (tag == "TAB") Keyboard.write(KEY_TAB);
      else if (tag == "ESC") Keyboard.write(KEY_ESC);
      else if (tag == "BACKSPACE" || tag == "BKSP") Keyboard.write(KEY_BACKSPACE);
      
      // === GESTIÓN DE MODIFICADORES (WIN, CTRL, ALT, SHIFT) ===
      else if (tag == "WIN" || tag == "GUI" || tag == "CTRL" || tag == "ALT" || tag == "SHIFT") {
        
        uint8_t modBit = 0;
        uint8_t keyDef = 0;

        // Definir qué bit de modificador activar
        if (tag == "WIN" || tag == "GUI") modBit = 8;
        else if (tag == "CTRL") modBit = 1;
        else if (tag == "SHIFT") modBit = 2;
        else if (tag == "ALT") modBit = 4;

        // Mirar si hay letra siguiente para hacer combo
        char nextChar = (i + 1 < len) ? texto.charAt(i + 1) : 0;
        uint8_t hidCode = getHIDCode(nextChar);
        
        if (hidCode != 0) {
            // ENVÍO ATÓMICO: Modificador + Código Tecla
            enviarRaw(hidCode, modBit); 
            i++; // Saltamos la letra
        } else {
            // Solo tecla modificadora (pulsar y soltar)
            // Mapeo manual porque enviarRaw(0, modBit) puede no actuar como pulsación suelta
            if (modBit == 8) keyDef = KEY_LEFT_GUI;
            if (modBit == 1) keyDef = KEY_LEFT_CTRL;
            if (modBit == 2) keyDef = KEY_LEFT_SHIFT;
            if (modBit == 4) keyDef = KEY_LEFT_ALT;
            
            Keyboard.press(keyDef);
            delay(100);
            Keyboard.releaseAll();
        }
      }
      
      else if (tag.startsWith("DELAY")) {
        String num = tag.substring(5);
        int d = num.toInt();
        if (d > 0) delay(d * 100); 
      }
    } 
    
    // 2. CORRECCIONES MENOR/MAYOR
    else if (c == '<') { enviarRaw(100, 0); }
    else if (c == '>') { enviarRaw(100, 2); }
    
    // 3. UTF-8
    else if (c == (char)0xC3) { 
       char next = texto.charAt(i+1); i++; 
       if (next == (char)0xA1) { Keyboard.print('\''); Keyboard.print('a'); } 
       else if (next == (char)0xB3) { Keyboard.print('\''); Keyboard.print('o'); } 
       else if (next == (char)0xA9) { Keyboard.print('\''); Keyboard.print('e'); } 
       else if (next == (char)0xAD) { Keyboard.print('\''); Keyboard.print('i'); } 
       else if (next == (char)0xBA) { Keyboard.print('\''); Keyboard.print('u'); } 
       else if (next == (char)0xB1) { Keyboard.print(';'); } 
       else if (next == (char)0x91) { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(';'); Keyboard.releaseAll(); } 
    }
    // 4. RESTO NORMAL
    else {
      escribirSimboloSimple(c);
      delay(20);
    }
  }
  Keyboard.releaseAll();
}

// ==========================================
// GESTIÓN DE MODO
// ==========================================
void activarModoUSB() {
  Serial.println("MODO USB ACTIVO");
  modoConfig = false;
  solicitarCambioUSB = false;
  dnsServer.stop();
  server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  USB.usbClass(0); USB.usbSubClass(0); USB.usbProtocol(0);
  Keyboard.begin();
  USB.begin();
  ponerColor(0, 255, 0); // VERDE
}

void cargarConfiguracion() {
  if (LittleFS.exists("/config.json")) {
    File file = LittleFS.open("/config.json", "r");
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, file);
    file.close();
    for(int i=0; i<16; i++) {
      String k = "key" + String(i);
      if(doc.containsKey(k)) macros[i] = doc[k].as<String>();
      else macros[i] = ""; 
    }
    if(doc.containsKey("wifi_pass")) {
      wifiPass = doc["wifi_pass"].as<String>();
      if(wifiPass.length() < 8) wifiPass = "12345678"; 
    }
  }
}

void handleSave() {
  DynamicJsonDocument doc(4096);
  for(int i=0; i<16; i++) {
    String p = "key" + String(i);
    if (server.hasArg(p)) { doc[p] = server.arg(p); macros[i] = server.arg(p); }
  }
  if (server.hasArg("wifi_pass")) {
     String wp = server.arg("wifi_pass");
     if(wp.length() >= 8) { wifiPass = wp; doc["wifi_pass"] = wp; } 
     else { doc["wifi_pass"] = wifiPass; }
  } else { doc["wifi_pass"] = wifiPass; }

  File file = LittleFS.open("/config.json", "w");
  serializeJson(doc, file);
  file.close();
  server.send(200, "text/plain", "OK");
  solicitarCambioUSB = true;
  tiempoCambio = millis();
}

void handleRoot() { server.send(200, "text/html", index_html); }
void handleGetValues() {
  String json = "{";
  for(int i=0; i<16; i++) json += "\"key" + String(i) + "\":\"" + macros[i] + "\",";
  json += "\"wifi_pass\":\"" + wifiPass + "\"}";
  server.send(200, "application/json", json);
}

void setup(){
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(50);
  ponerColor(0, 0, 0);

  if(!LittleFS.begin(true)){ Serial.println("Fallo LittleFS"); return; }
  cargarConfiguracion();

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.print("Iniciando AP con pass: "); Serial.println(wifiPass);
  WiFi.softAP("Lolin_Keypad", wifiPass.c_str());
  
  dnsServer.start(DNS_PORT, "*", apIP);
  ponerColor(255, 0, 0); // ROJO

  server.on("/", handleRoot);
  server.on("/getValues", handleGetValues);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() { server.send(200, "text/html", index_html); });
  server.begin();
}

void loop(){
  char customKey = customKeypad.getKey();
  if (modoConfig) {
    dnsServer.processNextRequest();
    server.handleClient();
    if (solicitarCambioUSB) { if (millis() - tiempoCambio > 2000) activarModoUSB(); }
    if (customKey) activarModoUSB();
  } else {
    if (customKey) {
      int index = -1;
      for(int i=0; i<16; i++) { if(keycodes[i] == customKey) { index = i; break; } }
      if (index != -1) {
        ponerColor(0, 0, 255); // AZUL
        ejecutarMacro(macros[index]);
        delay(100);
        ponerColor(0, 255, 0); // VERDE
      }
    }
  }
}
