//Library for temporized interrupts
#include <TimerOne.h>

//Define port we're  going to use
#define BTN1 A1
#define BTN2 A2
#define BTN3 A3
#define led1 13
#define led2 12
#define led3 11
#define led4 10
#define buzzerPin 3
#define latchPin 4
#define clockPin 7
#define dataPin 8

//We keep track of the LED states: true means the LED is off and false that it is on
bool led1state = true;
bool led2state = true;
bool led3state = true;
bool led4state = true;

void setup() {
  //Initializam interrupturile temporizate
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(manageTime);

  //Initializam modul pinilor pentru LEDuri
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  
  //Initializam modul pinilor pentru afisor
  pinMode(buzzerPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  //Scriem '1' pe leduri pentru a fi stinse la inceput
  digitalWrite(led1, led1state);
  digitalWrite(led2, led2state);
  digitalWrite(led3, led3state);
  digitalWrite(led4, led4state);
    
  //Scriem '1' pe pinul de buzzer pentru ca alarma sa nu porneasca in starea initiala
  digitalWrite(buzzerPin, HIGH);

  //Initializam portul serial cu un baud rate de 9600bits/s
  Serial.begin(9600);
}

const unsigned int DEBOUNCE_INTERVAL = 200;

//Keep track of when the last time each button has been pressed was
unsigned long lastFireBTN1 = 0l;
unsigned long lastFireBTN2 = 0l;
unsigned long lastFireBTN3 = 0l;

//Keep track of time and alarm time
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char alarmHours = 255;
unsigned char alarmMinutes = 255;

//In the initial state, the clock can be initialized with the current time serially
bool initHour = true;

//Define the three special states of the clock
bool displayAlarmMode = false;
bool disableAlarmMode = false;
bool setupMode = false;
void loop() {
  //Show alarm time by pressing BTN1
  if (!digitalRead(BTN1)) {
    //Debounce button BTN1
    if (millis() - lastFireBTN1 >= DEBOUNCE_INTERVAL) {
      if (!setupMode) {
        displayAlarmMode = !displayAlarmMode;
      } else {
        if (initHour) {
          hours++;
        } else {
          alarmHours++;
        }
      }
      lastFireBTN1 = millis();
      led1state = !led1state;
      digitalWrite(led1, led1state);
    }
  }

  //Disable alarms by pressing BTN2
  if (!digitalRead(BTN2)) {
    //Debounce button BTN2
    if (millis() - lastFireBTN2 >= DEBOUNCE_INTERVAL) {
      if (!setupMode) {
        disableAlarmMode = !disableAlarmMode;
      } else {
        if (initHour) {
          minutes++;
        } else {
          alarmMinutes++;
        }
      }
      lastFireBTN2 = millis();
      led2state = !led2state;
      digitalWrite(led2, led2state);
    }
  } 

  //Enter setup mode by pressing BTN3
  if (!digitalRead(BTN3)) {
    //Debounce button BTN3
    if (millis() - lastFireBTN3 >= DEBOUNCE_INTERVAL) { 
      //At the end of the first setupMode, the current time will have been initialized
      if (setupMode) {
        initHour = false;
      }
      //If we are about to set the alarm time, either start from the last valid alarm had there been one, or from 00:00
      else if (!initHour && (alarmHours == 255 || alarmMinutes == 255)) {
        alarmHours = 0;
        alarmMinutes = 0;
      }
      setupMode = !setupMode;
      lastFireBTN3 = millis();
      led3state = !led3state;
      digitalWrite(led3, led3state);
    }
  }

  //When in setupMode, if we have already set a valid alarm then 
  //display it. If we don't have a valid alarm time yet, then if 
  //BTN1 has been pressed display the alarm time, otherwise display 
  //the current time. 
  if (setupMode && alarmHours != 255 && alarmMinutes != 255) {
    showTime(alarmHours, alarmMinutes);
  } else {
    if (displayAlarmMode) {
      showTime(alarmHours, alarmMinutes);
    } else {
      showTime(hours, minutes);
    }
  }
}

//Variable used for settting the length of the alarm sound
unsigned char beepCount = 5;
/*
 * Increment time and time the buzzer
 */
void manageTime() {
  if (!setupMode) {
    //Make sure we have valid time
    if (++minutes == 60) {
      minutes = 0;
      if (++hours == 24) {
        hours = 0;
      }
    }

    //When current tim reaches the alarm time start beeping the buzzer and only stop it after beepCounter seconds
    if (hours == alarmHours && minutes == alarmMinutes) {
      if (!disableAlarmMode) {
        digitalWrite(buzzerPin, LOW);
      }
      beepCount--;
    }

    if (beepCount != 5) {
      beepCount--;
      if (beepCount == 0) {
        beepCount = 5;
        digitalWrite(buzzerPin, HIGH);
      }
    }
  } 
}

//Buffer string for receiving the serial input
String inputString="";
/*
 * Receive serial data for current time and alarm time
 */
void serialEvent() {
  if (setupMode) {
    if (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar != '\n') {
        inputString += inChar;
      } else {
        if (initHour) {
          hours = ((inputString[0] - 48) * 10 + (inputString[1] - 48)) % 24;
          minutes = ((inputString[2] - 48) * 10 + (inputString[3] - 48)) % 60;
          showTime(hours, minutes);
          initHour = false;
        } else {
          alarmHours = ((inputString[0] - 48) * 10 + (inputString[1] - 48)) % 24;
          alarmMinutes = ((inputString[2] - 48) * 10 + (inputString[3] - 48)) % 60;
          showTime(alarmHours, alarmMinutes);
        }
        inputString = "";
      }
    }
  }
}

/*
 * Display time on SSD
 * @hours is the number to be displayed on the two leftwardsmost SSD slots
 * @minutes is the number to be displayed on the two rightwardsmost SSD slots 
 */
void showTime(unsigned char hours, unsigned char minutes) {
  writeNumberToSegment(0, hours / 10 % 10);
  writeNumberToSegment(1, hours % 10);
  writeNumberToSegment(2, minutes / 10 % 10);
  writeNumberToSegment(3, minutes % 10);
}

//SSD required constants
const byte SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0X80, 0X90, 0xFF};
const byte SEGMENT_SELECT[] = {0x01, 0x02, 0x04, 0x08};
/*
 * Display hex caracter on one of the four slots
 * @Segment selects one of the four slots
 * @Value selects one of the 16 hex numbers to be printed
 */
void writeNumberToSegment(byte segment, byte value) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, SEGMENT_MAP[value]);
  shiftOut(dataPin, clockPin, MSBFIRST, SEGMENT_SELECT[segment] );
  digitalWrite(latchPin, HIGH);
}
