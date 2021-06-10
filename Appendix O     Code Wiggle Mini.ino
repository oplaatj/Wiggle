//#include <DS1307RTC.h>

//V1 focusing on : 1) unaware progress measurement 2) goal-setting/lifestyle 3)positive inspired progress measurement 4) unawareness/informed 5) subtle interactions
// Therefore the product 1)inform product when sported  4)look like connected to fitbit (provide self as you can tell the participants anything 3) move forward to indicate progress 4) 
// Is 10 to 14 dagen voldoende om meerdere variabelen te leren kennen als gebruiker?
// product gaat elke x op hetzelfde tijdstip vooruit, dit is in het begin grof en wordt langzamer subtiel
//Wiggles duren 30 seconden, elke 2 uur wordt er gerandomized of er een wiggle uit gestuurd wordt (dit moet gelogd worden)

#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include <Wire.h>     //RTC
#include <ds3231.h>   //RTC download file




//Button
int buttonPin = 2;
int buttonState = LOW;

unsigned long progInter = 172800000;       //   Interval for moving the disk up to the slope which is set at 2 days = 172.800.000
unsigned long prevTimeProg = 0;          //value to log the latest activity
unsigned long progTime; 
unsigned long wigInter = 3600000;       //Interval for Wiggling randomizer which is set at 3600000 which is 1h combine w/ randomizer 11 (change of 2 times a day roughly)
unsigned long prevTimeWig =0;                         // 86.400.000 is the duration of 1 day in millis
unsigned long wigTime;
unsigned long durWig;
unsigned long durWig2;
unsigned long durProg;

int randNumber = 0;

float spinPeriod = 1200;

Servo myservo;

ts t; //ts is a struct findable in ds3231.

void setup() {
  Serial.begin(9600);
  Wire.begin(); //start i2c (required for connection)
  DS3231_init(DS3231_INTCN); //register the ds3231 (DS3231_INTCN is the default address of ds3231, this is set by macro for no performance loss)
  

  myservo.attach(6);
  myservo.write(90);    //Servo not moving

  pinMode(buttonPin, INPUT);

  progTime = millis();
  wigTime = millis();

  t.hour=20; 
  t.min=00;
  t.sec=0;
  t.mday=10;
  t.mon=04;
  t.year=2021;
  DS3231_set(t);
  
 while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized."); 

  myservo.write(120);
  delay(300);
  myservo.write(90);

}

void loop() { 
  buttonState = digitalRead(buttonPin);

  if(buttonState == HIGH) {
    myservo.write(110);             //testline
    Serial.println("button high");
    DS3231_get(&t);
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile)  {
    dataFile.print(t.mday);
    dataFile.print(".");
    dataFile.print(t.mon);
    dataFile.print(".");
    dataFile.print(t.year);
    dataFile.print(" ; ");
    dataFile.print(t.hour);
    dataFile.print(":");
    dataFile.print(t.min);
    dataFile.print("  ;  ");
    dataFile.print(1);
    dataFile.println("");

    #ifdef CONFIG_UNIXTIME
    #endif
    dataFile.close();
    }
  }
  myservo.write(90);

  if((millis() - prevTimeWig) > wigInter) {                                       //if we have reached an interval of 2 days, disk will move upwards (positive intended), but every time, the movement duration will decrease ( * 0,9)
    Serial.println("in the wiggle loop");
    randomSeed(analogRead(A0));
    delay(100);
      randNumber = random(0,11);                                                  //theoretically the disk has the change to wiggle every 10 hours
    delay(100);
    Serial.println(randNumber); 
    if(randNumber == 1) {                                                              //mapping all data to sd card
         DS3231_get(&t);
      File wiggleFile = SD.open("wiggle.txt", FILE_WRITE);
      if(wiggleFile)  {
      wiggleFile.print(t.mday);
      wiggleFile.print(".");
      wiggleFile.print(t.mon);
      wiggleFile.print(".");
      wiggleFile.print(t.year);
      wiggleFile.print(" ; ");
      wiggleFile.print(t.hour);
      wiggleFile.print(":");
      wiggleFile.print(t.min);
      wiggleFile.print(" ; ");
      wiggleFile.print(1);
      wiggleFile.println(""); 

      wiggleFile.close();
      }  
      
      for(int i = 0; i < 3; i++) {                                                     //To create the wiggling behaviour, the disk will move up and down 3 times
      Serial.println("start wiggles");
      durWig = millis();                                                            //get the latest timestamp before entering the 5 seconds of climbing the slope
    
      while((millis()- durWig) < 1000)  {
        myservo.write(130); 
        delay(100);       
    }
      durWig2 = millis();
      while((millis()- durWig2) < 1000)  {
        myservo.write(50);
        delay(100);        
    }
    myservo.write(90);
    
    Serial.println("end wiggles");
    }  
  }
   prevTimeWig = millis();
   delay(100);                                                                               //duration of 5 seconds
    
  }

  if ((millis() - prevTimeProg) > progInter)   {                                     //reached time interval of 1 day, do a randomizer to see if a wiggle should be executed, If so, log all values  
    Serial.println("start progress");
    durProg = millis();
    delay(100);
    while((millis()- durProg) < spinPeriod)  {
      myservo.write(110);        
    }
    myservo.write(90);
    spinPeriod = spinPeriod*0.78;                                                    //The duration of pulling up the disk against the slope will decrease every time a spin has been executed
    Serial.print("speed: ");
    Serial.println(spinPeriod);
    DS3231_get(&t);
    File progressFile = SD.open("progress.txt", FILE_WRITE);
    if (progressFile)  {
    progressFile.print(t.mday);
    progressFile.print(".");
    progressFile.print(t.mon);
    progressFile.print(".");
    progressFile.print(t.year);
    progressFile.print(" ; ");
    progressFile.print(t.hour);
    progressFile.print(":");
    progressFile.print(t.min);
    progressFile.print("  ;  ");
    progressFile.print(spinPeriod);
    progressFile.println("");

    progressFile.close();
    } 
    prevTimeProg = millis();
  }





  

  
   

}
