
/*
 *
 * SparkFun ArduinoBLE (SparkFun Artemis) BLE Enviromental Sensor Node
 #
 # Exposes BME280 data via BLE characteristics
 * =======================================
 *
 * 
 *  HISTORY
 *    July, 2021     - Initial developement - KDB
 * 
 *==================================================================================
 * Copyright (c) 2021 SparkFun Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *==================================================================================
 * 
 */

#include <Wire.h>
#include "ArduinoBLE.h"
#include "SparkFunBME280.h"


BME280 mySensor;


// How many seconds between samples - ble notification of data update 
#define SAMPLE_SECS  60

// Include SparkFun functions to define "properties" out of characteristics
// This enables use of the SparkFun BLE Settings Web-App
#include "sf_ble_prop.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define kTargetServiceName  "Artemis"
#define kIDNameLength 6
//--------------------------------------------------------------------------------------
// Our Characteristic UUIDs - and yes, just made these up
//--------------------------------------------------------------------------------------


#define kCharacteristicTempUUID   "beb5483e-36e1-4688-b7f5-ea07311b260e"
#define kCharacteristicHumidityUUID     "beb5483e-36e1-4688-b7f5-ea07311b260d"
#define kCharacteristicPressureUUID   "beb5483e-36e1-4688-b7f5-ea07311b260f"

// helper for message limits
#define kMessageMax 64

//--------------------------------------------------------------------------------------
// ArduinoBLE Object declaration 
//
// Declare the main objects that define this BLE service here. Will connect everything 
// together later in the app logic. This enables these objects to be stack based

// Our BLE Service
BLEService bleService(kTargetServiceUUID);

// Define the BLE Service Characteristics - or "Properties"
//
// Note - for each characterisitic, a storage value is also defined for it
//
// >> Notifications <<
//    Only enable for those characteristics that will use notifications. Adding this type
//    to a Characteristic adds a BLE Descriptor. If not using notify, save the resources.


//---------------------------------------------------------------------------
// Characteristic- "Temperature" Float type - 
//
// This value has Notifications enabled.
// A float property - "offset value"
float tempValue = 0.0;
BLEFloatCharacteristic bleCharTemp(kCharacteristicTempUUID, BLERead | BLENotify | BLEWrite);

//---------------------------------------------------------------------------
//  Characteristic - "Humidity" - Float type 
//
float humidityValue = 0.0;

BLEFloatCharacteristic bleCharHumidity(kCharacteristicHumidityUUID, BLERead);


//---------------------------------------------------------------------------
//  Characteristic - "Presure" - Float type 
//
float pressureValue = 0.0;

BLEFloatCharacteristic bleCharPressure(kCharacteristicPressureUUID, BLERead);

//--------------- end object setup ------------------------
// We're using the Enable property to control the on-board LED...
const byte LED_TO_TOGGLE = 19;

//--------------------------------------------------------------------------------------
// Operational State
//--------------------------------------------------------------------------------------
// Value used to manage timing for a Notification example in loop.
unsigned long ticks;

// >> Work Timeout <<
// On BLE connect from the settings app client, the BLE system needs resources to process
// the various descriptor requests from the client app. Any additional work being 
// performed in loop() can impact the BLE systems performance.
//
// To provide a "work" pause on connect, this example implements a "on connection" event  
// is determined,and a work "timeout" is implemented for N seconds. 
//  
// Define work timeout in MS. 
const unsigned int bleOnConnectDelay = 3400;  // ms  on BLE connection "work" timeout


//---------------------------------------------------------------------------
void temperatureReadCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Temperature Read: ");
    tempValue = mySensor.readTempF();
    Serial.println(tempValue);
    bleCharTemp.setValue(tempValue);
}

void humidityReadCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Humidity Read: ");
    humidityValue = mySensor.readFloatHumidity();
    Serial.println(humidityValue);
    bleCharHumidity.setValue(humidityValue);
}

void pressureReadCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Pressure Read: ");
    pressureValue = mySensor.readFloatPressure();
    Serial.println(pressureValue);
    bleCharPressure.setValue(pressureValue);
}
//--------------------------------------------------------------------------------------
// Characteristic Setup
//--------------------------------------------------------------------------------------
// Connect and setup each characteristic broadcast from this service.
//
// >> SparkFun Web-App Configuration <<
//    
//   The characteristics are configured to work with the SparkFun BLE Settings Web-App.
// 
//   For each characteristic:
//      - The related SparkFun BLE Property function is called. This adds the BLE Descriptors 
//        used to define the attributes of this characteristic that the SparkFun BLE app uses
//      - Add the characteristic to the service
//      - Set the value of the characteristic to it's associated value variable (declared above)
//      - Set the value event handler (declared above)
// 
//   The setup sequence defines the user experience in the SparkFun BLE Application. 
//   
//   Specifically:
//          - The order that characteritics are setup (calls to sfe_bleprop_ functions),
//            defines the display order in the app.
//          - Adding a "Group Title" to a characteristic, causes the app to display this 
//            title before rendering the property GUI. A group title is just a title, nothing more.
//   
//   

void setupBLECharacteristics(BLEService& theService){

    // Set a title.
    BLEProperties.addTitle("Environmental Values");    
    
    BLEProperties.addFloat(bleCharTemp, "Temperature"); // setup property descriptor  
    theService.addCharacteristic(bleCharTemp);  
    bleCharTemp.setEventHandler(BLERead, temperatureReadCB);  

    BLEProperties.addFloat(bleCharHumidity, "Humidity"); // setup property descriptor  
    theService.addCharacteristic(bleCharHumidity);  
    bleCharHumidity.setEventHandler(BLERead, humidityReadCB);  

    BLEProperties.addFloat(bleCharPressure, "Pressure"); // setup property descriptor  
    theService.addCharacteristic(bleCharPressure);  
    bleCharPressure.setEventHandler(BLERead, pressureReadCB);  
}
//-------------------------------------------------------------------------
// A BLE client  is connected logic.
//-------------------------------------------------------------------------
//
// The system is setup to call callback methods to the below object. 
// A bool is used to keep track of connected state..
bool deviceConnected = false;

// General Connect callbacks
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  deviceConnected = true;
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  deviceConnected = false;  
}
//---------------------------------------------------------------------------------
// Build a device name that uses the BLE mac address
//---------------------------------------------------------------------------------
char szName[24];
void setUniqueBLEName(){

    // Build a unique name using the BLE address 
    String strAddress = BLE.address();
    Serial.print("BLE Address: ");Serial.println(strAddress);

    // Make a unique name using the address - pull out ":"s, shorten length to kIDNameLength and upcase it
    for(int i;   (i = strAddress.indexOf(":")) > -1; strAddress.remove(i,1));

    // shorten name and upcase it
    strAddress = strAddress.substring(strAddress.length()-kIDNameLength);
    strAddress.toUpperCase();
    snprintf(szName, sizeof(szName), "%s - %s", kTargetServiceName, strAddress.c_str());

    Serial.print("Device Name: "); Serial.println(szName);

    // name the device
    BLE.setLocalName(szName);
}
//---------------------------------------------------------------------------------
// Setup our system
//---------------------------------------------------------------------------------

void setup() {

    // start up serial port
    Serial.begin(115200);
    while (!Serial);

    Wire.begin();

    if(mySensor.beginI2C() == false){
        Serial.println("The sensor did not respond. Please check wiring.");
        while(1); //Freeze
    }
    Serial.println();
    Serial.println("Starting BLE Setup...");
    
    // led to display when connected
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // start BLE
    if ( ! BLE.begin()){
        Serial.println("starting BLE failed!");
        while (1);
    }

    // assign event handlers for connected, disconnected to peripheral
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // Build a unique name using the BLE address 
    setUniqueBLEName();
    
    // Setup service Characteristics
    setupBLECharacteristics(bleService);

    // add the service to the BLE device and start to broadcast.
    BLE.addService(bleService);
    BLE.setAdvertisedService(bleService);

    // broadcast BLE connection
    BLE.advertise();

    ticks = millis(); // for our notify example below
    Serial.println(F("OLA BLE ready for connections!"));
    digitalWrite(LED_BUILTIN, HIGH);
}

//-----------------------------------------------------------------------------------
// function to determine if work should pause while a new BLE Connection initializes
//
// Returns true if work should be performed. 
bool doWork(){
    
    // vars to keep track of state. Make static to live across function calls
    static unsigned int bleTicks =0;
    static bool wasConnected = false;

    // did we just connect? If so, give the BLE system most of our loop resouces
    // to manage the connection startup
    if(!wasConnected && deviceConnected){ // connection state change
        bleTicks = millis();
        Serial.println("start work delay");
    }
    // Are we at the end the work timeout?
    if(bleTicks && millis()-bleTicks > bleOnConnectDelay){
        bleTicks = 0;
        Serial.println("end work delay");
    }
    wasConnected = deviceConnected;

    // do work if ticks equals 0.
    return bleTicks == 0;

}
//-----------------------------------------------------------------------------------
void loop()
{
    // >> Update and Notification Example <<
    //
    // This section updates the offset characterisitc value every 5 secs if a
    // device is connected. Demostrates how to send a notification using the 
    // BLE API.
    if(millis() - ticks > SAMPLE_SECS * 1000){
        // Update the value of offset and set in BLE char
        // Should trigger a notification on client
        if(deviceConnected && doWork()){
            tempValue = mySensor.readTempF();
            Serial.print("Update Tempature: ");
            Serial.println(tempValue);
            bleCharTemp.setValue(tempValue);
        }
        ticks = millis();
    }
    
    // Do work, or pause work to give most resources to the BLE system on client connection initialization
    if(doWork()){
        ////////////////////////////
        // >> DO LOOP WORK HERE <<
        ///////////////////////////
        delay(200);    // Example *WORK*
    }

    // Pump the BLE service - everything is handled in callbacks.
    BLE.poll();

}
