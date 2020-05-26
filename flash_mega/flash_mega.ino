/*            Transmitter
For more details see: http://projectsfromtech.blogspot.com/

Connect the Tx pin of this board to the Rx of the board running Flash_the_LED__Tx.ino
Connect the Grounds of the two boards

Read integer value from Serial Monitor
Transmit that value to the Rx board
*/


int val = 0;


void setup()
{
  Serial.begin(115200);  
  Serial1.begin(115200);     
  Serial.println("Serial Monitor Connected");    //Is not an integer so it won't interfere
}

void loop()
{

if (Serial.available()) {
    /* read the most recent byte */
    val = Serial.parseInt();
    /*ECHO the value that was read, back to the serial port. */
    Serial1.write(val);
  }
   Serial.println(val);
 
  }


