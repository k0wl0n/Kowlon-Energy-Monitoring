#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define RELAY1 2
#define RELAY2 3
#define RELAY3 4
#define RELAY4 5

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/13, /* data=*/11, /* CS=*/10, /* reset=*/8);

Adafruit_ADS1115 ADC_1(0x49); //Create ADS1115 object
Adafruit_ADS1115 ADC_2(0x48); //Create ADS1115 object

const float SCALEFACTOR = 0.125F;   // This is the scale factor for the default +/- 4.096V Range we will use - see ADS1X15 Library
const float BURDEN_RESISTOR = 45;   // Burden resistor where measurement voltage gets measured.
const float VOLTAGE_MAINS = 220.00; // line voltage
const float COIL_WINDING = 2000;    // ratio of sensor 100:0.05

//int16_t rawADCvalue;  // The is where we store the value we receive from the ADS1115
float MAX_CURRENT_COIL = 0.0707; // Maximum current of sensor rated at  Rms 100 A
float MAX_VOLTAGE_ADC = 15.981;  // Maximum Voltage on burden resistor calculated by burden resistor * maxAmps (33 Ohm * 0.0707 A)

float voltage_adc_1 = 0.0; // The result of applying the scale factor to the raw value
float voltage_adc_2 = 0.0;
float voltage_adc_3 = 0.0;
float voltage_adc_4 = 0.0;

float current_coil = 0.0; // Calculated depending on the Voltage/Burden resistor

float power_1 = 0.0;
float power_2 = 0.0;
float power_3 = 0.0;
float power_4 = 0.0;
float power_total = 0.0;

void setup()
{
    Serial.begin(9600);
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Setup");
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.sendBuffer();

    ADC_1.setGain(GAIN_ONE);
    ADC_2.setGain(GAIN_ONE);

    ADC_1.begin();
    ADC_2.begin();

    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);

    delay(1000);
}
void loop()
{
    voltage_adc_1 = (calcVrms(32, 1)) / 1000.0;
    voltage_adc_2 = (calcVrms(32, 2)) / 1000.0;
    voltage_adc_3 = (calcVrms(32, 3)) / 1000.0;
    voltage_adc_4 = (calcVrms(32, 4)) / 1000.0;

    power_1 = calcPower(voltage_adc_1);
    power_2 = calcPower(voltage_adc_2);
    power_3 = calcPower(voltage_adc_3);
    power_4 = calcPower(voltage_adc_4);
    Serial.print('Power R : ');
    Serial.println(power_1);
    Serial.print('Power S : ');
    Serial.println(power_2);
    Serial.print('Power T : ');
    Serial.println(power_3);
    Serial.print('Power N : ');
    Serial.println(power_4);
    Serial.print('Power Total : ');
    Serial.println(power_4);
    power_total = power_1 + power_2 + power_3 + power_4;
}

double(unsigned int Samples, unsigned int phase)
{
    float voltage_rms;
    float squared_voltage;
    float sample_voltage_sum;
    float sample_voltage;

    for (unsigned int n = 0; n < Samples; n++)
    {
        switch (phase)
        {
        case 1:
        {
            sample_voltage = ADC_1.readADC_Differential_0_1();
            break;
        }
        case 2:
        {
            sample_voltage = ADC_1.readADC_Differential_2_3();
            break;
        }
        case 3:
        {
            sample_voltage = ADC_2.readADC_Differential_0_1();
            break;
        }
        case 4:
        {
            sample_voltage = ADC_2.readADC_Differential_2_3();
            break;
        }
        default:
            break;
        }
        squared_voltage = sample_voltage * sample_voltage;
        sample_voltage_sum += squared_voltage;
    }

    voltage_rms = (sqrt(sample_voltage_sum / Samples) * SCALEFACTOR) * sqrt(2);
    sample_voltage_sum = 0;
    return voltage_rms;
}

double calcPower(float voltage)
{
    double power;
    power = voltage / BURDEN_RESISTOR * COIL_WINDING * VOLTAGE_MAINS;
    return power;
}