#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

Preferences prefs;

HardwareSerial mySerial(2); // Use UART2
DFRobotDFPlayerMini player;
const char *ssid = "WelcomeRobot";
const char *password = "12345678";

WebServer server(80);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 110
#define SERVOMAX 495

int servoAngle[4] = {90, 90, 90, 90};

// Right side
// int RIGHT_SHOULDER_DOWN = 0;
// int RIGHT_SHOULDER_UP = 66;

// int RIGHT_ARM_DOWN = 0;
// int RIGHT_ARM_UP = 75;

// // Left side
// int LEFT_SHOULDER_DOWN = 171;
// // int LEFT_SHOULDER_UP = 90;
// int LEFT_SHOULDER_UP = 128; // test_

// int LEFT_ARM_DOWN = 0;
// int LEFT_ARM_UP = 90;

int RIGHT_SHOULDER_DOWN;
int RIGHT_SHOULDER_UP;

int RIGHT_ARM_DOWN;
int RIGHT_ARM_UP;

int LEFT_SHOULDER_DOWN;
int LEFT_SHOULDER_UP;

int LEFT_ARM_DOWN;
int LEFT_ARM_UP;

void loadConfig()
{
  prefs.begin("servo", true);

  RIGHT_SHOULDER_DOWN = prefs.getInt("rsd", 0);
  RIGHT_SHOULDER_UP = prefs.getInt("rsu", 66);

  RIGHT_ARM_DOWN = prefs.getInt("rad", 0);
  RIGHT_ARM_UP = prefs.getInt("rau", 75);

  LEFT_SHOULDER_DOWN = prefs.getInt("lsd", 171);
  LEFT_SHOULDER_UP = prefs.getInt("lsu", 128);

  LEFT_ARM_DOWN = prefs.getInt("lad", 0);
  LEFT_ARM_UP = prefs.getInt("lau", 90);

  prefs.end();
}
struct Step
{
  int s1, s2, s3, s4;
};

Step sequence[50];
int seqCount = 0;

bool playSequence = false;

uint16_t angleToPulse(int ang) { return map(ang, 0, 180, SERVOMIN, SERVOMAX); }

void moveServo(int ch, int ang) { pwm.setPWM(ch, 0, angleToPulse(ang)); }

void moveAll()
{
  for (int i = 0; i < 4; i++)
    moveServo(i, servoAngle[i]);
}

void checkSerial();

void moveTwoSmooth(int ch1, int start1, int end1, int ch2, int start2,
                   int end2)
{
  int steps = 40;

  for (int i = 0; i <= steps; i++)
  {
    int a1 = map(i, 0, steps, start1, end1);
    int a2 = map(i, 0, steps, start2, end2);

    moveServo(ch1, a1);
    delay(10);
    moveServo(ch2, a2);

    delay(20);
  }
}

void greetRaise()
{
  // 1️⃣ Arms up together
  moveTwoSmooth(1, RIGHT_ARM_DOWN, RIGHT_ARM_UP, 3, LEFT_ARM_DOWN, LEFT_ARM_UP);

  delay(500);

  // 2️⃣ Shoulders up together
  // moveTwoSmooth(0, RIGHT_SHOULDER_DOWN, RIGHT_SHOULDER_UP,
  //               2, LEFT_SHOULDER_DOWN, LEFT_SHOULDER_UP);
}

void greetLower()
{
  // 1️⃣ Shoulders down
  // moveTwoSmooth(0, RIGHT_SHOULDER_UP, RIGHT_SHOULDER_DOWN,
  //               2, LEFT_SHOULDER_UP, LEFT_SHOULDER_DOWN);
  delay(500);
  // 2️⃣ Arms down
  moveTwoSmooth(1, RIGHT_ARM_UP, RIGHT_ARM_DOWN, 3, LEFT_ARM_UP, LEFT_ARM_DOWN);
}

void greetRaiseOld()
{

  for (int i = 0; i <= RIGHT_SHOULDER_UP; i++)
  {
    moveServo(0, i);
    int leftPos =
        LEFT_SHOULDER_DOWN -
        ((LEFT_SHOULDER_DOWN - LEFT_SHOULDER_UP) * i / RIGHT_SHOULDER_UP);
    moveServo(3, leftPos);
    delay(15);
  }

  // Arm movement
  for (int i = 0; i <= RIGHT_ARM_UP; i++)
  {
    moveServo(1, i);
    int leftPos = (LEFT_ARM_UP * i / RIGHT_ARM_UP);
    moveServo(4, leftPos);
    delay(15);
  }
  moveServo(1, RIGHT_ARM_UP);
  delay(100);
  moveServo(4, LEFT_ARM_UP);
  delay(100);
  moveServo(0, RIGHT_SHOULDER_UP);
  delay(100);
  moveServo(3, LEFT_SHOULDER_UP);
}

void greetLowerOld()
{
  // for (int i = RIGHT_ARM_UP; i >= RIGHT_ARM_DOWN; i--)
  // {
  //   moveServo(1, i);
  //   int leftPos = (LEFT_ARM_UP * i / RIGHT_ARM_UP);
  //   moveServo(4, leftPos);
  //   delay(15);
  // }

  // for (int i = RIGHT_SHOULDER_UP; i >= RIGHT_SHOULDER_DOWN; i--)
  // {
  //   moveServo(0, i);
  //   int leftPos = LEFT_SHOULDER_DOWN - ((LEFT_SHOULDER_DOWN -
  //   LEFT_SHOULDER_UP) * i / RIGHT_SHOULDER_UP); moveServo(3, leftPos);
  //   delay(15);
  // }

  moveServo(0, RIGHT_SHOULDER_DOWN);
  moveServo(3, LEFT_SHOULDER_DOWN);
  delay(100);
  moveServo(1, RIGHT_ARM_DOWN);
  moveServo(4, LEFT_ARM_DOWN);
}
void root()
{

  String page = R"rawliteral(

<!DOCTYPE html>
<html>
<head>

<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body{
font-family:Arial;
background:#f2f2f2;
margin:10px;
}

.card{
background:white;
padding:15px;
margin-bottom:15px;
border-radius:10px;
box-shadow:0 2px 6px rgba(0,0,0,0.2);
}

h2{
text-align:center;
}

.slider{
width:100%;
}

.row{
display:flex;
gap:10px;
margin-top:5px;
}

input[type=number]{
width:80px;
padding:5px;
}

button{
padding:10px;
width:100%;
margin-top:10px;
font-size:16px;
border:none;
background:#007bff;
color:white;
border-radius:6px;
}

button.stop{
background:#e74c3c;
}

</style>

</head>

<body>

<h2>ESP32 Servo Controller</h2>

<div id="servos"></div>

<!--
<button onclick="saveStep()">Save Step</button>
<button onclick="start()">Start Loop</button>
<button class="stop" onclick="stop()">Stop</button>



 <button class="greet_sequence" onclick="greet_sequence()">Greet Sequence</button> -->
<button class="greet_raise" onclick="greet_raise()">Greet Raise</button>
<button class="greet_lower" onclick="greet_lower()">Greet Lower</button>

<script>

let servoCount=4
let servos=[]

function loadSettings(){

for(let i=0;i<servoCount;i++){

let min=localStorage.getItem("min"+i) || 0
let max=localStorage.getItem("max"+i) || 180
let val=localStorage.getItem("val"+i) || 90

servos.push({min:parseInt(min),max:parseInt(max),val:parseInt(val)})

}

}

function buildUI(){

let container=document.getElementById("servos")

for(let i=0;i<servoCount;i++){

let s=servos[i]

container.innerHTML+=`

<div class="card">

<h3>Servo ${i+1} : <span id="angle${i}">${s.val}</span>°</h3>

<input class="slider" type="range"
min="${s.min}"
max="${s.max}"
value="${s.val}"
id="slider${i}"
oninput="move(${i},this.value)">

<div class="row">
<label>Min</label>
<input type="number" value="${s.min}" onchange="setMin(${i},this.value)">
<label>Max</label>
<input type="number" value="${s.max}" onchange="setMax(${i},this.value)">
</div>

</div>
`

}

}

function move(id,val){

document.getElementById("angle"+id).innerText=val

localStorage.setItem("val"+id,val)

fetch(`/set?id=${id}&angle=${val}`)

}

function setMin(id,val){

localStorage.setItem("min"+id,val)
document.getElementById("slider"+id).min=val

}

function setMax(id,val){

localStorage.setItem("max"+id,val)
document.getElementById("slider"+id).max=val

}

function saveStep(){
fetch("/save")
}

function start(){
fetch("/start")
}

function stop(){
fetch("/stop")
}

function greet_sequence(){
fetch("/greet_sequence")
}
function greet_raise(){
fetch("/greet_raise")
}
function greet_lower(){
fetch("/greet_lower")
}


loadSettings()
buildUI()

</script>

</body>
</html>

)rawliteral";

  server.send(200, "text/html", page);
}
void setServo()
{

  int id = server.arg("id").toInt();
  int ang = server.arg("angle").toInt();

  servoAngle[id] = ang;

  moveServo(id, ang);

  server.send(200, "text/plain", "ok");
}

void saveStep()
{

  sequence[seqCount].s1 = servoAngle[0];
  sequence[seqCount].s2 = servoAngle[1];
  sequence[seqCount].s3 = servoAngle[2];
  sequence[seqCount].s4 = servoAngle[3];

  seqCount++;

  server.send(200, "text/plain", "saved");
}

void startSeq()
{
  playSequence = true;
  server.send(200, "text/plain", "start");
}

void stopSeq()
{
  playSequence = false;
  server.send(200, "text/plain", "stop");
}

void greet_sequence()
{
  greetRaise();
  delay(2000);
  greetLower();
  server.send(200, "text/plain", "greeted");
}

void greet_raise()
{
  player.play(1); // play the first track on SD card
  greetRaise();
  server.send(200, "text/plain", "raised");
}
void greet_lower()
{
  player.play(2); // play the second track on SD Card
  greetLower();
  server.send(200, "text/plain", "lowered");
}

void saveConfig()
{
  prefs.begin("servo", false);

  prefs.putInt("rsd", RIGHT_SHOULDER_DOWN);
  prefs.putInt("rsu", RIGHT_SHOULDER_UP);

  prefs.putInt("rad", RIGHT_ARM_DOWN);
  prefs.putInt("rau", RIGHT_ARM_UP);

  prefs.putInt("lsd", LEFT_SHOULDER_DOWN);
  prefs.putInt("lsu", LEFT_SHOULDER_UP);

  prefs.putInt("lad", LEFT_ARM_DOWN);
  prefs.putInt("lau", LEFT_ARM_UP);

  prefs.end();
}

void configPage();
void saveConfigHandler();
void setup()
{

  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  loadConfig();
  Serial.println(WiFi.localIP());

  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  player.begin(mySerial);
  player.volume(30); // volume 0-30
  // if (!player.begin(mySerial))
  // {
  //   Serial.println("DFPlayer not detected");
  //   while (true)
  //     ;
  // }
  pwm.begin();
  pwm.setPWMFreq(50);

  server.on("/", root);
  server.on("/config", configPage);
  server.on("/saveConfig", saveConfigHandler);
  server.on("/set", setServo);
  server.on("/save", saveStep);
  server.on("/start", startSeq);
  server.on("/stop", stopSeq);
  server.on("/greet_sequence", greet_sequence);
  server.on("/play_audiio", []()
            {
    player.play(1); // play the first track on SD card
    server.send(200, "text/plain", "playing"); });

  server.on("/greet_raise", greet_raise);
  server.on("/greet_lower", greet_lower);

  server.begin();
}
void loop()
{
  server.handleClient();

  checkSerial();

  // playServoSequence();
}

void checkSerial()
{
  if (Serial.available())
  {
    char cmd = Serial.read();

    Serial.print("Received: ");
    Serial.println(cmd);

    if (cmd == 'g')
    {
      player.play(1);
      greetRaise();
    }
    else if (cmd == 'l')
    {
      player.play(2);
      greetLower();
    }
    // else if (cmd == "play_seq")
    // {
    //   playSequence = true;
    // }
    // else if (cmd == "stop_seq")
    // {
    //   playSequence = false;
    // }
  }
}

void playServoSequence()
{
  static unsigned long lastMove = 0;
  static int index = 0;

  if (!playSequence || seqCount == 0)
    return;

  if (millis() - lastMove > 1000)
  {
    lastMove = millis();

    moveServo(0, sequence[index].s1);
    moveServo(1, sequence[index].s2);
    moveServo(2, sequence[index].s3);
    moveServo(3, sequence[index].s4);

    index++;

    if (index >= seqCount)
    {
      index = 0;
    }
  }
}

void configPage()
{
  String page = R"rawliteral(

<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:Arial;padding:20px;background:#f2f2f2}
input{width:100%;padding:10px;margin:5px 0}
button{padding:10px;width:100%;background:#007bff;color:white;border:none}
.card{background:white;padding:15px;border-radius:8px}
</style>
</head>

<body>

<div class="card">

<h3>Servo Calibration</h3>

<label>RIGHT_SHOULDER_DOWN</label>
<input id="rsd" value="%RSD%">

<label>RIGHT_SHOULDER_UP</label>
<input id="rsu" value="%RSU%">

<label>RIGHT_ARM_DOWN</label>
<input id="rad" value="%RAD%">

<label>RIGHT_ARM_UP</label>
<input id="rau" value="%RAU%">

<label>LEFT_SHOULDER_DOWN</label>
<input id="lsd" value="%LSD%">

<label>LEFT_SHOULDER_UP</label>
<input id="lsu" value="%LSU%">

<label>LEFT_ARM_DOWN</label>
<input id="lad" value="%LAD%">

<label>LEFT_ARM_UP</label>
<input id="lau" value="%LAU%">

<button onclick="save()">Save</button>

</div>

<script>

function save(){

let url="/saveConfig?"
+"rsd="+rsd.value
+"&rsu="+rsu.value
+"&rad="+rad.value
+"&rau="+rau.value
+"&lsd="+lsd.value
+"&lsu="+lsu.value
+"&lad="+lad.value
+"&lau="+lau.value

fetch(url).then(r=>alert("Saved"))

}

</script>

</body>
</html>

)rawliteral";

  page.replace("%RSD%", String(RIGHT_SHOULDER_DOWN));
  page.replace("%RSU%", String(RIGHT_SHOULDER_UP));
  page.replace("%RAD%", String(RIGHT_ARM_DOWN));
  page.replace("%RAU%", String(RIGHT_ARM_UP));

  page.replace("%LSD%", String(LEFT_SHOULDER_DOWN));
  page.replace("%LSU%", String(LEFT_SHOULDER_UP));
  page.replace("%LAD%", String(LEFT_ARM_DOWN));
  page.replace("%LAU%", String(LEFT_ARM_UP));

  server.send(200, "text/html", page);
}

void saveConfigHandler()
{
  RIGHT_SHOULDER_DOWN = server.arg("rsd").toInt();
  RIGHT_SHOULDER_UP = server.arg("rsu").toInt();

  RIGHT_ARM_DOWN = server.arg("rad").toInt();
  RIGHT_ARM_UP = server.arg("rau").toInt();

  LEFT_SHOULDER_DOWN = server.arg("lsd").toInt();
  LEFT_SHOULDER_UP = server.arg("lsu").toInt();

  LEFT_ARM_DOWN = server.arg("lad").toInt();
  LEFT_ARM_UP = server.arg("lau").toInt();

  saveConfig();

  server.send(200, "text/plain", "saved");
}