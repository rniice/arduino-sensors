// TITLE:   MMX - FLEX SENSOR TO LED CONTROL
// LICENSE: Use as you wish
// CREATOR: Michael Crockett  
// WEBSITE: michael-crockett.com


// GLOBALS

//settings: configuration of hardware 
int FLEXPin = A0;               // Vref to measure from
int REDPin = 4;                 // RED pin of the LED to PWM pin 4
int GREENPin = 5;               // GREEN pin of the LED to PWM pin 5
int BLUEPin = 6;                // BLUE pin of the LED to PWM pin 6

//settings and state variables: track the mode of the system
int mode = 0;                   //track the modes of the device 0: standard; 1: blink; 2: dimming_setting; 3: color_setting
int prevMode = 0;               //track the previous mode to switch back to after change

int degrees = 0;                // initialize degrees of mmx at 0 (full open)
bool gesture_event = false;     // initialize at false if there is currently an active gesture
int GestureCOUNT = 0;           // counts user input for changing mode or settings
int GestureDURATION = 0;        // counts the amount of time interpreting user gesture input
int GestureTIMEOUT = 5000;      // maximum milliseconds for a gesture to be counted
bool GestureOPEN = true;        // mmx starts in the open position
int GestureAngleTHRESHOLD = 45; // angle in degrees which constitues gesture change

//settings and state variables: track real time clock
const int SAMPLERate = 100;      // sample rate in milliseconds
unsigned long readMillis = 0;   //millis read from the internal clock
unsigned long currentMillis = 0;//initialize current amount of time in milliseconds as 0
unsigned long prevMillis = 0;   //initialize previous amount of time in milliseconds as 0

//settings and state variables: blink mode
int blinkDelay = 500;          //duration [in milliseconds] for on or off of led
bool blinkOn = false;           //track whether blink is on, or blink is off initialized to false
int blinkIntervalMillis = 0;    //track how much time has gone by since last blink change

//settings and state variables: brightness mode
int brightness = 120;           // LED brightness initialized to on
int brightnessTest = 120;       // start LED brightness set test at 255
int brightnessIncrement = 5;    // brightness value increment per cycle
int brightnessIntervalMillis = 0;//track how much time has gone by since last brightness change
int brightnessChangeDelay = 100;//duration [in milliseconds] before changing brightness

//settings and state variables: color mode
int color = 255;                // LED color initialized to white
int colorTest = 255;            // start LED color set test at 255
int colorIncrement = 5;         // color value increment per cycle
int colorIntervalMillis = 0;    // track how much time has gone by since last color change
int colorChangeDelay = 100;     // duration [in milliseconds] before changing color

//color options available (in percentages)
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };

//#define COMMON_ANODE                                        //uncomment this line if using a Common Anode LED

// SETUP
void setup() {
  Serial.begin(9600);                                         //initialize serial communications
  pinMode(FLEXPin, INPUT);                                    //set pin modes
  pinMode(REDPin, OUTPUT);
  pinMode(GREENPin, OUTPUT);
  pinMode(BLUEPin, OUTPUT);
}


// MAIN LOOP
void loop() {
  
  currentMillis = millis();                                   //elapsed time in milliseconds

  runMode();  

  gesture_event = checkGesture(degrees);                      //use the curvature to detect gesture

  //sample gesture only within SAMPLERate Interval
  if (currentMillis - prevMillis >= SAMPLERate){
    prevMillis = currentMillis;                               //save the last time interval was triggered
    
    degrees = getCurvature();                                 //check the current curvature

    if (gesture_event) {

      if (GestureCOUNT==2) {
        Serial.println("set to blink mode event detected");
        prevMode = mode;                                      //store the previous mode
        mode = 2;                                             //set to blink mode
      }
      else if (GestureCOUNT==3 ) {
        Serial.println("set brightness mode gesture event detected");
        prevMode = mode;                                      //store the previous mode        
        mode = 3;                                             //set to blink mode
      }
      else if (GestureCOUNT==4) {   //still not implemented  
        Serial.println("set color mode gesture event detected");
        prevMode = mode;                                      //store the previous mode        
        mode = 4;
      }
      else if (GestureCOUNT==5) {
        Serial.println("reset event detected");
        prevMode = mode;                                      //store the previous mode        
        mode = 5;
      }
      else {
        //Serial.println("standard mode detected");
        //GestureCOUNT = 0;                                     //rest gesture count 
        //GestureHOLD = 0;                                      //reset gesture closed timer
        //prevMode = mode;                                      //store the previous mode        
        //mode = 4;
      }
    }

  }
  
}


//RUN LOOP
void runMode(){

  //modes 0: standard; 1: blink; 2: dimming_setting; 3: color_setting; 4: reset to 0
  switch(mode) {
    case 0:                         //standard mode
      standardMode();
      Serial.println("triggering standardMode()");
      break;
    case 2:                         //set to blink mode
      blinkMode();
      Serial.println("triggering blinkMode()");
      break;
    case 3:                         //set brightness mode
      setBrightnessMode();
      Serial.println("triggering setBrightnessMode()");
      break;
    case 4:                         //set color mode
      setColorMode();
      Serial.println("triggering setColorMode()");
      break;
    case 5:
      Serial.println("triggering reset mode");
      mode = 0;
      break;
    }
    //if current mode is the same as the selected mode, switch back to standard mode
}


//FLEX SENSOR GESTURE DETECTION
bool checkGesture(int degrees) {

  if(GestureOPEN) {                          //if previous state is mmx open
    if(degrees >= GestureAngleTHRESHOLD){    //mmx has been closed
      GestureCOUNT++;
      GestureOPEN = false;
      return false;
    }
  }
  else {                                     //if previoius state is mmx closed
    if(degrees <= GestureAngleTHRESHOLD) {   //mmx has been opened
      GestureOPEN = true;     
      GestureDURATION += SAMPLERate;         //add to gestureDURATION clock to trigger start counting
      return true;
    }
    else {                                   //mmx is still closed after previously being closed
      return false;
    }
  }

  gestureTimer();                            //let gestureTimer take care of set/reset timer

}


void gestureTimer() {

  if (GestureDURATION > GestureTIMEOUT){
    GestureDURATION = 0;                     //reset back to zero
    Serial.println("resetting gesture event timer");
  }

  else if(GestureDURATION > 0){
    GestureDURATION += SAMPLERate;           //add to gestureDURATION clock
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

  if(GestureOPEN){

    blinkIntervalMillis += SAMPLERate;                   //increment up by SAMPLERATE for every time function called

    if (blinkIntervalMillis >= blinkDelay) {             //if exceeded threshold to switch blink, take action
      if (blinkOn) {
        setLED_STANDARD(0);
        blinkOn = false;                                 //sets the blinkOn to false for the next iteration
        blinkIntervalMillis = 0;                         //reset blinkIntervalMillis to restart counting again
      }

      else {
        setLED_STANDARD(brightness);
        blinkOn = true;                                  //sets the blinkOn to true for the next iteration
        blinkIntervalMillis = 0;                         //reset blinkIntervalMillis to restart counting again
      }
    }
  }

  else {                                                 //MMX is closed
    setLED_STANDARD(0);
  }

}


void setBrightnessMode() {
  brightnessIntervalMillis += SAMPLERate;              //increment up by SAMPLERATE for every time function called

  if (brightnessIntervalMillis >= brightnessChangeDelay) {
    setLED_STANDARD(brightnessTest);                   //start at highest brightness and increment down
    brightnessTest -= brightnessIncrement;             //reduce the brightnessTest value
  }

  else if (GestureOPEN == false) {                     //user has closed hand to confirm selection
    brightnessIntervalMillis = 0;                      //reset counter to 0;
    brightness = brightnessTest;                       //overwrite brightness setting
    mode = prevMode;                                   //go back to the previous mode set
  }
}


void setColorMode() {       //DO THIS VERY SIMILAR TO setBRIGHTNESS MODE
  colorIntervalMillis += SAMPLERate;              //increment up by SAMPLERATE for every time function called

  if (colorIntervalMillis >= colorChangeDelay) {
    setLED_STANDARD(colorTest);                   //start at highest brightness and increment down
    colorTest -= colorIncrement;             //reduce the brightnessTest value
  }

  else if (GestureOPEN == false) {                     //user has closed hand to confirm selection
    colorIntervalMillis = 0;                      //reset counter to 0;
    color = colorTest;                       //overwrite brightness setting
    mode = prevMode;                                   //go back to the previous mode set
  }

}


void setLED_STANDARD(int desired_brightness) { 
  desired_brightness = constrain(desired_brightness, 0, 255);
  setLED_COLOR(desired_brightness, desired_brightness, desired_brightness);

}


void setLED_COLOR(int red, int green, int blue) {
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
int getCurvature() {
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
