/**
   A trivial C/MRI -> JMRI interface
   =================================
   Sets up pin 13 (LED) as an output, and attaches it to the first output bit
   of the emulated SMINI interface.

   To set up in JMRI:
   1: Create a new connection,
      - type = C/MRI,
      - connection = Serial,
      - port = <arduino's port>,
      - speed = 9600
   2: Click 'Configure C/MRI nodes' and create a new SMINI node
   3: Click 'Add Node' and then 'Done'
   4: Restart J/MRI and it should say "Serial: using Serial on COM<x>" - congratulations!
   5: Open Tools > Tables > Lights and click 'Add'
   6: Add a new light at hardware address 1, then click 'Create' and close the window. Ignore the save message.
   7: Click the 'Off' state button to turn the LED on. Congratulations!

   Debugging:
   Open the CMRI > CMRI Monitor window to check what is getting sent.
   With 'Show raw data' turned on the output looks like:
      [41 54 01 00 00 00 00 00]  Transmit ua=0 OB=1 0 0 0 0 0

   0x41 = 65 = A = address 0
   0x54 = 84 = T = transmit, i.e. PC -> C/MRI
   0x01 = 0b00000001 = turn on the 1st bit
   0x00 = 0b00000000 = all other bits off
*/

#include "CMRI.h"
#include <WiFi.h>

#define TCP_PORT 9007
#define LED_BUILTIN 2

const char* ssid = "ssid";
const char* password =  "password";

WiFiServer wifiServer(TCP_PORT);
WiFiClient jmriClient;

CMRI cmri(1, 64, 64, jmriClient); // node number, number of inputs, number of outputs, strean client

void setup() {
  Serial.begin(115200); //Just for debug console feedback, not CMRI connection
  delay(1000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());

  wifiServer.begin();
  delay(1000);

  InitialiseConfig();
}

void loop() {
  // 1: main processing node of cmri library
  
  waitForJMRI();

  UpdateSensors();

  cmri.process();


  // 2: update output. Reads bit 0 of T packet and sets the LED to this
  int led = cmri.get_bit(0);
  //Serial.println("LED state "+String(led));
  if (led == 0) {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(12, LOW);
  }

  else {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(12, HIGH);
  }
}

bool waitForJMRI() {
  bool jmriConnected = jmriClient.connected();
  while (!jmriConnected) {
    jmriClient = wifiServer.available();
    if (jmriClient && jmriClient.connected()) {
      jmriConnected = true;
      Serial.println("JMRI Connected");
    }
  }
  return true;
}

void InitialiseConfig() {
  pinMode(23, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(12, OUTPUT);
}

void UpdateSensors() {
  cmri.set_bit(0, digitalRead(23));
}
