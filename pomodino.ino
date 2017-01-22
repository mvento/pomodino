/*
  Countdown on a single DMD display
 */

#include <SPI.h>
#include <DMD2.h>
#include <fonts/Droid_Sans_12.h>




SoftDMD dmd(1,1);  // DMD controls the entire display
DMD_TextBox box(dmd, 3, 3, 32, 16);  // "box" provides a text box to automatically write to/scroll the display
int speakerPin = 2;

class Countdown
{
public:
  Countdown(unsigned long totalMinutes = 0) {
    _startedAt = millis();
    unsigned long totalMilliseconds = totalMinutes * 60 * 1000;
    _finishAt = _startedAt + totalMilliseconds;
  }

  unsigned long secondsLeft() {
    if (this->isFinished())
      return 0;

    return (_finishAt - millis()) / 1000;
  }

  bool isFinished() {
    if (millis() > this->_finishAt)
      return true;
    
    return false;
  }


  char * toString() {
    char screen[6];
    screen[5] = 0;


    unsigned int secondsLeft = this->secondsLeft();  

    unsigned int minutes = secondsLeft / 60;
    unsigned int seconds = secondsLeft % 60;


    sprintf(screen, "%02d:%02d", minutes, seconds);

    return screen;
  }

private:
  unsigned long _startedAt;  
  unsigned long _finishAt;
};



class Pomodoro
{
public:
  Pomodoro() {
    WORK_SESSION = 25;
    BREAK_DURATION = 5;
    LONG_BREAK_DURATION = 15;
    WORK_SESSIONS_BEFORE_LONG_BREAK = 4; 

    _actualSession = 0;
    _actualCountDown = getNextCountDown();
  }

void startbreakNotification() {
  digitalWrite(speakerPin, HIGH);
  delay(250);
  digitalWrite(speakerPin, LOW);

}

void startlongbreakNotification() {
  for (int i=0; i<2; i++) {
    digitalWrite(speakerPin, HIGH);
    delay(50);
    digitalWrite(speakerPin, LOW);
    delay(50);
  }
  digitalWrite(speakerPin, HIGH);
  delay(250);
  digitalWrite(speakerPin, LOW);
}

void startworkNotification() {
  for (int i=0; i<2; i++) {
    digitalWrite(speakerPin, HIGH);
    delay(50);
    digitalWrite(speakerPin, LOW);
    delay(50);
  }
}



Countdown getNextCountDown () {

  if (!(_actualSession % 2)) {
      _actualSession++;
      Serial.println("WORK_SESSION");
      this->startworkNotification();
      return Countdown(WORK_SESSION);
  }

  if (_actualSession < WORK_SESSIONS_BEFORE_LONG_BREAK * 2) {
      _actualSession++;
      this->startbreakNotification();
      Serial.println("BREAK_DURATION");
      return Countdown(BREAK_DURATION);
  }

  _actualSession = 0;
  Serial.println("LONG_BREAK_DURATION");
  this->startlongbreakNotification();
  return Countdown(LONG_BREAK_DURATION);

}

bool check() {
  if ( _actualCountDown.isFinished() ) {
    _actualCountDown = this->getNextCountDown();

  }
}

Countdown getCountdown () {
   return _actualCountDown; 
}


private:
  unsigned int WORK_SESSION;
  unsigned int BREAK_DURATION;
  unsigned int LONG_BREAK_DURATION;
  unsigned int WORK_SESSIONS_BEFORE_LONG_BREAK;
  
  unsigned int _actualSession;
  Countdown _actualCountDown;
};


class Output
{
public:
  Output() {}
  
  void println(char * stringToPrint) {

    Serial.println(stringToPrint);
    Serial.println(_lastPrinted);
    if (!strcmp(stringToPrint, _lastPrinted))
      return;

    box.println(stringToPrint);


    strcpy(_lastPrinted, stringToPrint);

  }

  char _lastPrinted[6];
    
};


//////////////////////////////////////////////

void initDmd() {
  dmd.setBrightness(255);
  dmd.selectFont(Droid_Sans_12);
  dmd.begin();
}

Pomodoro * pomodoro;

unsigned long lastSecondsLeft;
Output output;



void setup() {
  Serial.begin (9600);
  initDmd();
  pinMode(speakerPin, OUTPUT);
  pomodoro = new Pomodoro();
}

void loop() {
    pomodoro->check();

    output.println(pomodoro->getCountdown().toString());

  delay(250);

}

