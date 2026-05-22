// Include standard Arduino library
#include "Arduino.h"

// --- Standalone Library Implementations ---

// 1. Minimal DS18B20 / 1-Wire Implementation
class DS18B20 {
  private:
    uint8_t pin;
    void reset() {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      delayMicroseconds(480);
      pinMode(pin, INPUT);
      delayMicroseconds(480);
    }
    void writeBit(uint8_t b) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      delayMicroseconds(b ? 2 : 60);
      pinMode(pin, INPUT);
      delayMicroseconds(b ? 60 : 2);
    }
    void writeByte(uint8_t data) {
      for (int i = 0; i < 8; i++) {
        writeBit(data & 1);
        data >>= 1;
      }
    }
    uint8_t readBit() {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      delayMicroseconds(2);
      pinMode(pin, INPUT);
      delayMicroseconds(10);
      uint8_t b = digitalRead(pin);
      delayMicroseconds(50);
      return b;
    }
    uint8_t readByte() {
      uint8_t data = 0;
      for (int i = 0; i < 8; i++) {
        data |= (readBit() << i);
      }
      return data;
    }
  public:
    DS18B20(uint8_t p) : pin(p) {}
    float readTempC() {
      reset();
      writeByte(0xCC); // Skip ROM
      writeByte(0x44); // Convert T
      delay(750);      // Wait for conversion

      reset();
      writeByte(0xCC); // Skip ROM
      writeByte(0xBE); // Read Scratchpad
      
      uint8_t lsb = readByte();
      uint8_t msb = readByte();
      reset();         // abort reading the rest of scratchpad
      
      int16_t raw = (msb << 8) | lsb;
      return (float)raw / 16.0;
    }
};

// 2. Minimal Ultrasonic (HC-SR04) Implementation
class NewPing {
  private:
    uint8_t trig;
    uint8_t echo;
  public:
    NewPing(uint8_t trigPin, uint8_t echoPin) : trig(trigPin), echo(echoPin) {
      pinMode(trig, OUTPUT);
      pinMode(echo, INPUT);
    }
    int ping_cm() {
      digitalWrite(trig, LOW);
      delayMicroseconds(2);
      digitalWrite(trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig, LOW);
      
      // 30ms timeout is roughly equal to ~5 meters
      long duration = pulseIn(echo, HIGH, 30000); 
      return duration * 0.034 / 2;
    }
};

// 3. Minimal 10k NTC Thermistor Implementation
class Thermistor {
  private:
    uint8_t pin;
  public:
    Thermistor(uint8_t analogPin) : pin(analogPin) {}
    float getTempC() {
      int raw = analogRead(pin);
      if (raw == 0) return -273.15; // Prevent division by zero
      
      // Beta equation parameters (assumes standard 10k NTC thermistor and 10k series resistor)
      float r = 10000.0 * ((1023.0 / (float)raw) - 1.0);
      float tKelvin = 1.0 / (log(r / 10000.0) / 3950.0 + 1.0 / 298.15);
      return tKelvin - 273.15;
    }
};

// --- End of Standalone Libraries ---


// Pin Definitions
#define DS18B20WP_PIN_DQ 2
#define HCSR04_PIN_TRIG  4
#define HCSR04_PIN_ECHO  3
#define THERMISTOR_PIN_CON1 A3

// Object initialization
DS18B20 ds18b20wp(DS18B20WP_PIN_DQ);
NewPing hcsr04(HCSR04_PIN_TRIG, HCSR04_PIN_ECHO);
Thermistor thermistor(THERMISTOR_PIN_CON1);

// Global variables
const unsigned long timeout = 10000; // Define timeout of 10 sec
char menuOption = 0;
unsigned long time0;

// Setup the essentials for your circuit to work.
void setup() 
{
    // Setup Serial which is useful for debugging
    Serial.begin(9600);
    while (!Serial); // wait for serial port to connect. Needed for native USB
    Serial.println(F("start"));
    
    menuOption = menu();
}

// Main logic of your circuit.
void loop() 
{
    if(menuOption == '1') {
        // DS18B20 1-Wire Temperature Sensor - Waterproof - Test Code
        float ds18b20wpTempC = ds18b20wp.readTempC();
        Serial.print(F("Temp: ")); Serial.print(ds18b20wpTempC); Serial.println(F(" [C]"));
    }
    else if(menuOption == '2') {
        // Ultrasonic Sensor - HC-SR04 - Test Code
        int hcsr04Dist = hcsr04.ping_cm();
        delay(10);
        Serial.print(F("Distance: ")); Serial.print(hcsr04Dist); Serial.println(F(" [cm]"));
    }
    else if(menuOption == '3') {
        // NTC Thermistor 10k - Test Code
        float thermistorTempC = thermistor.getTempC();
        Serial.print(F("Temp: ")); Serial.print(thermistorTempC); Serial.println(F(" [°C]"));
        delay(500); // Added slight delay to make thermistor reading readable in serial monitor
    }
    
    // Check if 10 seconds have passed
    if (millis() - time0 > timeout)
    {
        menuOption = menu();
    }
}

// Menu function for selecting the components to be tested
char menu()
{
    Serial.println(F("\nWhich component would you like to test?"));
    Serial.println(F("(1) DS18B20 1-Wire Temperature Sensor - Waterproof"));
    Serial.println(F("(2) Ultrasonic Sensor - HC-SR04"));
    Serial.println(F("(3) NTC Thermistor 10k"));
    Serial.println(F("(menu) send anything else or press on board reset button\n"));
    
    while (!Serial.available());

    // Read data from serial monitor if received
    while (Serial.available()) 
    {
        char c = Serial.read();
        if (isAlphaNumeric(c)) 
        {   
            if(c == '1') 
                Serial.println(F("Now Testing DS18B20 1-Wire Temperature Sensor - Waterproof"));
            else if(c == '2') 
                Serial.println(F("Now Testing Ultrasonic Sensor - HC-SR04"));
            else if(c == '3') 
                Serial.println(F("Now Testing NTC Thermistor 10k"));
            else
            {
                Serial.println(F("illegal input!"));
                return 0;
            }
            time0 = millis();
            return c;
        }
    }
    return 0;
}