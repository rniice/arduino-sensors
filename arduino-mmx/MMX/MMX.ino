// TITLE:   MMX - FLEX SENSOR TO LED CONTROL
// LICENSE: Michael Crockett
// CREATOR: Michael Crockett  
// WEBSITE: michael-crockett.com


// GLOBALS

int FLEXPin = A0;               // Vref to measure from
int REDPin = 4;                 // RED pin of the LED to PWM pin 4
int GREENPin = 5;               // GREEN pin of the LED to PWM pin 5
int BLUEPin = 6;                // BLUE pin of the LED to PWM pin 6
int SAMPLERate = 100;           // sample rate in milliseconds
//int brightness = 255;         // LED brightness
//int increment = 25;           // brightness increment
int GestureCOUNT = 0;           // counts user input for changing mode or settings
int GestureTIMEOUT = 500;       // maximum milliseconds for a gesture to be counted
float GestureHOLD = 0;          // amount of time in milliseconds gesture hold closed (ended on mmx open)
bool GestureOPEN = true;        // mmx starts in the open position
int GestureAngleTHRESHOLD = 30; // angle in degrees which constitues gesture change

//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE

// SETUP
void setup()
{
  // initialize serial communications
  Serial.begin(9600); 

  //set pin modes
  pinMode(FLEXPin, INPUT);
  pinMode(REDPin, OUTPUT);
  pinMode(GREENPin, OUTPUT);
  pinMode(BLUEPin, OUTPUT);
}


// MAIN LOOP
void loop()
{
  int degrees = getCurvature();

  /* print out the result
  Serial.print("analog input: ");
  Serial.print(degrees, DEC);
  Serial.print("   degrees: ");
  Serial.println(degrees, DEC);
  */

  bool gesture_event = checkGesture(degrees);

  if (gesture_event) 
  {
    if ( (GestureCOUNT==2) && (GestureHOLD>=3.0) )
    {
      Serial.println("set to blink mode event detected");
      GestureHOLD = 0;                                      //reset gesture closed timer
    }
    else if ( (GestureCOUNT==2) && (GestureHOLD>=5.0) )
    {
      Serial.println("set brightness mode event detected");
      GestureHOLD = 0;                                      //reset gesture closed timer
    }
    else if (GestureCOUNT==3) //still not implemented
    {
      Serial.println("save configuration gesture event detected");
      GestureHOLD = 0;                                      //reset gesture closed timer
    }
    else if (GestureCOUNT==5)
    {
      Serial.println("on/off event detected");
      GestureHOLD = 0;                                      //reset gesture closed timer
    }
  }
  
  // pause before taking the next reading
  delay(SAMPLERate);                     
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


//FLEX SENSOR GESTURE DETECTION
//performs logical calculations to check if an event is triggered
bool checkGesture(int degrees)
{
  if(GestureOPEN)                           //if previous state is mmx open
  {
    if(degrees >= GestureAngleTHRESHOLD)    //mmx has been closed
    {
      setLED_STANDARD(0);                   //turn off the led
      GestureCOUNT++;
      GestureOPEN = false;

      return false;
    }
   
  }

  else                                      //if previoius state is mmx closed
  {
    if(degrees <= GestureAngleTHRESHOLD)    //mmx has been opened
    {
      setLED_STANDARD(255);                 //turn on the led
      GestureOPEN = true;
            
      return true;
    }
    else                                    //mmx is still closed after previously being closed
    {
      GestureHOLD += SAMPLERate;            //increment the counter for GestureHOLD
      Serial.print("holding closed for");
      Serial.print(GestureHOLD, DEC);
      Serial.println("milliseconds");
      
      return false;
    }
    
  }
  
}
  

void setLED_STANDARD(int brightness)
{
  //brightness = brightness + increment;  // increment brightness for next loop iteration

  //if (brightness <= 0 || brightness >= 255)    // reverse the direction of the fading
  //{
    //increment = -increment;
  //}
  brightness = constrain(brightness, 0, 255);
  setLED_COLOR(brightness, brightness, brightness);

  //delay(300);  // wait for 20 milliseconds to see the dimming effect

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


