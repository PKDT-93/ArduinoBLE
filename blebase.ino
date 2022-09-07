#include <ArduinoBLE.h>
#include "LSM6DS3.h"
#include "Wire.h"
#include <Adafruit_MPR121.h>
#include "Adafruit_DRV2605.h"
#include <iostream>
#include <cstring>
#include "ArrayList.h"
template<typename T>

LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A
Adafruit_DRV2605 drv;             //DRV Instantiation

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite | BLENotify, 200);
const int ledPin = LED_BUILTIN; // pin to use for the LED
char data;

Adafruit_MPR121 cap = Adafruit_MPR121();

// Change value to select which haptic effect to play
uint8_t effect = 58;
int mpr_values[12];
int x;
String mprData;
String temp;
char *buf;
//char space = " ";

// Pass argument 0-7 to select x I2C port
void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.print(bus);
}

// Initializes DRV2605L
void drvInit() {
  TCA9548A(1);
  drv.begin();
  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
}

// Starts BLE Service to connect with Unity
void bleInit() {
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue((byte) 0x00);

  // start advertising
  BLE.advertise();
  Serial.println("BLE LED Peripheral");
}


// Prints out Accelerometer values
void printAccel() {
  Serial.print("\nAccelerometer:\n");
  Serial.print("X1 = ");
  Serial.print(myIMU.readFloatAccelX(), 4);
  Serial.print("\n");
  Serial.print("Y1 = ");
  Serial.print(myIMU.readFloatAccelY(), 4);
  Serial.print("\n");
  Serial.print("Z1 = ");
  Serial.print(myIMU.readFloatAccelZ(), 4);
  Serial.println();
}

// This method stores filtered data from the MPR as a string array and converts it into a char type array to be sent
// to Unity

void printMPR() {
  TCA9548A(0);
  //Serial.print("MPR Readings:\n");
  buf = (char*) malloc(sizeof(mpr_values));
  for (x = 0; x <= 11; x++) {
    mpr_values[x] = cap.filteredData(x);
    temp = (String) mpr_values[x] + '\t';
    mprData = mprData + temp;
  }
  temp = "";
  //Serial.print(mprData);
  //Serial.print("\n");
  mprData.toCharArray(buf, sizeof(mpr_values));
  mprData = "";
  switchCharacteristic.writeValue(buf);
  free(buf);
  //delay(10);
}

// Method that sends a signal to the DRV2605L for haptics
void Touch() {
  TCA9548A(1);
  drv.setWaveform(0, effect);  // play effect
  drv.setWaveform(1, 0);       // end waveform
  // play the effect!
  drv.go();
}

int convertData(const unsigned char data[], int length) {
  int temp = 0;
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];
    //if (b < 16) {
    //Serial.print("0");
    //}
    //Serial.println(b, HEX);
    //Serial.println(b);
    temp << 8;
    temp += data[i];
  }
  Serial.print("Value Received: ");
  Serial.print(temp);
  Serial.print("\n");
  return temp;
}

void checkThreshold() {
  if (switchCharacteristic.written()) {
    int temp = convertData(switchCharacteristic.value(), switchCharacteristic.valueLength());
    if (temp == 1) {
      Touch();
    }
  }
}
void setup() {
  Serial.begin(115200);
  Wire.begin();
  drvInit();
  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);
  // begin initialization
  while (!Serial);
  //Call .begin() to configure the IMUs
  if (myIMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  // Init MPR on bus number 0
  TCA9548A(0);
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  bleInit();
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();
  // if a BLE device(central) is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    // while the central is still connected to peripheral:
    digitalWrite(ledPin, LOW);
    while (central.connected()) {
      if (switchCharacteristic.written()) {
        Serial.println("Receiving value from Unity...");
        switchCharacteristic.read();
        int temp = convertData(switchCharacteristic.value(), switchCharacteristic.valueLength());
        if (temp == 1) {
          Serial.println("Vibrate motors");
          Touch();
        }
      } else {
        printMPR();
      }
    }
    // when the central(BLE) disconnects, print it out:
    digitalWrite(ledPin, HIGH);
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
