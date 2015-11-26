// TITLE:   MMX - FLEX SENSOR TO LED CONTROL
// LICENSE: Use as you wish
// CREATOR: Michael Crockett  
// WEBSITE: michael-crockett.com

#include <SimpleTimer.h>

//////////////////////////////NEOPIXEL STUFF//////////////////////////////////////
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   2    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    6    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 8

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = HIGH;
int showType = 0;
//////////////////////////////NEOPIXEL STUFF//////////////////////////////////////



// GLOBALS

//settings: configuration of hardware 
int FLEXPin = A0;               // Vref to measure from

//settings and state variables: track the mode of the system
int mode = 0;                   //track the modes of the device 0: standard; 1: blink; 2: dimming_setting; 3: color_setting
int prevMode = 0;               //track the previous mode to switch back to after change

int flex_sensor;                // analog voltage from A-D on sensor
int degrees = 0;                // initialize degrees of mmx at 0 (full open)

bool gesture_event = false;     // initialize at false if there is currently an active gesture
int GestureCOUNT = 0;           // counts user input for changing mode or settings
int GestureTIMEOUT = 5000;      // maximum milliseconds for a gesture to be counted
bool GestureOPEN = true;        // mmx starts in the open position
int GestureAngleTHRESHOLD = 45; // angle in degrees which constitues gesture change

//settings and state variables: track real time clock
const int sensorSampleRate = 100;// sample rate in milliseconds
const int updateOutputRate = 200; // sample rate in milliseconds      

//settings and state variables: blink mode
int blinkDelay = 500;          //duration [in milliseconds] for on or off of led
bool blinkOn = false;           //track whether blink is on, or blink is off initialized to false

//settings and state variables: brightness mode
int brightness = 120;           // LED brightness initialized to on
int brightnessTest = 120;       // start LED brightness set test at 255
int brightnessIncrement = 5;    // brightness value increment per cycle
int brightnessChangeDelay = 200;//duration [in milliseconds] before changing brightness

//settings and state variables: color mode
int color = 255;                // LED color initialized to white
int colorTest = 255;            // start LED color set test at 255
int colorIncrement = 5;         // color value increment per cycle
int colorChangeDelay = 2000;    // duration [in milliseconds] before changing color


//TIMER intervale setup
SimpleTimer timer;

int updateSensorIntervalID = timer.setInterval(sensorSampleRate, getCurvature);
int updateGestureIntervalID = timer.setInterval(sensorSampleRate, checkGesture);
int updateOutputIntervalID = timer.setInterval(updateOutputRate, runMode);

int updateBlinkIntervalID = timer.setInterval(blinkDelay, blinkMode);
int updateBrightnessIntervalID = timer.setInterval(brightnessChangeDelay, setBrightnessMode);
int updateColorIntervalID = timer.setInterval(colorChangeDelay, setColorMode);


// SETUP
void setup() {
  Serial.begin(9600);                                         //initialize serial communications
  pinMode(FLEXPin, INPUT);                                    //set pin modes

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'
}


// MAIN LOOP
void loop() {
  
  timer.run();

  if (gesture_event) {
    changeProgram();
  }

  //LED CONTROL STUFF FROM BUTTON
  // Get current button state.
  bool newState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if (newState == LOW && oldState == HIGH) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
      showType++;
      if (showType > 9)
        showType=0;
      startShow(showType);
    }
  }

  // Set the last button state to the old state.
  oldState = newState;
  //LED CONTROL STUFF FROM BUTTON
  
}


//RUN LOOP
void runMode(){

  //modes 0: standard; 1: blink; 2: dimming_setting; 3: color_setting; 4: reset to 0
  switch(mode) {
    case 0:                         //standard mode
      disableTimers();
      standardMode();
      break;
    case 2:                         //set to blink mode
      disableTimers();
      timer.enable(updateBlinkIntervalID);
      break;
    case 3:                         //set brightness mode
      disableTimers();
      timer.enable(updateBrightnessIntervalID);
      break;
    case 4:                         //set color mode
      disableTimers();
      timer.enable(updateColorIntervalID);
      setColorMode();
      break;
    case 5:
      mode = 0;
      break;
    }
}


void disableTimers(){                   //disables all timers running when switching modes
  timer.disable(updateBlinkIntervalID);                       //disable blinkInterval
  timer.disable(updateBrightnessIntervalID);                  //disable brightnessInterval
  timer.disable(updateColorIntervalID);                       //disable colorInterval
}


//FLEX SENSOR GESTURE DETECTION
void checkGesture() {

  if(GestureOPEN) {                          //if previous state is mmx open
    if(degrees >= GestureAngleTHRESHOLD){    //mmx has been closed
      GestureCOUNT++;
      GestureOPEN = false;
      gesture_event = false;
    }
  }
  else {                                     //if previoius state is mmx closed
    if(degrees <= GestureAngleTHRESHOLD) {   //mmx has been opened
      GestureOPEN = true;     
      gesture_event = true;
      timer.setTimeout(GestureTIMEOUT, gestureTimer);
    }
    else {                                   //mmx is still closed after previously being closed
      gesture_event = false;
    }
  }

}


void changeProgram() {

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
    mode = 0;
  }

}


void gestureTimer() {
  GestureCOUNT = 0;
  gesture_event = false;
  Serial.println("resetting gesture event timer");
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

      if (blinkOn) {
        setLED_STANDARD(0);
        blinkOn = false;                                 //sets the blinkOn to false for the next iteration
      }

      else {
        setLED_STANDARD(brightness);
        blinkOn = true;                                  //sets the blinkOn to true for the next iteration
      }
  }
  else {                                                 //MMX is closed
    setLED_STANDARD(0);
  }

}


void setBrightnessMode() {
  
    setLED_STANDARD(brightnessTest);                   //start at highest brightness and increment down
    brightnessTest -= brightnessIncrement;             //reduce the brightnessTest value

  /*
  else if (GestureOPEN == false) {                     //user has closed hand to confirm selection
    brightnessIntervalMillis = 0;                      //reset counter to 0;
    brightness = brightnessTest;                       //overwrite brightness setting
    mode = prevMode;                                   //go back to the previous mode set
  }
  */
}


void setColorMode() {       //DO THIS VERY SIMILAR TO setBRIGHTNESS MODE
  
  setLED_STANDARD(colorTest);                   //start at highest brightness and increment down
  colorTest -= colorIncrement;             //reduce the brightnessTest value

  /*
  else if (GestureOPEN == false) {                     //user has closed hand to confirm selection
    colorIntervalMillis = 0;                      //reset counter to 0;
    color = colorTest;                       //overwrite brightness setting
    mode = prevMode;                                   //go back to the previous mode set
  }
  */
}


void setLED_STANDARD(int desired_brightness) { 
  desired_brightness = constrain(desired_brightness, 0, 255);
  setLED_COLOR(desired_brightness, desired_brightness, desired_brightness);

}


void setLED_COLOR(int red, int green, int blue) {

  colorWipe(strip.Color(red, green, blue), 50);

}


//FLEX SENSOR READ CODE
void getCurvature() {
  flex_sensor = analogRead(FLEXPin);  // read the voltage from the voltage divider (sensor plus resistor)
  // convert the voltage reading to inches
  // the first two numbers are the sensor values for straight (768) and bent (853)
  // the second two numbers are the degree readings we'll map that to (0 to 90 degrees)
  degrees = map(flex_sensor, 745, 853, 0, 90);
  // note that the above numbers are ideal, your sensor's values will vary
  // to improve the accuracy, run the program, note your sensor's analog values
  // when it's straight and bent, and insert those values into the above function.
}


//ADAFRUIT BUTTON CYCLER FUNCTIONS
void startShow(int i) {
  switch(i){
    case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
            break;
    case 1: colorWipe(strip.Color(255, 0, 0), 50);  // Red
            break;
    case 2: colorWipe(strip.Color(0, 255, 0), 50);  // Green
            break;
    case 3: colorWipe(strip.Color(0, 0, 255), 50);  // Blue
            break;
    case 4: theaterChase(strip.Color(127, 127, 127), 50); // White
            break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50); // Red
            break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 50); // Blue
            break;
    case 7: rainbow(20);
            break;
    case 8: rainbowCycle(20);
            break;
    case 9: theaterChaseRainbow(50);
            break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}






