#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;

float SCALEFACTOR = 0.125F;      // This is the scale factor for the default +/- 4.096V Range we will use - see ADS1X15 Library
float MAX_CURRENT_COIL = 0.0707; // Maximum current of sensor rated at  Rms 100 A
float MAX_VOLTAGE_ADC = 45;      // Maximum Voltage on burden resistor calculated by burden resistor * maxAmps (33 Ohm * 0.0707 A)
float voltage_adc = 0.0;         // The result of applying the scale factor to the raw value
float VOLTAGE_MAINS = 220.00;    // line voltage
float current_coil = 0.0;        // Calculated depending on the Voltage/Burden resistor
float power;
float COIL_WINDING = 1000; // ratio of sensor 100:0.05

float sqV;
float sumV;
float sampleV;
float Vrms;

String line;
String PowerAsString;

void setup()
{
    Serial.begin(9600);
    ads.begin();
    ads.setGain(GAIN_ONE);
}
void loop()
{
    voltage_adc = (calcVrms(100)) / 1000.0;

    current_coil = ((voltage_adc)*MAX_CURRENT_COIL) / MAX_VOLTAGE_ADC;
    power = current_coil * COIL_WINDING * VOLTAGE_MAINS;

    Serial.println(power);
    delay(100); // wait for a second
}

double calcVrms(unsigned int Number_of_Samples)
{
    for (unsigned int n = 0; n < Number_of_Samples; n++)
    {
        sampleV = ads.readADC_Differential_0_1();
        sqV = sampleV * sampleV;
        sumV += sqV;
    }

    Vrms = (sqrt(sumV / Number_of_Samples) * SCALEFACTOR) * sqrt(2);
    sumV = 0;
    return Vrms;
}