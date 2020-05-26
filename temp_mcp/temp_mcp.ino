void setup() {
  // initialize the serial port
  Serial.begin(9600);
}

void loop() {
  float temperature = 0.0;   // stores the calculated temperature
  int sample;                // counts through ADC samples
  float ten_samples = 0.0;   // stores sum of 10 samples
 Serial.print(" Aval:- ");
    Serial.print(analogRead(A0)); 
  // take 10 samples from the MCP9700
  for (sample = 0; sample < 10; sample++) {
    // convert A0 value to temperature
   
    int reading = map(analogRead(A0),0,1023,0,4095);
    temperature = ( reading * 5.0 / 4095.0) ;
    temperature = (temperature - 0.5) / 0.01;
    // sample every 0.1 seconds
    delay(100);
    // sum of all samples
    ten_samples = ten_samples + temperature;
  }
  // get the average value of 10 temperatures
  temperature = ten_samples / 10.0;
  // send temperature out of serial port
      Serial.print("  ");
  Serial.print(temperature);
  Serial.println(" deg. C");
  ten_samples = 0.0;
}
