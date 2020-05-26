/*ReadDS18B20two
vers: 6Jly14- Modified by Brian (twice!) for 5 sensors,
better responsiveness. Derived from vers 18 July 2010
Started end of term eve WG/TA/LE/EW/JGB

Reading five DS18B20s (Changed to 5 from the 2
  allowed by the 18 July 2010 version of ReadDS18B20two)

See...
 http://sheepdogguides.com/arduino/ar3ne1tt2.htm
 ... for explanation of this code.

 Code adapted from code from nuelectronics.com demo

 Revise the next five lines, as necessary, for where
 you have your sensors connected. Remove pre-pended
 "//"s.   */

//#define tture1 33//no ; here
//#define tture2 35//no ; here
//#define tture3 37//no ; here
//#define tture4 39//no ; here
//#define tture5 41//no ; here

int tture =4;

int x = 0;
int count = 0;


/*Forward declarations. Only the last two need concern the user

Remmed out, as they seem unnecessary
void OneWireReset(int Pin);//Called by readTture
void OneWireOutByte(int Pin, byte d);//Called by readTture
byte OneWireInByte(int Pin);//Called by readTture
void readTture(byte Pin);//Of use to users
void printTture();//Of use to users
*/

//Following globals used to communicate results back
//from readTture(Pin), and to send data to printTture...

int HighByte, LowByte, TReading, SignBit,
   Tc_100, Whole,Fract;

unsigned long pause1;

void setup() {
   //For each tture sensor: Do a pinMode and a digitalWrite
  
      pinMode(tture, INPUT);
      digitalWrite(tture, LOW);//Disable internal pull-up.
   

   pinMode(13,OUTPUT);

   Serial.begin(9600);
   delay(300);//Wait for newly restarted system to stabilize
   Serial.print("Temperature measurement, Five sensors:\n\n");
}

/*
The loop takes approximately 60uS
The reading and printing take about 5.9mS

One sensor is updated every 200mS

All 5 are updated once per second.
*/

void loop(){
   digitalWrite(13,!digitalRead(13));
   pause1 = millis() % 200;// pause1 will loop from 0 to 199
   if(pause1 == 0){
       //Pass WHICH pin you want to read in "Pin"
   //Returns values in... (See global declarations)

   OneWireReset(4);
   OneWireOutByte(4,0xcc);
   OneWireOutByte(4,0x44); // perform temperature conversion,
        //  strong pullup for one sec

   OneWireReset(4);
   OneWireOutByte(4,0xcc);
   OneWireOutByte(4,0xbe);

   LowByte = OneWireInByte(4);
  HighByte = OneWireInByte(4);
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
   }

  /* if(pause1 == 150){
      printTture();//N.B.: Takes values from globals. Also...
      Serial.print("   ");

   if(count == 0){
      Serial.print("\n");//Start new line
   }
   }*/
}

//Everything below here... just copy it into your program "as is".
//You are only likely to need to use readTture(pin) and printTture()
//   directly. Others are subordinate to those.
//These routine access the following global variables...
//   int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;

void OneWireReset(int Pin) // reset.  Should improve to act as a presence pulse

{
   digitalWrite(Pin, LOW);
   pinMode(Pin, OUTPUT); // bring low for 500 us
   delayMicroseconds(500);
   pinMode(Pin, INPUT);
   delayMicroseconds(500);
}



void OneWireOutByte(int Pin, byte d) // output byte d (least sig bit first).

{
   byte n;
   for(n=8; n!=0; n--)
   {
      if ((d & 0x01) == 1)  // test least sig bit
      {
         digitalWrite(Pin, LOW);
         pinMode(Pin, OUTPUT);
         delayMicroseconds(5);
         pinMode(Pin, INPUT);
         delayMicroseconds(60);
      }

      else
      {
         digitalWrite(Pin, LOW);
         pinMode(Pin, OUTPUT);
         delayMicroseconds(60);
         pinMode(Pin, INPUT);
      }

      d=d>>1; // now the next bit is in
                     // the least sig bit position.
   }
}//end OneWireOutByte

byte OneWireInByte(int Pin) // read byte, least sig byte first

{
   byte d, n, b;

    d=0;//This critical line added 04 Oct 16
    //I hate to think how many derivatives of
    //this code exist elsewhere on my web pages
    //which have NOT HAD this. You may "get away"
    //with not setting d to zero here... but it
    //is A Very Bad Idea to trust to "hidden"
    //initializations!
    //The matter was brought to my attention by
    //a kind reader who was THINKING OF YOU!!!
    //If YOU spot an error, please write in, bring
    //it to my attention, to save the next person
    //grief.

   for (n=0; n<8; n++)
   {
      digitalWrite(Pin, LOW);
      pinMode(Pin, OUTPUT);
      delayMicroseconds(5);
      pinMode(Pin, INPUT);
      delayMicroseconds(5);
      b = digitalRead(Pin);
      delayMicroseconds(50);
      d = (d >> 1) | (b<<7); // shift d to right and
         //insert b in most sig bit position
   }

   return(d);

}

 
void printTture(){//Uses values from global variables.

   //See global declarations.
   //N.B.: No new line inside printTture

   if (SignBit) // If it's negative
   {
      Serial.print("-");
   }

   Serial.print(Whole);
   Serial.print(".");

   if (Fract < 10)
   {
      Serial.print("0");
   }
   Serial.print(Fract);
    
}
