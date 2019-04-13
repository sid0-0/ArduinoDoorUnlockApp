#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <Keypad.h>

/*
 * Current arrangement: 
 * Pins         Instrument                    Orientation/Wire colour
 * D2-D9        Keypad                        * towards D9, D towards D2
 * D10          Fingerprint Output            Yellow + Yellow
 * D11          Fingerprint Input             White+Green
 * D12          Servo input                   Orange+Blue
 * 
 * Servo Motor power      3.3V        Red+Orange
 * Servo Motor ground     0V          Brown+Yellow 
 * Fingerprint power      5V          Red+Red
 * Fingerprint ground     0V          Black+Brown
 */

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
bool passwordSet=true;
char password[4]={'1','2','3','4'};
//bool passwordSet=false;
//char password[4];
char input[4];
//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3

//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
int i=0;

int servoPin = 12;
Servo Servo1;
int servoLockAngle=0;
int servoUnlockAngle=90;
//for Servo 0 is locked and 90 unlocked

char keypressed='a';
unsigned long T=0;

SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint16_t id;
uint8_t p;

//*****************************************************************************
//Funtion to change password - Triggered by 'C'

void setNewPassword(){
  Serial.print("Enter a new password");
        i=0;
        while(i<4){
          keypressed=myKeypad.getKey();
          if(keypressed!=NO_KEY){
            Serial.print(keypressed);
            password[i++]=keypressed;
          }
        }
        i=0;
        Servo1.write(servoLockAngle);
        Serial.print("Password is: ");
        Serial.print(password);
        passwordSet=true;
  }

//*********************************************************************************

//enroll copied from examples

int addFingerprint(){
  p = -1;
  id=0;
  finger.getTemplateCount();
  Serial.print("Template count: ");
  Serial.println(finger.templateCount);
  id=finger.templateCount;
  id++;
  if(id>128)
    return -1;
  Serial.print("Waiting for valid finger to enroll as #"); 
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); 
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  
  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); 
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

//******************************************************************************************

//Function of options menu after unlock
//TODO: Fix overflow of T--messes up in long run--stays unlocked

void postUnlockUtility(){
  T=millis();
//  input=[];
  p=1;
  do{
    keypressed=myKeypad.getKey();
    if(millis()-T>=5000){
        p=finger.getImage();
    }
  }while(keypressed==NO_KEY && p!=FINGERPRINT_OK);
  if(p==FINGERPRINT_OK){
     keypressed='#';
   }
  switch(keypressed){
//    case '#':
//      delay(1000);
//      Servo1.write(servoLockAngle);
//      Serial.println("LOCKED");
      break;
    case 'C':
      setNewPassword();
      break;
    case 'A':
       if(addFingerprint()){
        Servo1.write(servoLockAngle);
        Serial.println("LOCKED");
        }
       break;
    case 'D':
        Serial.println("Press D again to confirm deletion of ALL fingerprints. Press any other button to cancel.");
        do{
          keypressed=myKeypad.getKey();
        }while(keypressed==NO_KEY);
        if(keypressed=='D'){
          Serial.println("Deleting all fingerprints.");
          finger.getTemplateCount();
          Serial.print(finger.templateCount);
          Serial.println(" templates present");
          Serial.println("Deleting...");
          finger.emptyDatabase();
          finger.getTemplateCount();
          Serial.print(finger.templateCount);
          Serial.println(" templates present");
        }
        else postUnlockUtility();
        break;
    default: 
//      delay(1000);
      Servo1.write(servoLockAngle);
      Serial.println("LOCKED");
      if(p==FINGERPRINT_OK){
          delay(2000);
       }
      break;
  }
}

//*******************************************************************


void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  Servo1.attach(servoPin); 
  Servo1.write(servoLockAngle); //LOCKED
  finger.begin(57600);
 if (finger.verifyPassword()) 
    Serial.println("Found Fingerprint device");
  else Serial.println("Fingerprint device not found");
}

void loop()
{
  keypressed = myKeypad.getKey();
  p=finger.getImage();

  if(passwordSet==false)
    Servo1.write(servoUnlockAngle);
  if (keypressed != NO_KEY){
      Serial.print(keypressed);
    if(passwordSet==false && keypressed=='C'){
        setNewPassword();
      }
      else if(passwordSet==true){
        input[i++]=keypressed;
        if(i==4)
        {
          i=0;
          if(password[0]==input[0]  && password[1]==input[1] && password[2]==input[2] && password[3]==input[3])
          {
            Serial.println("UNLOCKED");
            Servo1.write(servoUnlockAngle); 
            postUnlockUtility();
          }
          else{
            Serial.println("INCORRECT PIN");
          }
        }
      }
  }
  else if(p==FINGERPRINT_OK && passwordSet==true){
    p=finger.image2Tz();
    if(p==FINGERPRINT_OK){
      p=finger.fingerFastSearch();
      if(p==FINGERPRINT_OK){   
        Serial.print("Found ID #");
        Serial.print(finger.fingerID); 
        Serial.print(" with confidence of "); 
        Serial.println(finger.confidence);
        Serial.println("unlocked");
        Servo1.write(servoUnlockAngle); 
        postUnlockUtility(); 
      }
      else{
          Serial.println("No match found");
        }
    } 
  }
}
