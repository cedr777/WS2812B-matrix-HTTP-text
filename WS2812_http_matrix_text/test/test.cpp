/*
   ESP8266 (NodeMCU) Handling Web form data basic example
   <a href="https://circuits4you.com">https://circuits4you.com</a>
*/
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <sstream>
#include <string>

//WiFi related
#ifndef STASSID
#define STASSID "your-SSID"
#define STAPSK  "your-pw"
#endif

#define LEDstatus D0

const char* ssid = STASSID;
const char* password = STAPSK;


//
// WS2812 Setup
//

//ws2812 data pin
#define PIN D4
String showtext;
int lenghtpixel;
//text on startup is repeated infinity
String infrepeat = "on";
//initial color (green)
int mcolor = 2017;
//"speed" in ms delay before next pixel is drawn
int speed = 20;
//initial data on webpage
String webspeed = String("80");
String webrepeat = String("10");
String webcolor = "#00FF00";

int pass = 0;
int repeat;

//setup the matrix: 35 pixels x 8 columns starting from top left in zigzag rows with 800kHz bitstream
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(35, 8, PIN,
                            NEO_MATRIX_TOP    + NEO_MATRIX_LEFT +
                            NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);


int x = matrix.width();

/*
const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255), matrix.Color(255, 255, 255), matrix.Color(255, 100, 0)
};
*/



//
//Webpage
//
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body bgcolor="#303030">
  <font color="white">
  <h2>WS2812 control Panel</h2>
    <form action="/action_page">
    <h3>Color:</h3>
    <input type="color" name="webcolor" value="?color?">
    <br>
    <h3>Text:</h3>
    <input type="text" name="webtext" value="?text?">
    <br>
    <h3>Repeats:</h3>
    <input type="range" name="repeat" min="1" max="1000" value="?repeat?" oninput="this.form.repeatnumber.value=this.value" />
    <br>
    <input type="number" name="repeatnumber" min="1" max="1000" value="?repeat?" oninput="this.form.repeat.value=this.value" />
    <br>
    Infinity repeats?
    <input type="checkbox" name="infrepeat" checked>
    <br>
    <h3>Speed:</h3>
    <input type="range" name="speed" min="1" max="100" value="?speed?" oninput="this.form.speednumber.value=this.value" />
    <br>
    <input type="number" name="speednumber" min="1" max="100" value="?speed?" oninput="this.form.speed.value=this.value" />
    <br>
    <br>
    <br>
    <input type="submit" value="Submit">
    </fieldset>
</form>
</font>
</body>
</html>
)=====";


ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page;
 if (infrepeat == "on")
 {
   s.replace("<input type=\"checkbox\" name=\"infrepeat\">", "<input type=\"checkbox\" name=\"infrepeat\" checked>");
 }
 else
 {
   s.replace("<input type=\"checkbox\" name=\"infrepeat\" checked>", "<input type=\"checkbox\" name=\"infrepeat\">");
 }
 s.replace("?text?", showtext);
 s.replace("?repeat?", webrepeat);
 s.replace("?speed?", webspeed);
 s.replace("?color?", webcolor);
 server.send(200, "text/html", s); //Send web page
}


//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
  //get the data from client input and calculate to needed format
  showtext = server.arg("webtext");
  lenghtpixel = showtext.length() * 6;
  repeat = server.arg("repeat").toInt();
  infrepeat = server.arg("infrepeat");
  speed = map(server.arg("speed").toInt(), 1, 100, 100, 1);
  webcolor = server.arg("webcolor");
  int colors = (int) strtol( &webcolor[1], NULL, 16);
  int r = colors >> 16;
  int g = (colors >> 8) & 0xFF;
  int b = colors & 0xFF;
  mcolor = matrix.Color(r, g, b);

  //variables for storing the last state for the webpage
  webspeed = String(server.arg("speed"));
  webrepeat = server.arg("repeat");
Serial.println(repeat);
  Serial.println(webrepeat);


  //reset cursor and pass
  x = matrix.width();
  pass = 0;

  //debugging
  /*
  Serial.println("Text from webform: ");
  Serial.println(showtext);
  Serial.println("colors alltogether:");
  Serial.println(colors);
  Serial.println("r int from webform: ");
  Serial.println(r);
  Serial.println("g int from webform: ");
  Serial.println(g);
  Serial.println("b int from webform: ");
  Serial.println(b);
  Serial.println("parsed matrix color:");
  Serial.println(mcolor);
  Serial.println("Repeats from webform: ");
  Serial.println(repeat);
  Serial.println("Infinity repeats? ");
  Serial.println(infrepeat);
  Serial.println("Speed from webform:");
  Serial.println(server.arg("speed"));
  Serial.println("Speed calculated to delay time:");
  Serial.println(speed);
  */
  String s = MAIN_page;
  if (infrepeat == "on")
  {
    s.replace("<input type=\"checkbox\" name=\"infrepeat\">", "<input type=\"checkbox\" name=\"infrepeat\" checked>");
  }
  else
  {
    s.replace("<input type=\"checkbox\" name=\"infrepeat\" checked>", "<input type=\"checkbox\" name=\"infrepeat\">");
  }
  s.replace("?text?", showtext);
  s.replace("?repeat?", webrepeat);
  s.replace("?speed?", webspeed);
  s.replace("?color?", webcolor);
  Serial.println(s);
  server.send(200, "text/html", s); //Send web page
}



//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  pinMode(LEDstatus, OUTPUT);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LEDstatus, HIGH);
    delay(500);
    digitalWrite(LEDstatus, LOW);
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println("WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    showtext = WiFi.localIP().toString();
    lenghtpixel = showtext.length() * 6;
    for (int i = 0; i < 10; i++)
    {
      digitalWrite(LEDstatus, HIGH);
      delay(50);
      digitalWrite(LEDstatus, LOW);
      delay(50);
    }
  }

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(10);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready for OTA");
}

void loop(void)
{
  server.handleClient();
  ArduinoOTA.handle();
  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.setTextColor(mcolor);
  matrix.print(showtext);
  if(--x < -lenghtpixel
  ) {
    x = matrix.width();

  if(infrepeat != "on")
  {
    if(++pass >= repeat)
    {
      showtext = "";
    }
  }

  }
  matrix.show();
  digitalWrite(LEDstatus, HIGH);
  delay(speed);
  digitalWrite(LEDstatus, LOW);
}
