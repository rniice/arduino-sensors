// TITLE:   MMX - FLEX SENSOR TO LED CONTROL
// LICENSE: Michael Crockett
// CREATOR: Michael Crockett  
// WEBSITE: michael-crockett.com


// GLOBALS

int FLEXPin = A0;         //Vref to measure from
int REDPin = 4;           // RED pin of the LED to PWM pin 4
int GREENPin = 5;         // GREEN pin of the LED to PWM pin 5
int BLUEPin = 6;          // BLUE pin of the LED to PWM pin 6
int SAMPLERate = 100;     //sample rate in milliseconds
//int brightness = 255;   // LED brightness
//int increment = 25;     // brightness increment


// SETUP FOR DEBUGGING
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

  // print out the result
  Serial.print("analog input: ");
  Serial.print(degrees, DEC);
  Serial.print("   degrees: ");
  Serial.println(degrees, DEC);

  if(degrees <= 30)
  {
    setLED_STANDARD(255);
  }
  else{
    setLED_STANDARD(0);
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


void setLED_STANDARD(int brightness)
{
  //brightness = brightness + increment;  // increment brightness for next loop iteration

  //if (brightness <= 0 || brightness >= 255)    // reverse the direction of the fading
  //{
    //increment = -increment;
  //}
  brightness = constrain(brightness, 0, 255);
  analogWrite(REDPin, brightness);
  analogWrite(GREENPin, brightness);
  analogWrite(BLUEPin, brightness);

  delay(300);  // wait for 20 milliseconds to see the dimming effect


  
}



//SAMPLING FLEX SENSOR CODE




//LED OUTPUT CODE
