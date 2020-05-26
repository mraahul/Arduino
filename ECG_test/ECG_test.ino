#include <firFilter.h>
firFilter Filter;
int filtered;

void setup() {
  // initialize the serial communication:
  Serial.begin(9600);
  Filter.begin();
  pinMode(11, INPUT); // Setup for leads off detection LO +
  pinMode(10, INPUT); // Setup for leads off detection LO -

}

void loop() {
  
 /* if((digitalRead(10) == 1)||(digitalRead(11) == 1)){
    Serial.println('!');
  }
  else{
    // send the value of analog input 0:
      Serial.println(analogRead(A0));
  }
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
*/
        int average= 0;
        int reading;
        if ((digitalRead (10) == 1) || (digitalRead (11) == 1)) 
        {
            Serial.println("!");
        } 
        else 
        { // send the value of analog input 0: 
         /* for (int i = 0; i <13; i ++) 
          { 
            reading += analogRead (A0); 
          } 
          average= reading / 13; */
          average = analogRead (A0); 
         //filtered= Filter.run(average);
         // Serial.println (filtered); 
          
            delay (1); Serial.println (average); 
          }
      }
          
