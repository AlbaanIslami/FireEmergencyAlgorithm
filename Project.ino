/**
 * @author: Alban,Nezar on 2022-12-20.
 * @project: Project_group_2
 * @Configuration management: 
 * *Configure and connect WeMos D1 Mini with MCP23008 was written by Alban.
 * *Configure and connect WeMos D1 Mini with Max3485 was written by Alban.
 * *Configure and connect WeMos D1 Mini with RGB was written by Nezar.
 * *Configure and connect WeMos D1 Mini with OLED was written by Nezar.
 * @Communecation: The connection between nodes and servers was written by Alban
 * @Configuration and positioning Nodes was written by Alban.
 * @Nodes status (Fire, Normal, Exit) was written by Nezar.
 * @Evacuation was written by Nezar
 */
#include <SPI.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pu2clr_mcp23008.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#ifndef STASSID
#define STASSID "TN-ZP9518"
#define STAPSK  "8WrognijWatt"
#endif

/*__________________OLED___________________*/
#define OLED_RESET 0  // D3 OLED RESET CONTROL
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
Adafruit_SSD1306 display(OLED_RESET);
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*__________________END_OLED_____________________________*/

/*__________________WIFI_Connection_____________________________*/
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* host = "192.168.1.155";
const uint16_t port = 3000;
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

/*__________________END_WIFI_Connection_____________________________*/

/*__________________MCP_23008_____________________________*/
MCP mcp;
const int East = 0;//D0 //pin10 mcp23008
const int  West = 1;//D1 //pin11 mcp23008
const int  North = 2;//D2 //pin12 mcp23008
const int  South = 3;//D3 //pin13 mcp23008
const int serverLed = 4; //D4 //pin14 mcp23008

int EastIsConnected;
int WestIsConnected;
int NorthIsConnected;
int SouthIsConnected;

/*_________________END_MCP_23008_____________________________*/

/*__________________RGB_LED_____________________________*/
const int LEDPIN  = 14;  //D5
const int NUMPIXELS =  1;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_RGB + NEO_KHZ800);
/*_________________END_RGB_LED_____________________________*/

/*__________________ANALOG_MODE_____________________________*/
#define analog       A0
int sensorValue;
float voltage;

/*________________VARIABLS_____________________________*/
String receivedNodNumber = "";
bool isNodeExit = false;
bool isNodeInFire = false;
bool isNodeNormal = false;
String sendNumber = "";
char *nodeName = "";
bool initializeNodes = false;
bool checkEnd = false;
String lastNode1 = "";
String lastNode2 = "";
bool isThisLastNode = false;
String readStatus = "";
String readStatus1 = "";
String readStatus2 = "";
char *fireNumber = "";
String sendFireNumber = "";
String fireInNode = "";
String readExit = "";
String exitNumber_1 = "";
String exitNumber_2 = "";
bool fireMove = false;
String message = "";

/*____________________SERIAL___________________________*/

#define RXPin1      12  //D6 Serial Receive pin
#define TXPin1      2   //D4 Serial Transmit pin

#define RXPin2      13  //D7 Serial Receive pin
#define TXPin2      0   //D3 Serial Transmit pin

#define SERIAL_COMMUNICATION_CONTROL_PIN1 D0 //D0 Transmission set pin
#define RS485_TX_PIN1_VALUE HIGH
#define RS485_RX_PIN1_VALUE LOW
#define SERIAL_COMMUNICATION_CONTROL_PIN2 15 //D8 Transmission set pin
#define RS485_TX_PIN2_VALUE HIGH
#define RS485_RX_PIN2_VALUE LOW

SoftwareSerial RS485Serial1(RXPin1, TXPin1); // RX, TX
SoftwareSerial RS485Serial2(RXPin2, TXPin2); // RX, TX
/*___________________END_SERIAL___________________________*/

void setup()  {
  /*___________________________SERIAL_SETUP__________________________*/
  Serial.begin(9600);
  pinMode(SERIAL_COMMUNICATION_CONTROL_PIN1, OUTPUT); // initialize serial mode for MAX3485 NO.1
  pinMode(SERIAL_COMMUNICATION_CONTROL_PIN2, OUTPUT); // initialize serial mode for MAX3485 NO.2
  RS485Serial1.begin(9600);   // set the data rate
  RS485Serial2.begin(9600);   // set the data rate
  delay(1000);
  /*__________________________END_SERIAL_SETUP__________________________*/

  /*___________________________MCP_SETUP__________________________*/
  mcp.setup(0x20, 0B00001111); // 1 input, 0 Output - I²C Address 0x20, GPIO 0-3 are inputs and  4-7 are outputs
  mcp.setRegister(REG_GPPU, 0B00001111); // sets GPIO 0 to 3 with internal pull up resistors
  /*__________________________END_MCP_SETUP__________________________*/

  /*___________________________RGB_LED_SETUP__________________________*/
  pixels.begin();
  pixels.show();
  /*__________________________END_RGB_LED_SETUP__________________________*/

  /*___________________________OLED_SETUP__________________________*/
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  /*__________________________END_OLED_SETUP__________________________*/

  /*________________________WIFI_NETWORK_SETUP_________________________*/
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);
  /*_______________________END_WIFI_NETWORK_SETUP_________________________*/
}

void loop() {

  while (!initializeNodes) {
    initNods();             // Used to initialize all nodes numbers. line 205
  }

  while (!isThisLastNode) {
    readLastNode();         // Used to notify all nodes that the last node is initialized and ready to go. line 389
  }

  while (!checkEnd) {
    nodeStatus();           // Used to know evrery node status (Fire, Normal, Exit) and notify other nodes. line 420
  }
  while (!isNodeExit) {
    checkNodeExit();        // Used to notify all node which exit is posible (7 or 8). line 746
  }
  while (!isNodeInFire) {
    evacuation(exitNumber_1, exitNumber_2, fireInNode); // Used to evacuate agents and lead them to exit node. line 797 
  }
}

/*This function used to recogniz node 1 then setup other nodes by using check node function*/
void initNods() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }
  mcp.turnGpioOn(serverLed);
  EastIsConnected = mcp.gpioRead(East);
  WestIsConnected = mcp.gpioRead(West);
  NorthIsConnected = mcp.gpioRead(North);
  SouthIsConnected = mcp.gpioRead(South);

  if (EastIsConnected == HIGH && WestIsConnected == LOW
      && NorthIsConnected == LOW && SouthIsConnected == HIGH) {
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
    sendNumber = "1";
    delay(1000);
    RS485Serial2.print(sendNumber);
    Serial.println("NODE1");
    client.println("NODE1");
    client.println("Send Node Number: " + sendNumber);
    Serial.println("Send Node Number: " + sendNumber);
    initializeNodes = true;
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    nodeName = "NODE1";
    display.write(nodeName);
    display.display();
  } else {
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE);  // Disable RS485 Transmit and start receiving

    receivedNodNumber = RS485Serial1.readString();
  }

  checkNode();
  delay(1000);

}

/* This function used to check all nodes that not has a 'No1' and setup the nodes name and number*/
void checkNode() {
  /* Received 1 */
  if (receivedNodNumber == "1") {
    if (EastIsConnected == HIGH && WestIsConnected == HIGH
        && NorthIsConnected == LOW && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit
      sendNumber = "2";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE2");
      client.println("NODE2");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      delay(1000);
      nodeName = "NODE2";
      display.write(nodeName);
      display.display();
    }
    if (EastIsConnected == HIGH && WestIsConnected == LOW
        && NorthIsConnected == HIGH && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit
      sendNumber = "3";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE3");
      client.println("NODE3");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE ); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE);  // Read
      delay(1000);
      nodeName = "NODE3";
      display.write(nodeName);
      display.display();
    }
  }
  /* Received 2 */
  if (receivedNodNumber == "2") {
    if (EastIsConnected == HIGH && WestIsConnected == HIGH
        && NorthIsConnected == LOW && SouthIsConnected == LOW) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit
      sendNumber = "4";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE4");
      client.println("NODE4");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      delay(1000);
      nodeName = "NODE4";
      display.write(nodeName);
      display.display();
    }
    if (EastIsConnected == LOW && WestIsConnected == HIGH
        && NorthIsConnected == HIGH && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit
      sendNumber = "5";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE5");
      client.println("NODE5");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      delay(1000);
      nodeName = "NODE5";
      display.write(nodeName);
      display.display();
    }

  }

  /* Received 3 */
  if (receivedNodNumber == "3") {
    if (EastIsConnected == HIGH && WestIsConnected == LOW
        && NorthIsConnected == HIGH && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      sendNumber = "6";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE6");
      client.println("NODE6");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE ); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE);  // Read
      delay(1000);
      nodeName = "NODE6";
      display.write(nodeName);
      display.display();
    }
    if (EastIsConnected == LOW && WestIsConnected == HIGH
        && NorthIsConnected == HIGH && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      delay(1000);
      sendNumber = "8";
      delay(4000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE8");
      client.println("NODE8");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE ); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE);  // Read
      delay(1000);
      nodeName = "NODE8";
      display.write(nodeName);
      display.display();
    }
  }

  /* Received 4 */
  if (receivedNodNumber == "4") {
    if (EastIsConnected == HIGH && WestIsConnected == HIGH
        && NorthIsConnected == LOW && SouthIsConnected == HIGH) {
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      sendNumber = "7";
      delay(1000);
      RS485Serial2.print(sendNumber);
      Serial.println("NODE7");
      client.println("NODE7");
      Serial.println("Received Node Number: " + receivedNodNumber);
      client.println("Received Node Number: " + receivedNodNumber);
      initializeNodes = true;
      delay(1000);
      nodeName = "NODE7";
      display.write(nodeName);
      display.display();
    }
  }

}

// Used to notify all nodes that the last node is initialized and ready to go
void readLastNode() {
  if (sendNumber == "8") {
    lastNode1 = "8";
    lastNode();
  } else {
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    delay (1000);
    lastNode1 = RS485Serial1.readString();
    if (lastNode1 == "8") {
      Serial.println("Received Last Node Number: " + lastNode1);
      client.println("Received Last Node Number: " + lastNode1);
      lastNode();
    }
  }
}

// Used to renotify other nodes that the last node is initialized and ready to go
void lastNode() {
  if (lastNode1 == "8") {
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
    delay(1000);
    RS485Serial2.print("8");
    Serial.println("Send Last Node Number: " + lastNode1);
    client.println("Send Last Node Number: " + lastNode1);
    isThisLastNode = true;
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    delay(1000);
  }
}

/* This function used to setup the status for every node as (normal nodes, output nodes, fire nodes) */
void nodeStatus() {
  sensorValue = analogRead(analog);
  voltage = sensorValue * (3.2 / 1023);
  //Serial.println(voltage);
  delay(200);
  //Fire
  if (voltage > 3.0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("FIRE");
    display.display();
    pixels.setPixelColor(0, 255, 0, 0); //red
    pixels.show();
    delay(4000);
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Transmit
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
    sendFireNumber = sendNumber + "10";
    delay(3000);
    RS485Serial1.print(sendFireNumber);
    RS485Serial2.print(sendFireNumber);
    Serial.printf("Send Status FIRE For Node %s : %s", sendNumber, sendFireNumber);
    client.println("Send Status FIRE For Node " + sendNumber + ": " + sendFireNumber);
    checkEnd = true;
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    for (int i = 55; i > 0; i--) {
      delay(1000);
    }
    isNodeExit = true;
    isNodeInFire = true;
    display.clearDisplay();
    display.display();
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    Serial.println("Node : " + sendNumber + " is dead");
    client.println("Node : " + sendNumber + " is dead");
    mcp.turnGpioOff(serverLed);
  }
  //Normal
  if (voltage < 0.5) {
    isNodeNormal = true;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("NORM");
    display.display();
    pixels.setPixelColor(0, 0, 0, 255); //blue
    pixels.show();
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    readStatus1 = RS485Serial2.readString();
    delay(100);
    readStatus2 = RS485Serial1.readString();
    if (readStatus1 == "110" || readStatus2 == "110" || readStatus1 == "110110" || readStatus2 == "110110") {
      readStatus = "110";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      checkEnd = true;
      fireInNode = "1";
      Serial.flush();
      fireNumber = "NODE1";
      fireMessage();
    } else if (readStatus1 == "210" || readStatus2 == "210" || readStatus1 == "210210" || readStatus2 == "210210") {
      readStatus = "210";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      fireInNode = "2";
      Serial.flush();
      fireNumber = "NODE2";
      fireMessage();
    } else if (readStatus1 == "310" || readStatus2 == "310" || readStatus1 == "310310" || readStatus2 == "310310") {
      readStatus = "310";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      fireInNode = "3";
      Serial.flush();
      fireNumber = "NODE3";
      fireMessage();
    } else if (readStatus1 == "410" || readStatus2 == "410" || readStatus1 == "410410" || readStatus2 == "410410" || readStatus1 == "410410410" || readStatus2 == "410410410") {
      readStatus = "410";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      fireInNode = "4";
      Serial.flush();
      fireNumber = "NODE4";
      fireMessage();
    } else if (readStatus1 == "510" || readStatus2 == "510" || readStatus1 == "510510" || readStatus2 == "510510" || readStatus1 == "510510510" || readStatus2 == "510510510") {
      readStatus = "510";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      fireInNode = "5";
      Serial.flush();
      fireNumber = "NODE5";
      fireMessage();
    } else if (readStatus1 == "610" || readStatus2 == "610" || readStatus1 == "610610" || readStatus2 == "610610" || readStatus1 == "610610610" || readStatus2 == "610610610") {
      readStatus = "610";
      Serial.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      client.println("Received Status FIRE For Node " + sendNumber + ": " + readStatus);
      fireInNode = "6";
      Serial.flush();
      fireNumber = "NODE6";
      fireMessage();
    }
  }
  //Exit
  if (voltage > 1.0 && voltage < 2.5) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("EXIT");
    display.display();
    pixels.setPixelColor(0, 0, 255, 0); //green
    pixels.show();
    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
    readStatus1 = RS485Serial1.readString();
    if (readStatus1 == "110") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 1");
      client.println("Fire in Node 1");
      fireInNode = "1";
      Serial.flush();
      fireNumber = "NODE1";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      if (sendNumber == "7") {
        delay(4000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      if (sendNumber == "8") {
        delay(6000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    } else if (readStatus1 == "210") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 2");
      client.println("Fire in Node 2");
      fireInNode = "2";
      Serial.flush();
      fireNumber = "NODE2";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      delay(5000);
      RS485Serial2.print(sendNumber + 20);
      Serial.println("Send Exit code: " + sendNumber + 20);
      client.println("Send Exit code: " + sendNumber + 20);
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    } else if (readStatus1 == "310") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 3");
      client.println("Fire in Node 3");
      fireInNode = "3";
      Serial.flush();
      fireNumber = "NODE3";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      if (sendNumber == "7") {
        delay(5000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      if (sendNumber == "8") {
        delay(6000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    } else if (readStatus1 == "410") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 4");
      client.println("Fire in Node 4");
      fireInNode = "4";
      Serial.flush();
      fireNumber = "NODE4";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      if (sendNumber == "7") {
        delay(5000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      if (sendNumber == "8") {
        delay(6000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    } else if (readStatus1 == "510") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 5");
      client.println("Fire in Node 5");
      fireInNode = "5";
      Serial.flush();
      fireNumber = "NODE5";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      if (sendNumber == "7") {
        delay(2000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      if (sendNumber == "8") {
        delay(10000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    } else if (readStatus1 == "610") {
      Serial.println("Fire Code " + readStatus1);
      client.println("Fire Code " + readStatus1);
      Serial.println("Fire in Node 6");
      client.println("Fire in Node 6");
      fireInNode = "6";
      Serial.flush();
      fireNumber = "NODE6";
      checkEnd = true;
      delay(2000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(readStatus1);
      Serial.println("Resend Fire Code: " + readStatus1);
      client.println("Resend Fire Code: " + readStatus1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write("FIRE");
      display.setCursor(0, 20);
      display.write(fireNumber);
      display.display();
      checkEnd = true;
      if (sendNumber == "7") {
        delay(2000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      if (sendNumber == "8") {
        delay(7000);
        RS485Serial2.print(sendNumber + 20);
        Serial.println("Send Exit code: " + sendNumber + 20);
        client.println("Send Exit code: " + sendNumber + 20);
      }
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
    }
  }
}

// Used to notify all node which exit is posible (7 or 8)
void checkNodeExit() {
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
  String temp1 = "";
  String temp2 = "";
  temp1 = RS485Serial1.readString();
  temp2 = RS485Serial2.readString();
  if (temp1 == "720" || temp2 == "720") {
    readExit = "720";
    Serial.println("Received Exit code is Number: " + readExit);
    client.println("Received Exit code is Number: " + readExit);
  } else if (temp1 == "820" || temp2 == "820") {
    readExit = "820";
    Serial.println("Received Exit code is Number: " + readExit);
    client.println("Received Exit code is Number: " + readExit);
  }
  if (sendNumber == "4" || sendNumber == "5" || sendNumber == "7") {
    if (readExit == "720" || sendNumber == "7") {
      exitNumber_1 = "7";
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Transmit
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      delay(1000);
      RS485Serial1.print(readExit);
      RS485Serial2.print(readExit);
      Serial.println("Resend the code of Exit Node: " + readExit);
      isNodeExit = true;
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      Serial.println("Exit Node is Number: " + exitNumber_1);
      client.println("Exit Node is Number: " + exitNumber_1);
    }
  }
  if (sendNumber == "1" || sendNumber == "2" || sendNumber == "3" || sendNumber == "6" || sendNumber == "8") {
    if (readExit == "820" || sendNumber == "8") {
      exitNumber_2 = "8";
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Transmit
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      delay(2000);
      RS485Serial1.print(readExit);
      RS485Serial2.print(readExit);
      Serial.println("Resend the code of Exit Node: " + readExit);
      isNodeExit = true;
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      Serial.println("Exit Node is Number: " + exitNumber_2);
      client.println("Exit Node is Number: " + exitNumber_2);
    }
  }
}

// Used to evacuate agents and lead them to exit node by using the number of the node has a fire and exite numbers
void evacuation(String exitNumber_1, String exitNumber_2, String fireNumber) {
  Serial.println("Exit Node is Number: " + exitNumber_1 + " " + exitNumber_2 + " Fire Node is Number: " + fireNumber);
  client.println("Exit Node is Number: " + exitNumber_1 + " " + exitNumber_2 + " Fire Node is Number: " + fireNumber);
  Serial.println("Evacuation");
  client.println("Evacuation");
  
  // Exit Node number 7 and Fire Node number 1 
  if (exitNumber_1 == "7" && fireNumber == "1") {
    if (sendNumber == "4") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);;
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "5") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "7") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "410") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "410") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 4");
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;
    
    // Exit Node number 8 and Fire Node number 1 
  } else if (exitNumber_2 == "8" && fireNumber == "1") {
    if (sendNumber == "2") {
      Serial.println("Fire Code 110");
      client.println("Fire Code 110");
      client.println("Fire in Node 1");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "3" ) {
      Serial.println("Fire Code 110");
      client.println("Fire Code 110");
      client.println("Fire in Node 1");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "6") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "8") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          for (int i = 8; i > 0; i--) {
            pixels.setPixelColor(0, 255, 255, 255); //white
            pixels.show();
            delay(200);
            pixels.setPixelColor(0, 0, 0, 0);      //Black
            pixels.show();
            delay(1000);
          }
          evacuation();
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 22; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 7 and Fire Node number 2 
  } else if (exitNumber_1 == "7" && fireNumber == "2") {
    if (sendNumber == "4") {
      Serial.println("Fire Code 210");
      client.println("Fire Code 210");
      client.println("Fire in Node 2");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "5") {
      Serial.println("Fire Code 210");
      client.println("Fire Code 210");
      client.println("Fire in Node 2");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);;
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "7") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "410") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "410") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 4");
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 8 and Fire Node number 2 
  } else if (exitNumber_2 == "8" && fireNumber == "2") {
    if (sendNumber == "1") {
      Serial.println("Fire Code 210");
      client.println("Fire Code 210");
      client.println("Fire in Node 2");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "3") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 1");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write("NODE3");
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "6") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "8") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
        String temp = "";
        Serial.flush();
        temp = RS485Serial1.readString();
        temp = RS485Serial2.readString();
        if (temp != "") {
          readExit = temp;
        }
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code 510");
          client.println("Fire Code 510");
          client.println("Fire in Node 5");
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 22; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 7 and Fire Node number 3
  } else if (exitNumber_1 == "7" && fireNumber == "3") {
    if (sendNumber == "4") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "5") {
      Serial.println("Fire Code 310");
      client.println("Fire Code 310");
      client.println("Fire in Node 3");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "7") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "410") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "410") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 4");
          dangerousGreen();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 8 and Fire Node number 3 
  } else if (exitNumber_2 == "8" && fireNumber == "3") {
    if (sendNumber == "1") {
      Serial.println("Fire Code 310");
      client.println("Fire Code 310");
      client.println("Fire in Node 3");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "2") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 1");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "6") {
      Serial.println("Fire Code 310");
      client.println("Fire Code 310");
      client.println("Fire in Node 3");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "8") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
        String temp = "";
        Serial.flush();
        temp = RS485Serial1.readString();
        temp = RS485Serial2.readString();
        if (temp != "") {
          readExit = temp;
        }
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code 610");
          client.println("Fire Code 610");
          client.println("Fire in Node 6");
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 22; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 7 and Fire Node number 4
  } else if (exitNumber_1 == "7" && fireNumber == "4") {
    if (sendNumber == "5") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(readExit);
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "7") {
      Serial.println("Fire Code 410");
      client.println("Fire Code 410");
      client.println("Fire in Node 4");
      for (int i = 15; i > 0; i--) {
        pixels.setPixelColor(0, 255, 255, 255); //white
        pixels.show();
        delay(500);
        pixels.setPixelColor(0, 0, 0, 0);      //Black
        pixels.show();
        delay(500);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.write(nodeName);
        display.setCursor(0, 20);
        display.write("EVAC");
        display.display();
      }
      dangerousGreen();
      delay(1000);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    isNodeInFire = true;

    // Exit Node number 8 and Fire Node number 4 
  } else if (exitNumber_2 == "8" && fireNumber == "4") {
    if (sendNumber == "1") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "2") {
      Serial.println("Fire Code 410");
      client.println("Fire Code 410");
      client.println("Fire in Node 4");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Trasmit spicial
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit spicial
      RS485Serial1.print(sendNumber + 10);
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "3") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 1");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write("NODE3");
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "6") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "8") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
        String temp = "";
        Serial.flush();
        temp = RS485Serial1.readString();
        temp = RS485Serial2.readString();
        if (temp != "") {
          readExit = temp;
        }
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code 510");
          client.println("Fire Code 510");
          client.println("Fire in Node 5");
          dangerousGreen();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write("FIRE");
          display.setCursor(0, 20);
          display.write(nodeName);
          display.display();
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          for (int i = 22; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 7 and Fire Node number 5
  } else if (exitNumber_1 == "7" && fireNumber == "5") {
    if (sendNumber == "4") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "7") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "410") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "410") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 4");
          dangerousGreen();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 8 and Fire Node number 5
  } else if (exitNumber_2 == "8" && fireNumber == "5") {
    if (sendNumber == "1") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "2") {
      Serial.println("Fire Code 510");
      client.println("Fire Code 510");
      client.println("Fire in Node 5");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Trasmit spicial
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Trasmit spicial
      RS485Serial1.print(sendNumber + 10);
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "3" ) {
      Serial.println("Fire Code 510");
      client.println("Fire Code 510");
      client.println("Fire in Node 5");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "6") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 3");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "8" ) {
      Serial.println("Fire Code 510");
      client.println("Fire Code 510");
      client.println("Fire in Node 5");
      evacuation();
      dangerousGreen();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    isNodeInFire = true;

    // Exit Node number 7 and Fire Node number 6
  } else if (exitNumber_1 == "7" && fireNumber == "6") {
    if (sendNumber == "4") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "210") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "210") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 2");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "5") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
        readExit = RS485Serial2.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 6");
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(readExit);
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "7") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "410") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "410") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 4");
          dangerousGreen();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    isNodeInFire = true;

    // Exit Node number 8 and Fire Node number 6
  } else if (exitNumber_2 == "8" && fireNumber == "6") {
    if (sendNumber == "1") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "310") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "310") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          Serial.println("Fire in Node 3");
          client.println("Fire in Node 3");
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Transmit
          RS485Serial1.print("310");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "2") {
      while (!fireMove) {
        digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
        readExit = RS485Serial1.readString();
        if (readExit != "110") {
          pixels.setPixelColor(0, 255, 255, 255); //white
          pixels.show();
          delay(200);
          pixels.setPixelColor(0, 0, 0, 0);      //Black
          pixels.show();
          display.clearDisplay();
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("EVAC");
          display.display();
        }
        if (readExit == "110") {
          Serial.println("Fire Code " + readExit);
          client.println("Fire Code " + readExit);
          client.println("Fire in Node 1");
          dangerousBlue();
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.write(nodeName);
          display.setCursor(0, 20);
          display.write("FIRE");
          display.display();
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
          RS485Serial2.print(sendNumber + 10);
          Serial.println("The Node Number: " + sendNumber + " has Fire");
          client.println("The Node Number: " + sendNumber + " has Fire");
          Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
          for (int i = 10; i > 0; i--) {
            pixels.setPixelColor(0, 255, 0, 0);
            pixels.show();
            delay(1000);
          }
          digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
          for (int i = 25; i > 0; i--) {
            delay(1000);
          }
          display.clearDisplay();
          display.display();
          pixels.setPixelColor(0, 0, 0, 0);
          pixels.show();
          fireMove = true;
          Serial.println("Node : " + sendNumber + " is dead");
          client.println("Node : " + sendNumber + " is dead");
          mcp.turnGpioOff(serverLed);
        }
        isNodeInFire = true;
      }
    }
    if (sendNumber == "3" ) {
      Serial.println("Fire Code 610");
      client.println("Fire Code 610");
      client.println("Fire in Node 6");
      evacuation();
      dangerousBlue();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    if (sendNumber == "8" ) {
      Serial.println("Fire Code 610");
      client.println("Fire Code 610");
      client.println("Fire in Node 6");
      evacuation();
      dangerousGreen();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.write(nodeName);
      display.setCursor(0, 20);
      display.write("FIRE");
      display.display();
      delay(1000);
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
      RS485Serial2.print(sendNumber + 10);
      Serial.println("The Node Number: " + sendNumber + " has Fire");
      client.println("The Node Number: " + sendNumber + " has Fire");
      Serial.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      client.println("Send To Another Nodes Fire Code: " + sendNumber + 10);
      for (int i = 10; i > 0; i--) {
        pixels.setPixelColor(0, 255, 0, 0);
        pixels.show();
        delay(1000);
      }
      digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
      for (int i = 25; i > 0; i--) {
        delay(1000);
      }
      display.clearDisplay();
      display.display();
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.show();
      fireMove = true;
      Serial.println("Node : " + sendNumber + " is dead");
      client.println("Node : " + sendNumber + " is dead");
      mcp.turnGpioOff(serverLed);
    }
    isNodeInFire = true;
  }
  else {
    Serial.println("Error");
    isNodeInFire = true;
  }
}

/* This Method used by nodes has notified that there is a fire in one of nodes*/ 
void evacuation() {
  client.println("Evacuation");
  for (int i = 10; i > 0; i--) {
    pixels.setPixelColor(0, 255, 255, 255); //white
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);      //Black
    pixels.show();
    delay(500);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("EVAC");
    display.display();
  }
}

/* This method is used by normal nodes when a fire breaks out in a neighboring node */
void dangerousBlue() {
  int red = 0;
  int green = 0;
  int blue = 255;
  Serial.println("Dangerous");
  client.println("Dangerous");
  for (int i = 10; i > 0; i--) {
    pixels.setPixelColor(0, red, green, blue);
    pixels.show();
    delay(1000);
    red += 25.5;
    green += 13;
    blue -= 25.5;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("DANG");
    display.display();
  }
  for (int i = 10; i > 0; i--) {
    delay(1000);
  }
}

/*This method is used by the exit nodes when a fire breaks out in one of the neighboring nodes*/
void dangerousGreen() {
  int red = 0;
  int green = 255;
  int blue = 0;
  Serial.println("Dangerous");
  client.println("Dangerous");
  for (int i = 10; i > 0; i--) {
    pixels.setPixelColor(0, red, green, blue);
    pixels.show();
    delay(1000);
    red += 25.5;
    green -= 12;
    blue  = 0;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.write(nodeName);
    display.setCursor(0, 20);
    display.write("DANG");
    display.display();
  }
  for (int i = 10; i > 0; i--) {
    delay(1000);
  }
}

/* This Method used by the node has a fire and to inform other nodes about the fire*/
void fireMessage() {
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_TX_PIN1_VALUE); // Transmit
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_TX_PIN2_VALUE); // Transmit
  RS485Serial1.print(readStatus);
  RS485Serial2.print(readStatus);
  Serial.println("Send Status FIRE: " + readStatus);
  client.println("Send Status FIRE: " + readStatus);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.write("FIRE");
  display.setCursor(0, 20);
  display.write(fireNumber);
  display.display();
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN1, RS485_RX_PIN1_VALUE); // Read
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN2, RS485_RX_PIN2_VALUE); // Read
  checkEnd = true;
}
