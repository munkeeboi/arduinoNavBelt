/* Source: http://bildr.org/2011/01/hmc6352/ */

#include <fix_fft.h>
#include <Wire.h>

const int Threshold = 5;

char im[128];
char data[128];
char temp[4];
//int bF = 3; // 83
//int bR = 5; // 163 - throw these guys out
//int bB = 6; // 241
//int bL = 9; // 353

int bF = 2; // 83
int bR = 3; // 163 - throw these guys out
int bB = 4; // 241
int bL = 5;

float northb = 0;
float eastb = 90;
float southb = 180;
float westb = 270;

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

/* Buzz case for given direction: North */
void caseForDir(float dirBearing) {
  float diff = dirBearing - getBearing();
  while (diff >= 180) diff -= 360;
  while (diff < -180) diff += 360;
  Serial.println(diff);
  if (diff < 20 && diff > -20) {
    digitalWrite(bF, HIGH);
  } else if ((diff > 135) || (diff < -135)) {
    digitalWrite(bB, HIGH);
  } else if ((diff >= 20) && (diff <= 135)) {
    digitalWrite(bR, HIGH);
  } else if ((diff <= -20) && (diff >= -135)) {
    digitalWrite(bL, HIGH);
  } else {
    Serial.println("HEY LOOK AT ME");
    digitalWrite(bF, HIGH);
  }
}

/* Buzz the buzzer associated with the given direction */
void buzz(char dir) {
  digitalWrite(bF, LOW);
  digitalWrite(bR, LOW);
  digitalWrite(bB, LOW);
  digitalWrite(bL, LOW);
        
  switch (dir) {
    case 'N': {
      caseForDir(northb);
      // digitalWrite(bF, HIGH);
      break;
    }
    case 'E': {
      caseForDir(eastb);
      // digitalWrite(bR, HIGH);
      break;
    }
    case 'S': {
      caseForDir(southb);      
      // digitalWrite(bB, HIGH);
      break;
    }
    case 'W': {
      caseForDir(westb);
      // digitalWrite(bL, HIGH);
      break;
    }
    default: {
      Serial.write("Aiaiaiaiai");
    }
  }
}

int detectTone(char input[]) {
  // TODO: pick out multiple frequencies instead of just one
  for (int i = 8; i < 64; i++) {
           if (input[i] > Threshold) {
              return i;
           }
  }
  return -1;
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
      else{
        //this could bR done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
        // I am only interessted in the abBolute value of the transformation
        for (i=0; i< 64;i++){
           data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
        }
        
        status = detectTone(data);
        
        Serial.println(status);
        
        /*
        if (status > -1) {
          if (status > 50) {
            buzz('N');
          } else if (status > 40) {
            buzz('S');
          } else if (status > 20) {
            buzz('E');
          } else if (status > 10) {
            buzz('W');
          } else {
            Serial.write("?????");
          }
        }
        */
        if (status > -1) {
          if (status > 40) {
            buzz('W');
          } else if (status > 25) {
            buzz('S');
          } else if (status > 15) {
            buzz('E');
          } else if (status > 5) {
            buzz('N');
          } else {
            Serial.write("?????");
          }
        }
        else {
            digitalWrite(bF, LOW);
            digitalWrite(bR, LOW);
            digitalWrite(bB, LOW);
            digitalWrite(bL, LOW);
        }
        
         // Serial.println(status);
/*
        for (int i = 0; i < 64; i++) {
           sprintf(temp, "%d", data[i]);
           Serial.print(temp);
           Serial.write(","); 
        }
        Serial.println();
*/
        
        //do something with the data values 1..64 and ignore im
        // show_big_bars(data,0);
      }
    
    tt = millis();
   }
}
