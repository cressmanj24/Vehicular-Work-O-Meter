// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class
// 10/7/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added multiple output formats
//                 - added seamless Fastwire support
//      2011-10-07 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "SD.h"
#include "SPI.h"
#include "TinyGPS.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

TinyGPS gp20u07;

String data;
String msg;

const int chipSelect = BUILTIN_SDCARD;

int16_t ax, ay, az;
int16_t gx, gy, gz;
float axg, ayg, azg, magnitude;
float axgAvg, aygAvg, azgAvg, magAvg;
int count;

float dist;

float lat, lon, lastLat, lastLon;

unsigned long tim, lastTim, age;

#define LED_PIN 13
bool blinkState = false;

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(38400);
    
    Serial1.begin(9600);
    
    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    lastLat = TinyGPS::GPS_INVALID_F_ANGLE;
    Serial.print("Acquiring satellites");
    while(lastLat == TinyGPS::GPS_INVALID_F_ANGLE){
      smartDelay(1000);
      gp20u07.f_get_position(&lastLat, &lastLon, &age);
      Serial.print(".");
    }
    Serial.println("Done");
    
    
    // configure Arduino LED for
    pinMode(LED_PIN, OUTPUT);

    Serial.print("Initializing SD card...");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  msg = ",FRESH_RESTART";
}

void loop() {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    axg = (float(ax) - 1000.0)/16800.0;
    ayg = (float(ay) + 204.0)/16400.0;
    azg = (float(az)/* - 3600.0*/)/16700.0;
    count++;
    
    axgAvg = (float(count - 1) * axgAvg + axg)/float(count);
    aygAvg = (float(count - 1) * aygAvg + ayg)/float(count);
    azgAvg = (float(count - 1) * azgAvg + azg)/float(count);
    
//    magnitude = sqrt(sq(axg)+sq(ayg)+sq(azg));
    smartDelay(100);
//    while (Serial1.available()){
//      char c = Serial1.read();
//      if(gp20u07.encode(c))
        gp20u07.f_get_position(&lat, &lon, &age);
//    }
    dist = TinyGPS::distance_between(lat, lon, lastLat, lastLon);

    if (dist >= 10){
      data = String(axgAvg) + "," + String(aygAvg) + "," + String(azgAvg) + "," + String(sqrt(sq(axgAvg)+sq(aygAvg)+sq(azgAvg))) + "," + String(dist) + "," + String(age) + "," + String(aygAvg * dist) + "," + String(lon) + "," +  String(lat) + msg;
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      
      if (dataFile) {
        dataFile.println(data);
        dataFile.close();
  
        Serial.println(data);
      }
      else Serial.println("Danger, Will Robinson.  Error with SD card");
      lastLat = lat;
      lastLon = lon;
      axgAvg = 0;
      aygAvg = 0;
      azgAvg = 0;
      count = 0;
      msg = ",STALE_POTATO";
    }



    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do {
    while (Serial1.available())
      gp20u07.encode(Serial1.read());
  } while (millis() - start < ms);
}
