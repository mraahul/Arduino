#include<Keypad.h> // KEYPAD LIBRARY FOR KEYPAD INTERFACING 
#include<LiquidCrystal.h> // LIBRARY FOR LCD INTERFACING
#include<String.h>
#define buzzer 20 // DEFINING PIN FOR BUZZER.

const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {5, 4, 9, 10}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6};    // connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String password = "5689";  // SETTING DEFAULT PASSWORD.
int cursorPosition = 0; // VARIABLE FOR DETERMINING THE cursorPosition.

int wrong = 0; // VARIABLE FOR CALCULATING THE WRONG INPUT.
int total = 0; // VARIABLE TO DETERMINE THE NUMBER OF WRONG ATTEMPTS.

void setup()
{
  lcd.begin(16, 2);
  lcd.print("Smart Door Lock!!");
  lcd.setCursor(0, 2);
  delay(1500);
  lcd.clear();
  delay(100);
}

void loop()
{
  //  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  delay(100);

  char pressed = keypad.getKey(); // TAKING THE INPUT FROM KEYPAD
  String key[4];

  if (pressed) // IF THE KEY IS PRESSED
  {
    lcd.setCursor(cursorPosition, 2);
    lcd.print('*');
    delay(500);
    if (pressed == '*' || pressed == '#')
    {
      lcd.clear();
    }

    else if (pressed == password[cursorPosition])
    {
      key[cursorPosition] = pressed;
      cursorPosition++;
    }

    else if (pressed != password[cursorPosition] )
    { // IN CASE OF WRONG INPUT INCREMENT BOTH WRONG AND cursorPosition.
      wrong++;
      cursorPosition ++;
    }

    if (cursorPosition == 4)
    { // WHEN cursorPosition == 4 THEN CHECK THE FOLLOWING
      if ( wrong > 0) // IF ANY WRONG INPUT IF GIVEN THEN INCREMENT TOTAL AND
        // SET WRONG AND cursorPosition TO ZERO.
      {
        total++;
        wrong = 0;
        cursorPosition = 0;
        lcd.clear();
        lcd.clear();
        lcd.print("Access Denied:(");
        delay(1000);
      }

      else if (cursorPosition == 4 && wrong == 0)
      { // IF NO WRONG VALUE IS GIVEN THEN DISPLAY THE ACCEPTED PASSWORD AND
        cursorPosition = 0;
        wrong = 0;
        lcd.clear();
        lcd.print("Access Granted:)");
        delay(2000);
        lcd.clear();
      }

      if (total == 4) // IF TOTAL OF 4 ATTEMPTS ARE DONE BY ENTERING WRONG PASSWORD
      {
        total = 0;
        delay(500);
      }
    }
  }
}

