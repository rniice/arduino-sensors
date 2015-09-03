/*
  Relay Controller controlled with thermistor.
  Swithced fan controller with Red LED for fan off and Green LED for fan on.
  2/13/2015
*/
  
//GLOBAL VARIABLES                        
int fTemp = 0;                          // Attach 100 K Thermistor to analog pin 0.
int led = 7;                            // attach green led to pin 7.
int led2 = 12;                          // attach red led to pin 12.
int heater = 5;                         // attach base of transistor to digital pin 5.
int incomingByte = 0;                   // for incoming serial data
double fTempSet = 25.0;                 // user selected set temperature initialized at 25C.
//GLOBAL VARIABLES


void setup(void) {

  {
    Serial.begin(9600);                   // opens serial port, sets data rate to 9600 bps
    pinMode (led, OUTPUT);                // sets the led pin 7 up as an output.
    pinMode (led2, OUTPUT);               // sets the led pin 12 up as an output.
    pinMode (heater, OUTPUT);             // sets the heater relay on pin 5 up as an output.
    pinMode (fTemp, INPUT);               // data read from the temperature sensor (thermistor)
  }

}


double Thermistor(int RawADC) {

  //double R2 =
  // See http://en.wikipedia.org/wiki/Thermistor
  // See http://iwantmyreal.name/blog/2012/09/23/measuring-the-temperature-with-an-arduino-and-a-thermistor/
  // See http://playground.arduino.cc/ComponentLib/Thermistor2 last example for code used
  // R0 = 100 K-ohm
  // R_Balance = 10 K-ohm   //tie to ground using a 10k-Ohm
  // B = 3950
  // C = 50 M-ohm
  double Temp, R, V_thermistor, R0, R_Balance, B, C, T0, LogVal;
  
  R0 = 100000;
  R_Balance = 10000; 
  B = 3950;
  C = 50000000;
  T0 = 298;
  
  
  V_thermistor = float(RawADC)/1023.0f * 5.0f;  //calculate voltage drop across the thermistor using RawADC
   
  R = R_Balance * 5.0f/float(V_thermistor) - float(R_Balance); 
  LogVal = log(R/R0);
  Temp = 1.0f/(1.0f/T0+(1.0f/B)*LogVal+(1.0f/C)*LogVal*LogVal*LogVal);
  
  Temp = Temp - 273.15;           // Convert Kelvin to Celcius
  
  return Temp;
}

void alert(void) {
    digitalWrite (heater, LOW);        // turns off heater relay.
    Serial.println();  
    Serial.println("THERMISTOR READ PROBLEM"); // notify the user there is a problem
    digitalWrite (led2, LOW);          // turns off red led
    digitalWrite (led, HIGH);          // turns on the green led.
    delay(200); 
    digitalWrite (led2, HIGH);          // turns off red led
    digitalWrite (led, LOW);          // turns on the green led.
    delay(200); 
    digitalWrite (heater, LOW);        // turns off heater relay.  
    digitalWrite (led2, LOW);          // turns off red led
    digitalWrite (led, HIGH);          // turns on the green led.
    delay(200); 
    digitalWrite (led2, HIGH);          // turns off red led
    digitalWrite (led, LOW);          // turns on the green led.
    delay(200); 
    digitalWrite (led2, LOW);          // turns off red led
    digitalWrite (led, HIGH);          // turns on the green led.
}

void printTemp(void) {
  //double temp = Thermistor(analogRead(0));  // Read sensor analog voltage value
  //double temp = Thermistor(fTemp);  // Read sensor analog voltage value
  double temp = Thermistor(analogRead(A0));


  Serial.print("Temperature is: ");
  Serial.print(temp);
  Serial.println(" C");
  
  Serial.print("Set Point is: ");        //shows the setpoint or set temp
  Serial.print(fTempSet);                //prints out the current value of the user set temp
  Serial.println(" C");

    if (temp < 0)                        //if there is a problem reading temperature
    {
       alert();                          //call the alert function                        
    }
    else if (temp < fTempSet)            //if current temperature is below setpoint
    {
      digitalWrite (led2, HIGH);         // turns on red led
      digitalWrite (led, LOW);           // turns off the green led.
      digitalWrite (heater, HIGH);       // turns on heater relay.
      Serial.println("Heater is On ");   // notify the user that the heater is now on
      Serial.println();
      delay(1000);                       // wait at least 1 seconds before redoing the loop or delete.
    }
    else                                 // if the if equation evaluates to false the else statement will execute.
    {
      digitalWrite (led2, LOW);          // turns off red led
      digitalWrite (led, HIGH);          // turns on the green led.
      digitalWrite (heater, LOW);        // turns off heater relay.
      Serial.println("Heater is Off");   // notify the user that the heater is now off
      Serial.println();
      delay(1000);                       // wait at least 1 seconds before redoing the loop or delete.
    }   
}
 
void loop(void) {
  printTemp();                           //calculates the temperature and prints it to console
  
  Serial.println("Please enter new set temperature in Celsius."); 
  
  // send data only when you receive data:
  if (Serial.available()) {
    fTempSet = Serial.parseFloat();
  }
  else {
    Serial.println();
  }
  
  delay(1100);                           //chill out for 5 seconds for user to input new temperature
}
