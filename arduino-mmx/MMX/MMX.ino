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

int GestureCOUNT = 0;           // counts user input for changing mode or settings
int GestureTIMEOUT = 500;       // maximum milliseconds for a gesture to be counted
float GestureHOLD = 0;          // amount of time in milliseconds gesture hold closed (ended on mmx open)
bool GestureOPEN = true;        // mmx starts in the open position
int GestureAngleTHRESHOLD = 30; // angle in degrees which constitues gesture change

//settings and state variables: track real time clock
const int SAMPLERate = 50;      // sample rate in milliseconds
unsigned long currentMillis = 0;//initialize current amount of time in milliseconds as 0
unsigned long prevMillis = 0;   //initialize previous amount of time in milliseconds as 0

//settings and state variables: blink mode
int blinkDelay = 500;          //duration [in milliseconds] for on or off of led
bool blinkOn = false;           //track whether blink is on, or blink is off initialized to false
int blinkIntervalMillis = 0;    //track how much time has gone by since last blink change

//settings and state variables: brightness mode
int brightness = 255;           // LED brightness initialized to on
int brightnessTest = 255;       // start LED brightness set test at 255
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

//vector colorOptions[6] = {black, white, red, green, blue, yellow}; 


//#define COMMON_ANODE                                     //uncomment this line if using a Common Anode LED

// SETUP
void setup() {
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
        prevMode = mode;                                      //store the previous mode
        mode = 1;                                             //set to blink mode
      }
      else if ( (GestureCOUNT==2) && (GestureHOLD>=5.0) ) {
        Serial.println("set brightness mode gesture event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        prevMode = mode;                                      //store the previous mode        
        mode = 2;                                             //set to blink mode
      }
      else if ( (GestureCOUNT==3) && (GestureHOLD>=5.0) ) {   //still not implemented
        Serial.println("set color mode gesture event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        prevMode = mode;                                      //store the previous mode        
        mode = 3;
      }
      else if (GestureCOUNT==5) {
        Serial.println("on/off event detected");
        GestureCOUNT = 0;                                     //rest gesture count 
        GestureHOLD = 0;                                      //reset gesture closed timer
        prevMode = mode;                                      //store the previous mode        
        mode = 4;
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

  runMode();  
}


//RUN LOOP
void runMode(){
  Serial.print("mode equals: ");
  Serial.println(mode, DEC);

  switch(mode) {
    case 0:                         //standard mode
      standardMode();
      Serial.println("runMode triggering standardMode()");
      break;
    case 1:                         //set to blink mode
      blinkMode();
      Serial.println("runMode triggering blinkMode()");
      break;
    case 2:                         //set brightness mode
      setBrightnessMode();
      Serial.println("runMode triggering setBrightnessMode()");
      break;
    case 3:                         //set color mode
      setColorMode();
      Serial.println("runMode triggering setColorMode()");
      break;
    case 4:
      Serial.println("runMode triggering on/off");
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
      Serial.println("MMX closed");
      return false;
    }
  }
  else {                                     //if previoius state is mmx closed
    if(degrees <= GestureAngleTHRESHOLD) {   //mmx has been opened
      GestureOPEN = true;     
      GestureHOLD = 0;                       //reset GestureHOLD since opened event occurred
      Serial.println("MMX opened");
      return true;
    }
    else {                                   //mmx is still closed after previously being closed
      GestureHOLD += SAMPLERate;             //increment the counter for GestureHOLD
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
      blinkIntervalMillis = 0;                           //reset blinkIntervalMillis to restart counting again
    }

    else {
      setLED_STANDARD(brightness);
      blinkOn = true;                                  //sets the blinkOn to true for the next iteration
      blinkIntervalMillis = 0;                           //reset blinkIntervalMillis to restart counting again
    }
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

 //modes 0: standard; 1: blink; 2: dimming_setting; 3: color_setting


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
