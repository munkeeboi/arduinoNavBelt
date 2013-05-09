/* Source: http://bildr.org/2011/01/hmc6352/ */

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

int bF = 2;
int bR = 3;
int bB = 4;
int bL = 5;
int bErr = 6;

int HMC6352SlaveAddress = 0x42;
int HMC6352ReadAddress = 0x41; //"A" in hex, A command is: 


/*
83
127
163
193
241
293
353
397
449 stop
*/

/**
 * Get bearing from the electronic compass.
 */
float getBearing() {
  //"Get Data. Compensate and Calculate New Heading"
  Wire.beginTransmission(HMC6352SlaveAddress);
  Wire.write(HMC6352ReadAddress);              // The "Get Data" command
  Wire.endTransmission();

  //time delays required by HMC6352 upon receipt of the command
  //Get Data. Compensate and Calculate New Heading : 6ms
  delay(6);

  Wire.requestFrom(HMC6352SlaveAddress, 2); //get the two data bytes, MSB and LSB

  //"The heading output data will be the value in tenths of degrees
  //from zero to 3599 and provided in binary format over the two bytes."
  byte MSB = Wire.read();
  byte LSB = Wire.read();

  float headingSum = (MSB << 8) + LSB; //(MSB / LSB sum)
  float headingInt = headingSum / 10;
  return headingInt;
}

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

/**
 * Select buzzer for given absolute heading. 'dir' is the absolute heading in degrees
 */
void buzz(float dir) {
	float myBearing = getBearing();
	float desired = dir;

	buzzForDir(desired - myBearing);
        // p("%d %d %d %d\n", (int) desired, (int) myBearing, (int) (desired-myBearing), pinForDir(desired-myBearing));
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

/* Get heading from bin number  */
float getHeadingFromBin(int binNum) {
   return ((float) binNum * 7.6556 - 0.6551) - 75.0;
}

void setup() {
  Serial.begin(9600);
  pinMode(bF, OUTPUT);
  pinMode(bR, OUTPUT);
  pinMode(bB, OUTPUT);
  pinMode(bL, OUTPUT);
    HMC6352SlaveAddress = HMC6352SlaveAddress >> 1; // I know 0x42 is less than 127, but this is still required
  Wire.begin();
  prevbin = -1;
}

void loop(){
  int static i = 0;
  static long tt;
  int val;

  int bin;
   
   if (millis() > tt){
      if (i < 128){
        val = analogRead(0); //pin_adc
        data[i] = val / 4 - 128;
        im[i] = 0;
        i++;   
        
      }
      else {
        //this could bR done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
        // I am only interessted in the abBolute value of the transformation
        for (i=0; i< 64;i++){
           data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
        }
        
        bin = detectTone(data);
        
//        Serial.println(bin);
        Serial.println(getBearing());
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
        } else if (bin > 57) {
          // send the stop signal
          digitalWrite(bF, HIGH);
          digitalWrite(bR, HIGH);
	  digitalWrite(bB, HIGH);
	  digitalWrite(bL, HIGH);
        } else {
          // find the direction we map to
          buzz(getHeadingFromBin(bin));
        }
        
        prevbin = bin;
      }
    tt = millis();
   }
}

/*

467 stop

*/


//midpoints


/*
N 11
NE 17
E 21
SE 26
S 33
SW 40
W 48/7
NW 52-53
stop 63
*/
