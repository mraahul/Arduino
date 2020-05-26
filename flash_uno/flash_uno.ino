/*                Receiver
For more details see: http://projectsfromtech.blogspot.com/

Connect the Tx pin of this board to the Rx of the board running Flash_the_LED__Tx.ino
Connect the Grounds of the two boards

Receive an integer value over the serial and 
flashes the LED the appropriate number of times
*/
int val = 0;
const int led = 11;
int incoming = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(led, OUTPUT);     
}

void loop()
{
 if (Serial.available());
  {
    val = Serial.parseInt();
    Serial.print("val pre_reset:- ");
    Serial.print(val);           //Reads integers as integer rather than ASCI. Anything else returns 0
  
  for (int i = 0; i < val ; i++)      //Flash the LED the appropriate number of times
  {
    digitalWrite(led, HIGH);   
    delay(500);               
    digitalWrite(led, LOW);   
    delay(500);   
  }
  val = 0;
    Serial.print("  val after_reset:- ");
    Serial.println(val);
}

}

