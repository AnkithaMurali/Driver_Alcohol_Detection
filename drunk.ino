#include <MQUnifiedsensor.h>
#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>



#define FONA_RX 8
#define FONA_TX 9
#define FONA_RST 4
int doubtpin = 11;
int relay = 13;

#define         Board                   ("Arduino NANO")
#define         Pin                     (A3)  
#define         Type                    ("MQ-3") 
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) 
#define         RatioMQ4CleanAir        (4.4)

MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

char replybuffer[255];

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {
  Serial.begin(9600);
  pinMode(doubtpin, INPUT);
  pinMode(13, OUTPUT);

  // Init the sensor
  float ratioInCleanAir = MQ3.calibrate(1.0); // You may need to adjust the argument based on your calibration process

  Serial.print("R0 preconfigured: ");
  Serial.println(MQ3.getR0());
  Serial.print("R0 clean air: ");
  Serial.println(ratioInCleanAir);

  fonaSerial->begin(4800);
  if (!fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  Serial.println(F("FONA is OK"));

  // Print SIM card IMEI number.
  char imei[16] = {0}; // MUST use a 16-character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: ");
    Serial.println(imei);
  }

  fonaSerial->print("AT+CNMI=2,1\r\n");
  Serial.println("FONA Ready");
  char fonaNotificationBuffer[64]; // for notifications from the FONA
  char smsBuffer[250];              // set up the FONA to send a +CMTI notification when an SMS is received
}

void loop() {
  // Read the sensor and print in the serial port
  int lecture = MQ3.readSensor("", true); // Return Alcohol concentration

  int gL = lecture / 1000;
  Serial.print("Alcohol ppm measured: ");
  Serial.print(lecture);
  Serial.println("ppm");

  Serial.print("Alcohol gL measured: ");
  Serial.print(gL);
  Serial.println("g/L");

  if (digitalRead(doubtpin) == HIGH) {
    Serial.println("Greater");
    digitalWrite(13, HIGH);

    if (!fona.sendSMS("7979952235", "Hey,This Car Number is driving while drunk")) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("Sent!"));
      delay(4000);
    }
  } else {
    digitalWrite(13, HIGH);
    delay(300);
  }
}