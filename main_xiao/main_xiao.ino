// Author: Emilio Orcullo
// SpaceLab 2022-2023
// Seeeduino Xiao Capsule Code

// Libraries
#include "DFRobot_EOxygenSensor.h"
#include <SoftwareSerial.h>

// O2 sensor setup
#define OXYGEN_I2C_ADDRESS E_OXYGEN_ADDRESS_0
DFRobot_EOxygenSensor_I2C oxygen(&Wire, OXYGEN_I2C_ADDRESS);
float oxygenData = 0;
String oxygenEntry = "";

// MicroLab setup
SoftwareSerial mySerial(6, 7);    // RX pin used on arduino, TX pin used on arduino
#define RequestData 0x5A          //command request from microlab to send text data
#define maxbuffer   1020          //maximum number of charators in text buffer
uint8_t TextReady = 0;            //0=text not ready to be taken, 1=text ready to be taken
long arraypointer = 0;            //current pointer into text buffer
char ArduinoData[1024] = "\xFF";  //Text buffer holds text to be sent to the microlab

// Function Prototypes (?)
void CheckforText(void);              //Routine to check for text request                             //
void arrayprint(uint8_t x);           //write a char to the text buffer one at a time

void setup() {
  Serial.begin(9600);     // start USB serial port
  //mySerial.begin(9600);   // start MicroLab serial
  
  oxygen.begin();         // start O2 sensor
  Serial.print("Starting O2 Comms...");
  // IMPORTANT: O2 SENSOR MUST BE ON
  while (!oxygen.begin()) {                  // check if O2 is detected
    Serial.print(".");  // report debug message
    delay(1000);                             // wait a bit to retry
  }
  Serial.print(" Finished!");
}

void loop() {
  if (mySerial.available()) {                   // was the request received ?
    Serial.write("serial opened...");
    uint8_t y = (mySerial.read());              // yes, get the request byte
    if (y = RequestData) {                    // check if request is valid
      Serial.write("valid command...");
      delay(15);                              // delay 15 millisec to sync with microlab
      arraypointer = 0;                       // point text pointer to the beginning of array
      uint8_t x = ' ';
      dataRead();
      while(x != 0xFF){
        x = (ArduinoData[arraypointer]); // do this - get value from text buffer into x
        mySerial.write(x);                  // send that value to the microlab
        delay(10);                           // 10 millisec for microlab to process and store
        arraypointer++;                     // point text pointer to the next location
      }
      TextReady = 0;                         // Text transfered reset to get new text
      arraypointer = 0;                      // reset text pointer, text read
      Serial.write("end");
    }
    else {                                    //if it was not a request text command abort
      Serial.println ("\r\n unknown command");  //output to arduino terminal bad request abort
    }                                         //Transfer finished
  }                                            //check availiable finished
}

void dataRead(void) {
  Serial.println("read start");
  oxygenData = oxygen.readOxygenConcentration();  // read oxygen data
  oxygenEntry = "Oxygen Concentration: " + String(oxygenData);
  //Serial.println(oxygenEntry);

  if (oxygenEntry.length() > maxbuffer) { // check if data entry is too long
    oxygenEntry[maxbuffer - 1] = NULL;
  }

  for(int i = 0; i < oxygenEntry.length(); i++){
    ArduinoData[i] = oxygenEntry[i];
    //Serial.println(ArduinoData[i]);
    if(i+1 > oxygenEntry.length()){ ArduinoData[i+1] = 0xFF; }
  }
  Serial.println("read end");
  Serial.println("");
}


//----------------------------------------------
//  Function to add a charator to the text buffer
//
//  this routine overwrites the end of text marker(0xFF)
//  with the value that was sent to this function in x
//  then it will place a new end of text marker(0xFF) at
//  the end of the valid text buffer.  this operation
//  repeats with every charator sent to this routine
//  therefore, always keeping the buffer valid.
//
//----------------------------------------------
//
void arrayprint(uint8_t x) {                      //Function entry with charator in x
  ArduinoData[arraypointer] = x;              //place x in buffer pointed to by arraypointer
  Serial.write(x);              //OPTIONAL TO OUTPUT DATA WRITTEN INTO TEXT BUFFER, REMOVE FOR FLIGHT
  arraypointer++;                             //move pointer to next location in buffer
  if (arraypointer > maxbuffer) {             //check for maximum number of charators in buffer
    arraypointer = maxbuffer;                 //if over maximum reset to maximum
  }                                           //now arraypointer point to next location
  ArduinoData[arraypointer] = 0xFF;           //always put end of text marker at the end of file
}                                             //Function finished

//----------------------------------------------
//  Function to transfer text to the microlab
//
//  this checks for a text transfer command from the microlab
//  if no command received, it will just return.  The microlab is waiting for the tranfer text command, if
//  none is sent from the microlab, the function will abort the request and continue
//  its operations and will request later.  The request from the
//  microlab is generated every takephoto event.
//
//----------------------------------------------
//
void CheckforText(void) {                       //entry to check for text request
  if (mySerial.available()) {                   // was the request received ?
    uint8_t y = (mySerial.read());              // yes, get the request byte
    if (y = RequestData) {                    // check if request is valid
      delay(15);                              // delay 15 millisec to sync with microlab
      arraypointer = 0;                       // point text pointer to the beginning of array
      uint8_t x = ' ';
      do {                                      // if value fetched is not 0xFF do it again
        x = (ArduinoData[arraypointer]); // do this - get value from text buffer into x
        mySerial.write(x);                  // send that value to the microlab
        delay(10);                           // 10 millisec for microlab to process and store
        arraypointer++;                     // point text pointer to the next location
      } while ((x) != 0xFF);                   // value fetched is 0xFF - transfer finished
      TextReady = 0;                         // Text transfered reset to get new text
      arraypointer = 0;                      // reset text pointer, text read
    }
    else {                                    //if it was not a request text command abort
      Serial.println ("\r\n unknown command");  //output to arduino terminal bad request abort
    }                                         //Transfer finished
  }                                            //check availiable finished
}                                               //Function finished
