/*
  Countdown on a single DMD display
 */

#include <SPI.h>
#include <DMD2.h>
#include <fonts/Droid_Sans_12.h>
#include "DHT.h"
#define DHTTYPE DHT22 
#define DHTPIN 3


SoftDMD dmd(1,1);  // DMD controls the entire display
DMD_TextBox box(dmd, 3, 3, 32, 16);  // "box" provides a text box to automatically write to/scroll the display
int speakerPin = 2;
int dht22Pin = 3;


enum 
{
  DISPLAY_POMODORO,
  DISPLAY_TEMP,
};



class DhtSensor {
public:
  int temperature;
  int humidity;

  DhtSensor() {
    temperature = 0;
    humidity = 0;
    dht = new DHT(DHTPIN, DHTTYPE);
    dht->begin();


  }

  char * toString() {

    char screen[5];
    screen[4] = 0;

   this->humidity = dht->readHumidity();
   this->temperature = dht->readTemperature();

    sprintf(screen, "%02d%02d\%", this->temperature, this->humidity);

    Serial.println(screen);

    return screen;
  }

  void check() {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht->readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht->readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht->readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht->computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht->computeHeatIndex(t, h, false);

    // Serial.print("Humidity: ");
    // Serial.print(h);
    // Serial.print(" %\t");
    // Serial.print("Temperature: ");
    // Serial.print(t);
    // Serial.print(" *C ");
    // Serial.print(f);
    // Serial.print(" *F\t");
    // Serial.print("Heat index: ");
    // Serial.print(hic);
    // Serial.print(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");

  }

  DHT * dht;

};


class Countdown
{
public:
  Countdown(unsigned long totalMinutes = 0) {
    this->_startedAt = millis();
    this->_totalMilliseconds = totalMinutes * 60 * 1000;
    this->_finishAt = _startedAt + this->_totalMilliseconds;
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

  void reset() {
     _startedAt = millis();
    _finishAt = _startedAt + this->_totalMilliseconds;
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
  unsigned long _totalMilliseconds;
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

    // Serial.println(stringToPrint);
    // Serial.println(_lastPrinted);
    if (!strcmp(stringToPrint, _lastPrinted))
      return;

    box.println(stringToPrint);


    strcpy(_lastPrinted, stringToPrint);

  }

  char _lastPrinted[6];
    
};

class Pomodino {
public:
  Pomodino() {
    pomodoro = new Pomodoro();
    dhtSensor = new DhtSensor();

    checkTemperature = new Countdown(minutesToCheckTemperature);
    displayTemperature = new Countdown(minutesToDisplayTemperature);

    displayState = DISPLAY_TEMP;
  }

  void display() {
    switch (displayState) {
      case DISPLAY_POMODORO:
        output.println(pomodoro->getCountdown().toString());
        break;

      case DISPLAY_TEMP:
        dhtSensor->check();
        output.println(dhtSensor->toString());
        break;
    }
  }

  void check() {
    pomodoro->check();
    if (checkTemperature->isFinished()) {
      checkTemperature->reset();
      dhtSensor->check();   
      displayState = DISPLAY_TEMP;
      displayTemperature->reset();
    }

    if (displayState == DISPLAY_TEMP && displayTemperature->isFinished()) {
      displayState = DISPLAY_POMODORO;
    }

    this->display();

    delay(250);
  }

private:
  unsigned int displayState;
  unsigned int minutesToCheckTemperature = 3;
  unsigned int minutesToDisplayTemperature = 1;

  Countdown * checkTemperature;
  Countdown * displayTemperature;

  Pomodoro * pomodoro;
  DhtSensor * dhtSensor;
  Output output;

};


//////////////////////////////////////////////

void initDmd() {
  dmd.setBrightness(255);
  dmd.selectFont(Droid_Sans_12);
  dmd.begin();
}



unsigned long lastSecondsLeft;
Pomodino * pomodino;

DhtSensor * dhtSensor;


void setup() {
  Serial.begin (115200);
  initDmd();
  pinMode(speakerPin, OUTPUT);
  pomodino = new Pomodino();
}

void loop() {
  pomodino->check();
} 

