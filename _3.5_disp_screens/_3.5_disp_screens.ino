#pragma GCC optimize ("-O2")
#pragma GCC push_options

#include <Adafruit_GFX.h>    // Hardware-specific library
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <TouchScreen.h>
#include <avr/interrupt.h>
#include <ArduinoJson.h>
#include <ArduinoSort.h>
#include <SPI.h>
#define USE_SDFAT
#include <SdFat.h>           // Use the SdFat library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

SdFatSoftSpi<12, 11, 13> SD; //Bit-Bang on the Shield pins
#define SD_CS          10

#define FOSC 16000000UL// Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

#define PALETTEDEPTH          8     // support 256-colour Palette
#define BMPIMAGEOFFSET        54
#define BUFFPIXEL             20
#define NO_OF_PAGES           9
#define BUZZER                52
#define AIRBUBBLE             53
#define SCREENSAVER_INTERVAL  60
char namebuf[32] = "/";   //BMP files in root directory

typedef void (*pvFunc_Handler)(void);
typedef void (*updatesd) (String);

enum _tft_pages {
  _loginpage = 0,
  _gttvolpage,
  _flowratepage,
  _bedpage,
  _drugpage,
  _submedpage,
  _startstoppage,
  _alertpage,
  _screensaverpage
} _pages;


#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define BACK_G      0x0193  //0x036F  //0x026F
#define BUTTON      0x3C64  //0x0DAF
#define RECT        0x00AF  //0x30AF  //0x0DFF, 0xAD0F
#define KEYS        0xAD0F
#define GTT_BUTTON  0x3C64


//#define YP A1  // must be an analog pin, use "An" notation!   //do not used for 3.5" tft
//#define XM A2  // must be an analog pin, use "An" notation!
//#define YM 7   // can be a digital pin
//#define XP 6   // can be a digital pin

int XP = 7, XM = A1, YP = A2, YM = 6;  //most common configuration (3.5" tft)

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10
#define MAXPRESSURE 1000

boolean _page_token       = true;
boolean _bedflag          = true;
boolean _nextbed_flag     = false;
boolean _prevbed_flag     = false;
boolean _next_flag        = false;
boolean _subnext_flag     = false;
boolean _medflag          = true;
boolean _submedflag       = true;
boolean _prev_flag        = false;
boolean _subprev_flag     = false;
boolean _serial_event     = false;
boolean _start_count      = false;
boolean _enterid          = false;
boolean _enterpin         = false;
boolean _id_ok            = false;
boolean _volkeypad        = false;
boolean _startflow        = false;
boolean _screen_saver     = false;
boolean _readflag         = false;
boolean _readcatflag      = false;

volatile boolean _sensor1_flag     = false;
volatile boolean _stop_interrupt   = false;
volatile boolean _rxflag           = false;

volatile char rxbuff[256];
volatile int _rxi = 0;


uint8_t _change_flag = 0;
uint8_t _ack = 0;

float total_bedpagecnt = 0, total_pagecnt = 0, total_subpagecnt = 0;
float rate = 100, vol = 0;

String nurse_name         = "";
String bed_name           = "none";
String drug_name          = "none";
String serial_recv        = "";
//String rxbuff             = "";

String bedbuff[30];
String buff[25];
String parsebuff[60];
String submed[60];

int _page_no = 0;
int gtt = 0;
int bedindex = 0;
int drugindex = 0;
int drug = 0, ttf = 0, batt_lvl = 0;
int subdrug = 0;
int findex = 0, fcatindex = 0;
int bednm = 0, bed_pagecnt = 1, nm = 0, nm1 = 0, catIndex = 0, sub_catIndex = 0, cat_pagecnt = 1, subcat_pagecnt = 1;
unsigned int screencnt = 0;
uint8_t _rfid_flag = 0;
uint32_t pin = 0, id = 0;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
// TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TouchScreen myTouch(XP, YP, XM, YM, 300);
//TSPoint pt;

/****************************************************************************************************************/
/*------------------------------------ Diagnose Touch For 3.5" TFT LCD -----------------------------------------*/
/****************************************************************************************************************/

void showpins(int A, int D, int value, const char *msg)
{
  char buf[40];
  sprintf(buf, "%s (A%d, D%d) = %d", msg, A - A0, D, value);
  Serial.println(buf);
}

boolean diagnose_pins()
{
  int i, j, value, Apins[2], Dpins[2], Values[2], found = 0;
  //    Serial.begin(9600);
  Serial.println("Making all control and bus pins INPUT_PULLUP");
  Serial.println("Typical 30k Analog pullup with corresponding pin");
  Serial.println("would read low when digital is written LOW");
  Serial.println("e.g. reads ~25 for 300R X direction");
  Serial.println("e.g. reads ~30 for 500R Y direction");
  Serial.println("");
  for (i = A0; i < A5; i++) pinMode(i, INPUT_PULLUP);
  for (i = 2; i < 10; i++) pinMode(i, INPUT_PULLUP);
  for (i = A0; i < A4; i++) {
    for (j = 5; j < 10; j++) {
      pinMode(j, OUTPUT);
      digitalWrite(j, LOW);
      value = analogRead(i);   // ignore first reading
      value = analogRead(i);
      if (value < 100) {
        showpins(i, j, value, "Testing :");
        if (found < 2) {
          Apins[found] = i;
          Dpins[found] = j;
          Values[found] = value;
          found++;
        }
      }
      pinMode(j, INPUT_PULLUP);
    }
  }
  if (found == 2) {
    Serial.println("Diagnosing as:-");
    int idx = Values[0] < Values[1];
    for (i = 0; i < 2; i++) {
      showpins(Apins[i], Dpins[i], Values[i],
               (Values[i] < Values[!i]) ? "XM,XP: " : "YP,YM: ");
    }
    XM = Apins[!idx]; XP = Dpins[!idx]; YP = Apins[idx]; YM = Dpins[idx];
    //        myTouch = TouchScreen_kbv(XP, YP, XM, YM, 300); //Local library
    myTouch = TouchScreen(XP, YP, XM, YM, 300);     //Global library
    return true;
  }
  Serial.println("BROKEN TOUCHSCREEN");
  return false;
}

/****************************************************************************************************************/
/*---------------------------------------------- Display Text --------------------------------------------------*/
/****************************************************************************************************************/

void disp_text(int x, int y, int size, long color, char* data) {
  tft.setCursor(x, y + 12);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.print(data);
}

/****************************************************************************************************************/
/*------------------------------------------------ BUTTONS -----------------------------------------------------*/
/****************************************************************************************************************/

void draw_Button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);                  //fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  tft.drawFastHLine((x + 1), y, w, WHITE);          //drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
  tft.drawFastHLine((x + 1), (y + 1), w, WHITE);
  tft.drawFastVLine((x + w), y, h, WHITE);          //drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
  tft.drawFastVLine((x + w + 1), y, h, WHITE);
}

void draw_RoundButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h , uint16_t r, uint16_t color, uint16_t br_color) {
  tft.fillRoundRect(x, y, w, h, r, color);                  //fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  tft.drawRoundRect(x, y, w, h, r, br_color);
  tft.drawRoundRect(x, y, (w - 1), h, r, br_color);
  tft.drawRoundRect((x + 1), y, (w - 1), h, r, br_color);
}

void draw_Table(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  tft.drawFastHLine(x , y , w, color);
  tft.drawFastHLine(x , (y + h) , w, color);
  tft.drawFastHLine((x - 1) , (y + 1) , w, color);
  tft.drawFastHLine((x - 1) , (y + 1 + w) , w, color);
  tft.drawFastHLine(x , (y + 23) , w, color);

  tft.drawFastVLine(x , y , h, color);
  tft.drawFastVLine((x + 160) , (y + 23) , (h - 22) , color);
  tft.drawFastVLine((x - 1) , (y + 1) , h, color);
  tft.drawFastVLine((x + 159) , (y + 24) , (h - 22), color);
  tft.drawFastVLine((x + w) , (y + 1) , h, color);
  tft.drawFastVLine((x + w + 1) , (y + 1) , h, color);
}

void draw_Box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  tft.drawFastHLine(x , y , w, color);
}

void draw_Arrow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  int i = 0;
  tft.fillRect(x, y, w, h, color);
  i = h / 2;
  x = x + w;
  for (int k = 0; k < i - 1; k++) {
    h = h - 2;
    tft.drawFastVLine(x++, ++y, h, color);
    //    Serial.print("X = "); Serial.print(x);
    //    Serial.print(" Y = "); Serial.print(y);
    //    Serial.print(" H = "); Serial.print(h);
    //    Serial.println("");
  }
}

void draw_LeftArrow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  int i = 0, H = 0;
  i = h / 2;
  for (int k = 0; k < i - 1; k++) {
    H = H + 2;
    tft.drawFastVLine(x++, y--, H, color);
  }
  tft.fillRect((x), y, w, h, color);
}

void draw_DownArrow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  //  tft.fillRect(73, 210, 14, 40, WHITE);
  int ax = 0, aw = w;
  ax = x - (aw >> 1);
  tft.fillRect(x, y, w, h, color);
  y += h;

  for (int i = 0; i < aw ; i++) {
    tft.drawFastHLine(ax, y, (w * 2), color);
    ax++;
    y++;
    w--;
  }
}

void back_Button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  draw_Button(x, y, w, h, color);
  disp_text(214, 15, 2, WHITE, "  Back");
  //back_Button(214, 5, 101, 35, BUTTON);
}


/*****************************************************************************************************/
/*---------------------------------------------- KEYPAD ---------------------------------------------*/
/*****************************************************************************************************/

void disp_Keypad(void) {
  tft.fillScreen(0x0021);
  tft.setFont(&FreeMonoBold18pt7b);
  tft.fillRoundRect(90, 70, 300, 240, 20, WHITE);
  tft.drawRoundRect(90, 70, 300, 240, 20, 0xFE00);
  tft.drawRoundRect(91, 71, 298, 238, 20, 0xFE00);

  tft.drawFastHLine(90, 130, 300, 0xFE00);
  tft.drawFastHLine(90, 131, 300, 0xFE00);
  tft.drawFastHLine(90, 190, 300, 0xFE00);
  tft.drawFastHLine(90, 191, 300, 0xFE00);
  tft.drawFastHLine(90, 250, 300, 0xFE00);
  tft.drawFastHLine(90, 251, 300, 0xFE00);

  tft.drawFastVLine(165, 70, 240, 0xFE00);
  tft.drawFastVLine(166, 70, 240, 0xFE00);
  tft.drawFastVLine(240, 70, 240, 0xFE00);
  tft.drawFastVLine(241, 70, 240, 0xFE00);
  tft.drawFastVLine(315, 70, 240, 0xFE00);
  tft.drawFastVLine(316, 70, 240, 0xFE00);
  disp_text(118, 98, 1, 0x0021, "1");
  disp_text(192, 98, 1, 0x0021, "2");
  disp_text(267, 98, 1, 0x0021, "3");
  //disp_text(340, 98, 1, 0x0021, "<");
  draw_LeftArrow(340, 98, 5, 22, 0x0021);
  disp_text(118, 158, 1, 0x0021, "4");
  disp_text(192, 158, 1, 0x0021, "5");
  disp_text(267, 158, 1, 0x0021, "6");
  disp_text(340, 158, 1, 0x0021, "C");
  disp_text(118, 218, 1, 0x0021, "7");
  disp_text(192, 218, 1, 0x0021, "8");
  disp_text(267, 218, 1, 0x0021, "9");
  disp_text(340, 218, 1, 0xF000, "X");
  tft.fillCircle(127, 282, 2, 0x0021);
  disp_text(192, 278, 1, 0x0021, "0");
  disp_text(253, 278, 1, 0x0021, "");
  disp_text(325, 278, 1, 0x0021, "OK");
}

void simple_keypad (uint16_t x, uint16_t y, uint16_t B_color, uint16_t Bb_color, uint16_t br_color) {
  int _bx = x, _by = y;

  tft.fillRoundRect(x, y, 225, 200, 20, B_color);
  tft.drawRoundRect(x, y, 225, 200, 20, Bb_color);
  tft.drawRoundRect(x + 1, y + 1, 223, 199, 20, Bb_color);

  for (int i = 0; i < 3; i++) {
    _by = _by + 50;
    tft.drawFastHLine(_bx, _by, 225, Bb_color);
    tft.drawFastHLine(_bx, _by + 1, 225, Bb_color);
  }

  for (int i = 0; i < 2; i++) {
    _bx = _bx + 75;
    tft.drawFastVLine(_bx, y, 200, Bb_color);
    tft.drawFastVLine(_bx + 1, y, 200, Bb_color);
  }

  disp_text(x + 30, y + 25, 1, br_color, "1");
  disp_text(x + 105, y + 25, 1, br_color, "2");
  disp_text(x + 180, y + 25, 1, br_color, "3");

  disp_text(x + 30, y + 75, 1, br_color, "4");
  disp_text(x + 105, y + 75, 1, br_color, "5");
  disp_text(x + 180, y + 75, 1, br_color, "6");

  disp_text(x + 30, y + 125, 1, br_color, "7");
  disp_text(x + 105, y + 125, 1, br_color, "8");
  disp_text(x + 180, y + 125, 1, br_color, "9");

  draw_LeftArrow(x + 35, y + 175, 3, 20, br_color);
  disp_text(x + 105, y + 175, 1, br_color, "0");
  disp_text(x + 160, y + 175, 1, br_color, "OK");

}

void key_pressed(int key_val) {
  static String pstr = " ";
  String str = String(key_val);
  int len = 0;
  static uint8_t num_chk = 0;
  static boolean dot_flag = true;

  Serial.println("Key Pressed");
  if (key_val >= 0 && key_val <= 9) {
    if (pstr == " ") {
      pstr = str;
      num_chk = 0;
    }
    else if (num_chk <= 1) {
      pstr += str;
      Serial.println("Concat");
      if (dot_flag == false) {
        num_chk++;
      }
    }
  }

  if (key_val == 12 && dot_flag == true) {    // Check if '.' is pressed
    Serial.println(". Detected");
    dot_flag = false;
    if (pstr == " ") {
      pstr = "0.";
    }
    else {
      pstr += ".";
    }
  }

  if (((pstr.length() >= 5) && dot_flag == true) || (atof(pstr.c_str()) > 3000.00)) {   // Check if string length >= 5 if no '.' is present and also if value is > 1200 then reset it to max value
    pstr = "3000";
    draw_RoundButton(100, 10, 280, 50, 15, WHITE, 0xFE00);  //clear value field
  }

  if (key_val == 10) {    // Check if Backspace was pressed

    len = pstr.length();
    Serial.print("len = ");
    Serial.println(len);
    tft.fillRect((116 + (20 * (len - 1))), 15, 24, 40, WHITE);
    pstr = pstr.substring(0, len - 1);
    if (pstr.length() == 0) {
      pstr = " ";
      dot_flag = true;
      num_chk = 0;
    }
    Serial.print(F("Old_val = "));
    Serial.println(pstr);
    //disp_text(8, 40, 2, BLACK, (String(old_val)).c_str());
  }

  if (key_val == 11) {    // Check if Clear was pressed

    //    tft.fillRect(85, 65, 310, 40, WHITE);  //  Shows Entered Value.
    //    tft.drawFastHLine(85, 64, 311, 0xAD0F);
    //    tft.drawFastHLine(85, 105, 311, 0xAD0F);
    //    tft.drawFastVLine(84, 64, 41, 0xAD0F);
    //    tft.drawFastVLine(395, 64, 41, 0xAD0F);
    draw_RoundButton(100, 10, 280, 50, 15, WHITE, 0xFE00);
    pstr = " ";
    dot_flag = true;
    num_chk = 0;
  }

  if (key_val == 14) {    // Cancel
    pstr = " ";
    dot_flag = true;
    num_chk = 0;
    _volkeypad = false;
  }

  if (_volkeypad == true && key_val == 13) {    // OK
    Serial.println("Volume");
    Serial.print("PSTR = ");
    Serial.println(pstr);
    vol = atof(pstr.c_str());
    pstr = " ";
    dot_flag = true;
    num_chk = 0;
    _volkeypad = false;
  }
  tft.setFont(&FreeMonoBold18pt7b);
  disp_text(115, 32, 1, 0x0021, pstr.c_str());
  tft.setFont(&FreeSans9pt7b);
}

void simplekey_pressed(uint16_t key_val) {
  static String npstr = "", disp = "";
  String str = String(key_val);
  int len = 0;
  static uint8_t num_chk = 0;
  tft.fillRect(260, 1, 220, 20, 0x0021);
  Serial.println("Key Pressed");

  tft.setFont(&FreeSans9pt7b);
  if (_enterpin == true) {
    len = npstr.length();
    if (len < 4) {
      if (key_val >= 0 && key_val <= 9) {
        if (npstr == " ") {
          npstr = str;
          num_chk = 0;
          disp = "x";
        }
        else if (num_chk <= 1) {
          //strcat(pstr, str.c_str());
          npstr += str;
          disp += "x";
          Serial.println("Concat");
          //          Serial.println(disp);
        }
      }
    }

    if (key_val == 10) {
      //clear;
      npstr = "";
      num_chk = 0;  
      disp = "";
      draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
      disp_text(268, 80, 1, 0xA510, "Enter Pin");
    }

    if (key_val == 11) {
      //ok;
      Serial.println(npstr);
      if (len < 4) {
        Serial.println(F("Pin must contain only 4 digit."));
        //disp_text(248, 2, 1, BLACK, "Pin must have 4 digit.");
        // tft.fillRect(240, 60, 240, 20, 0x0193);
        npstr = "";
        num_chk = 0;
        disp = "";
        draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xF000);
        disp_text(268, 80, 1, 0xA510, "Invalid Pin");
      }
      else {
        pin = npstr.toInt();
        npstr = "";
        num_chk = 0;
        disp = "";
        Serial.println(F("Pin Check"));
      }
    }
    disp_text(268, 80, 1, 0x0021, disp.c_str());
  }
  else if (_enterid == true) {
    //npstr = "";
    //num_chk = 0;
    if (key_val >= 0 && key_val <= 9) {
      if (npstr == " " || npstr == "") {
        npstr = str;
        num_chk = 0;
      }
      else if (num_chk <= 1) {
        //strcat(pstr, str.c_str());
        npstr += str;
        Serial.println("Concat");
      }
    }

    if (key_val == 10) {    // Check if Backspace was pressed

      len = npstr.length();
      Serial.print("len = ");
      Serial.println(len);


      if (npstr.length() == 0) {
        npstr = "";
        num_chk = 0;
      }
      else {
        tft.fillRect((268 + (10 * (len - 1))), 22, 12, 36, WHITE);
      }
      npstr = npstr.substring(0, len - 1);
      Serial.print(F("Old_val = "));
      Serial.println(npstr);
      //disp_text(8, 40, 2, BLACK, (String(old_val)).c_str());
    }

    if (key_val == 11) {
      id = npstr.toInt();
      Serial.print("Nurse ID = ");
      Serial.println(id);
      npstr = "";
      num_chk = 0;
    }

    disp_text(268, 35, 1, 0x0021, npstr.c_str());
  }
}

/*****************************************************************************************************/
/*---------------------------------------------- ICONS ----------------------------------------------*/
/*****************************************************************************************************/

void draw_PatientIcon(int x, int y, uint16_t color, uint16_t bcolor) {
  /****PATIENT*****/
  //  tft.drawFastVLine(1, 5, 40, 0x0193);
  //  tft.drawFastVLine(2, 5, 40, 0x0193);
  //  tft.drawFastVLine(3, 5, 40, 0x0193);
  //  tft.drawFastVLine(4, 5, 40, 0x0193);
  int _px = x, _py = y;
  for (int i = 0; i < 4; i++) {
    tft.drawFastVLine(_px + i, _py, 40, color);
  }

  _px = x + 3;
  _py = y + 18;

  //  tft.drawFastHLine(4, 23, 45, 0x0193);
  //  tft.drawFastHLine(4, 24, 45, 0x0193);
  //  tft.drawFastHLine(4, 25, 45, 0x0193);
  //  tft.drawFastHLine(4, 26, 45, 0x0193);
  //  tft.drawFastHLine(4, 27, 45, 0x0193);
  //  tft.drawFastHLine(4, 28, 45, 0x0193);
  for (int i = 0; i < 6; i++) {
    tft.drawFastHLine(_px, _py + i, 45, color);
  }

  _px = x + 44;
  _py = y + 20;

  //  tft.drawFastVLine(45, 25, 20, 0x0193);
  //  tft.drawFastVLine(46, 25, 20, 0x0193);
  //  tft.drawFastVLine(47, 25, 20, 0x0193);
  //  tft.drawFastVLine(48, 25, 20, 0x0193);

  for (int i = 0; i < 4; i++) {
    tft.drawFastVLine(_px + i, _py, 20, color);
  }

  _px = x + 14;
  _py = y + 8;

  //  tft.fillRoundRect(15, 13, 34, 15, 7, 0x0193);
  tft.fillRoundRect(_px, _py, 34, 15, 7, color);

  _px = x + 9;
  _py = y + 8;
  //  tft.fillCircle(10, 13, 4, 0x0193);
  tft.fillCircle(_px, _py, 4, color);

  _px = x + 25;
  _py = y - 5;

  //  tft.drawFastVLine(26, 0, 50, 0x3679);
  //  tft.drawFastVLine(27, 0, 50, 0x3679);
  tft.drawFastVLine(_px, _py, 45, bcolor);
  tft.drawFastVLine(++_px, _py, 45, bcolor);

  _px = x + 14;
  _py = y + 17;

  //  tft.drawFastHLine(15, 22, 11, 0x3679);
  //  tft.drawFastHLine(15, 21, 11, 0x3679);
  tft.drawFastHLine(_px, _py, 11, bcolor);
  tft.drawFastHLine(_px, --_py, 11, bcolor);
  /****PATIENT*****/
  tft.fillRect(x + 5, y + 15, 8, 3, color);
}

void draw_DropIcon(int x, int y, int r, int offset, uint16_t color, uint16_t bcolor) {
  tft.fillCircle(x, y, r, color);
  tft.fillRect((x - r), (y - r), (r * 2) + 5, r, bcolor);
  tft.fillTriangle(x, (r + offset), (x - r), y, (x + r), y, color);
}

void draw_BottleIcon(int x, int y, int w, int h, int r, int offset, uint16_t borcolor, uint16_t bottcolor) {

  tft.fillCircle((x + (w / 2)), (y - 3), (r / 2), borcolor);

  tft.fillRoundRect(x, y , w, h, r + offset, borcolor);
  tft.fillRoundRect((x + (r / 2)), (y + (r / 2)), (w - r), (h - r), r + offset, bottcolor);
  tft.fillRoundRect((x + (r / 2) + 1) + offset, (y + (h / 2)) - 1, (w - (r / 2) * 3) - offset, (h * 0.45), (r - 2) + offset, borcolor);

  int _by = y + (w / 4), _br = r;
  tft.drawFastHLine(x + (w >> 1), _by, ((w >> 1) - (r >> 1)), borcolor);
  if (r <= 4) {
    _br = r >> 1;
  }
  _by = y + (w / 4) + _br - offset;
  tft.drawFastHLine((x + (w >> 1) + (r - offset) * 2), _by, ((w >> 1) - (r >> 1)) - ((r - offset) * 2), borcolor);
  _by = y + (w / 4) + (_br - offset) * 2;
  tft.drawFastHLine(x + (w >> 1), _by, ((w >> 1) - (r >> 1)), borcolor);
  _by = y + (w / 4) + (_br - offset) * 3;
  tft.drawFastHLine((x + (w >> 1) + (r - offset) * 2), _by, ((w >> 1) - (r >> 1)) - ((r - offset) * 2), borcolor);
  tft.fillRoundRect(x + (w / 3), (y + h), (w / 3), _br - offset, (_br >> 1), borcolor);
}

void draw_HumanIcon(int x, int y, int r, uint16_t color, uint16_t bgcolor) {

  int _hr = r, _hy = y;

  tft.fillCircle(x, y, r, color);
  _hr = r * 3;
  _hy = _hy + _hr + (r * 1.25);
  Serial.println(_hy);
  tft.fillCircle(x, _hy, _hr, color);
  _hr = r * 5;
  _hy = _hy + _hr + (r * 0.5) - r;
  Serial.println(_hy);
  tft.fillCircle(x, _hy, _hr, bgcolor);
}

void draw_DrugIcon(int x, int y, uint16_t color) {
  tft.drawLine( x + 5, y + 45, x + 45, y + 5, color); // centre line

  tft.drawLine( x + 13, y + 36, x + 17, y + 32, color); // centre line 1
  tft.drawLine( x + 13, y + 37, x + 17, y + 33, color); // centre line 2 - fill color

  tft.drawLine( x + 14, y + 37, x + 18, y + 33, color); // centre line 2

  for (int r = 1; r <= 7; r++)
  {
    tft.drawLine( x + 14 + r, y + 28 + r, x + 34 + r, y + 8 + r , color); //ux

    tft.drawLine( x + 14 + r, y + 28 + r, x + 34 + r / 2, y + 8 + r / 2 , color); //ux color fill
    tft.drawLine( x + 14 + r / 2, y + 28 + r / 2, x + 34 + r, y + 8 + r , color); //ux color filll

    tft.drawLine( x + 14, y + 28, x + 34 + r, y + 8 + r , color); //ux color fill
    tft.drawLine( x + 14 + r, y + 28 + r, x + 34, y + 8 , color); //ux color fill

    tft.drawLine( x + 14, y + 28, x + 34 + r / 2, y + 8 + r / 2 , color); //ux color fill
    tft.drawLine( x + 14 + r / 2, y + 28 + r / 2, x + 34, y + 8 , color); //ux color fill

    tft.drawLine( x + 14 + r / 2, y + 28 + r / 2, x + 34, y + 8 , color); //ux color fill
  }
  tft.drawLine( x + 22, y + 36, x + 42, y + 16 , color); // down


  tft.drawLine( x + 43, y + 2, x + 48, y + 7, color); //3  - line 1
  tft.drawLine( x + 42, y + 3, x + 47, y + 8, color); //3    -line 2
  tft.drawLine( x + 43, y + 3, x + 47, y + 7, color); //3   - fill color


  tft.drawLine( x + 31, y + 6, x + 44, y + 19, color); //2  -line 1
  tft.drawLine( x + 32, y + 5, x + 45, y + 18, color); //2  -line 2
  tft.drawLine( x + 31, y + 5, x + 45, y + 19, color); //2  fill color


  tft.drawLine( x + 14, y + 27, x + 23, y + 36, color); //1
}

void draw_BatteryIcon(int x, int y, int w, int h, uint16_t color, uint16_t bgcolor) {
  int _bx = x;
  tft.fillRect(x, y, w, h, color);
  tft.fillRect(x + 2, y + 2, w - 4, h - 4, bgcolor);
  _bx = _bx + (w / 4);
  tft.drawFastVLine(_bx, y, h, color);
  _bx = _bx + (w / 4);
  tft.drawFastVLine(_bx, y, h, color);
  _bx = _bx + (w / 4);
  tft.drawFastVLine(_bx, y, h, color);
  tft.fillRect((x + w), (y + (h / 4)), 2, (h / 2), color);
}

void draw_FlowrateIcon(uint16_t circle1_color, uint16_t circle2_color) {
  /*****FRICON****/
  tft.fillCircle(120, 25, 20, circle1_color);   //0xF980
  tft.fillCircle(120, 25, 15,  circle2_color);    //0xFFEF
  tft.fillCircle(120, 25, 4, circle1_color);      //0xF980
  draw_DownArrow(119, 25, 4, 11, circle1_color);  //0xF980
  /*****FRICON****/
}

void draw_Startbutton() {
  tft.fillCircle(360, 25, 19, 0xF980);
  tft.fillTriangle(355, 14, 370, 24, 355, 34, WHITE);
}

void draw_Stopbutton() {
  tft.fillCircle(360, 25, 19, 0xF980);
  tft.fillRect(352, 15, 6, 20, WHITE);
  tft.fillRect(362, 15, 6, 20, WHITE);
}

void draw_AlertIcon() {
  tft.fillTriangle(440, 5, 420, 40, 460, 40, 0xEE40);
  tft.fillRect(438, 16, 4, 15, BLACK);
  tft.fillCircle(439, 35, 2, BLACK);
}
/*****************************************************************************************************/
/*-------------------------------------------- Load Icons -------------------------------------------*/
/*****************************************************************************************************/

void load_icons() {
  tft.fillScreen(0x4FFF);

  for (int i = 0; i < 6; i++)
  {
    tft.drawRect(i * 80, 0, 79, 50, 0x4A69);
    //    tft.fillRect(i * 80, 51, 79, 3, 0xF980);
    tft.drawFastVLine((80 * i) - 1, 0, 50, 0x4A69);
  }
  //  tft.drawFastVLine(79, 0, 50, 0xA510);
  //  tft.drawFastVLine(159, 0, 50, 0xA510);


  //tft.fillRect(0, 55, 480, 210,  0x0002);
  //tft.fillRect(0, 55, 70, 210,  0x02B5);



  tft.drawRect(0, 266, 79, 53,  0x4A69);
  tft.drawFastVLine(79, 266, 53, 0x4A69);
  tft.drawRect(80, 266, 109, 53 ,  0x4A69);
  tft.drawFastVLine(189, 266, 53, 0x4A69);
  tft.drawRect(190, 266, 109 , 53,  0x4A69);
  tft.drawFastVLine(299, 266, 53, 0x4A69);
  tft.drawRect(300, 266, 179, 53,  0x4A69);

  draw_DropIcon(40, 30, 15, -10,  0xF980, 0x4FFF);
  draw_DropIcon(15, 300, 12, 268, 0x3186, 0x4FFF);

  //  disp_text(44, 285, 1, BLACK, "60");
  //  disp_text(35, 300, 1, BLACK, "dpml");

  draw_FlowrateIcon(0xF980, 0xFFEF);

  draw_PatientIcon(175, 5, 0xF980,  0x4FFF);
  draw_PatientIcon(193, 275, 0x3186,  0x4FFF);

  tft.setFont(&FreeSans9pt7b);
  disp_text(244, 285, 1, BLACK, "BED");
  //disp_text(244, 300, 1, BLACK, "101A");
  disp_text(244, 300, 1, BLACK, bed_name.c_str());


  draw_DrugIcon(250, 1, 0xF980);


  draw_BottleIcon(85, 277, 25, 35, 4, 0, 0x3186,  0x4FFF);
  //  disp_text(114, 285, 1, BLACK, "3000.00");
  disp_text(130, 300, 1, BLACK, "ml");

  disp_text(302, 285, 1, BLACK, "SIV NO : 205");

  // tft.fillRect(0, 51, 79, 3, 0xF980);

  //-------------------battery----------------------//

  tft.fillRect(459, 276, 10, 3, BLACK);

  for (int i = 0; i < 2; i++)
    tft.drawRect(454 - i, 280 - i, 20 + (i * 2), 30 + (i * 2), BLACK);

  for (int i = 0; i < 5; i++)
    tft.fillRect(458, 283 + i * 5, 12, 4, BLACK);
  //======================  battery  ===========================//


  //------------------------------ charger -------------------------//

  tft.fillCircle(430, 292, 18,  0x0900);  //4560,3CA0,22C0
  tft.fillCircle(430, 292, 15,   0x4FFF);
  tft.fillCircle(430, 292, 10,  0x0900);
  tft.fillRect(418, 282, 24, 10,  0x4FFF);
  tft.fillRect(433, 305, 8, 8,  0x4FFF);
  tft.fillRect(429, 302, 4, 8, 0x0900);

  tft.fillRect(423, 280, 3, 10, 0x0900);
  tft.fillRect(435, 280, 3, 10, 0x0900);

  //======================== charger =================================//



  //--------------to pause the flow----------------//
  draw_Startbutton();

  //================== to pause the flow ===========================//


  //----------------------Alert---------------------//
  // tft.fillTriangle(73, 34, 10, 63, 73, 93,  RED);
  draw_AlertIcon();
  //=======================  Alert ===============================//
}

/****************************************************************************************************************/
/*--------------------------------------------- Update Functions -----------------------------------------------*/
/****************************************************************************************************************/
void Func_Nurse(String data) {
  Serial.println("Nurse Update.");


}

void Func_Norm(String Data) {
  String _temp = "";
  Serial.println("Normal Update.");
  StaticJsonBuffer < 256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(Data);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  //( { "Flag":"A0", "Flowrate": "123", "TTF": 47, "Vol": "98.16666", "Infused": "1.833333", "MAC": "60:01:94:23:7E:AD", "flowAlert": "0", "ttfAlert": "4", "Bed_No": "150", "Nurse_Id": "111", "Wi-Fi": "0" } )
  float ratecount = atof(root["Flowrate"]);
  float volremain = atof(root["Vol"]);
  float volcount = atof(root["Infused"]);
  String MAC = root["MAC"];
  uint8_t flowflag = atoi(root["flowAlert"]);
  int hour = 0, min = 0;
  ttf = root["TTF"];

  if (_startflow == true) {
    //_page_no = _flowratepage;
    //_serial_flag = false;
    //serial_recv = "";
    if (_startflow == true && _page_no == _flowratepage) {

      tft.setFont(&FreeSansBold18pt7b);
      tft.fillRect(268, 85, 200, 40,  0x0021);
      disp_text(270, 105, 1, WHITE , String(ratecount).c_str());

      tft.setFont(&FreeSans12pt7b);
      disp_text(270, 225, 1, WHITE, drug_name.c_str());
      disp_text((270 + (String(ratecount).length() * 20)) , 105, 1, WHITE, "ml/hr");

      tft.setFont(&FreeSansBold18pt7b);
      hour = ttf / 60;
      if (hour < 9) {
        _temp = "0";
        _temp += String(hour);
      }
      else {
        _temp = String(hour);
      }
      tft.fillRect(268, 145, 40, 40,  0x0021);
      disp_text(270, 165, 1, WHITE, _temp.c_str());

      min = ttf - (hour * 60);
      if (min < 9) {
        _temp = "0";
        _temp += String(min);
      }
      else {
        _temp = String(min);
      }
      tft.fillRect(345, 145, 40, 40,  0x0021);
      disp_text(345 , 165, 1, WHITE, _temp.c_str());
      tft.setFont(&FreeSans12pt7b);
    }
    else if (_startflow == true && _page_no == _screensaverpage) {

      tft.setFont(&FreeSansBold18pt7b);
      tft.fillRect(200, 70, 150, 40,  0x0021);
      disp_text(200, 90, 1, WHITE , String(volcount).c_str());    //len*20


      //volremain = 100 - volcount;
      tft.fillRect(200, 120, 150, 40,  0x0021);
      disp_text(200, 140, 1, WHITE , String(volremain).c_str());


      tft.fillRect(5, 220, 250, 40,  0x0021);
      disp_text(5 , 240, 1, WHITE , String(ratecount).c_str());
      Serial.print("Ratecount = ");
      Serial.println(ratecount);
      tft.setFont(&FreeSans12pt7b);
      disp_text((200 + (String(volremain).length() * 18)), 140, 1, WHITE, "ml");
      disp_text((200 + (String(volcount).length() * 18)), 90, 1, WHITE, "ml");
      disp_text(5 + (String(ratecount).length() * 18), 240, 1, WHITE, "ml/hr");

      tft.setFont(&FreeSansBold18pt7b);
      hour = ttf / 60;
      if (hour < 9) {
        _temp = "0";
        _temp += String(hour);
      }
      else {
        _temp = String(hour);
      }
      tft.fillRect(310, 220, 40, 40,  0x0021);
      disp_text(310 , 240, 1, WHITE , _temp.c_str());

      min = ttf - (hour * 60);
      if (min < 9) {
        _temp = "0";
        _temp += String(min);
      }
      else {
        _temp = String(min);
      }
      tft.fillRect(375, 220, 40, 40,  0x0021);
      disp_text(375, 240, 1, WHITE , _temp.c_str());
      tft.setFont(&FreeSans12pt7b);
      disp_text(195, 185, 1, WHITE, drug_name.c_str());
    }
  }
  String pay = "{\"Flag\":A0,\"Ack\":0}";
  char datbuff[pay.length() + 1];
  pay.toCharArray(datbuff, pay.length() + 1);
  Serial.println(pay);
  for (int j = 0; j < pay.length(); j++) {
    USART_Transmit(datbuff[j]);
    Serial.print(datbuff[j]);
    //delay(1);
  }
  USART_Transmit('\n');
  //Serial1.println("{\"Flag\":A0,\"Ack\":0}");
}

void sort_String (int last) {
  int x1 = 0, x2 = 0;

  Serial.print("Sort Last index = ");
  Serial.println(last);

  String _tempbuff[last];
  const char *sbuff[last];

  for (int i = 0; i < last; i++) {
    sbuff[i] = parsebuff[i].c_str();
  }
  Serial.println("Sending");

  int n = sizeof (sbuff) / sizeof (sbuff[0]);
  //  sort (sbuff, n);
  Serial.println(F("/*******UPDATED DRUGNAME LIST*******/"));
  for (int i = 0; i < last; i++) {
    Serial.println(sbuff[i]);
  }

  String parsebuff1 = "";
  for (int i = 0; i < last; i++) {
    //    submed[i] = sbuff[i];
    //    submed[i] += "\n";
    parsebuff1 += sbuff[i];
    submed[i] = parsebuff1;
    parsebuff1 = "";
    //String parsebuff1(sbuff[i]);
    //parsebuff1 += "\n";
  }

  Serial.println(F("/*******DRUGNAME LIST*******/"));
  for (int i = 0; i < last; i++) {
    Serial.println(submed[i]);
  }
}

void Func_Drug(String Data) {
  //_submedflag = true;
  _medflag = true;
  Serial.println("Drug Update.");
  Data.trim();
  String str = "";
  String buff[20];
  String subbuff[20];
  int x1 = 0, x2 = 0, rec = -1, len = 0, i0 = 0, i1 = 0, k = 0;
  int drug_lastindex = 0;
  StaticJsonBuffer < 256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(Data);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  Serial.println("");
  String Drugcount = root["Drugcount"];
  String Drugcat   = root["Drugcat"];
  String Drugname[5];

  for (int i = 0; i < Drugcount.toInt(); i++) {
    String Drugname0 = root["Drugname"][i];
    Drugname[i] = Drugname0;
  }

  for (int i = 0; i < Drugcount.toInt(); i++) {
    Serial.println(Drugname[i]);
  }
  File myFile;
  String _temp1 = "";
  String _temp = "/druglist/";
  _temp += Drugcat;
  _temp += ".txt";
  _temp.trim();


  if (SD.exists(_temp.c_str())) {

    Serial.println("Exists");

    str = "";
    if (_readflag == false) {
      myFile = SD.open(_temp.c_str(), O_READ);
      while (myFile.available()) {
        str += (char)myFile.read();
        //Serial.println(str);
      }

      myFile.close();
      str.trim();
      _temp1 = str;
      len = _temp1.length();
      i0 = _temp1.lastIndexOf("#", len);
      i1 = _temp1.lastIndexOf("\n", i0);
      _temp1 = _temp1.substring(i1 + 1, i0);
      _temp1.trim();
      drug_lastindex = (_temp1).toInt();
    }
    else if (_readflag == true) {
      //      for (int i = 0; i < findex; i++) {
      //        str += submed[i];
      //      }
      //      str.trim();
      drug_lastindex = findex;
    }

    Serial.println(str);
    Serial.print("updt Last index = ");
    Serial.println(drug_lastindex);
    //    Serial.println("TEST");
    String str1 = str;
    str1.trim();
    str1 += "\n";

    // myFile.flush();
    File SDfile;
    SDfile = SD.open(_temp.c_str(), FILE_WRITE); //_temp.c_str()

    if (SDfile) {
      for (int m = 0; m < Drugcount.toInt(); m++) {
        //str1 += String(drug_lastindex + 1) + "#" + Drugname[m] + "\n";
        SDfile.println(String(drug_lastindex + 1) + "#" + Drugname[m]);
        drug_lastindex++;
      }
    }
    else {
      Serial.println("Error Updating File");
    }
    SDfile.close();
    // Serial.println(str1);
    /*
        x1 = 0; x2 = 0;
        for (int i = 0; i < drug_lastindex; i++) {      //read into buff
          x1 = str1.indexOf("#", x2);
          x2 = str1.indexOf("\n", x1);
          parsebuff[i] = str1.substring(x1 + 1, x2);
          Serial.print("parsebuff = ");
          Serial.println(str1.substring(x1 + 1, x2));
        }

        sort_String(drug_lastindex);
    */
  }
  else {

    myFile = SD.open("/druglist/" + Drugcat + ".txt", FILE_WRITE);
    for (int i = 0; i < Drugcount.toInt(); i++) {
      _temp = String(i + 1) + "#" + Drugname[i];
      myFile.println(_temp);
    }
    myFile.close();


    if (_readcatflag == false) {
      myFile = SD.open("/druglist/catlist.txt");
      while (myFile.available()) {
        _temp = myFile.readString();
      }
      Serial.println(_temp);
      myFile.close();
      _temp.trim();
      len = _temp.length();
      i0 = _temp.lastIndexOf("#", len);
      i1 = _temp.lastIndexOf("\n", i0);
      _temp = _temp.substring(i1 + 1, i0);
      //_temp.trim();
      drug_lastindex = _temp.toInt();
    }
    else if (_readcatflag == true) {
      drug_lastindex = fcatindex;
    }



    Serial.print("Last index = ");
    Serial.println(drug_lastindex);

    myFile = SD.open("/druglist/catlist.txt", FILE_WRITE);
    if (myFile) {
      _temp = String(drug_lastindex + 1) + "#" + Drugcat;
      myFile.println(_temp);
    }
    else {
      Serial.println(F("Error Updating File."));
    }
    myFile.close();

    //    for (int i = 0; i < drug_lastindex; i++) {      //change buff len
    //      x1 = str.indexOf("#", x2);
    //      x2 = str.indexOf("\n", x1);
    //      buff[i] = str.substring(x1 + 1, x2);
    //    }
    //
    //    for (int i = 0; i < drug_lastindex; i++) {          //change buff len
    //      Serial.print("Buff = ");
    //      Serial.println(buff[i]);
    //    }
    //
    //    for (int i = 0; i < drug_lastindex; i++) {
    //      if (Drugcat.compareTo(buff[i]) > 0) {
    //        Serial.println("Drugcat is greater");
    //      }
    //      else {
    //        Serial.println("Drugcat is smaller");
    //        rec = i + 1;
    //        Serial.print("Rec = ");
    //        Serial.println(rec);
    //        break;
    //      }
    //    }
    //    k = 0;
    //
    //    if (rec != -1) {
    //      SD.remove("/druglist/catlist.txt");
    //      myFile = SD.open("/druglist/catlist.txt", FILE_WRITE);
    //      for (int j = 0; j < drug_lastindex + 1; j++) {
    //        if ((j + 1) == rec) {
    //          _temp = String(rec) + "#" + Drugcat;
    //        }
    //        else {
    //          _temp = String(j + 1) + "#" + buff[k];
    //          k++;
    //        }
    //
    //        myFile.println(_temp);
    //      }
    //      myFile.close();
    //    }
    //    else {
    //      myFile = SD.open("/druglist/catlist.txt", FILE_WRITE);
    //      _temp = String(drug_lastindex + 1) + "#" + Drugcat;
    //      myFile.println(_temp);
    //      myFile.close();
    //
    //      myFile = SD.open("/druglist/catlist.txt");
    //      while (myFile.available()) {
    //        _temp = myFile.readString();
    //      }
    //      Serial.println(_temp);
    //      myFile.close();
    //    }
    //
    //    Serial.println(F("/*******UPDATED DRUGCAT LIST*******/"));
    //    Serial.println("");
    //    str = "";
    //    _temp = "/druglist/catlist.txt";
    //    if (myFile.open(_temp.c_str(), O_READ)) {
    //      while (myFile.available()) {
    //        str += (char)myFile.read();
    //      }
    //      Serial.println(str);
    //      myFile.close();
    //    }
    //    else {
    //      Serial.println("Error Opening File.");
    //    }

    Serial.println("done.");
    Serial.println("Does not Exists");
  }
  //Serial1.println("{\"Flag\":A2,\"Ack\":1}");
  String pay = "{\"Flag\":A0,\"Ack\":1}";
  char datbuff[pay.length() + 1];
  pay.toCharArray(datbuff, pay.length() + 1);
  Serial.println(pay);
  for (int j = 0; j < pay.length(); j++) {
    USART_Transmit(datbuff[j]);
    Serial.print(datbuff[j]);
    //delay(1);
  }
  USART_Transmit('\n');
}

void Func_Bed(String data) {
  Serial.println("Bed Update.");
}

int jsonparser(String Data) {
  Data.trim();
  Serial.println(Data);
  StaticJsonBuffer <256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(Data);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  Serial.println("");
  String Flag = root["Flag"];

  Serial.println(Flag);
  if (Flag == "A0") {
    _ack = 0;
    Serial.println("Norm");
    //Serial1.println("{\"Flag\":A0,\"Ack\":0}");
    return 0;
  }
  else if (Flag == "A1") {
    _ack = 1;
    Serial.println("Nurse");
    //Serial1.println("{\"Flag\":A1,\"Ack\":1}");
    return 1;
  }
  else if (Flag == "A2") {
    _ack = 1;
    Serial.println("Drug");
    //Serial1.println("{\"Flag\":A2,\"Ack\":1}");
    return 2;
  }
  //return (Flag.toInt());
}
/****************************************************************************************************************/
/*------------------------------------------------- PAGES ------------------------------------------------------*/
/****************************************************************************************************************/

void Func_Loginpage() {
  tft.fillScreen(0x0021);
  _change_flag = 0;
  tft.setFont(&FreeSans9pt7b);
  draw_HumanIcon(80, 100, 20, WHITE, 0x0021);
  draw_DownArrow(73, 210, 14, 40, WHITE);
  disp_text(38, 150, 1, 0x0021, "Scan Card");

  tft.setFont(&FreeMonoBold18pt7b);
  disp_text(160, 160, 1, WHITE, "OR");
  simple_keypad(240, 110, WHITE, 0xFE00, 0x0021);
  draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
  draw_RoundButton(260, 20, 185, 40, 5, WHITE, 0xFE00);

  tft.setFont(&FreeSans9pt7b);
  disp_text(268, 35, 1, 0xA510, "Enter ID");   //0xCCC0 0x3300 0x0001 0xC618 0xA510
  disp_text(268, 80, 1, 0xA510, "Enter Pin");   //0xCCC0 0x3300 0x0001 0xC618 0xA510
}

void Func_GttVolpage() {
  tft.fillRect(0, 0, 78, 49, 0x0193);
  draw_DropIcon(40, 30, 15, -10,  0xF980, 0x0193);

  tft.fillRect(0, 50, 480, 5, 0x4FFF);  
  tft.fillRect(1, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 210,  0x0021);
  tft.fillRect(0, 55, 70, 210,  0x0111);

  draw_DropIcon(35, 115, 30, 35, WHITE, 0x0111);
  tft.drawFastHLine(70 , 160 , 420, 0x0193);

  tft.setFont(&FreeSansBold12pt7b);
  disp_text(12, 115, 1, 0x0111, "GTT");

  draw_BottleIcon(8, 180 , 55, 75, 6, 1, WHITE, 0x0111);

  tft.setFont(&FreeSans18pt7b);
  disp_text(100, 110, 1, WHITE, "10");
  disp_text(190, 110, 1, WHITE, "15");
  disp_text(280, 110, 1, WHITE, "20");
  disp_text(370, 110, 1, WHITE, "60");

  disp_text(100, 200, 1, WHITE, "25");
  disp_text(190, 200, 1, WHITE, "50");
  disp_text(280, 200, 1, WHITE, "75");
  disp_text(370, 200, 1, WHITE, "100");
  tft.setFont(&FreeMonoBold18pt7b);
  disp_text(450, 200, 1, WHITE, "+");

  tft.fillRoundRect(240, 230, 80, 30, 8, 0xF980);
  tft.setFont(&FreeSans12pt7b);
  disp_text(260, 240, 1, WHITE, "OK");

  tft.setFont(&FreeSans9pt7b);
  tft.fillRect(42, 282, 25, 18, 0x4FFF);
  disp_text(44, 285, 1, BLACK, String(gtt).c_str());
  disp_text(35, 300, 1, BLACK, "dpml");

  tft.fillRect(112, 282, 70, 18, 0x4FFF);
  disp_text(114, 285, 1, BLACK, String(vol).c_str());
  _stop_interrupt = false;
}

void Func_Flowratepage() {
  int x = 3 , y = 105;

  tft.fillRect(81, 0, 78, 49, 0x0193);
  draw_FlowrateIcon(0xF980, 0xFFEF);

  tft.fillRect(0, 50, 480, 5, 0x4FFF);
  tft.fillRect(81, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 210,  0x0021);

  tft.drawFastHLine(0 , y + 25 , 480, 0x0193);
  tft.drawFastHLine(0 , y + 85 , 480, 0x0193);

  tft.setFont(&FreeSansBold18pt7b);
  disp_text(x, y, 1, WHITE, "Flow Rate");
  disp_text(x + 245, y, 1, WHITE, ": ");
  disp_text(x, y + 60, 1, WHITE , "Time to Finish ");
  disp_text(x + 245, y + 60, 1, WHITE, ": ");
  disp_text(x, y + 120, 1, WHITE , "Drug Name ");
  disp_text(x + 245, y + 120, 1, WHITE, ": ");
  disp_text(x + 265, y, 1, WHITE , "00");
  disp_text(x + 265, y + 60, 1, WHITE, "00");
  disp_text(x + 342 , y + 60, 1, WHITE, "00");

  tft.setFont(&FreeSans12pt7b);
  disp_text(x + 310 , y, 1, WHITE, "ml/hr");
  disp_text(x + 385, y + 60, 1, WHITE, "min");
  disp_text(x + 310, y + 60, 1, WHITE, "hr");
  disp_text(x + 265, y + 120, 1, WHITE, drug_name.c_str());
}

void Func_Bedpage() {
  int i = 0, i0 = 0, i1 = 0, len = 0;
  static int bed_lastindex = 0;
  static String str = "";
  String _temp = "", _temp1 = "";
  static int x1 = 0, x2 = 1;
  int y = 70, x = 55;
  SdFile file;

  tft.fillRect(161, 0, 78, 49, 0x0193);
  draw_PatientIcon(175, 5, 0xF980,  0x0193);

  tft.fillRect(0, 50, 480, 5, 0x4FFF);
  tft.fillRect(161, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 210,  0x0021);

  tft.setFont(&FreeSansBold12pt7b);

  if (_bedflag == true) {
    _bedflag = false;
    str = "";
    _temp = "/bedlist/bedlist.txt";
    if (file.open(_temp.c_str(), O_READ)) {
      while (file.available()) {
        str += (char)file.read();
      }
      Serial.println(str);
      file.close();
    }
    else {
      Serial.println("Error Opening File.");
    }

    len = str.length();
    i0 = str.lastIndexOf("#", len);
    i1 = str.lastIndexOf("\n", i0);
    _temp1 = str.substring(i1 + 1, i0);
    Serial.println(i0);
    Serial.print("Last index = ");
    _temp1.trim();
    Serial.println(_temp1);

    bed_lastindex = _temp1.toFloat();
    bedindex = _temp1.toInt();
    total_bedpagecnt = _temp1.toFloat();
    while (fmod(total_bedpagecnt, 4.00) != 0) {
      total_bedpagecnt++;
    }
    total_bedpagecnt /= 4;
    Serial.print("Total Pages = ");
    Serial.println(total_bedpagecnt);

    for (i = 0; i < bed_lastindex; i++) {        //change buff len
      x1 = str.indexOf('\n', x1 + 1); //+3);
      bedbuff[i] = str.substring(x2 + 1, x1);
      x2 = str.indexOf('#', x1 + 1);
    }
  }


  if (_nextbed_flag == true) {    //next page
    _nextbed_flag = false;


    if (bed_pagecnt >= total_bedpagecnt) {
      bed_pagecnt = total_bedpagecnt;
      //nm = 15 - 5; //total_pagecnt * 2;
    }
    else {
      bednm += 4;
      bed_pagecnt++;
      bedindex -= 4;
    }
    Serial.println(bed_pagecnt);
    Serial.println(total_bedpagecnt);
  }

  if (_prevbed_flag == true) {   //prev flag
    _prevbed_flag = false;
    bed_pagecnt--;
    bednm = bednm - 4;
    bedindex += 4;
    if (bed_pagecnt < 1) {
      bed_pagecnt = 1;
      bednm = 0;
      //drugindex = drug_lastindex;
    }
    Serial.println(bed_pagecnt);
    Serial.println(bed_pagecnt);
    Serial.println(bednm);
  }

  tft.fillRoundRect(205, 230, 70, 30, 10, WHITE);
  tft.drawRoundRect(205, 230, 70, 30, 10, 0xFE00);
  tft.drawRoundRect(206, 230, 69, 30, 10, 0xFE00);
  tft.drawRoundRect(205, 230, 69, 30, 10, 0xFE00);

  disp_text(234, 239, 1, 0x0021, String(bed_pagecnt).c_str());

  if (bed_pagecnt <= 1) {
    //draw_Button(380, 260, 70, 55, 0xF000);
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow
  }
  else if (bed_pagecnt > 1 && bed_pagecnt < total_bedpagecnt) {
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow

    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(59, 244, 2, 15, 0x0021);
  }
  else if (bed_pagecnt >= total_bedpagecnt) {
    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(57, 244, 2, 15, 0x0021);
  }

  if (bedindex >= 4) {
    for (i = 0 + bednm; i < 4 + bednm; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (bedbuff[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }
  else if (bedindex < 4) {
    for (i = 0 + bednm; i < bedindex + bednm; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (bedbuff[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }

  for (i = 0; i < 30; i++) {          //change buff len
    Serial.print("Bed Buff = ");
    Serial.println(bedbuff[i]);
  }
  tft.setFont(&FreeSans9pt7b);
  disp_text(244, 300, 1, BLACK, bed_name.c_str());
}

void Func_Drugpage() {
  int i = 0, i0 = 0, i1 = 0, len = 0;
  static int drug_lastindex = 0;
  static String str = "";
  String _temp = "", _temp1 = "";
  int x1 = 0, x2 = 0;

  int y = 70, x = 55;
  SdFile file;

  //  tft.fillRect(241, 0, 78, 49, 0x0193);
  //  draw_DrugIcon(250, 1, 0xF980);

  tft.fillRect(0, 50, 480, 5, 0x4FFF);
  tft.fillRect(241, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 211,  0x0021);
  tft.setFont(&FreeSansBold12pt7b);

  if (_medflag == true) {
    _medflag = false;
    str = "";
    _temp = "/druglist/catlist.txt";
    if (file.open(_temp.c_str(), O_READ)) {
      while (file.available()) {
        str += (char)file.read();
      }
      //Serial.println(str);
      file.close();
    }
    else {
      Serial.println("Error Opening File.");
    }

    len = str.length();
    i0 = str.lastIndexOf("#", len);
    i1 = str.lastIndexOf("\n", i0);
    _temp1 = str.substring(i1 + 1, i0);
    Serial.println(i0);
    Serial.print("Last index = ");
    _temp1.trim();
    Serial.println(_temp1);

    drug_lastindex = _temp1.toFloat();
    drugindex = _temp1.toInt();
    total_pagecnt = _temp1.toFloat();
    while (fmod(total_pagecnt, 4.00) != 0) {
      total_pagecnt++;
    }
    total_pagecnt /= 4;
    Serial.print("Total Pages = ");
    Serial.println(total_pagecnt);
    _readcatflag = true;
    fcatindex = drug_lastindex;

    x1 = 0; x2 = 0;
    for (i = 0; i < drug_lastindex; i++) {        //change buff len
      x1 = str.indexOf("#", x2);
      x2 = str.indexOf("\n", x1);
      buff[i] = str.substring(x1 + 1, x2);
    }

    sortArray(buff, drug_lastindex);
  }

  //tft.fillScreen(0xF100);

  if (_next_flag == true) {    //next page
    _next_flag = false;


    if (cat_pagecnt >= total_pagecnt) {
      cat_pagecnt = total_pagecnt;
      //nm = 15 - 5; //total_pagecnt * 2;
    }
    else {
      nm += 4;
      cat_pagecnt++;
      drugindex -= 4;
    }
    Serial.println(cat_pagecnt);
    Serial.println(total_pagecnt);
  }

  if (_prev_flag == true) {   //prev flag
    _prev_flag = false;
    cat_pagecnt--;
    nm = nm - 4;
    drugindex += 4;
    if (cat_pagecnt < 1) {
      cat_pagecnt = 1;
      nm = 0;
      //drugindex = drug_lastindex;
    }
    Serial.println(cat_pagecnt);
    Serial.println(total_pagecnt);
    Serial.println(nm);
  }

  tft.fillRoundRect(205, 230, 70, 30, 10, WHITE);
  tft.drawRoundRect(205, 230, 70, 30, 10, 0xFE00);
  tft.drawRoundRect(206, 230, 69, 30, 10, 0xFE00);
  tft.drawRoundRect(205, 230, 69, 30, 10, 0xFE00);
  disp_text(234, 239, 1, 0x0021, String(cat_pagecnt).c_str());

  if (cat_pagecnt <= 1) {
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow
  }
  else if (cat_pagecnt > 1 && cat_pagecnt < total_pagecnt) {
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow

    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(59, 244, 2, 15, 0x0021);
  }
  else if (cat_pagecnt >= total_pagecnt) {
    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(59, 244, 2, 15, 0x0021);
  }

  if (drugindex >= 4) {
    for (i = 0 + nm; i < 4 + nm; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (buff[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }
  else if (drugindex < 4) {
    for (i = 0 + nm; i < drugindex + nm; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (buff[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }

  for (i = 0; i < 24; i++) {          //change buff len
    Serial.print("Buff = ");
    Serial.println(buff[i]);
  }

  _submedflag = true;

  tft.setFont(&FreeSans9pt7b);
}

void Func_Medpage() {
  int index = 0;
  String _temp = "", _temp1 = "";
  static String str = "";
  char* fs = "";
  int x1 = 0, x2 = 1, i0 = 0, i1 = 0, len = 0;
  int y = 70, x = 55;
  static int sub_lastindex = 0;

  tft.fillRect(0, 55, 480, 211,  0x0021);
  tft.setFont(&FreeSansBold12pt7b);

  if (_submedflag == true) {
    _submedflag = false;
    str = "";

    File myFile;
    index = (cat_pagecnt - 1) * 4 + catIndex;

    Serial.print("Index = ");
    Serial.println(index);
    Serial.print("Drug Selected = ");
    Serial.println(buff[index]);

    buff[index].trim();

    _temp = "/druglist/";
    _temp += buff[index] + ".TXT";
    _temp.trim();
    Serial.println(_temp);

    fs = _temp.c_str();
    Serial.println(fs);

    myFile = SD.open(fs); //((buff[index] + ".TXT").c_str());

    if (myFile) {
      while (myFile.available()) {
        str += (char)myFile.read();
      }
      //Serial.println(str);
      myFile.close();
    }
    else {
      Serial.println("Error Opening File.");
    }
    str.trim();
    len = str.length();
    i0 = str.lastIndexOf("#", len);
    i1 = str.lastIndexOf("\n", i0);
    _temp1 = str.substring(i1 + 1, i0);
    Serial.println(i0);
    Serial.print("Last index = ");
    _temp1.trim();
    Serial.println(_temp1);
    sub_lastindex = _temp1.toFloat();
    subdrug = _temp1.toInt();
    total_subpagecnt = _temp1.toFloat();
    while (fmod(total_subpagecnt, 4.00) != 0) {
      total_subpagecnt++;
    }
    total_subpagecnt /= 4;
    Serial.print("Total Pages = ");
    Serial.println(total_subpagecnt);
    findex = sub_lastindex;
    _readflag = true;
    x1 = 0; x2 = 0;
    for (int i = 0; i < sub_lastindex; i++) {      //read into buff
      x1 = str.indexOf("#", x2);
      x2 = str.indexOf("\n", x1);
      submed[i] = str.substring(x1 + 1, x2);
      //Serial.print("submed = ");
      //Serial.println(str.substring(x1 + 1, x2));
    }

    //sort_String(sub_lastindex);
    sortArray(submed, sub_lastindex);

    //    for (int i = 0; i < sub_lastindex; i++) {
    //      Serial.println(submed[i]);
    //    }

    //    for (int i = 0; i < sub_lastindex; i++) {        //Store in buff
    //      x1 = str.indexOf('\n', x1 + 1); //+3);
    //      submed[i] = str.substring(x2 + 1, x1);
    //      //x2 = x1 + 3;
    //      x2 = str.indexOf('#', x1 + 1);
    //      Serial.println(submed[i]);
    //    }

    //    for (int i = 0; i < sub_lastindex; i++) {
    //      submed[i] = parsebuff[i];
    //    }
  }


  if (_subnext_flag == true) {    //next page
    _subnext_flag = false;

    if (subcat_pagecnt >= total_subpagecnt) {
      subcat_pagecnt = total_subpagecnt;
      //nm1 = 15 - 5; //total_pagecnt * 2;
      //subdrug = 2;
    }
    else {
      nm1 += 4;
      subcat_pagecnt++;
      subdrug -= 4;
    }
    Serial.println(subcat_pagecnt);
    Serial.println(total_subpagecnt);
    Serial.println(subdrug);
  }

  if (_subprev_flag == true) {    //prev page
    _subprev_flag = false;
    subcat_pagecnt--;
    nm1 = nm1 - 4;
    subdrug += 4;
    if (subcat_pagecnt < 1) {
      subcat_pagecnt = 1;
      nm1 = 0;
      //subdrug = 12;
    }
    Serial.println(subcat_pagecnt);
    Serial.println(total_subpagecnt);
    Serial.println(nm);
  }

  tft.fillRoundRect(205, 230, 70, 30, 10, WHITE);
  tft.drawRoundRect(205, 230, 70, 30, 10, 0xFE00);
  tft.drawRoundRect(206, 230, 69, 30, 10, 0xFE00);
  tft.drawRoundRect(205, 230, 69, 30, 10, 0xFE00);
  disp_text(234, 239, 1, 0x0021, String(subcat_pagecnt).c_str());

  if (subcat_pagecnt <= 1) {
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow
  }
  else if (subcat_pagecnt > 1 && subcat_pagecnt < total_subpagecnt) {
    tft.fillRoundRect(380, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(380, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(380, 230, 72, 30, 10, 0xFE00);
    draw_Arrow(415, 237, 2, 15, 0x0021);    // Arrow

    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(59, 244, 2, 15, 0x0021);
  }
  else if (subcat_pagecnt >= total_subpagecnt) {
    tft.fillRoundRect(30, 230, 70, 30, 10, WHITE); //tft.fillRoundRect(x1, y1, w, h, radius, color);
    tft.drawRoundRect(30, 230, 71, 30, 10, 0xFE00);
    tft.drawRoundRect(30, 230, 72, 30, 10, 0xFE00);
    draw_LeftArrow(59, 244, 2, 15, 0x0021);
  }

  if (subdrug >= 4) {
    for (int i = 0 + nm1; i < 4 + nm1; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (submed[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }
  else if (subdrug < 4) {
    for (int i = 0 + nm1; i < subdrug + nm1; i++) {
      tft.drawFastHLine(0, x, 480, 0xFE00);
      disp_text(5, y, 1, WHITE, (submed[i]).c_str());
      x += 40;
      y += 40;
      tft.drawFastHLine(0, x, 480, 0xFE00);
    }
  }
  tft.setFont(&FreeSans9pt7b);
}

void Func_Startstoppage() {
  //tft.fillRect(321, 0, 78, 49, 0x0193);
  //draw_Stopbutton();

  tft.fillRect(0, 50, 480, 5, 0x4FFF);
  tft.fillRect(321, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 211,  0x0021);

  tft.setFont(&FreeSans12pt7b);

  if (_change_flag % 2 == 0) {
    disp_text(80, 100, 1, WHITE , "Do you want to start the flow ?  ");
  }
  else {
    disp_text(80, 100, 1, WHITE , "Do you want to pause the flow ?  ");
    disp_text(50, 125, 1, WHITE , "(You will be redirected to Login page)");
  }

  for (int j = 0; j < 2; j++)
  {
    tft.fillRoundRect(150 + j * 130, 170, 80, 40, 8, 0xFBE0);
  }

  disp_text(165, 185, 1, WHITE, "YES");
  disp_text(305, 185, 1, WHITE, "NO");
}

void Func_Alertpage() {
  tft.fillRect(401, 0, 78, 49, 0x0193);
  draw_AlertIcon();
  tft.fillRect(0, 50, 480, 5, 0x4FFF);
  tft.fillRect(401, 50, 77, 5, 0xF980);

  tft.fillRect(0, 55, 480, 211,  0x0021);
}

void Func_Screensaverpage() {

  //  tft.fillScreen(0x0021);
  int x = 5 , y = 90; long color = WHITE;
  String v1 = "0", v2 = "0", fr = "0", hr = "00", min = "00";

  tft.fillRect(1, 50, 478, 5, 0xF980);
  tft.fillRect(0, 55, 480, 211,  0x0021);

  tft.drawFastHLine(0 , 110 , 350, 0x0193);
  tft.drawFastHLine(0 , 160 , 480, 0x0193);
  tft.drawFastHLine(0 , 210 , 480, 0x0193);
  tft.drawFastVLine(350 , 55 , 105, 0x0193);

  tft.setFont(&FreeSans18pt7b);
  disp_text(x, y, 1, color , "Infused ");
  disp_text(x + 170, y, 1, color , ": ");
  disp_text(x, y + 45, 1, color , "Remaining ");
  disp_text(x + 170, y + 50, 1, color , ": ");
  disp_text(x, y + 95, 1, color , "Drug name");
  disp_text(x + 170, y + 90, 1, color , ": ");

  tft.setFont(&FreeSansBold18pt7b);
  disp_text(x + 195, y, 1, color , v1.c_str());    //len*20
  disp_text(x + 195, y + 50, 1, color , v2.c_str());
  disp_text(x , y + 150, 1, color , fr.c_str());
  disp_text(x + 305 , y + 150, 1, color , hr.c_str());
  disp_text(x + 370 , y + 30 + 120, 1, color , min.c_str());

  tft.setFont(&FreeSans12pt7b);
  //  disp_text(x + 255, y, 1, color, "ml");
  disp_text((200 + (v1.length() * 20)), y, 1, color, "ml");
  disp_text((200 + (v2.length() * 20)), y + 50, 1, color, "ml");
  disp_text(x + 190, y + 95, 1, color, drug_name.c_str());
  disp_text((5 + ((fr.length()) * 20)), y + 150, 1, color, "ml/hr");
  disp_text((310 + ((hr.length()) * 20)), y + 150, 1, color, "hr");
  disp_text((375 + (min.length() * 20)), y + 150, 1, color, "min");

  draw_DropIcon(420, 120, 25, 55, WHITE, 0x0021);
  _stop_interrupt = true;
  //attachInterrupt(digitalPinToInterrupt(20), Sensor1_Detect, CHANGE);
  EIMSK = 0x01;          //Enable only INT0
}

void lasttab_open(uint16_t oldtab) {
  Serial.print("Old Tab = ");
  Serial.println(oldtab);
  switch (oldtab) {
    case 1: //GTTVOLPAGE
      tft.fillRect(0, 0, 78, 49, 0x4FFF);
      draw_DropIcon(40, 30, 15, -10, 0xF980, 0x4FFF);
      break;
    case 2: //FLOWRATEPAGE
      tft.fillRect(81, 0, 78, 49, 0x4FFF);
      draw_FlowrateIcon(0xF980, 0xFFEF);
      break;
    case 3: //BEDLISTPAGE
      tft.fillRect(161, 0, 78, 49, 0x4FFF);
      draw_PatientIcon(175, 5, 0xF980, 0x4FFF);
      break;
    case 4: //DRUGLISTPAGE
      tft.fillRect(241, 0, 78, 49, 0x4FFF);
      draw_DrugIcon(250, 1, 0xF980);
      break;
    case 5: //MEDLISTPAGE
      tft.fillRect(241, 0, 78, 49, 0x4FFF);
      draw_DrugIcon(250, 1, 0xF980);
      break;
    case 6: //START_STOP_PAGE
      tft.fillRect(321, 0, 78, 49, 0x4FFF);
      if (_change_flag % 2 == 0) {
        draw_Startbutton();
      }
      else {
        draw_Stopbutton();
      }
      break;
    case 7:
      tft.fillRect(401, 0, 78, 49, 0x4FFF);
      draw_AlertIcon();
      break;
  }
}
/****************************************************************************************************************/
/*-----------------------------------------------Touch Detect---------------------------------------------------*/
/****************************************************************************************************************/

int   touchdetect(int page_chk) {
  static boolean chk = false;
  static int old_page_chk = 0;
  int X = 0, Y = 0;

  TSPoint p = myTouch.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (page_chk >= NO_OF_PAGES || page_chk < 0) {            //Update page value whenever new pages are created.
    Serial.print("Page CHK Error = ");
    Serial.println(page_chk);
    page_chk = old_page_chk;
  }
  old_page_chk = page_chk;

  if (p.z > 5 && p.z < 1000) {

    Serial.println(page_chk);
    Serial.print(F("X = "));            Serial.print(p.x);
    Serial.print(F("\tY = "));          Serial.print(p.y);
    Serial.print(F("\tPressure = "));   Serial.println(p.z);

    //     p.x = tft.height() - (map(p.x, TS_MINX, TS_MAXX,  tft.height(), 0));    // Map X Axis Value in between 0 to 320
    //     p.y = tft.width() - (map(p.y, TS_MAXY, TS_MINY,  tft.width(), 0));      // Map Y Axis Value in between 0 to 240
    //        LANDSCAPE CALIBRATION    480 x 320
    //        x = map(p.y, LEFT = 959, RT = 80, 0, 480)
    //        y = map(p.x, TOP = 879, BOT = 139, 0, 320)

    //p.x = tft.height() - (map(p.x, 910, 94,  tft.height(), 0));    //(0, 480) Map X Axis Value in between 0 to 320
    //p.y = tft.width() - (map(p.y, 950, 126,  tft.width(), 0));     //(0, 320)

    //Serial.print(F("Mapped X = "));        Serial.println(p.y);   X = p.y;  //Width
    //Serial.print(F("Mapped Y = "));        Serial.println(p.x);   Y = p.x;  //Height

    X = map(p.y, 959, 80, 0, 480);
    Y = map(p.x, 879, 139, 0, 320);
    Serial.print(F("Mapped X = "));
    Serial.print(X);
    Serial.print(F("  Mapped Y = "));
    Serial.println(Y);


    if (X > 260 && X < 445 && Y > 20 && Y < 60 && page_chk == _loginpage) {
      draw_RoundButton(260, 20, 185, 40, 5, WHITE, 0xFE00);
      disp_text(275, 2, 1, WHITE, "Enter Login ID.");
      Serial.println(F("Enter ID"));
      _enterid = true;
      _enterpin = false;
      return _loginpage;
    }
    else if (X > 260 && X < 445 && Y > 65 && Y < 105 && page_chk == _loginpage) {
      draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
      Serial.println(F("Enter Pin"));
      _enterpin = true;
      _enterid = false;
      return _loginpage;
    }
    else if (X > 240 && X < 315 && Y > 110 && Y < 160 && page_chk == _loginpage) {
      Serial.println("Key 1");

      tft.fillRoundRect(242, 112, 73, 47, 20, 0x0021);
      tft.fillRect(255, 112, 60, 48, 0x0021);
      tft.fillRect(242, 135, 59, 25, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(270, 135, 1, WHITE, "1");
      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(1);
      tft.setFont(&FreeMonoBold18pt7b);
      delay (100);
      tft.fillRoundRect(242, 112, 73, 47, 20, WHITE);
      tft.fillRect(255, 112, 60, 48, WHITE);
      tft.fillRect(242, 135, 59, 25, WHITE);
      disp_text(270, 135, 1, 0x0021, "1");
      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 315 && X < 390 && Y > 110 && Y < 160 && page_chk == _loginpage) {
      Serial.println("Key 2");

      tft.fillRect(317, 112, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 135, 1, WHITE, "2");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(2);

      delay(100);
      tft.fillRect(317, 112, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 135, 1, 0x0021, "2");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 390 && X < 465 && Y > 110 && Y < 160 && page_chk == _loginpage) {
      Serial.println("Key 3");

      tft.fillRoundRect(392, 112, 72, 47, 20, 0x0021);
      tft.fillRect(392, 112, 60, 48, 0x0021);
      tft.fillRect(405, 135, 59, 25, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 135, 1, WHITE, "3");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(3);

      delay(100);
      tft.fillRoundRect(392, 112, 72, 47, 20, WHITE);
      tft.fillRect(392, 112, 60, 48, WHITE);
      tft.fillRect(405, 135, 59, 25, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 135, 1, 0x0021, "3");
      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 240 && X < 315 && Y > 160 && Y < 210 && page_chk == _loginpage) {
      Serial.println("Key 4");

      tft.fillRect(242, 162, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(270, 185, 1, WHITE, "4");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(4);

      delay(100);
      tft.fillRect(242, 162, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(270, 185, 1, 0x0021, "4");
      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 315 && X < 390 && Y > 160 && Y < 210 && page_chk == _loginpage) {
      Serial.println("Key 5");

      tft.fillRect(317, 162, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 185, 1, WHITE, "5");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(5);

      delay(100);
      tft.fillRect(317, 162, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 185, 1, 0x0021, "5");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 390 && X < 465 && Y > 160 && Y < 210 && page_chk == _loginpage) {
      Serial.println("Key 6");

      tft.fillRect(392, 162, 71, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 185, 1, WHITE, "6");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(6);

      delay(100);
      tft.fillRect(392, 162, 71, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 185, 1, 0x0021, "6");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 240 && X < 315 && Y > 210 && Y < 260 && page_chk == _loginpage) {
      Serial.println("Key 7");

      tft.fillRect(242, 212, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(270, 235, 1, WHITE, "7");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(7);

      delay(100);
      tft.fillRect(242, 212, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(270, 235, 1, 0x0021, "7");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 315 && X < 390 && Y > 210 && Y < 260 && page_chk == _loginpage) {
      Serial.println("Key 8");

      tft.fillRect(317, 212, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 235, 1, WHITE, "8");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(8);

      delay(100);
      tft.fillRect(317, 212, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 235, 1, 0x0021, "8");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 390 && X < 465 && Y > 210 && Y < 260 && page_chk == _loginpage) {
      Serial.println("Key 9");

      tft.fillRect(392, 212, 71, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 235, 1, WHITE, "9");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(9);

      delay(100);
      tft.fillRect(392, 212, 71, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(420, 235, 1, 0x0021, "9");

      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 240 && X < 315 && Y > 260 && Y < 310 && page_chk == _loginpage) {
      Serial.println("Key C");

      tft.fillRoundRect(242, 262, 72, 47, 20, 0x0021);
      tft.fillRect(255, 262, 60, 47, 0x0021);
      tft.fillRect(242, 262, 59, 25, 0x0021);
      draw_LeftArrow(275, 285, 3, 20, WHITE);
      simplekey_pressed(10);

      delay(100);
      tft.fillRoundRect(242, 262, 72, 47, 20, WHITE);
      tft.fillRect(255, 262, 60, 47, WHITE);
      tft.fillRect(242, 262, 59, 25, WHITE);
      draw_LeftArrow(275, 285, 3, 20, 0x0021);
      return _loginpage;
    }
    else if (X > 315 && X < 390 && Y > 260 && Y < 310 && page_chk == _loginpage) {
      Serial.println("Key 0");

      tft.fillRect(317, 262, 73, 47, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 285, 1, WHITE, "0");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(0);

      delay(100);
      tft.fillRect(317, 262, 73, 47, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(345, 285, 1, 0x0021, "0");
      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 390 && X < 465 && Y > 260 && Y < 310 && page_chk == _loginpage) {
      Serial.println("Key OK");
      tft.fillRoundRect(392, 262, 72, 47, 20, 0x0021);
      tft.fillRect(392, 262, 59, 47, 0x0021);
      tft.fillRect(405, 262, 59, 25, 0x0021);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(405, 285, 1, WHITE, "OK");

      tft.setFont(&FreeSans9pt7b);
      simplekey_pressed(11);

      delay(100);

      tft.fillRoundRect(392, 262, 72, 47, 20, WHITE);
      tft.fillRect(392, 262, 59, 47, WHITE);
      tft.fillRect(405, 262, 59, 25, WHITE);
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(405, 285, 1, 0x0021, "OK");
      if (_enterid == true) {
        if (id == 123456) {
          _id_ok = true;
          _enterpin = true;
          _enterid = false;
          pin = 0;
          draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
        }
        else {
          _id_ok = false;
          tft.setFont(&FreeSans9pt7b);
          disp_text(275, 2, 1, WHITE, "Invalid ID.");
          draw_RoundButton(260, 20, 185, 40, 5, WHITE, 0xFE00);
          disp_text(268, 35, 1, 0xA510, "Enter ID");
          _enterid = false;
          pin = 0;
        }
      }
      else if (_enterpin == true) {
        if (pin == 1234 && _id_ok == true) {
          _page_token = true;
          _enterpin = false;
          _enterid = false;
          _id_ok = false;
          _startflow = false;
          TIMSK1 &= (0 << OCIE1A);
          tft.setFont(&FreeSans9pt7b);
          load_icons();

          return _gttvolpage;
          
        }
        else if (_id_ok == false) {
          tft.setFont(&FreeSans9pt7b);
          disp_text(275, 2, 1, BLACK, "Invalid ID.");
          _enterpin = false;
          draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
          disp_text(268, 80, 1, 0xA510, "Enter Pin");
        }
        else {
          tft.setFont(&FreeSans9pt7b);
          disp_text(275, 2, 1, BLACK, "Invalid Pin.");
          //return _loginpage;
          draw_RoundButton(260, 65, 185, 40, 5, WHITE, 0xFE00);
          disp_text(268, 80, 1, 0xA510, "Enter Pin");
          _enterpin = false;
          _enterid = false;
        }

      }
      tft.setFont(&FreeSans9pt7b);
      return _loginpage;
    }
    else if (X > 90 && X < 170 && Y > 90 && Y < 150 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 110, 1, 0x0900, "10");
      disp_text(190, 110, 1, WHITE, "15");
      disp_text(280, 110, 1, WHITE, "20");
      disp_text(370, 110, 1, WHITE, "60");
      gtt = 10;
      return _gttvolpage;
    }
    else if (X > 170 && X < 260 && Y > 90 && Y < 150 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 110, 1, WHITE, "10");
      disp_text(190, 110, 1, 0x0900, "15");
      disp_text(280, 110, 1, WHITE, "20");
      disp_text(370, 110, 1, WHITE, "60");
      gtt = 15;
      return _gttvolpage;
    }
    else if (X > 260 && X < 350 && Y > 90 && Y < 160 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 110, 1, WHITE, "10");
      disp_text(190, 110, 1, WHITE, "15");
      disp_text(280, 110, 1, 0x0900, "20");
      disp_text(370, 110, 1, WHITE, "60");
      gtt = 20;
      return _gttvolpage;
    }
    else if (X > 350 && X < 440 && Y > 90 && Y < 160 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 110, 1, WHITE, "10");
      disp_text(190, 110, 1, WHITE, "15");
      disp_text(280, 110, 1, WHITE, "20");
      disp_text(370, 110, 1, 0x0900, "60");
      gtt = 60;
      return _gttvolpage;
    }
    else if (X > 90 && X < 170 && Y > 170 && Y < 220 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 200, 1, 0x0900, "25");
      disp_text(190, 200, 1, WHITE, "50");
      disp_text(280, 200, 1, WHITE, "75");
      disp_text(370, 200, 1, WHITE, "100");
      disp_text(450, 200, 1, WHITE, "+");
      vol = 25;
      return _gttvolpage;
    }
    else if (X > 170 && X < 260 && Y > 170 && Y < 220 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 200, 1, WHITE, "25");
      disp_text(190, 200, 1, 0x0900, "50");
      disp_text(280, 200, 1, WHITE, "75");
      disp_text(370, 200, 1, WHITE, "100");
      disp_text(450, 200, 1, WHITE, "+");
      vol = 50;
      return _gttvolpage;
    }
    else if (X > 260 && X < 350 && Y > 170 && Y < 220 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 200, 1, WHITE, "25");
      disp_text(190, 200, 1, WHITE, "50");
      disp_text(280, 200, 1, 0x0900, "75");
      disp_text(370, 200, 1, WHITE, "100");
      disp_text(450, 200, 1, WHITE, "+");
      vol = 75;
      return _gttvolpage;
    }
    else if (X > 350 && X < 440 && Y > 170 && Y < 220 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 200, 1, WHITE, "25");
      disp_text(190, 200, 1, WHITE, "50");
      disp_text(280, 200, 1, WHITE, "75");
      disp_text(370, 200, 1, 0x0900, "100");
      disp_text(450, 200, 1, WHITE, "+");
      vol = 100;
      return _gttvolpage;
    }
    else if (X > 440 && X < 480 && Y > 180 && Y < 220 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 200, 1, WHITE, "25");
      disp_text(190, 200, 1, WHITE, "50");
      disp_text(280, 200, 1, WHITE, "75");
      disp_text(370, 200, 1, WHITE, "100");
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(450, 200, 1, 0x0900, "+");
      delay(10);
      disp_Keypad();
      draw_RoundButton(100, 10, 280, 50, 15, WHITE, 0xFE00);
      _volkeypad = true;
      return _gttvolpage;
    }
    else if (X > 245 && X < 330 && Y > 225 && Y < 250 && page_chk == _gttvolpage && _volkeypad == false) {
      tft.setFont(&FreeSans18pt7b);
      disp_text(100, 110, 1, WHITE, "10");
      disp_text(190, 110, 1, WHITE, "15");
      disp_text(280, 110, 1, WHITE, "20");
      disp_text(370, 110, 1, WHITE, "60");

      disp_text(100, 200, 1, WHITE, "25");
      disp_text(190, 200, 1, WHITE, "50");
      disp_text(280, 200, 1, WHITE, "75");
      disp_text(370, 200, 1, WHITE, "100");
      tft.setFont(&FreeMonoBold18pt7b);
      disp_text(450, 200, 1, WHITE, "+");

      tft.setFont(&FreeSans9pt7b);
      tft.fillRect(42, 282, 25, 18, 0x4FFF);
      disp_text(44, 285, 1, BLACK, String(gtt).c_str());
      disp_text(35, 300, 1, BLACK, "dpml");

      tft.fillRect(112, 282, 70, 18, 0x4FFF);
      disp_text(114, 285, 1, BLACK, String(vol).c_str());
      //      return _gttvolpage;

      //{"Flag":"A0", "GTT": "10", "Volume": 98.16666, "Drugcat": 10, "Drugname": "11", "pinchAlert": "0", "Ack": "0" }
      //String pay = "$2,150," + String(vol) + ",15," + String(gtt) + ",0,1#";
      String pay = "{\"Flag\":\"A0\",\"GTT\":";
      pay += String(gtt);
      pay += ",\"Vol\":";
      pay += String(vol);
      pay += ",\"Drugcat\":10";
      pay += ",\"Drugname\":11";
      pay += ",\"pinchAlert\":0";
      pay += ",\"Ack\":";
      pay += String(_ack);
      pay += ",\"NurseID\":123456";
      pay += "}";
      char datbuff[pay.length() + 1];
      pay.toCharArray(datbuff, pay.length() + 1);
      Serial.println(pay);
      for (int j = 0; j < pay.length(); j++) {
        USART_Transmit(datbuff[j]);
        Serial.print(datbuff[j]);
        //delay(1);
      }
      USART_Transmit('\n');
      //Serial1.println(pay);
      Serial.println("");
      Serial.println(pay);
      _startflow = true;
      TIMSK1 |= (1 << OCIE1A);  // enable Screensaver
      draw_Stopbutton();
      _page_token = true;
      _page_no = _flowratepage;
      lasttab_open(old_page_chk);
      return _flowratepage;
    }
    else if (X > 90 && X < 165 && Y > 70 && Y < 130 && _volkeypad == true ) {    // Vol key1
      key_pressed(1);
      return _gttvolpage;
    }
    else if (X > 165 && X < 270 && Y > 70 && Y < 130 && _volkeypad == true ) {    // Vol key2
      key_pressed(2);
      return _gttvolpage;
    }
    else if (X > 270 && X < 345 && Y > 70 && Y < 130 && _volkeypad == true ) {    // Vol key3
      key_pressed(3);
      return _gttvolpage;
    }
    else if (X > 345 && X < 420 && Y > 70 && Y < 130 && _volkeypad == true ) {    // Vol key backspace
      key_pressed(10);
      return _gttvolpage;
    }
    else if (X > 90 && X < 165 && Y > 130 && Y < 190 && _volkeypad == true ) {    // Vol key4
      key_pressed(4);
      return _gttvolpage;
    }
    else if (X > 165 && X < 270 && Y > 130 && Y < 190 && _volkeypad == true ) {    // Vol key5
      key_pressed(5);
      return _gttvolpage;
    }
    else if (X > 270 && X < 345 && Y > 130 && Y < 190 && _volkeypad == true ) {    // Vol key6
      key_pressed(6);
      return _gttvolpage;
    }
    else if (X > 345 && X < 420 && Y > 130 && Y < 190 && _volkeypad == true ) {    // Vol key Clear
      key_pressed(11);
      return _gttvolpage;
    }
    else if (X > 90 && X < 165 && Y > 190 && Y < 250 && _volkeypad == true ) {    // Vol key7
      key_pressed(7);
      return _gttvolpage;
    }
    else if (X > 165 && X < 270 && Y > 190 && Y < 250 && _volkeypad == true ) {    // Vol key8
      key_pressed(8);
      return _gttvolpage;
    }
    else if (X > 270 && X < 345 && Y > 190 && Y < 250 && _volkeypad == true ) {    // Vol key9
      key_pressed(9);
      return _gttvolpage;
    }
    else if (X > 345 && X < 420 && Y > 190 && Y < 250 && _volkeypad == true ) {    // Vol key Cancel
      key_pressed(14);
      Serial.println("Cancel");
      _volkeypad = false;
      _page_token = true;
      load_icons();
      return _gttvolpage;
    }
    else if (X > 90 && X < 165 && Y > 250 && Y < 310 && _volkeypad == true ) {    // Vol key .
      key_pressed(12);
      return _gttvolpage;
    }
    else if (X > 165 && X < 270 && Y > 250 && Y < 310 && _volkeypad == true ) {    // Vol key 0
      key_pressed(0);
      return _gttvolpage;
    }
    else if (X > 270 && X < 345 && Y > 250 && Y < 310 && _volkeypad == true ) {    // Vol key none
      //key_pressed(1);
      return _gttvolpage;
    }
    else if (X > 345 && X < 420 && Y > 250 && Y < 310 && _volkeypad == true ) {    // Vol key OK
      key_pressed(13);
      _page_token = true;
      load_icons();
      return _gttvolpage;
    }
    else if (X > 395 && X < 470 && Y > 225 && Y < 250 && page_chk == _bedpage) {  //bed nextpage
      if (bed_pagecnt >= total_bedpagecnt) {
        return page_chk;
      }
      else {
        _page_token = true;
        _nextbed_flag = true;

        return _bedpage;
      }
    }
    else if (X > 30 && X < 100 && Y > 225 && Y < 250 && page_chk == _bedpage) {  //bed prevpage
      if (bed_pagecnt == 1) {
        return page_chk;
      }
      else {
        _page_token = true;
        _prevbed_flag = true;

        return _bedpage;
      }
    }
    else if (X > 0 && X < 480 && Y > 55 && Y < 95 && page_chk == _bedpage) {      //Bed0
      Serial.print("Selected Bed = ");
      bed_name = bedbuff[((bed_pagecnt - 1) * 4) + 0];
      bed_name = bed_name.substring(3, bed_name.length());
      tft.fillRect(242, 300, 50, 18, 0x4FFF);
      disp_text(244, 300, 1, BLACK, bed_name.c_str());
      Serial.println(bed_name);
      return _bedpage;
    }
    else if (X > 0 && X < 480 && Y > 95 && Y < 135 && page_chk == _bedpage) {      //Bed1
      Serial.print("Selected Bed = ");
      tft.fillRect(242, 300, 50, 18, 0x4FFF);
      bed_name = bedbuff[((bed_pagecnt - 1) * 4) + 1];
      bed_name = bed_name.substring(3, bed_name.length());
      disp_text(244, 300, 1, BLACK, bed_name.c_str());
      Serial.println(bed_name);
      return _bedpage;
    }
    else if (X > 0 && X < 480 && Y > 135 && Y < 175 && page_chk == _bedpage) {      //Bed2
      Serial.print("Selected Bed = ");
      tft.fillRect(242, 300, 50, 18, 0x4FFF);
      bed_name = bedbuff[((bed_pagecnt - 1) * 4) + 2];
      bed_name = bed_name.substring(3, bed_name.length());
      disp_text(244, 300, 1, BLACK, bed_name.c_str());
      Serial.println(bed_name);
      return _bedpage;
    }
    else if (X > 0 && X < 480 && Y > 175 && Y < 215 && page_chk == _bedpage) {      //Bed3
      Serial.print("Selected Bed = ");
      tft.fillRect(242, 300, 50, 18, 0x4FFF);
      bed_name = bedbuff[((bed_pagecnt - 1) * 4) + 3];
      bed_name = bed_name.substring(3, bed_name.length());
      disp_text(244, 300, 1, BLACK, bed_name.c_str());
      Serial.println(bed_name);
      return _bedpage;
    }
    else if (X > 395 && X < 470 && Y > 225 && Y < 250 && page_chk == _drugpage) {  //drug nextpage
      if (cat_pagecnt >= total_pagecnt) {
        return page_chk;
      }
      else {
        _page_token = true;
        _next_flag = true;
        return _drugpage;
      }
    }
    else if (X > 30 && X < 100 && Y > 225 && Y < 250 && page_chk == _drugpage) {  //drug prevpage
      if (cat_pagecnt == 1) {
        return page_chk;
      }
      else {
        _page_token = true;
        _prev_flag = true;
        return _drugpage;
      }
    }
    else if (X > 0 && X < 480 && Y > 55 && Y < 95 && page_chk == _drugpage) {   // drugpage Med1 select
      _page_token = true;
      catIndex = 0;
      return _submedpage;
    }
    else if (X > 0 && X < 480 && Y > 95 && Y < 135 && page_chk == _drugpage) {   // drugpage Med2 select
      if (drugindex >= 2) {
        _page_token = true;
        catIndex = 1;
        return _submedpage;
      }
      else {
        return page_chk;
      }
    }
    else if (X > 0 && X < 480 && Y > 135 && Y < 175 && page_chk == _drugpage) {   // drugpage Med3 select
      if (drugindex >= 3) {
        _page_token = true;
        catIndex = 2;
        return _submedpage;
      }
      else
      {
        return page_chk;
      }
    }
    else if (X > 0 && X < 480 && Y > 175 && Y < 215 && page_chk == _drugpage) {   // drugpage Med4 select
      if (drugindex >= 4) {
        _page_token = true;
        catIndex = 3;
        return _submedpage;
      }
      else
      {
        return page_chk;
      }
    }
    else if (X > 395 && X < 470 && Y > 225 && Y < 250 && page_chk == _submedpage) {   // subdrugpage nextpage
      if (subcat_pagecnt >= total_subpagecnt) {
        return page_chk;
      }
      else {
        _page_token = true;
        _subnext_flag = true;

        return _submedpage;
      }
    }
    else if (X > 30 && X < 100 && Y > 225 && Y < 250 && page_chk == _submedpage) {   // subdrugpage prevpage
      if (subcat_pagecnt == 1) {
        return page_chk;
      }
      else {
        _page_token = true;
        _subprev_flag = true;

        return _submedpage;
      }
    }
    else if (X > 0 && X < 480 && Y > 55 && Y < 95 && page_chk == _submedpage) {   // drugpage subMed1 select
      _page_token = true;
      sub_catIndex = 0;
      drug_name = submed[(subcat_pagecnt - 1) * 4 + sub_catIndex];
      Serial.println(submed[(subcat_pagecnt - 1) * 4 + sub_catIndex]);
      lasttab_open(old_page_chk);
      return _flowratepage;
    }
    else if (X > 0 && X < 480 && Y > 95 && Y < 135 && page_chk == _submedpage) {   // drugpage subMed2 select
      if (subdrug >= 2) {
        _page_token = true;
        sub_catIndex = 1;
        drug_name = submed[(subcat_pagecnt - 1) * 4 + sub_catIndex];
        Serial.println(submed[(subcat_pagecnt - 1) * 4 + sub_catIndex]);
        lasttab_open(old_page_chk);
        return _flowratepage;
      }
      else {
        return page_chk;
      }

    }
    else if (X > 0 && X < 480 && Y > 135 && Y < 175 && page_chk == _submedpage) {   // drugpage subMed3 select
      if (subdrug >= 3) {
        _page_token = true;
        sub_catIndex = 2;
        drug_name = submed[(subcat_pagecnt - 1) * 4 + sub_catIndex];
        Serial.println(submed[(subcat_pagecnt - 1) * 4 + sub_catIndex]);
        lasttab_open(old_page_chk);
        return _flowratepage;
      }
      else {
        return page_chk;
      }
    }
    else if (X > 0 && X < 480 && Y > 175 && Y < 215 && page_chk == _submedpage) {   // drugpage subMed4 select
      if (subdrug >= 4) {
        _page_token = true;
        sub_catIndex = 3;
        drug_name = submed[(subcat_pagecnt - 1) * 4 + sub_catIndex];
        Serial.println(submed[(subcat_pagecnt - 1) * 4 + sub_catIndex]);
        lasttab_open(old_page_chk);
        return _flowratepage;
      }
      else {
        return page_chk;
      }
    }
    else if (X > 150 && X < 230 && Y > 170 && Y < 210 && page_chk == _startstoppage) {  //start or stop yes

      _page_token = true;
      _change_flag++;
      //      _pause_flag = true;
      Serial.println("yes" );
      if (_change_flag % 2 == 0) {
        tft.fillScreen(0x0021);
        TIMSK1 &= (0 << OCIE1A);    //Disable Screensaver
        //noInterrupts();
        EIMSK = 0x00;
        return _loginpage;
      }
      else {
        lasttab_open(old_page_chk);
        tft.fillRect(321, 0, 78, 49, 0x4FFF);
        //UCSR1B |= (1 << RXCIE1);
        //        String pay = "$2,150," + String(vol) + ",15," + String(gtt) + ",0,1#";
        //
        //        char datbuff[pay.length() + 1];
        //        pay.toCharArray(datbuff, pay.length() + 1);
        //        Serial.println(pay);
        //        for (int j = 0; j < pay.length(); j++) {
        //          USART_Transmit(datbuff[j]);
        //          delay(2);
        //        }
        //        USART_Transmit('\n');
        //interrupts();             // enable all interrupts

        _startflow = true;
        _page_no = _flowratepage;
        TIMSK1 |= (1 << OCIE1A);  // enable Screensaver
        draw_Stopbutton();
        return _flowratepage;
      }
    }
    else if (X > 280 && X < 360 && Y > 170 && Y < 210 && page_chk == _startstoppage) {   //Start or stop no
      _page_token = true;
      //      _pause_flag = false;
      Serial.println("no" );
      lasttab_open(old_page_chk);
      return _flowratepage;
    }
    else if (X > 0 && X < 80 && Y > 0 && Y < 50 && (page_chk != _loginpage && page_chk != _screensaverpage) && (_volkeypad == false && _startflow == false)) {       //GTT_Volume Page
      _page_token = true;
      lasttab_open(old_page_chk);
      return _gttvolpage;
    }
    else if (X > 80 && X < 160 && Y > 0 && Y < 50 && (page_chk != _loginpage && page_chk != _screensaverpage) && (_volkeypad == false && _startflow == false)) {     //FlowRate Page
      _page_token = true;
      lasttab_open(old_page_chk);
      return _flowratepage;
    }
    else if (X > 161 && X < 240 && Y > 0 && Y < 50 && (page_chk != _loginpage && page_chk != _screensaverpage) && (_volkeypad == false && _startflow == false)) {    //Bedlist Page
      if (page_chk == _bedpage) {
        Serial.println("Do nothing");
      }
      else {
        _page_token = true;
        //_bedflag = true;

        lasttab_open(old_page_chk);
        return _bedpage;
      }
    }
    else if (X > 241 && X < 320 && Y > 0 && Y < 50 && (page_chk != _loginpage && page_chk != _screensaverpage) && (_volkeypad == false && _startflow == false)) {    //Drug Page
      _page_token = true;
      lasttab_open(old_page_chk);
      tft.fillRect(241, 0, 78, 49, 0x0193);
      draw_DrugIcon(250, 1, 0xF980);
      nm = 0;
      nm1 = 0;
      cat_pagecnt = 1;
      subcat_pagecnt = 1;
      subdrug = 0;
      drugindex = 0;
      _medflag = true;  //to read medlist from sdcard

      return _drugpage;
    }
    else if (X > 321 && X < 400 && Y > 0 && Y < 50 && (page_chk != _loginpage) && _volkeypad == false) {    //Start_Stop Page
      _page_token = true;
      lasttab_open(old_page_chk);
      return _startstoppage;
    }
    else if (X > 401 && X < 480 && Y > 0 && Y < 50 && (page_chk != _loginpage) && _volkeypad == false) {    //Alert Page
      _page_token = true;
      lasttab_open(old_page_chk);
      return _alertpage;
    }
  }
  else {
    //    Serial.print("Returning page_chk = ");
    //    Serial.println(page_chk);
    return page_chk;
  }
}

/****************************************************************************************************************/
/*------------------------------------------Display Image From SD-----------------------------------------------*/
/****************************************************************************************************************/

uint16_t read16(File & f) {
  uint16_t result;         // read little-endian
  f.read(&result, sizeof(result));
  return result;
}

uint32_t read32(File & f) {
  uint32_t result;
  f.read(&result, sizeof(result));
  return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
  File bmpFile;
  int bmpWidth, bmpHeight;    // W+H in pixels
  uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
  uint32_t bmpImageoffset;    // Start of image data in file
  uint32_t rowSize;           // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
  uint8_t bitmask, bitshift;
  boolean flip = true;        // BMP is stored bottom-to-top
  int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
  uint32_t pos;               // seek position
  boolean is565 = false;      //

  uint16_t bmpID;
  uint16_t n;                 // blocks read
  uint8_t ret;

  if ((x >= tft.width()) || (y >= tft.height()))
    return 1;               // off screen

  bmpFile = SD.open(nm);      // Parse BMP header
  bmpID = read16(bmpFile);    // BMP signature
  (void) read32(bmpFile);     // Read & ignore file size
  (void) read32(bmpFile);     // Read & ignore creator bytes
  bmpImageoffset = read32(bmpFile);       // Start of image data
  (void) read32(bmpFile);     // Read & ignore DIB header size
  bmpWidth = read32(bmpFile);
  bmpHeight = read32(bmpFile);
  n = read16(bmpFile);        // # planes -- must be '1'
  bmpDepth = read16(bmpFile); // bits per pixel
  pos = read32(bmpFile);      // format
  if (bmpID != 0x4D42) ret = 2; // bad ID
  else if (n != 1) ret = 3;   // too many planes
  else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
  else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette
  else {
    bool first = true;
    is565 = (pos == 3);               // ?already in 16-bit format
    // BMP rows are padded (if needed) to 4-byte boundary
    rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
    if (bmpHeight < 0) {              // If negative, image is in top-down order.
      bmpHeight = -bmpHeight;
      flip = false;
    }

    w = bmpWidth;
    h = bmpHeight;
    if ((x + w) >= tft.width())       // Crop area to be loaded
      w = tft.width() - x;
    if ((y + h) >= tft.height())      //
      h = tft.height() - y;

    if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
      bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
      bitmask = 0xFF;
      if (bmpDepth < 8)
        bitmask >>= bmpDepth;
      bitshift = 8 - bmpDepth;
      n = 1 << bmpDepth;
      lcdbufsiz -= n;
      palette = lcdbuffer + lcdbufsiz;
      for (col = 0; col < n; col++) {
        pos = read32(bmpFile);    //map palette to 5-6-5
        palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
      }
    }

    // Set TFT address window to clipped image bounds
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    for (row = 0; row < h; row++) {
      // For each scanline...
      // Seek to start of scan line.  It might seem labor-
      // intensive to be doing this on every line, but this
      // method covers a lot of gritty details like cropping
      // and scanline padding.  Also, the seek only takes
      // place if the file position actually needs to change
      // (avoids a lot of cluster math in SD library).
      uint8_t r, g, b, *sdptr;
      int lcdidx, lcdleft;
      if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
      else        // Bitmap is stored top-to-bottom
        pos = bmpImageoffset + row * rowSize;
      if (bmpFile.position() != pos) { // Need seek?
        bmpFile.seek(pos);
        buffidx = sizeof(sdbuffer); // Force buffer reload
      }

      for (col = 0; col < w; ) {  //pixels in row
        lcdleft = w - col;
        if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
        for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
          uint16_t color;
          // Time to read more pixel data?
          if (buffidx >= sizeof(sdbuffer)) { // Indeed
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0; // Set index to beginning
            r = 0;
          }
          switch (bmpDepth) {          // Convert pixel from BMP to TFT format
            case 24:
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              color = tft.color565(r, g, b);
              break;
            case 16:
              b = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              if (is565)
                color = (r << 8) | (b);
              else
                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
              break;
            case 1:
            case 4:
            case 8:
              if (r == 0)
                b = sdbuffer[buffidx++], r = 8;
              color = palette[(b >> bitshift) & bitmask];
              r -= bmpDepth;
              b <<= bmpDepth;
              break;
          }
          lcdbuffer[lcdidx] = color;

        }
        tft.pushColors(lcdbuffer, lcdidx, first);
        first = false;
        col += lcdidx;
      }           // end cols
    }               // end rows
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
    ret = 0;        // good render
  }
  bmpFile.close();
  return (ret);
}


void USART_Transmit(unsigned char data )
{
  /* Wait for empty transmit buffer */
  while ( !( UCSR1A & (1 << UDRE1)) );
  /* Put data into buffer, sends the data */
  UDR1 = data;
}

/****************************************************************************************************************/
/*------------------------------------------------- SETUP ------------------------------------------------------*/
/****************************************************************************************************************/


void setup() {
  uint16_t ID;

  Serial.begin(9600);

  bool ret = true;
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
  ret = diagnose_pins();
#else
  Serial.println("Only AVR can diagnose Touch Pins");
  Serial.println("Other targets use default Pins");
  char buf[40];
  sprintf(buf, "Touch Pin Wiring XP=%d XM=A%d YP=A%d YM=%d",
          XP, XM - A0, YP - A0, YM);
  Serial.println(buf);
#endif
  digitalWrite(A0, HIGH);
  pinMode(A0, OUTPUT);
  //Serial.print("Show BMP files on TFT with ID:0x");
  ID = tft.readID();
  //Serial.println(ID, HEX);
  if (ID == 0x0D3D3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(1);

  //tft.setFont(&FreeSans9pt7b); //FreeSansBold12pt7b
  //tft.setFont(&FreeSansBold12pt7b);
  tft.setFont(&FreeMonoBold18pt7b);

  bool good = SD.begin(SD_CS);
  if (!good) {
    Serial.print(F("cannot start SD"));
    //while (1);
  }

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);

  pinMode(40, OUTPUT);

  digitalWrite(40, HIGH);

//  attachInterrupt(digitalPinToInterrupt(20), Sensor1_Detect, FALLING);

  tft.invertDisplay(true);
  tft.fillScreen(0x0021); //0x0043

//  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 62500;            // compare match register 16MHz/1024/0.25Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12 | 1 << CS10);    // 256 prescaler
//TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3  = 0;

//  OCR3B = 31250;            // compare match register 16MHz/256/8Hz    (~125ms)
//  TCCR3B |= (1 << WGM32);   // CTC mode
//  TCCR3B |= (1 << CS32);    // 256 prescaler
//  TIMSK3 |= (1 << OCIE3A);  // enable timer compare interrupt
//  interrupts();             // enable all interrupts

  /* Serial1 Enable with Interrupt */
  /* Set baud rate */
  UBRR1H = (unsigned char)(MYUBRR >> 8);
  UBRR1L = (unsigned char)MYUBRR;
  /* Set frame format: 8data, 2stop bit */
  UCSR1C |= (1 << USBS1) | (1 << UCSZ10) | (1 << UCSZ11); //(1 << USBS1) 
  /* Enable receiver and transmitter */
  UCSR1B |= (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);

  //  EICRB = 0x02;          //INT0, triggered on falling edge
  //  EIMSK = 0x00;          //Disable only INT0

    _page_token = true;

  //Serial1.begin(9600);
  //_rxflag = true;
  //interrupts();             // enable all interrupts
}
int icount = 0;
void loop() {
  static pvFunc_Handler Func_pages[NO_OF_PAGES] = {Func_Loginpage, Func_GttVolpage, Func_Flowratepage, Func_Bedpage, Func_Drugpage, Func_Medpage, Func_Startstoppage, Func_Alertpage, Func_Screensaverpage};
  static updatesd Func_Usd[4] = {Func_Norm, Func_Nurse, Func_Drug, Func_Bed};

  static int page_no = 0, dpcount = 0;
  static int x = 0, f = 0;
  static unsigned long currentmillis = 0, serialmillis = 0, currentmicros = 0;
  static float ratecount = 0, volcount = 0;
  static boolean _serial_flag = false;
  // String rxbuff = ""; //"{\"Flag\":\"2\",\"Drugcount\":\"5\",\"Drugcat\":\"Med6\",\"Drugname\":[\"Uaxdrvghtr\",\"Iyzyrvghtr\",\"Hgxxrvghtr\",\"Kdbcrvghtr\",\"Jaddovghtr\"]}";
  static uint8_t wifi_flag = 0;
  int len = 0, i0, i1, i2, ledflag, pinch, buzz;
  String _temp = "";
  float volremain = 0;
  int min = 0, hour = 0;

  if (_page_token == true) {
    _page_token = false;
    (Func_pages[page_no])();
  }

  //  while (Serial1.available()) {
  //    rxbuff = Serial1.readString();
  //    Serial.println(rxbuff);
  //    _rxflag = true;
  //  }

  //  if (_serial_event == true) {
  //    _serial_event = false;
  //    rxbuff = serial_recv;
  //    _rxflag = true;
  //  }

  if (_rxflag == true) {
    _rxflag = false;
    serial_recv = "";
    for (int j = 0; j < icount; j++) {
      serial_recv += rxbuff[j];
    }
    Serial.println(serial_recv);
    Func_Usd[jsonparser(serial_recv)](serial_recv);
  }


  //  if (_rxflag == true) {
  //    _rxflag = false;
  //    Serial.print(rxbuff);
  //    Func_Usd[jsonparser(rxbuff)](rxbuff);
  //    //rxbuff = "";
  //  }



  //  if (_startflow == true && _serial_event == true && _rxflag == true ) {
  //
  //    _serial_event = false;
  //    _serial_flag = true;
  //    _rxflag = false;
  //
  //    serial_recv.trim();
  //    len = serial_recv.length();
  //
  //    i0 = serial_recv.indexOf(',', 3);
  //    _temp = serial_recv.substring(3, i0);
  //
  //    Serial.print("temp = ");
  //    Serial.println(_temp);      //FR
  //
  //    ratecount = atof(_temp.c_str()); //_temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', 4);
  //    i2 = serial_recv.indexOf(',', i0 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //SR
  //
  //    rate = atof(_temp.c_str()); //_temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);  //VR
  //
  //    volcount = atof(_temp.c_str()); //_temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //SV
  //
  //    vol = atof(_temp.c_str());//_temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //Drug
  //
  //    drug = _temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //GTT
  //
  //    gtt = _temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //Batt
  //
  //    batt_lvl = _temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //wifi
  //
  //    wifi_flag = _temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //rfidflag
  //
  //    _rfid_flag = _temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //dpcount
  //
  //    dpcount = _temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //TTF
  //
  //    ttf = _temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //name
  //
  //    nurse_name = _temp;
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //alerts
  //
  //
  //    ledflag = _temp.toInt();
  //
  //    i1 = serial_recv.indexOf(',', i2 + 1);
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //
  //    _temp = serial_recv.substring(i1 + 1, i2);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //pinch
  //
  //    pinch = _temp.toInt();
  //
  //    i2 = serial_recv.indexOf(',', i1 + 1);
  //    i1 = serial_recv.indexOf('#', i2 + 1);
  //
  //    _temp = serial_recv.substring(i2 + 1, i1);
  //    Serial.print("temp = ");
  //    Serial.println(_temp);    //buzz
  //
  //    buzz = _temp.toInt();
  //  }
  //
  //  if (_serial_flag == true && _startflow == true) {
  //
  //    _serial_flag = false;
  //    serial_recv = "";
  //    if (_startflow == true && page_no == _flowratepage) {
  //
  //      tft.setFont(&FreeSansBold18pt7b);
  //      tft.fillRect(268, 85, 200, 40,  0x0021);
  //      disp_text(270, 105, 1, WHITE , String(ratecount).c_str());
  //
  //      tft.setFont(&FreeSans12pt7b);
  //      disp_text(270, 225, 1, WHITE, drug_name.c_str());
  //      disp_text((270 + (String(ratecount).length() * 20)) , 105, 1, WHITE, "ml/hr");
  //
  //      tft.setFont(&FreeSansBold18pt7b);
  //      hour = ttf / 60;
  //      if (hour < 9) {
  //        _temp = "0";
  //        _temp += String(hour);
  //      }
  //      else {
  //        _temp = String(hour);
  //      }
  //      tft.fillRect(268, 145, 40, 40,  0x0021);
  //      disp_text(270, 165, 1, WHITE, _temp.c_str());
  //
  //      min = ttf - (hour * 60);
  //      if (min < 9) {
  //        _temp = "0";
  //        _temp += String(min);
  //      }
  //      else {
  //        _temp = String(min);
  //      }
  //      tft.fillRect(345, 145, 40, 40,  0x0021);
  //      disp_text(345 , 165, 1, WHITE, _temp.c_str());
  //      tft.setFont(&FreeSans12pt7b);
  //    }
  //    else if (_startflow == true && page_no == _screensaverpage) {
  //
  //      tft.setFont(&FreeSansBold18pt7b);
  //      tft.fillRect(200, 70, 150, 40,  0x0021);
  //      disp_text(200, 90, 1, WHITE , String(volcount).c_str());    //len*20
  //
  //
  //      volremain = 100 - volcount;
  //      tft.fillRect(200, 120, 150, 40,  0x0021);
  //      disp_text(200, 140, 1, WHITE , String(volremain).c_str());
  //
  //
  //      tft.fillRect(5, 220, 250, 40,  0x0021);
  //      disp_text(5 , 240, 1, WHITE , String(ratecount).c_str());
  //      Serial.print("Ratecount = ");
  //      Serial.println(ratecount);
  //      tft.setFont(&FreeSans12pt7b);
  //      disp_text((200 + (String(volremain).length() * 18)), 140, 1, WHITE, "ml");
  //      disp_text((200 + (String(volcount).length() * 18)), 90, 1, WHITE, "ml");
  //      disp_text(5 + (String(ratecount).length() * 18), 240, 1, WHITE, "ml/hr");
  //
  //      tft.setFont(&FreeSansBold18pt7b);
  //      hour = ttf / 60;
  //      if (hour < 9) {
  //        _temp = "0";
  //        _temp += String(hour);
  //      }
  //      else {
  //        _temp = String(hour);
  //      }
  //      tft.fillRect(310, 220, 40, 40,  0x0021);
  //      disp_text(310 , 240, 1, WHITE , _temp.c_str());
  //
  //      min = ttf - (hour * 60);
  //      if (min < 9) {
  //        _temp = "0";
  //        _temp += String(min);
  //      }
  //      else {
  //        _temp = String(min);
  //      }
  //      tft.fillRect(375, 220, 40, 40,  0x0021);
  //      disp_text(375, 240, 1, WHITE , _temp.c_str());
  //      tft.setFont(&FreeSans12pt7b);
  //      disp_text(195, 185, 1, WHITE, drug_name.c_str());
  //    }
  //  }

////////////////*******************************//////////////////////////////////////////*********************//////////////
  // if (_screen_saver == true) {
  //   _screen_saver = false;
  //   _page_token = true;
  //   TIMSK1 &= (0 << OCIE1A);
  //   lasttab_open(_flowratepage);
  //   page_no = _screensaverpage;
  //   // TIMSK3 |= (1 << OCIE3A);  // enable timer compare interrupt
  // }

  // if (_startflow == true && page_no != _screensaverpage && page_no != _alertpage && page_no != _loginpage) {
  //   TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  // }

  // if (_sensor1_flag == true) {
  //   _sensor1_flag = false;
  //   EIMSK = 0x00;
  //   draw_DropIcon(420, 120, 25, 55, WHITE, 0x0021);
  //   delay(200);
  //   draw_DropIcon(420, 120, 25, 55, 0x0021, 0x0021);
  //   _serial_event = true;
  //   EIMSK = 0x01;
  // }

////////////////*******************************//////////////////////////////////////////*********************//////////////

  //  if (_sensor1_flag == true && (currentmicros + 30000 >= micros())) {
  //    currentmicros = micros();
  //    draw_DropIcon(420, 120, 25, 55, 0x0021, 0x0021);
  //  }
  //  else if (currentmicros + 30000 < micros()) {
  //    currentmicros = micros();
  //  }


  x = page_no;
  page_no = touchdetect(x);
  Serial.print("\nPageTouch:- ");
  Serial.println(page_no);
  //_page_no = page_no;
}

/****************************************************************************************************************/
/*----------------------------------------------- Interrupts ---------------------------------------------------*/
/****************************************************************************************************************/

/*============================================= Serial Interrupt ================================================*/

//void serialEvent1() {
//  //detachInterrupt(20);
//  draw_DropIcon(420, 120, 25, 55, 0x0021, 0x0021);
//  serial_recv = "";
//  while (Serial1.available()) {
//    serial_recv = Serial1.readString();
//    //Serial.print(serial_recv);
//  }
//  _serial_event = true;
//  if (_stop_interrupt == true) {
//    // attachInterrupt(digitalPinToInterrupt(20), Sensor1_Detect, CHANGE);
//  }
//
//  //delay(100);
//  draw_DropIcon(420, 120, 25, 55, WHITE, 0x0021);
//}

ISR(USART1_RX_vect) {
  // EIMSK = 0x00;
  //Serial.println("Inside Serial Isr");
  if (UDR1 == '\n') {
    icount = _rxi;
    _rxi = 0;
    _rxflag = true;
  }
  else {
    rxbuff[_rxi] = UDR1;
    _rxflag = false;
    _rxi++;
  }
  // EIMSK = 0x01;
}

/*========================================== End of Serial Interrupt =============================================*/

/*============================================= Sensor1 Interrupt ================================================*/

//__attribute__((interrupt)) void Sensor1_Detect() {
//  Serial.println("Inside ISR");
//  if (_sensor1_flag == true) {
//    _sensor1_flag = false;
//    draw_DropIcon(420, 120, 25, 55, WHITE, 0x0021);
//  }
//  else {
//    _sensor1_flag = true;
//    draw_DropIcon(420, 120, 25, 55, 0x0021, 0x0021);
//  }
//  //_sensor1_flag = true;
//  //draw_DropIcon(420, 120, 25, 55, WHITE, 0x0021);
//  //TIMSK3 |= (1 << OCIE3A);  // enable timer 3 compare interrupt
//  //interrupts();
//}

//ISR(INT0_vect)
//{
//  //EIMSK = 0x00;
//  Serial.println("Inside Isr");
//  _sensor1_flag = true;
//  _serial_event = false;
//  //EIMSK = 0x01;
//}

/*========================================== End of Sensor1 Interrupt =============================================*/

ISR(TIMER1_COMPA_vect)
{
  if (screencnt >= (SCREENSAVER_INTERVAL / 4)) {
    _screen_saver = true;
    _page_no = _screensaverpage;
    screencnt = 0;
  }
  screencnt++;
}

#pragma GCC pop_options
