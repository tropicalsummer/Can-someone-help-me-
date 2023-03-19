#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#include <FirebaseESP8266.h>
#endif
                                                                                                                                                    
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define RST_PIN         5 //
#define SS_PIN          4 //
#define MQ3PIN A0

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;          //create a MIFARE_Key struct named 'key', which will hold the card information

//-----------------------------------------------------------------------------------------------
const String mobileNumber = "+639567006233";
//const String mobileNumber = "+639207672750";
unsigned long lastBTTime = 0;
unsigned long USER_BTDelay = 1000;
const int ALCOHOL_LEVEL = 100;
//-----------------------------------------------------------------------------------------------
//char c = ' ';
long mq3Value=0;
bool isDrunk = false;
String rfid = "";
bool readsuccess;
//================================================================================================
SoftwareSerial SIM800(2,3); //RX,TX if arduino arduino.RX<=gsm.TX, arduino.TX->10K->gsm.RX, arduino.GND->20K->gsm.RX 
SoftwareSerial BTSerial(4, 5);
// ---------------------------------------------------------------------------------------------------

// Define LCD pinout
const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;

// Define I2C Address - change if reqiuired
const int i2c_addr = 0x27;

LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "B2NT-T01BUg"
#define WIFI_PASSWORD "jayson15"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyCOOgpTOYwx4ddINAQabKcMMoxJJ1QQqYQ"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://liquordetector-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setupGSM();
void checkGSM();
void setupRFID();
bool checkRFID();

void sendSMS(String receiver, String msg);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  pinMode(MQ3PIN, INPUT);
  setupGSM();
  //setupBT();
  setupRFID();    

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  //auth.user.email = USER_EMAIL;
  //auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    //signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  /** Timeout options.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */
  delay(1000);

  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Welcome to");
  lcd.setCursor(0,1);
  lcd.print("Liquor Detector");
  delay(2000);
  lcd.clear();  
  
}

void loop() {
  //checkGSM();
  Serial.println("waiting...");
  readsuccess = checkRFID();

  if(readsuccess){
    Serial.print("RFID =>  "); 
    Serial.println(rfid);
  }

   return;
   
//  mq3Value = analogRead(MQ3PIN);
//  float adcValue=0;
//  for(int i=0;i<10;i++)
//  {
//    adcValue+= analogRead(MQ3PIN);
//    delay(10);
//  }
//  float v= (adcValue/10) * (5.0/2200.0);
//  float mgL= 0.67 * v;
//  Serial.print("BAC:");
//  Serial.print(mgL);
//  Serial.print(" mg/L");
//  lcd.setCursor(0,0);
//  lcd.print("BAC: ");
//  lcd.print(mgL,4);
//  lcd.print(" mg/L        ");
//  lcd.setCursor(0,1);
//
//  if(mgL > 0.7 && mgL < 0.8) {
//      lcd.print("Tipsy   ");
//      Serial.println("    Tipsy");
//      //notify app here via firebase
//  }  else if(mgL > 0.8) { 
//      lcd.print("Drunk   ");
//      Serial.println("    Drunk");
//      if (!isDrunk) {
//        if (mq3Value > ALCOHOL_LEVEL) {
//          isDrunk = true;
//          sendSMS(mobileNumber, "Student ID 0 is drunk!");
//        }
//      }
//  } else {
//      lcd.print("Normal  ");
//      Serial.println("    Normal");
//  }

//// Firebase.ready() should be called repeatedly to handle authentication tasks.
//
//  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
//  {
//    sendDataPrevMillis = millis();
//    Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, F("/sensor/gas/double"), mq3Value) ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, F("/sensor/gas/double")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
//
//    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/test/int"), count) ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/test/int")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
//    int iVal = 0;
//    Serial.printf("Get int ref... %s\n", Firebase.getInt(fbdo, F("/test/int"), &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());
//    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/test/float"), count + 10.2) ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get float... %s\n", Firebase.getFloat(fbdo, F("/test/float")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());
//    Serial.printf("Set double... %s\n", Firebase.setDouble(fbdo, F("/test/double"), count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get double... %s\n", Firebase.getDouble(fbdo, F("/test/double")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/test/string"), "Hello Madia!") ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/rfid/string"), "ACBAT5W1") ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/rfid/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/rfid/student/string"), "ACBAT5W1") ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/rfid/student/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/rfid/teacher/string"), "ACBAT5W1") ? "ok" : fbdo.errorReason().c_str());
//    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/rfid/teacher/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
//    Serial.println();
//
//    // For generic set/get functions.
//
//    // For generic set, use Firebase.set(fbdo, <path>, <any variable or value>)
//
//    // For generic get, use Firebase.get(fbdo, <path>).
//    // And check its type with fbdo.dataType() or fbdo.dataTypeEnum() and
//    // cast the value from it e.g. fbdo.to<int>(), fbdo.to<std::string>().
//
//    // The function, fbdo.dataType() returns types String e.g. string, boolean,
//    // int, float, double, json, array, blob, file and null.
//
//    // The function, fbdo.dataTypeEnum() returns type enum (number) e.g. fb_esp_rtdb_data_type_null (1),
//    // fb_esp_rtdb_data_type_integer, fb_esp_rtdb_data_type_float, fb_esp_rtdb_data_type_double,
//    // fb_esp_rtdb_data_type_boolean, fb_esp_rtdb_data_type_string, fb_esp_rtdb_data_type_json,
//    // fb_esp_rtdb_data_type_array, fb_esp_rtdb_data_type_blob, and fb_esp_rtdb_data_type_file (10)
//
//    count++;
//  }
     
//checkBT(); 
// delay(500);
//   if ((millis() - lastBTTime) > USER_BTDelay) {
//      lastBTTime = millis();
//      Serial.println("Sending BT data");
//      BTSerial.println(mq3Value);  
//   }

} 

void setupGSM()
{
  //Begin serial communication with Arduino and SIM800L
  SIM800.begin(9600);
  
  Serial.println(" ");
  Serial.println("Initializing...");
  delay(1000);
  Serial.println("GSM modem ready!");

  SIM800.println("AT"); 
  checkGSM();

  SIM800.println("AT+CREG?"); //Check whether it has registered in the network
  checkGSM();

  SIM800.println("AT+CSCS=\"GSM\""); 
  checkGSM();

  SIM800.println("AT+CMGF=1"); 
  checkGSM();
  SIM800.println("AT+CNMI=2,2,0,1,1"); 
  checkGSM();

}

void sendSMS(String receiver, String msg)
{
  if(receiver.length() == 0) {
    //do nothing
  }
  else {
    checkGSM();
    SIM800.println("AT+CMGF=1"); // Configuring TEXT mode
    checkGSM();
    SIM800.println("AT+CMGS=\"" + receiver + "\""); //me
    checkGSM();
    SIM800.print(msg); //text content
    checkGSM();
    SIM800.write(26);
    delay(3000); // new
  }
  
}
void checkGSM()
{
  delay(500);
  while (Serial.available()) 
  {
    SIM800.write(Serial.read());//Forward what Serial received to Software Serial Port
  }

  String cmd;
  
  SIM800.listen();
  while(SIM800.available()) 
  {
    cmd += (char)SIM800.peek(); //take a look but not retrieve message from the serial buffer
    Serial.write(SIM800.read());//Forward what Software Serial received to Serial Port
  }
  
  if (cmd.length() > 0) {
     Serial.println(cmd);
  } else {
        Serial.println("Checking SMS");
  }
}

void setupBT() 
{
  BTSerial.begin(9600);
  delay(1000);
  Serial.println("Bluetooth ready");
}

//void checkBT() 
//{
//  Serial.println("Checking BT");
//  String cmd;
//  BTSerial.listen();
//  if (BTSerial.available()) {
//      c = BTSerial.read();
//      Serial.write(c);
//      cmd = (char)c;
//      Serial.println(cmd);
//  }
//  if (Serial.available()) {
//      c = Serial.read();
//      Serial.print("BTData => ");
//      Serial.print(c);
//      Serial.println(" ");
//  }  
//}

void setupRFID() {
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

//  // Prepare the security key for the read and write functions.
//  for (byte i = 0; i < 6; i++) {
//    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
//  }

}

bool checkRFID(){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return false;
    }
  // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
      return false;
    }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Message :");
  content.toUpperCase();
  Serial.print(content);
  Serial.println(" ");
  String x = String(content);
  rfid = content;
  delay(100);
  mfrc522.PICC_HaltA();
  return true;
}
