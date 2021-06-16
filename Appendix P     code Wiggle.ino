#include <AccelStepper.h>
#include <Arduino_JSON.h>
#include "OOCSI.h"

#define stepPin 2
#define directionPin 4
#define enablePin D5
#define endstopPinL D6
#define endstopPinR D7
#define intervenePin D0
#define ledPin D3

AccelStepper stepper(1, directionPin, stepPin);

#define DEBUG true

#define LOW_INTERVENTION_LEVEL 0
#define MEDIUM_INTERVENTION_LEVEL 1
#define HIGH_INTERVENTION_LEVEL 2

#define ATTEMPT_INTERVENTION_STATE 0
#define INVITE_INTERVENTION_STATE 1
#define PERFORM_INTERVENTION_STATE 2

#define MIN_PROGRESS_POSITION 0
#define MAX_PROGRESS_POSITION 800

#define ENDSTOP_ACTIVE 1

#define buttonActive 1

int interventionLevel = MEDIUM_INTERVENTION_LEVEL;
int interventionState = ATTEMPT_INTERVENTION_STATE;
bool isIntervening = false;

int progressPosition = 0;
int lastProgressPosition;

int interveneValue = 0;
int inviteValue = 0;

long wigglePerformMovement = 0;         //movement definer for how far the disk should movei in invite wiggle and perform wiggle
int wigglePerformAddMedium = 150;
int wigglePerformAddHigh = 300;
long wiggleInviteMovement = 0;
int wiggleInviteAdded = 150;
long wigglePerformDuration = 20000;

int endstopValueLeft = 0;
int endstopValueRight = 0;

int numberOfInvites = 0;
int numberOfWiggles = 0;


/////////////////////WIFI & OOCSI//////////////////////////////////

const char* ssid = "NAME";
// Password of your Wifi network.
const char* password = "PASSWORD";

// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "JordyWiggleCode";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "oocsi.id.tue.nl";
// channel name to send data to
const char* CHANNEL_NAME = "wigglekanaal";            //This channel must be a different channel then the receiving channel, which is set to be wigglechannel

// OOCSI reference for the entire sketch
OOCSI oocsi = OOCSI();

///////////////////////////////////////////////////////////////////



void setup() {
  setupSerial();
  setupProgressMotor();
  setupEndstops();
  setupIntervenButton();
  //setupWifi();
  //setupOocsi();
  delay(4000);
}

void loop() {

  handleSerialCommands();
  updateProgressPosition();

  //processOOCSI();
  //oocsiMessage();
  //oocsi.check();

  switch (interventionLevel) {
    case LOW_INTERVENTION_LEVEL:
      // not extra
      break;
    case MEDIUM_INTERVENTION_LEVEL:
    case HIGH_INTERVENTION_LEVEL:
      switch (interventionState) {
        case ATTEMPT_INTERVENTION_STATE:
          attemptWiggle();
          break;
        case INVITE_INTERVENTION_STATE:
          defineInviteWiggle();
          executeInviteWiggle();
          delay(10);
          watchInterveneButton();
          stopInviteWiggle();
          break;
        case PERFORM_INTERVENTION_STATE:
          definePerformWiggle();
          executeWiggle();
          resetProgressPosition();
          break;
      }
      break;
  }
}


void handleSerialCommands() {
  while (Serial.available() > 0) {
    int testCommand = Serial.read();
    debug("Test command was:");
    debug(String(testCommand));

    switch (testCommand) {
      case 48: // press '0'
        //setProgressPosition(0);
        setInterventionLevel(HIGH_INTERVENTION_LEVEL);
        setInterventionState(INVITE_INTERVENTION_STATE);
        break;
      case 49: // press '1'
        setProgressPosition(200);
        break;
      case 50: // press '2'
        setProgressPosition(400);

        break;
      case 51: // press '3'
        setProgressPosition(800);
        break;
      case 52:  //press '4'
        // attemptWiggle(2);
        attemptWiggle();
        debug("returned value of attemptWiggle was: ");
        debug(interventionState);
        break;
    }
  }
}



void instantMotorStop() {
  stepper.stop();
}

void updateProgressPosition() {
  // if (stepper.currentPosition() != stepper.targetPosition()) {
  stepper.run();
  // }
}




//void watchEndstops() {
//  endstopValueLeft = digitalRead(endstopPinL);
//  endstopValueRight = digitalRead(endstopPinR);
//  //debug("right is: ");
//  //debug(endstopValueLeft);
//  delay(100);
//}
//
//void executeEndstops()  {
//  digitalWrite(enablePin, LOW);
//  if (endstopValueLeft == ENDSTOP_ACTIVE)  {
//    instantMotorStop();
//    stepper.runToNewPosition(MIN_PROGRESS_POSITION);
//    setInterventionState(ATTEMPT_INTERVENTION_STATE);
//  }
//  if (endstopValueRight == ENDSTOP_ACTIVE)   {
//    instantMotorStop();
//    stepper.runToNewPosition(progressPosition);
//    setInterventionState(ATTEMPT_INTERVENTION_STATE);
//  }
//  digitalWrite(enablePin, HIGH);
//}



unsigned long previousAttempt = 0;
unsigned long attemptInterval = 3600000;                                        //equals one hour
int MEDIUM_RANDOMIZER = 12;
int HIGH_RANDOMIZER = 5;

void attemptWiggle() {
  if ((millis() - previousAttempt) > attemptInterval)    {
    previousAttempt = millis();
    int randomVal = 0;
    switch (interventionLevel) {
      case MEDIUM_INTERVENTION_LEVEL:
        randomVal = random(0, MEDIUM_RANDOMIZER);
        debug("the LOW output ");
        break;
      case HIGH_INTERVENTION_LEVEL:
        randomVal = random(0, HIGH_RANDOMIZER);
        debug("the HIGH output ");
        break;
    }
    if (randomVal == 1) {
      setInterventionState(INVITE_INTERVENTION_STATE);
      debug("the disk will start to Wiggle");
    }
    else {
      debug("the disk wont wiggle");
    }
    debug(randomVal);
  }
}




unsigned long startTimeInvite = 0;
long inviteDuration = 20000;                          //20 seconds 
bool inviting = false;

void defineInviteWiggle() {
  debug("I want to invite!!!");
  if (inviting == false)  {
    numberOfInvites = numberOfInvites + 1;
    digitalWrite(enablePin, LOW);
    inviting = true;
    startTimeInvite = millis();
    progressPosition = stepper.currentPosition();
    wiggleInviteMovement = progressPosition + wiggleInviteAdded;
    //    stepper.moveTo(wiggleInviteMovement);
    //    stepper.run();
  }
}

void executeInviteWiggle() {
  int breakoutInvite = true;
  while ((millis() - startTimeInvite) < inviteDuration && breakoutInvite == true) {
    interveneValue = digitalRead(intervenePin);
    if (interveneValue == HIGH) {
      delay(10);
      inviting = false;
      breakoutInvite = false;
      delay(10);
      break;
    }
    delay(10);
    if (stepper.currentPosition() == wiggleInviteMovement)  {
      stepper.moveTo(progressPosition);
      delay(10);
      debug("moves to last prog");
    }
    if (stepper.currentPosition() == progressPosition) {
      stepper.moveTo(wiggleInviteMovement);
      delay(10);
      debug("moves to wiggle invite pos");
    }
    delay(10);
    stepper.run();
  }
  digitalWrite(enablePin, HIGH);
}

void stopInviteWiggle() {
  if (inviting == true)  {
    inviting = false;
    resetProgressPosition();
    setInterventionState(ATTEMPT_INTERVENTION_STATE);
    digitalWrite(enablePin, HIGH);
  }
}

void watchInterveneButton() {
  interveneValue = digitalRead(intervenePin);
  if (interveneValue == buttonActive) {
    digitalWrite(ledPin, HIGH);                                                                       //Led will be turned off at the end of the perform state
    inviting = false;
    numberOfWiggles = numberOfWiggles + 1;
    setInterventionState(PERFORM_INTERVENTION_STATE);
  }
}




void definePerformWiggle() {
  switch (interventionLevel) {
    case MEDIUM_INTERVENTION_LEVEL:
      wigglePerformMovement = progressPosition + wigglePerformAddMedium;
      break;
    case HIGH_INTERVENTION_LEVEL:
      wigglePerformMovement = progressPosition + wigglePerformAddHigh;
  }
  delay(10);
  if(progressPosition >= 500)  {
    wigglePerformMovement = MAX_PROGRESS_POSITION - progressPosition;
  }
}

unsigned long startTimePerform;

void executeWiggle() {
  digitalWrite(enablePin, LOW);
  startTimePerform = millis();
  delay(100);
  stepper.moveTo(wigglePerformMovement);
  while ((millis() - startTimePerform) < wigglePerformDuration)  {
    delay(10);
    stepper.run();
    if (stepper.currentPosition() >= wigglePerformMovement) {
      stepper.moveTo(progressPosition);
      //stepper.move(-50);
      stepper.run();
    }
    if (stepper.currentPosition() <= progressPosition) {
      stepper.moveTo(wigglePerformMovement);
      //stepper.move(50);
      stepper.run();
    }
    //    watchEndstops();
    //    if (endstopValueLeft == HIGH || endstopValueRight == HIGH) {
    //      break;
    //    }
  }
  digitalWrite(ledPin, LOW);
  delay(10);
  digitalWrite(enablePin, HIGH);
  setInterventionState(ATTEMPT_INTERVENTION_STATE);

}

void resetProgressPosition()  {
  stepper.stop();
  debug("reset position activated");
  if (stepper.currentPosition() != progressPosition) {
    setProgressPosition(progressPosition);
    delay(20);
  }
  else {
    digitalWrite(enablePin, HIGH);
  }
}


void setInterventionLevel(int newLevel) {
  debug("Intervention level is requested to be set to: ");
  debug(newLevel);

  if (newLevel > HIGH_INTERVENTION_LEVEL) {
    debug("Invalid intervention level requested! ");
    debug(newLevel);
    return;
  }
  interventionLevel = newLevel;
}



void setInterventionState(int newState) {
  debug("Intervention state is requested to be set to: ");
  debug(newState);
  if (newState > PERFORM_INTERVENTION_STATE) {
    return;
  }
  interventionState = newState;
}



void setProgressPosition(int newPosition) {
  digitalWrite(enablePin, LOW);
  debug("Progress position is requested to be set to: ");
  debug(newPosition);

  if (newPosition < MIN_PROGRESS_POSITION) {
    newPosition = MIN_PROGRESS_POSITION;
  }

  if (newPosition > MAX_PROGRESS_POSITION) {
    newPosition = MAX_PROGRESS_POSITION;
  }

  progressPosition = newPosition;
  stepper.moveTo(progressPosition);
  delay(10);
  while (stepper.currentPosition() != progressPosition) {
    delay(20);
    Serial.println(stepper.currentPosition());
    stepper.run();
  }
  digitalWrite(enablePin, HIGH);
}

void debug(String message) {
  if (DEBUG == true) {
    Serial.println(message);
  }
}

void debug(int message) {
  if (DEBUG == true) {
    Serial.println(String(message));
  }
}

//------------------------------------------------------------//
//                          SETUP                             //
//------------------------------------------------------------//

void setupSerial() {
  Serial.begin(9600);
}

void setupProgressMotor() {
  stepper.setMaxSpeed(200);
  stepper.setSpeed(200);
  stepper.setAcceleration(50);
  //  stepper.setSpeed(100);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
}

void setupEndstops()  {
  pinMode(endstopPinL, INPUT);
  pinMode(endstopPinR, INPUT);
}



void setupIntervenButton() {
  pinMode(intervenePin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void setupWifi() {
  //is integrated in the oocsi setup
}

void setupOocsi() {
  oocsi.connect(OOCSIName, hostserver, ssid, password, processOOCSI);
  Serial.println("subscribing to wigglechannel");
  oocsi.subscribe("wigglechannel");

  Serial.print("is ");
  Serial.print(OOCSIName);
  Serial.print(" a client? --> ");
  Serial.println(oocsi.containsClient(OOCSIName));
  
}


int receivedProgression = 0;
int receivedInterventionLevel = 0;



void processOOCSI() {
  receivedProgression = (oocsi.getInt("progressPos", 0));                                     //change names to proper names
  delay(10);
  receivedInterventionLevel = (oocsi.getInt("levelInterv", -1));                             //change name to proper name
  delay(10);
  if (receivedProgression != 0 && receivedProgression != progressPosition) {
    setProgressPosition(receivedProgression);
  }
  if (receivedInterventionLevel != -1 && receivedInterventionLevel != interventionLevel) {
    setInterventionLevel(receivedInterventionLevel);
  }
}


unsigned long startSendMessage = 0;
unsigned long messageInterval = 3605000;               //the last number of this calculation is the number of hours

void oocsiMessage() {
  if ((millis() - startSendMessage) > messageInterval) {
    oocsi.newMessage(CHANNEL_NAME);
    oocsi.addString("device_id", "da1ee478b91ae415e");
    oocsi.addInt("", interventionLevel);
    oocsi.addInt("progressPosition", progressPosition);
    oocsi.addInt("invitations", numberOfInvites);
    oocsi.addInt("actualWiggles", numberOfWiggles);
    oocsi.addInt("interventionLevel", interventionLevel);
    oocsi.sendMessage();

    // prints out the raw message (how it is sent to the OOCSI server)
    oocsi.printSendMessage();
    startSendMessage = millis();
  }
}
