    // Flex sensor test program
// Mike Grusin, SFE, 2011
// This program is free, use it however you wish!

// HARDWARE:
// Make the following connections between the Arduino and the flex sensor
// Note that the flex sensor pins are interchangeable

// Sensor pin - GND
// Sensor pin - Analog In 0, with 10K resistor to +5V

// INSTRUCTIONS:
// Upload this sketch to your Arduino, then activate the Serial Monitor
// (set the Serial Monitor to 9600 baud)

void setup()
{
  // initialize serial communications
  Serial.begin(9600); 
}

void loop()
{
  int sensor, degrees;
  
  // read the voltage from the voltage divider (sensor plus resistor)
  sensor = analogRead(A0);
  
  // convert the voltage reading to inches
  // the first two numbers are the sensor values for straight (768) and bent (853)
  // the second two numbers are the degree readings we'll map that to (0 to 90 degrees)
  degrees = map(sensor, 745, 853, 0, 90);
  // note that the above numbers are ideal, your sensor's values will vary
  // to improve the accuracy, run the program, note your sensor's analog values
  // when it's straight and bent, and insert those values into the above function.
  
  // print out the result
  Serial.print("analog input: ");
  Serial.print(sensor,DEC);
  Serial.print("   degrees: ");
  Serial.println(degrees,DEC);
  
  // pause before taking the next reading
  delay(100);                     
}
