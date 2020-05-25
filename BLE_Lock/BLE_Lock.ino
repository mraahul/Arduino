#include <Keypad.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include<String.h>

const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
SoftwareSerial hc06(2, 3); // RX, TX

const byte ROWS = 4; // four rows
const byte COLS = 3; // three columns

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {5, 4, 9, 10}; // connect to the row pinouts of the keypad.
byte colPins[COLS] = {8, 7, 6};    // connect to the column pinouts of the keypad.

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String password = "5689";  // setting default password.
String userPassword = " ";
int cursorPosition = 0; // variable for determining the cursorposition.

int wrong = 0; // variable for calculating the wrong input.
int total = 0; // variable to determine the number of wrong attempts.

void setup() {
  Serial.begin(9600);
  hc06.begin(9600);

  lcd.begin(16, 2);
  lcd.print("Smart Door Lock!!");
  lcd.setCursor(0, 2);
  delay(1500);
  lcd.clear();
  delay(100);
}
void BluetoothInput()
{
  while (hc06.available() > 0)
  {
    userPassword = hc06.readString();
    Serial.print("Recieved String: ");
    Serial.println(userPassword);

    if (password == userPassword)
    {
      Serial.println("Access Granted:)");
      hc06.println("Access Granted:)");
      lcd.setCursor(0, 0);
      lcd.print("Access Granted:)");
      lcd.setCursor(0, 1);
      lcd.print("via BT");
      delay(2000);
    }
    else {
      Serial.println("Access Denied:(");
      hc06.println("Access Denied:(");
      lcd.setCursor(0, 0);
      lcd.print("Access Denied:(");
      lcd.setCursor(0, 1);
      lcd.print("via BT");
      delay(2000);
    }
    lcd.clear();
  }
}

void KeypadInput()
{
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  delay(100);

  char pressed = keypad.getKey(); // TAKING THE INPUT FROM KEYPAD
  String key[4];

  if (pressed) // if the key is pressed
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
    { // in case of wrong input increment both wrong and cursorposition.
      wrong++;
      cursorPosition ++;
    }

    if (cursorPosition == 4)
    { // when cursorposition == 4 then check the following
      if ( wrong > 0) // if any wrong input if given then increment total and
        // set wrong and cursorposition to zero.
      {
        total++;
        wrong = 0;
        cursorPosition = 0;
        lcd.clear();
        Serial.println("Access Denied:(");
        lcd.print("Access Denied:(");
        delay(1000);
      }

      else if (cursorPosition == 4 && wrong == 0)
      { // if no wrong value is given then display the accepted password and
        cursorPosition = 0;
        wrong = 0;
        lcd.clear();
        lcd.print("Access Granted:)");
        Serial.println("Access Granted:(");
        delay(2000);
        lcd.clear();
      }

      if (total == 4) // if total of 4 attempts are done by entering wrong password
      {
        total = 0;
        delay(500);
      }
    }
  }
}

void loop() {
  KeypadInput();
  BluetoothInput();
}
