/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Arduino.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#include <Adafruit_Fingerprint.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Adafruit_SSD1306.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

//__OLED__
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t id;
String IDname;
uint8_t usedId[] = {1, 2, 3, 4};
int lengthOfUsedId = sizeof(usedId) / sizeof(usedId[0]);
boolean idIsUsed;

#include <SD.h>
#define SS_PIN 5 //CHIP SELECT
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18 // clock
File myFile;
void saveDataToSDcard(uint8_t ident, String idn)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("/idList.txt", "a");
  // if the file opened okay, write to it:
  if (myFile)
  {
    Serial.print("Writing to accessData.txt...");
    myFile.println(String(id) + ";" + IDname);

    // close the file:
    myFile.close();
    Serial.println("done.");

    // open the file for reading:
    myFile = SD.open("/idList.txt");
    if (myFile)
    {
      Serial.println("READING...");
      Serial.println("idList.txt:");
      // read from the file until there's nothing else in it:
      while (myFile.available())
      {
        Serial.write(myFile.read());
      }
      // close the file:
      myFile.close();
    }
  }
  else
  {
    Serial.println("error opening file");
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // For Yun/Leo/Micro/Zero/...
  delay(100);
  //OLED display setup
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //init DS card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SS_PIN))
  {
    Serial.println("initialization failed!");
    while (1)
      ;
  }
  Serial.println("initialization done.");
  //init fingerprint enroll
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Adafruit Fingerprint sensor enrollment");
  display.display();
  delay(2000);

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
    {
      delay(1);
    }
  }

  Serial.println(F("Reading sensor parameters"));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Reading sensor parameters");
  display.display();
  delay(2000);
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Done.");
  display.display();
  delay(2000);
}

uint8_t readnumber(void)
{
  uint8_t num = 0;

  while (num == 0)
  {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

String readName(void)
{
  String name = "none";

  while (name == "none")
  {
    while (!Serial.available())
      ;
    name = Serial.readString();
  }
  return name;
}

uint8_t getFingerprintEnroll()
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Waiting for valid finger to enroll as #");
  display.setCursor(7, 20);
  display.println(id);
  display.display();
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Image taken");
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println(".");
      display.display();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Communication error");
      display.display();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Imaging error");
      display.display();
      break;
    default:
      Serial.println("Unknown error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Unknown error");
      display.display();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Image converted");
    display.display();
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Image too messy");
    display.display();
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Communication error");
    display.display();
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Could not find fingerprint features");
    display.display();
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Could not find fingerprint features");
    display.display();
    return p;
  default:
    Serial.println("Unknown error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Unknown error");
    display.display();
    return p;
  }

  Serial.println("Remove finger");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Remove finger");
  display.display();
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.print("ID ");
  display.println(id);
  display.display();
  p = -1;
  Serial.println("Place same finger again");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Place same finger again");
  display.display();
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Image taken");
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Communication error");
      display.display();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Imaging error");
      display.display();
      break;
    default:
      Serial.println("Unknown error");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(7, 5);
      display.println("Unknown error");
      display.display();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Image converted");
    display.display();
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Image too messy");
    display.display();
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Communication error");
    display.display();
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Could not find fingerprint features");
    display.display();
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Could not find fingerprint features");
    display.display();
    return p;
  default:
    Serial.println("Unknown error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Unknown error");
    display.display();
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.print("Creating model for #");
  display.println(id);
  display.display();

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Prints matched!");
    display.display();
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Communication error");
    display.display();
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Fingerprints did not match");
    display.display();
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Unknown error");
    display.display();
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.print("ID ");
  display.println(id);
  display.display();

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.print("Finally, what is your name?");
    Serial.print("Finally, what is your name?");
    IDname = readName();
    display.display();
    delay(2000);
    saveDataToSDcard(id, IDname);
    Serial.println("Stored!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Stored!");
    display.display();
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Communication error");
    display.display();
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Could not store in that location");
    display.display();
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Error writing to flash");
    display.display();
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(7, 5);
    display.println("Unknown error");
    display.display();
    return p;
  }

  return true;
}

void loop() // run over and over again
{
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.println("Please type in the ID # (from 1 to 127)");
  display.display();
  id = readnumber();
  do
  {
    for (uint8_t i = 0; i < lengthOfUsedId; i++)
    {
      if (id == usedId[i])
      {
        idIsUsed = true;
        Serial.println("The ID you enered is already in use. Please try again.");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 5);
        display.println("The ID you enered is already in use. Please try again.");
        display.display();
        id = readnumber();
      }
      else
      {
        idIsUsed = false;
      }
    }
  } while (idIsUsed == true);
  if (id == 0)
  { // ID #0 not allowed, try again!
    Serial.println("0 is not allowed. Please try again.");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 5);
    display.println("0 is not allowed. Please try again.");
    display.display();
    return;
  }
  usedId[lengthOfUsedId] = id;
  Serial.print("Enrolling ID #");
  Serial.println(id);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(7, 5);
  display.println("Enrolling ID #");
  display.setCursor(7, 20);
  display.println(id);
  display.display();

  while (!getFingerprintEnroll())
    ;
}
