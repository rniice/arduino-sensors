// TITLE:   MMX - FLEX SENSOR TO LED CONTROL
// LICENSE: Use as you wish
// CREATOR: Michael Crockett  
// WEBSITE: michael-crockett.com


// GLOBALS

int FLEXPin = A0;               // Vref to measure from
int REDPin = 4;                 // RED pin of the LED to PWM pin 4
int GREENPin = 5;               // GREEN pin of the LED to PWM pin 5
int BLUEPin = 6;                // BLUE pin of the LED to PWM pin 6
int brightness = 255;           // LED brightness initialized to off
//int increment = 25;           // brightness increment
int GestureCOUNT = 0;           // counts user input for changing mode or settings
int GestureTIMEOUT = 500;       // maximum milliseconds for a gesture to be counted
float GestureHOLD = 0;          // amount of time in milliseconds gesture hold closed (ended on mmx open)
bool GestureOPEN = true;        // mmx starts in the open position
int GestureAngleTHRESHOLD = 30; // angle in degrees which constitues gesture change

const int SAMPLERate = 50;      // sample rate in milliseconds

unsigned long currentMillis = 0;//initialize current amount of time in milliseconds as 0
unsigned long prevMillis = 0;   //initialize previous amount of time in milliseconds as 0

int blinkDelay = 1000;          //duration [in milliseconds] for on or off of led
bool blinkOn = false;           //track whether blink is on, or blink is off initialized to false
int blinkIntervalMillis = 0;    //track how much time has gone by since last blink change

int mode = 0;                   //track the modes of the device 0: standard; 1: blink; 2: dimming_setting; 3: color_setting

//#define COMMON_ANODE                                     //uncomment this line if using a Common Anode LED

// SETUP
void setup()
{
  Serial.begin(9600);                                       //initialize serial communications
  pinMode(FLEXPin, INPUT);                                  //set pin modes
  pinMode(REDPin, OUTPUT);
  pinMode(GREENPin, OUTPUT);
  pinMode(BLUEPin, OUTPUT);
}


// MAIN LOOP
void loop() {
  unsigned long currentMillis = millis();                   //elapsed time in milliseconds

  if (currentMillis - prevMillis >= SAMPLERate){
    prevMillis = currentMillis;                         // save the last time interval was triggered
    
    int degrees = getCurvature();                             //check the current curvature

    //Serial.print("analog input: ");                         //print out the result for debugging
    //Serial.print(degrees, DEC);
    //Serial.print("   degrees: ");
    //Serial.println(degrees, DEC);
 
    bool gesture_event = checkGesture(degrees);               //use the curvature to detect gesture

    if (gesture_event) {
      if ( (GestureCOUNT==2) && (GestureHOLD>=3.0) ) {
        Serial.println("set to blink mode event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        mode = 1;                                             //set to blink mode
      }
      else if ( (GestureCOUNT==2) && (GestureHOLD>=5.0) ) {
        Serial.println("set brightness mode event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        mode = 2;                                             //set to blink mode
      }
      else if (GestureCOUNT==3) {                             //still not implemented
        Serial.println("save configuration gesture event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        mode = 3;
      }
      else if (GestureCOUNT==5) {
        Serial.println("on/off event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        mode = 4;
      }
    }

    else {
      runMode();        //keep running  the currently set mode       
    }

  }
}


//RUN LOOP
void runMode(){
  switch(mode) {
    case 0: //standard mode
      standardMode();
      break;
    case 1:
      blinkMode();
      break;
    case 2:
      //do stuff
      break;
    }
  
    //if current mode is the same as the selected mode, switch back to standard mode
}


//FLEX SENSOR GESTURE DETECTION
bool checkGesture(int degrees)
{
  if(GestureOPEN){                          //if previous state is mmx open
    if(degrees >= GestureAngleTHRESHOLD){   //mmx has been closed
      GestureCOUNT++;
      GestureOPEN = false;
      return false;
    }
  }
  else {                                    //if previoius state is mmx closed
    if(degrees <= GestureAngleTHRESHOLD){   //mmx has been opened
      GestureOPEN = true;     
      return true;
    }
    else{                                   //mmx is still closed after previously being closed
      GestureHOLD += SAMPLERate;            //increment the counter for GestureHOLD
      Serial.print("holding closed for ");
      Serial.print(GestureHOLD, DEC);
      Serial.println(" milliseconds");
      return false;
    }
  } 
}


void standardMode(){
  if(GestureOPEN){
    setLED_STANDARD(brightness);
  }
  else {
    setLED_STANDARD(0);
  }
}


void blinkMode(){

  blinkIntervalMillis += SAMPLERate;                   //increment up by SAMPLERATE for every time function called

  if (blinkIntervalMillis >= blinkDelay) {             //if exceeded threshold to switch blink, take action
    if (blinkOn) {
      setLED_STANDARD(0);
      blinkOn = false;                                 //sets the blinkOn to false for the next iteration
    }

    else {
      setLED_STANDARD(brightness);
      blinkOn = true;                                  //sets the blinkOn to true for the next iteration
    }

    blinkIntervalMillis = 0;                           //reset blinkIntervalMillis to restart counting again

  }

}


void setLED_STANDARD(int brightness)
{
  brightness = constrain(brightness, 0, 255);
  setLED_COLOR(brightness, brightness, brightness);
}


void setLED_COLOR(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(REDPin, red);
  analogWrite(GREENPin, green);
  analogWrite(BLUEPin, blue);
}


//FLEX SENSOR READ CODE
int getCurvature()
{
  int flex_sensor, degrees;
  
  flex_sensor = analogRead(FLEXPin);  // read the voltage from the voltage divider (sensor plus resistor)
  
  // convert the voltage reading to inches
  // the first two numbers are the sensor values for straight (768) and bent (853)
  // the second two numbers are the degree readings we'll map that to (0 to 90 degrees)
  degrees = map(flex_sensor, 745, 853, 0, 90);
  // note that the above numbers are ideal, your sensor's values will vary
  // to improve the accuracy, run the program, note your sensor's analog values
  // when it's straight and bent, and insert those values into the above function.
  
  return degrees;
}
