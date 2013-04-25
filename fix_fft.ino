#include <fix_fft.h>

const int Threshold = 7;

char im[128];
char data[128];
char temp[4];
int bN = 3; // 83
int bE = 5; // 163 - throw these guys out
int bS = 6; // 241
int bW = 9; // 353

/*
83
127
163
193
241
293
353
397

*/

/* Buzz the buzzer associated with the given direction */
void buzz(char dir) {
  digitalWrite(bN, LOW);
  digitalWrite(bE, LOW);
  digitalWrite(bS, LOW);
  digitalWrite(bW, LOW);
        
  switch (dir) {
    case 'N': {
      digitalWrite(bN, HIGH);
      break;
    }
    case 'E': {
      digitalWrite(bE, HIGH);
      break;
    }
    case 'S': {
      digitalWrite(bS, HIGH);
      break;
    }
    case 'W': {
      digitalWrite(bW, HIGH);
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
  pinMode(bN, OUTPUT);
  pinMode(bE, OUTPUT);
  pinMode(bS, OUTPUT);
  pinMode(bW, OUTPUT);
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
        //this could be done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
        // I am only interessted in the absolute value of the transformation
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
