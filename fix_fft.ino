/* Source: http://bildr.org/2011/01/hmc6352/ */

#include <fix_fft.h>
#include <Wire.h>

const int Threshold = 5;

char im[128];
char data[128];
char temp[4];

int bF = 2;
int bR = 3;
int bB = 4;
int bL = 5;

enum DIRECTION {NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, STOP};
float bearings[] = {0, 45, 90, 135, 180, 225, 270, 315, -1};
int cutoffs[] = {14, 19, 23, 30, 37, 44, 50, 58};
DIRECTION dirs[] = {NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, STOP};

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
	if (bearing < 20 && bearing > -20) {
		digitalWrite(bF, HIGH);
	} else if ((bearing > 135) || (bearing < -135)) {
		digitalWrite(bB, HIGH);
	} else if ((bearing >= 20) && (bearing <= 135)) {
		digitalWrite(bR, HIGH);
	} else if ((bearing <= -20) && (bearing >= -135)) {
		digitalWrite(bL, HIGH);
	} else {
		Serial.println("HEY LOOK AT ME");
		digitalWrite(bF, HIGH);
	}
}

/**
 * Select buzzer for given absolute heading.
 */
void buzz(DIRECTION dir) {
	float myBearing = getBearing();
	float desired = bearings[dir];
	
	buzzForDir(desired - myBearing);
}

int detectTone(char input[]) {
	int maxval = Threshold;
	int maxindex = -1;
	for (int i = 8; i < 64; i++) {
		if (input[i] > maxval) {
			maxindex = i;
			maxval = input[i];
		}
	}
	return maxindex;
}

void setup() {
  Serial.begin(9600);
  pinMode(bF, OUTPUT);
  pinMode(bR, OUTPUT);
  pinMode(bB, OUTPUT);
  pinMode(bL, OUTPUT);
    HMC6352SlaveAddress = HMC6352SlaveAddress >> 1; // I know 0x42 is less than 127, but this is still required
  Wire.begin();
}

void loop(){
  int static i = 0;
  static long tt;
  int val;

  int status;
   
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
        
        status = detectTone(data);
        
        Serial.println(status);
        
        if (status < 0) {
            digitalWrite(bF, LOW);
            digitalWrite(bR, LOW);
            digitalWrite(bB, LOW);
            digitalWrite(bL, LOW);
        } else {
			
			for (int index = cutoffs.length-1; index >= 0; index--) {
				if (cutoffs[index] < status) break;
			}
			index++;
			
			if (index == STOP) {
				digitalWrite(bF, HIGH);
				digitalWrite(bR, HIGH);
				digitalWrite(bB, HIGH);
				digitalWrite(bL, HIGH);
			} else {
				buzz((DIRECTION) index);
			}
		}
      }
    
    tt = millis();
   }
}

midpoints


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
