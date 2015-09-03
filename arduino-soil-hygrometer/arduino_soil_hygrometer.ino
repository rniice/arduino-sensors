//SPECIFICATIONS:
//simple water sensor, can be used to detect soil moisture.
//Operating voltage:3.3V~5V
//Dual output: analog (more accurate), digital
//Power indicator (green) and digital switching output indicator (red)
//LM393 comparator chip, stable

//USAGE:
//ANALOG READ MODE: int 0 - 1023; //0 - highest moisture content; //1023 - lowest moisture content
//DIGITAL READ MODE: int 0 - 1023; //0 - highest moisture content; //1023 - lowest moisture content

//PREPROCESSOR: 
//Define the I/O Pins.
#define RESPONSE_ANALOG A0
#define RESPONSE_DIGITAL 5
#define LED 13
//Define measurement constants
#define analog_WET 2.3
#define analog_DRY 5
#define slope_analog 100.0f/(analog_DRY-analog_WET)
#define offset_analog 100.0f - slope_analog * analog_DRY
 
void setup() {
  // Initialize the serial communication
  Serial.begin(9600);
 
  // Set up pins
  pinMode(LED, OUTPUT);  
  pinMode(RESPONSE_ANALOG, INPUT);
  pinMode(RESPONSE_DIGITAL, INPUT);
}
 
// Uses an comparitor sensor to get the measured soil moisture in mV
int getMoisture() {
  //analog read mode:
  int value_reading;                                    //reading from rawADC
  float value_voltage;                                  //resolved to 5V line voltage
  int value_moisture;                                   //resolved to 0% to 100% wet
  
  digitalWrite(LED, HIGH);                              //indicate taking a measurement
  delay(5000);                                          //delay long enough to allow user to see measurement being taken
  value_reading = analogRead(RESPONSE_ANALOG);          //analog mode rawADC 0 - 1023
  value_voltage = float(value_reading)/1023.0f * 5.0f;  //calculate voltage drop across the thermistor using RawADC
  value_moisture = 100 - round(slope_analog * value_voltage + offset_analog);  //convert to 0 to 100% range 
  
  if (value_moisture > 100) {                           //ensure not to exceed bounds
    value_moisture = 100;
  }    
  else if (value_moisture < 0) {
    value_moisture = 0;
  }
  
  //digital read mode not completed:
  //value = digitalRead(RESPONSE_DIGITAL);              //digital mode read
  
  digitalWrite(LED, LOW);                               //indicate finishing taking a measurement
  
  //Serial.print("value_voltage is: ");
  //Serial.print(value_voltage);
  //Serial.println("V");  
  
  return value_moisture;
}
 
void loop() {
  // Take an hygrometer reading every 5 seconds.
  int moisture = getMoisture();  
  Serial.print(moisture);
  //Serial.println(" digial units value");
  Serial.println("% soil moisture level");  
  delay(5000);
}
