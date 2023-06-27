#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <IRremote.h>
#include <RTClib.h>
#include <Wire.h>
RTC_DS1307 RTC;
IRrecv IR(3);


#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 8
#define SPEED_TIME 75
#define PAUSE_TIME  0

#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

#define mills  60000;

// Clock


MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
char D1Text[20] = "";
char D2Text[20] = "";



void resetDisplay(){
  P.displayReset(0);
  P.displayReset(1);
}

//-----------------RTC CLOCK------------

void getTime(DateTime now) {
  uint8_t h = now.hour() > 12 ? now.hour()-12 : now.hour();
  sprintf(D1Text, "%d:%02d", h, now.minute());
  sprintf(D2Text, "%02d", now.second());
}


//-----------------RTC CLOCK-------------


void setup(void) {
  IR.enableIRIn();
  Serial.begin(9600);

  Wire.begin();
  RTC.begin(); 

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");// This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    delay(1000);
  }

  P.begin(2);
  P.setInvert(false);
  P.setZone(0, 0, 3);
  P.setZone(1, 4, 7);

  P.displayZoneText(0, D1Text, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(1, D2Text, PA_CENTER, SPEED_TIME, 1000, PA_PRINT, PA_NO_EFFECT);
}


//---------------countdown -- timer---------------

//timer variables
unsigned long countdownStartTime = 0;
unsigned long countdownDuration = 0;

//start
void startCountdown() {
  countdownStartTime = millis();
}



//setTime
int h = 0, m = 0;

bool next = false;
void setTime() {
  countdownDuration = ((h*60) + m) * mills;
  sprintf(D1Text, "%d:%02d", h, m);
  next ? sprintf(D2Text, "MIN") : sprintf(D2Text, "HR");
}


// timer
void countDown() {
  if(countdownStartTime != 0) {
    unsigned long currentTime = millis();
    if(currentTime - countdownStartTime >= countdownDuration) {
      countdownStartTime = 0;
      strcpy(D1Text, "Time");
      strcpy(D2Text, "UP!");
    } else {
      unsigned long remainingTime = countdownDuration - (currentTime - countdownStartTime);
      int hours = remainingTime / 3600000;
      int minutes = (remainingTime % 3600000) / 60000;
      int seconds = (remainingTime % 60000) / 1000;
      sprintf(D1Text, "%d:%02d", hours, minutes);
      sprintf(D2Text, "%02d", seconds);
    }
  } 
}


//-----------------countdown --  timer-------------------

int scene = 0;
void RemoteController() {
  if (IR.decode()) {
    long long key = IR.decodedIRData.decodedRawData;
    Serial.println(IR.decodedIRData.decodedRawData, HEX);

    switch (key) {
      case 0x57A8FF00: // play
       Serial.println("Play");
       if(countdownDuration == 0) {
        scene = 1;
       } else {
        Serial.println(countdownDuration);
        startCountdown();
        scene = 2;
       }
       
       break;

      case 0x3DC2FF00: // reset
        Serial.println("Reset");
        countdownDuration = 0;
        countdownStartTime = 0;
        h=0;
        m=0;
        scene = 0;
        break;

      case 0x6F90FF00:
        Serial.println("NEXT");
        next = true;
        break;

      case 0x1FE0FF00:
        Serial.println("PREV");
        next = false;
        break;

      case 0xFD02FF00:
        Serial.println("+");
        next ? m++ : h++;
        setTime();
        break;

      case 0x6798FF00:
        Serial.println("-");
        if(next && m) m--;
        if(!next && h) h--;
        setTime();
        break;
      
      case 0x9768FF00: //zero
        // resetDisplay();
        scene = 0;
        break;
      
      case 0xCF30FF00: //one
        // resetDisplay();
        scene = 1;
        break;

      case 0xE718FF00: //two
        // resetDisplay();
        scene = 2;
        break;

      // case 0x857AFF00: //three
      //   scene = 3;
      //   break;

      // case 0x4FB0FF00: //cancel
        
      //   break;

      default:
        Serial.println("Something messy!");
    }

    delay(100);
    IR.resume();
  }
}


void loop(void)
{
  DateTime now = RTC.now();
  P.displayAnimate();
  RemoteController();
  resetDisplay();

  switch(scene) {
    case 0:
      getTime(now);
      break;

    case 1:
      setTime();
      break;

    case 2:
      countDown();
      break;

    default:
    Serial.println("Nothing!");
    getTime(now);
  }
  
}
