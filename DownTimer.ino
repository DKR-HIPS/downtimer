/*********************

DownTimer Version: 2022-02-08 v1.0.5
Arduino as a countdown timer with display on LCD, output relais to switch some external device.
Parts: DS3231 realtime clock with onboard EEPROM memory and 16x2 LC-display via I2C. One-button date/time setup.

**********************/

#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <DS3231.h>
#include <I2CEEPROM.h>

#define LCDADDR 0x27       // I2C-address for LCD module (e.g. joy-it brand)
#define CHIP_ADDRESS 0x57  // I2C-address for 24C32-type EEPROM chip on the DS3231 module

#define STARTBUTTON 7     // Pin 7 input for single-button operation (pushed = LOW)
#define SWITCHPIN 8       // Pin 8 as output for the switch relais (active = HIGH)
#define SOUNDPIN 5        // Pin 5 for output of audio signals (use 100R resistor with 8R speaker)
#define SETDISABLE 6      // Pin 6 prevents access to the date/time setup if LOW (e.g. via jumper)
#define MEMDISABLE 3      // Pin 3 disables using the I2C EEPROM memory if LOW (e.g. via jumper)

#define TCOUNTDOWN 240    // Timer start value (in minutes) to count down
#define TRESERVE 15       // Reserve time (default: 15 min) - if used, countdown time is minus this value on next day
#define WARNTIME 5        // Countdown timepoint to warn with sound, default: 5 min ahead
#define CHANCETIME 60     // Chance: if time (10 minutes or more) not used on 2 days, will add doubled to 3rd day (0 = disable)
#define ADDTIME 5         // Reward: add up to "this value x random(1,6) minutes" to chance time (default=5, 0 = disable)

#define NIGHT 22          // the hour when LCD backlight should automatically switch off, default: 22
#define MORNING 8         // the hour when LCD backlight is automatically switched on, default: 8
#define RESETHOUR 6       // the hour when the daily time contingent is reset, default: 6
#define DISPSPEED 5       // Interval (in seconds) for changing display content, default: 5

char devicename[] = "Network";     // Name of the controlled device, up to 7 characters
const char* switchlabel[]  = {"BLOCKED","OFF","ON"};   // Labels for 0, 1 and 2 switch states, e.g. BLOCKED, OFF and ON

LiquidCrystal_I2C lcd(LCDADDR, 16, 2);  // the 16x2 LCD module, called as "lcd"

I2CEEPROM i2c_eeprom(CHIP_ADDRESS);

DS3231 myRTC;   // the RTC module called as "myRTC"
bool Century=false;
bool h12;
bool PM;

bool displaylight = true;
bool lightswitch = true;
bool reservetime = false;
byte switchstate = 1;
unsigned int countdown = TCOUNTDOWN;
byte displaycounter = 2 * DISPSPEED;
byte actioncounter = 60;

void setup() {

  pinMode(STARTBUTTON, INPUT_PULLUP);
  pinMode(SETDISABLE, INPUT_PULLUP);
  pinMode(MEMDISABLE, INPUT_PULLUP);
  pinMode(SOUNDPIN, OUTPUT);
  pinMode(SWITCHPIN, OUTPUT);
  digitalWrite(SWITCHPIN, LOW);
  randomSeed(analogRead(0));
  
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Down-Timer");

  // check if the button is being held down during power-up for more than 2 seconds
  if (digitalRead(STARTBUTTON) == LOW && digitalRead(SETDISABLE) == HIGH) 
  { 
  lcd.setCursor(0,1);
  lcd.print("Set date/time ? ");
  unsigned int setdcount = 0;
   while(digitalRead(STARTBUTTON) == LOW)
    {
     setdcount++;
     delay(20);
     if(setdcount == 100)
       {
        lcd.setCursor(14,1);
        lcd.print(">>");
        makesound(0);
       }
    }
   if(setdcount > 100)
    {
     setd();   //call the function to set date/time in the RTC after button is released
    }
  }
  else if (digitalRead(STARTBUTTON) == LOW)
   {
    lcd.setCursor(0,1);
    lcd.print("Setup disabled!");
    delay(1000);
   }

  if (digitalRead(MEMDISABLE) == HIGH)  // check the state before power-off from EEPROM memory
  {
   byte readyear = i2c_eeprom.read(0);
   byte readmonth = i2c_eeprom.read(1);
   byte readday = i2c_eeprom.read(2);
   byte readhour = i2c_eeprom.read(3);
   byte readminute = i2c_eeprom.read(4);
   byte readswitch = i2c_eeprom.read(5);
   byte readreserve = i2c_eeprom.read(6);
   unsigned int readcounter = 256 * i2c_eeprom.read(7) + i2c_eeprom.read(8);
   if(readyear < 100 && readmonth < 13 && readday < 32 && readswitch < 3 && readhour < 24 && readminute < 60 && readreserve < 2)
   {
    int memtime = 60*readhour + readminute;
    int nowtime = 60*myRTC.getHour(h12,PM) + myRTC.getMinute();
    int restime = 60*RESETHOUR;
    unsigned long memdate = 10000*readyear + 100*readmonth + readday ;
    unsigned long nowdate = 10000*myRTC.getYear() + 100*myRTC.getMonth(Century) + myRTC.getDate() ;
    if(nowdate == memdate && nowtime < restime)
    {
     countdown = readcounter;
     switchstate = readswitch;
     reservetime = readreserve;
     actioncounter = 20;
    }
    else if(nowdate == memdate && memtime > restime && nowtime > restime)
    {
     countdown = readcounter;
     switchstate = readswitch;
     reservetime = readreserve;
     actioncounter = 20;
    }
    else if((nowdate-memdate) == 1 && nowtime < restime)
    {
     countdown = readcounter;
     switchstate = readswitch;
     reservetime = readreserve;
     actioncounter = 20;
    }
    else if(readreserve == true)
    {
     countdown = TCOUNTDOWN - TRESERVE;
    }
    if(switchstate == 2)
      { digitalWrite(SWITCHPIN, HIGH); }
   }
  }
  else
   {
    lcd.setCursor(0,1);
    lcd.print("EEPROM disabled!");
    delay(1000);
   }
  
  lcd.setCursor(0,1);
  lcd.print(String(devicename)+" "+((char)126)+String(countdown)+" min  ");
  delay(2000);

  if(myRTC.getHour(h12,PM) < MORNING || myRTC.getHour(h12,PM) > NIGHT)
    {
      lightswitch = false;
      displaylight = false;
      lcd.noBacklight();
      makesound(0);
    }
}

String thisdate; // formatted date variable
String thistime; // formatted time variable
String lasttime;
byte thisyear;
byte thismonth;
byte thisday;
byte thishour;
byte thisminute;
byte thissecond;

bool buttondown = false;
bool blinksymbol = false;
bool nowreset = false;
bool timechance = false;
byte timecounter = 1;
byte buttoncounter = 0;

void loop()
{
  
timecounter--;
if (timecounter < 1)
{
  timecounter = 10;
  thisyear = myRTC.getYear();
  thismonth = myRTC.getMonth(Century);
  thisday = myRTC.getDate();
  thishour = myRTC.getHour(h12,PM);
  thisminute = myRTC.getMinute();
  thissecond = myRTC.getSecond();
  thistime = nice(thishour)+":"+nice(thisminute)+":"+nice(thissecond);
}

if (digitalRead(STARTBUTTON) == LOW && buttondown == false)    // when button is pressed
  {
    buttondown = true;
  }
else if (digitalRead(STARTBUTTON) == LOW && buttondown == true)
  { 
    buttoncounter++;
    if (buttoncounter > 60)
    {
       if (displaylight == false)
        {
         makesound(0);
         displaylight = true;
         lcd.backlight();
        }
    lcd.setCursor(0,0);
    lcd.print("Button:         ");
    lcd.setCursor(0,1);
    if (switchstate == 0 && reservetime == true && countdown == 0)
        { lcd.print("No time credit!  "); }
    else if (switchstate == 1 && reservetime == false && countdown > 0)
        { lcd.print("Countdown starts"); }
    else if (switchstate == 2 && reservetime == false && countdown == 1)
        { lcd.print("Add reserve time"); }
    else if (switchstate == 0 && reservetime == false && countdown > 0)
        { lcd.print("Reserve time!   "); }
    else if (switchstate == 2 && reservetime == false && countdown > 1)
        { lcd.print("Countdown pause "); }
    else
        { lcd.print("No options.     "); }  
    }
  }
else if (digitalRead(STARTBUTTON) == HIGH && buttondown == true)
  { 
   buttondown = false;
   
   if (buttoncounter <= 60)
    {
     makesound(0);
     if (displaylight == true)
      {
       displaylight = false;
       lcd.noBacklight();
      }
    else
      {
       displaylight = true;
       lcd.backlight();
      }
     }
     
   if (buttoncounter > 60)
     {
      if (switchstate == 2 && reservetime == false && countdown > 1)
        {
         switchstate = 1;
         actioncounter = 60;
         countdown--;
         displaycounter = DISPSPEED;
         digitalWrite(SWITCHPIN, LOW);
         makesound(1);
        }
      else if (switchstate == 2 && reservetime == false && countdown == 1)
        {
         actioncounter = 60;
         countdown = TRESERVE + 1;
         reservetime = true;
         displaycounter = DISPSPEED;
         makesound(2);
        }
      else if (switchstate == 1 && countdown > 0)
        {
         switchstate = 2;
         actioncounter = 60;
         displaycounter = DISPSPEED;
         digitalWrite(SWITCHPIN, HIGH);
         makesound(2);  
        }
      else if (switchstate == 0 && reservetime == false)
        {
         switchstate = 2;
         reservetime = true;
         actioncounter = 60;
         displaycounter = DISPSPEED;
         digitalWrite(SWITCHPIN, HIGH);
         makesound(2); 
        }
      memwrite();
      }
      
  buttoncounter = 0;
  }

if (thistime != lasttime)
{  
  lasttime = thistime;
  thisdate = String(thisyear+2000)+"-"+nice(thismonth)+"-"+nice(thisday);
    
  displaycounter--;
  if (displaycounter > DISPSPEED && buttoncounter <= 60) 
   {
    lcd.setCursor(0,0);
    lcd.print("Date: "+thisdate);
    lcd.setCursor(0,1);
    lcd.print("Time: "+thistime+"  ");
   }
  else if (buttoncounter <= 60)
   {
    lcd.setCursor(0,0);
    lcd.print(String(devicename)+": "+String(switchlabel[switchstate])+"     ");
    lcd.setCursor(0,1);
    if (switchstate == 0)
       {
        lcd.print("Reset at "+String(RESETHOUR)+":00 h");
       }
    if (switchstate == 1)
       {
        lcd.print("Credit: "+String(countdown)+" min   ");
       }
     if (switchstate == 2)
        {
         lcd.print(String(countdown)+" min remain    ");
        }  
     }
  
  showblink(switchstate);
    
  if (displaycounter < 1) 
   {
    // it will rotate between status and date/time display using the given cycle time
    displaycounter = 2 * DISPSPEED ;

    // reset the blocked status at the specified hour, typically in the early morning
    if (thishour == RESETHOUR && nowreset == false )
    {
     nowreset = true;
     if (switchstate == 0)
      {
       switchstate = 1;
      }
      
     if (reservetime == true)
       {
        countdown = TCOUNTDOWN - TRESERVE;
        reservetime = false;
        timechance = false;
       }    
     else if (countdown >= CHANCETIME && timechance == true && CHANCETIME >= 10)
       {
        timechance = false;
        countdown = TCOUNTDOWN + (2 * CHANCETIME) + (ADDTIME * random(1,6));
       }
     else if (countdown >= CHANCETIME && timechance == false && CHANCETIME >= 10)
       { 
        timechance = true;
        countdown = TCOUNTDOWN;
       }
     else
       { 
        timechance = false;
        countdown = TCOUNTDOWN;
       }     
    }
    if (thishour != RESETHOUR && nowreset == true )
     {
     nowreset = false;
     }

    // deactivate LCD backlight automatically at nighttime and activate in the morning
    if (lightswitch == true && thishour == NIGHT && thisminute == 0 )
      {
       lightswitch = false;
       displaylight = false;
       lcd.noBacklight(); 
      }
    if (lightswitch == false && thishour == MORNING && thisminute == 0 )
      {
       lightswitch = true;
       displaylight = true;
       lcd.backlight(); 
      }
    }

    // countdown interval check
 if (switchstate == 2)
 {
  actioncounter--;
  if (actioncounter == 0)
    {
     actioncounter = 60;
     countdown--;
       if (countdown == WARNTIME || countdown == 1)
          { 
           makesound(3);
          }  
       if (countdown == (CHANCETIME+WARNTIME) && CHANCETIME >= 10)
          { 
           lcd.setCursor(0,1);
           lcd.print("Chance -"+String(CHANCETIME)+" min   ");
           makesound(3); 
          } 
          
       if (countdown == 0)
       {
        switchstate = 0;
        digitalWrite(SWITCHPIN, LOW);
        lcd.clear();
        lcd.print(String(devicename)+": "+String(switchlabel[switchstate])+"     ");
        lcd.setCursor(0,1);
        lcd.print("Countdown over! ");
        makesound(1);
        if (reservetime == false)
         {
          countdown = TRESERVE;
         }
        delay(1000);
       }     
     memwrite();
     }
  }
  
 }

// the basic loop delay is 20 milliseconds
delay(20);
}

// the function "nice" generates from int-number a string with leading zero
String nice(byte thisvalue)
{
  String nicevalue;
  if (thisvalue<10)
   {
    nicevalue = "0"+String(thisvalue);
   }
  else
   {
    nicevalue = String(thisvalue);
   }
  return nicevalue;
}

// prints the blinking star im the right-bottom corner
void showblink(byte checkstate)
{
 if (checkstate == 2 && buttoncounter <= 60)
  {
    if (blinksymbol == true)
        {
         lcd.setCursor(15,1);
         lcd.print("*");
         blinksymbol = false;
        }
     else
        {
         lcd.setCursor(15,1);
         lcd.print(" ");
         blinksymbol = true;
        }
  }
 return;
}

// outputs a pre-defined sound signal
void makesound(byte stype)
{
  if (stype == 0)
     { tone(SOUNDPIN, 1760, 30); }
  if (stype == 1)
     { 
      tone(SOUNDPIN, 880, 100);
      delay(100);
      tone(SOUNDPIN, 440, 150);
     }
  if (stype == 2)
     {
       tone(SOUNDPIN, 440, 100);
       delay(100);
       tone(SOUNDPIN, 880, 150);
     }
  if (stype == 3)
     {
       tone(SOUNDPIN, 1320, 150);
       delay(200);
       tone(SOUNDPIN, 1320, 100);
     }
  return;
}

// writes the current timer status to EEPROM memory
void memwrite()
{
 if (digitalRead(MEMDISABLE) == HIGH)
  {
   i2c_eeprom.write(0, thisyear);
   i2c_eeprom.write(1, thismonth);
   i2c_eeprom.write(2, thisday);
   i2c_eeprom.write(3, thishour);
   i2c_eeprom.write(4, thisminute);
   i2c_eeprom.write(5, switchstate);
   i2c_eeprom.write(6, reservetime);
   i2c_eeprom.write(7, highByte(countdown));
   i2c_eeprom.write(8, lowByte(countdown));
  }
 return;
}

// the function "setd" provides a simple one-button date/time setup routine
bool setd()
{
  int setval[] = { 0,0,21,1,1 };
  int maxval[] = { 23,59,50,12,31 };
  int minval[] = { 0,0,21,1,1 };
  int monval[] = { 0,31,29,31,30,31,30,31,31,30,31,30,31 };
  int rowval[] = { 0,0,1,1,1,0 };
  int posval[] = { 7,10,9,12,15,15 };
  int maxvalue;
  lcd.clear();
  lcd.print("Time: 00:00 h");
  lcd.setCursor(0,1);
  lcd.print("Date: 2021-01-01");
  lcd.blink();
  for(int setsel = 0; setsel < 5; setsel++ )
  {  
  bool wsetd = false;
  lcd.setCursor( posval[setsel],rowval[setsel] );
  while(wsetd == false)
     {
      if (digitalRead(STARTBUTTON) == LOW) 
        { 
         setval[setsel]++;
         if (setsel == 4)
           {
            maxvalue = monval[setval[3]];  // max. days setting depends on the month
            }
         else
           {
            maxvalue = maxval[setsel];
            }
         
         if (setval[setsel] > maxvalue) 
           { 
            setval[setsel] = minval[setsel]; 
           }
         lcd.setCursor( posval[setsel]-1,rowval[setsel] );
         lcd.print(nice( setval[setsel] ));
         lcd.setCursor( posval[setsel], rowval[setsel] );
         int nsetd = 0;
         while(digitalRead(STARTBUTTON) == LOW)
           {
           nsetd++;
           delay(10);
           if (nsetd == 150)    // if button hold down for more than 1.5 sec
             {
              wsetd = true; 
              lcd.setCursor( posval[setsel+1], rowval[setsel+1] );
             } 
           }
         }
       delay(20);
     }
  }
  lcd.noBlink();
  lcd.clear();
  lcd.print("Date/time set!");
  
  myRTC.setClockMode(false);
  myRTC.setYear(setval[2]);
  myRTC.setMonth(setval[3]);
  myRTC.setDate(setval[4]);
  myRTC.setDoW(1);
  myRTC.setHour(setval[0]);
  myRTC.setMinute(setval[1]);
  myRTC.setSecond(0);

  delay(500);
  return true;
}
