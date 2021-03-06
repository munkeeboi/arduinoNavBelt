/* 
  Group 11: Don't Worry About It (DWAI)
  Members: Amy, Daniel, Jonathan, Krithin, Thomas
  Project: The NavBelt
  
  Sources: 
  (Compass Module) http://bildr.org/2011/01/hmc6352/ 
*/

#include <fix_fft.h>
#include <Wire.h>
#include <stdarg.h>

void p(char *fmt, ... ){
      char tmp[128]; // resulting string limited to 128 chars
      va_list args;
      va_start (args, fmt );
      vsnprintf(tmp, 128, fmt, args);
      va_end (args);
      Serial.print(tmp);
}

const int Threshold = 5;

char im[128];
char data[128];
char temp[4];

int prevbin;

/* PIN numbers referring to directions: front (F), right (R), back (B), left (L), or error (ERR) */
int bF = 2;
int bR = 3;
int bB = 4;
int bL = 5;
int bErr = 6;

int HMC6352SlaveAddress = 0x42;
int slaveAddress;
byte headingData[2];
int headingValue;

/**
 * Get bearing from the electronic compass.
 */
float getBearing() {
  int i;
  //"Get Data. Compensate and Calculate New Heading"
  Wire.beginTransmission(slaveAddress);
  Wire.write("A");              // The "Get Data" command
  Wire.endTransmission();

  //time delays required by HMC6352 upon receipt of the command
  //Get Data. Compensate and Calculate New Heading : 6ms
  delay(10);

  Wire.requestFrom(slaveAddress, 2); //get the two data bytes, MSB and LSB
  i = 0;
  
  while(Wire.available() && i < 2)
  { 
    headingData[i] = Wire.read();
    i++;
  }
  headingValue = headingData[0]*256 + headingData[1];  // Put the MSB and LSB together
  
  return (float) headingValue/10.0;
}

/* Calibration Function */
void calibration() {
    delay(2000);
    //Serial.println("Get Ready in 2");
    delay(2000);
    //Serial.println("Start");
    digitalWrite(bL, HIGH);
    digitalWrite(bR, HIGH);
    delay(1000);
    digitalWrite(bL, LOW);
    digitalWrite(bR, LOW);
    Wire.beginTransmission(slaveAddress);
    Wire.write("C");   
    Wire.endTransmission();
    delay(10);
    //Serial.println("Keep rotating in 20 seconds NOW");
    delay(20000);
    //Serial.println("Stop rotating please!");
    delay(10);
    Wire.beginTransmission(slaveAddress);
    Wire.write("E");   
    Wire.endTransmission();
    delay(10);
    Wire.beginTransmission(slaveAddress);
    Wire.write("L");   
    Wire.endTransmission();
    digitalWrite(bL, HIGH);
    digitalWrite(bR, HIGH);
    delay(1000);
    digitalWrite(bL, LOW);
    digitalWrite(bR, LOW);
    //Serial.print("Finished, thanks! The NavBelt will be ready in 5 seconds to guide you to your next destination :)"); // This gives you a chance to stop the loop returning 
    delay(5000); 
}

/* Given a current heading, determine which pin to output the bearing to the next direction. */
int pinForDir (float bearing) {
  while (bearing < -180) bearing += 360;
  while (bearing >= 180) bearing -= 360;
  
  if (bearing < 20 && bearing > -20) {
    return bF;
  } else if ((bearing > 135) || (bearing < -135)) {
    return bB;
  } else if ((bearing >= 20) && (bearing <= 135)) {
    return bR;
  } else if ((bearing <= -20) && (bearing >= -135)) {
    return bL;
  } else {
    return bErr;
  }
}

/**
 *  Buzz the buzzer for given relative direction.
 */
void buzzForDir(float bearing) {
  digitalWrite(bF, LOW);
  digitalWrite(bR, LOW);
  digitalWrite(bB, LOW);
  digitalWrite(bL, LOW);
  
  while (bearing >= 180) bearing -= 360;
  while (bearing < -180) bearing += 360;
  // Serial.println(bearing);
  digitalWrite(pinForDir(bearing), HIGH);
}

/* Pretty labeling */
char prettyLabel(int dir) {
   switch(dir) {
     case 2: return 'F';
     case 3: return 'R';
     case 4: return 'B';
     case 5: return 'L';
     default: return '\0';
   } 
}


/**
 * Select buzzer for given absolute heading. 'dir' is the absolute heading in degrees
 */
void buzz(float dir) {
  float myBearing = getBearing();
  float desired = dir;

  buzzForDir(desired - myBearing);
  p("Desired: %d | Your Bearing: %d | Diff: %d | Pin: %c\n", (int) desired, (int) myBearing, (int) (desired-myBearing), prettyLabel(pinForDir(desired-myBearing)));
}

int detectTone(char input[]) {
  int maxval = Threshold;
  int maxindex = -1;
  for (int i = 9; i < 64; i++) {
    if (input[i] > maxval) {
      maxindex = i;
      maxval = input[i];
    }
  }
  if (maxindex == 8 || maxindex == 9) {
    maxindex = 10;
  }   
  return maxindex;
}

/* Get heading from bin number (determined experimentally) */
float getHeadingFromBin(int binNum) {
  return ((float) binNum * 7.6556 - 0.6551) - 75.0;
}


void setup() {
  Serial.begin(9600);
  pinMode(bF, OUTPUT);
  pinMode(bR, OUTPUT);
  pinMode(bB, OUTPUT);
  pinMode(bL, OUTPUT);
  slaveAddress = HMC6352SlaveAddress >> 1; // I know 0x42 is less than 127, but this is still required
  Wire.begin();
  prevbin = -1;
}


void loop(){
  int static i = 0;
  static long tt;
  int val;

  int bin;
   
  if (millis() > tt){
    if (i < 128) {
      val = analogRead(0); //pin_adc
      data[i] = val / 4 - 128;
      im[i] = 0;
      i++;       
    } else {
      //this could be done with the fix_fftr function without the im array.
      fix_fft(data,im,7,0);
      // I am only interested in the absolute value of the transformation
      for (i=0; i< 64;i++){
        data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
      }
        
      bin = detectTone(data);
        
      // Serial.println(bin);
      // Serial.println(getBearing());
      if (bin < 0) {
        if (prevbin < 0) {
          digitalWrite(bF, LOW);
          digitalWrite(bR, LOW);
          digitalWrite(bB, LOW);
          digitalWrite(bL, LOW);
        } else if (prevbin > 57) {
          // send the stop signal
          digitalWrite(bF, HIGH);
          digitalWrite(bR, HIGH);
          digitalWrite(bB, HIGH);
          digitalWrite(bL, HIGH);
        } else {
            buzz(getHeadingFromBin(prevbin));
        }
      } else if (bin > 62) {
          calibration(); 
      } else if (bin > 57) {
        // send the stop signal
        digitalWrite(bF, HIGH);
        digitalWrite(bR, HIGH);
        digitalWrite(bB, HIGH);
        digitalWrite(bL, HIGH);
      }
      else {
        // find the direction we map to
        buzz(getHeadingFromBin(bin));
      }
        
      if (bin <= 62) {
        prevbin = bin;  
      }
    }
    tt = millis();
   }
}

/* Frequencies for each direction determined experimentally. The following
  are special chosen frequencies for 'stop' and 'calibration mode':
    stop: 467 
    calibration mode (bin 63): 483
*/

