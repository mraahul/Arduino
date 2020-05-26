 int x=0;
int lastx=0;
int lasty=0;
int LastTime=0;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
 
#define UpperThreshold 430
#define LowerThreshold 410
 
void setup() {
 Serial.begin(9600);
}
 
 
void loop() 
{
  if(x>127)  
  {
//    oled.clearDisplay();
    x=0;
    lastx=x;
  }
 
  int value=analogRead(0);
  Serial.println(value);
 // oled.setTextColor(WHITE);
  int y=60-(value/16);
 // oled.writeLine(lastx,lasty,x,y,WHITE);
  lasty=y;
  lastx=x;
  // calc bpm
 
  if(value>UpperThreshold)
  {
    if(BeatComplete)
    {
      BPM=millis()-LastTime;
      BPM=int(60/(float(BPM)/1000));
      BPMTiming=false;
      BeatComplete=false;
    }
    if(BPMTiming==false)
    {
      LastTime=millis();
      BPMTiming=true;
    }
  }
  if((value<LowerThreshold)&(BPMTiming))
    BeatComplete=true;
    // display bpm
  //Serial.print(BPM);
//  Serial.println(" BPM");
  x++;
}
