int DR1_Pin = A0;    // Drip counter 1 pin no
int DR2_Pin = A1;    // Drip counter 2 pin no
int BBD_Pin = A3;    // bubble detector pin no
int pinch_valve_pin = 49; // pinch Valve controll pin

int dr1_edge_detected_flag = 0 ;
int dr2_edge_detected_flag = 0;
int dr1_DropCount;
int dr2_DropCount;
int bubble_detector_value;
int drop_counter_1_value = 0;  // variable to store the value coming from the sensor
int drop_counter_2_value = 0;
int bubble_detect_flag ;
unsigned long int previousTime ;

///////////Threshold values //////////////

int DR1_VAL_Threshold = 1015;
int DR2_VAL_Threshold = 1015;
int BBD_VAL_Threshold = 880;

//////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(pinch_valve_pin, OUTPUT);
}

void loop() {
  // read the value from the sensor:
  unsigned  long int currentTime = millis();

  drop_counter_1_value = analogRead(DR1_Pin); //  To take raw values of Drop counter 1
  drop_counter_2_value = analogRead(DR2_Pin); // to take raw values of Drop counter 2
  bubble_detector_value = analogRead(BBD_Pin); // to detect bubbles.

  /////* to detect out drops from drop sensor 1 *///////
  if (drop_counter_1_value > DR1_VAL_Threshold || dr1_edge_detected_flag == 1)
  {
    dr1_edge_detected_flag = 1;
    if (drop_counter_1_value < DR1_VAL_Threshold)
    {
      dr1_DropCount++;
      dr1_edge_detected_flag = 0;
    }
  }

  //////////////////////////////////////////////////////////////

  /////////To detect drop from drop counter 2//////////////////

  /////////////////////////////////////////////////////////////
  if (drop_counter_2_value > DR2_VAL_Threshold || dr2_edge_detected_flag == 1)
  {
    dr2_edge_detected_flag = 1;
    if (drop_counter_2_value < DR2_VAL_Threshold)
    {
      dr2_DropCount++;
      dr2_edge_detected_flag = 0;
    }// delay(20);
  }
  /**********************************************************/
  /*           to detect bubbles in line of flow            */
  /**********************************************************/
  if (bubble_detector_value <= BBD_VAL_Threshold)
  {
    Serial.println("bubble has been detected");
    bubble_detect_flag = 1;
  }

  else
  {
    bubble_detect_flag = 0;
  }

  if (bubble_detect_flag == 1)
  {
    digitalWrite(pinch_valve_pin , HIGH); /////////controlling pinch valve to stop flow of IV
  }

  if (bubble_detect_flag == 0)
  {
    digitalWrite(pinch_valve_pin , LOW); //controlling pinch valve To start flow of IV
  }

  if ((previousTime + 20000) <= currentTime)
  {
    previousTime = currentTime;
    sendDataToServer( 1 , 0);
    Serial.print("dropCount1 : ");
    Serial.println(dr1_DropCount);
    Serial.print("dropCount2 : ");
    Serial.println(dr2_DropCount);
    dr1_DropCount = 0 ;
    dr2_DropCount = 0 ;
  }
}
//////////////////////////////////////////////////////////////
/////////Sending data to server vai NRF24 and ESP8266//////////
//////////////////////////////////////////////////////////////

void sendDataToServer(int via_Gateway , int direct_To_Server)
{

  if (via_Gateway == 1)
  {
    via_Gateway = 0;
  }

  if (direct_To_Server == 1)
  {
    direct_To_Server = 0;
  }
}
