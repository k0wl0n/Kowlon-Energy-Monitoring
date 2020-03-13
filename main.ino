#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <ATM90E32.h>
#include <PCF8574.h>
#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>

// Set i2c address
PCF8574 expander_1(0x20);
// PCF8574 expander_2(0x24);

/***** Another SS PINS ****
 * SPI	MOSI	MISO	CLK	CS
 * VSPI	GPIO 23	GPIO 19	GPIO 18	GPIO 5
 * HSPI	GPIO 13	GPIO 12	GPIO 14	GPIO 15
 * For SS PIN
 * 0 27 35 13 14 15 16 17 21 22 25 26
*/
/***** CALIBRATION SETTINGS *****/
/* 
 * 4485 for 60 Hz (North America)
 * 389 for 50 hz (rest of the world)
 */

/* 
 * 0 for 10A (1x)
 * 21 for 100A (2x)
 * 42 for between 100A - 200A (4x)
 */

/* 
 * For meter <= v1.3:
 *    42080 - 9v AC Transformer - Jameco 112336
 *    32428 - 12v AC Transformer - Jameco 167151
 * For meter > v1.4:
 *    37106 - 9v AC Transformer - Jameco 157041
 *    38302 - 9v AC Transformer - Jameco 112336
 *    29462 - 12v AC Transformer - Jameco 167151
 * For Meters > v1.4 purchased after 11/1/2019 and rev.3
 *    7611 - 9v AC Transformer - Jameco 157041
 */

/*
 * 25498 - SCT-013-000 100A/50mA
 * 39473 - SCT-016 120A/40mA
 * 46539 - Magnalab 100A
 */

#if defined ESP8266
const int CS_pin = 16;
/***
 * D5/14 - CLK
 * D6/12 - MISO
 * D7/13 - MOSI
*/
#elif defined ESP32
const int CS_pin = 5;
const int CS_pin_second = 4;
/***
 * 18 - CLK
 * 19 - MISO
 * 23 - MOSI
*/
#endif

ATM90E32 eic_first{};  //initialize the IC class
ATM90E32 eic_second{}; //initialize the IC class

// #define RELAY1 33
// #define RELAY2 32
// #define RELAY3 35
// #define RELAY4 34

// #define RELAY1_SECOND 2
// #define RELAY2_SECOND 3
// #define RELAY3_SECOND 4
// #define RELAY4_SECOND 5

#define UP 18
#define OK 19
#define DOWN 17
#define BACK 16

// U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/13, /* data=*/11, /* CS=*/10, /* reset=*/8);
/***
 * LCD Library
 * Robotdyn Arduino Mega mini
 * SCK,MOSI,SS/CS
 * 10 -> 53
 * 11 -> 51
 * 13 -> 52
 * ESP32
 * 14 - CLK -> E
 * 13 - MOSI -> R/W
 * 27 - SS -> RS
*/

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 14, 13, 27);

Adafruit_ADS1115 ADC_1(0x48); //Create ADS1115 object

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 11;

unsigned short LineFreq = 389;
unsigned short PGAGain = 42;
unsigned short VoltageGain = 37106;
unsigned short CurrentGainCT1 = 40023; //SCT-019-000 200/30mA
unsigned short CurrentGainCT2 = 40023; //SCT-019-000 200/30mA
unsigned short CurrentGainCT3 = 40023; //SCT-019-000 200/30mA
unsigned short CurrentGainCT4 = 12300; //SCT-019-000 200/30mA
unsigned short CurrentGainCT5 = 4048;  //SCT-013-000 100A/50mA
unsigned short CurrentGainCT6 = 4046;  //SCT-013-000 100A/50mA
float CurrentGainCT7 = 33;             //SCT-013-000 100A/50mA
float CurrentGainCT8 = 33;             //SCT-013-000 100A/50mA
float multiplier = 0.0625F;

// board integrated ATM90E32 CircuitSetup
// CT1   CT2  CT3  CT4  CT5  CT6
// 1R    2S   3T   4P   5R   6S
// 200A  200A 200A 200A 100A 100A

// CT7   CT8 ADS1115
// 7T    8P
// 100A  100A

//Riley Setup
unsigned int RC1 = 5, RC2 = 30, RC3 = 45, RC4 = 70;
unsigned int RC1_SECOND = 15, RC2_SECOND = 35, RC3_SECOND = 80, RC4_SECOND = 80;

void setup()
{
    Serial.begin(115200);

    //LCD Init
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
    u8g2.drawStr(0, 10, "Setup");
    u8g2.sendBuffer();

    ADC_1.setGain(GAIN_TWO);
    ADC_1.begin();

    // pinMode(13, OUTPUT);

    //Custom Keypad Init
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(OK, INPUT_PULLUP);
    pinMode(BACK, INPUT_PULLUP);

    // expander_2.pinMode(P0, INPUT_PULLUP);
    // expander_2.pinMode(P1, INPUT_PULLUP);
    // expander_2.pinMode(P2, INPUT_PULLUP);
    // expander_2.pinMode(P3, INPUT_PULLUP);

    //RELAY INIT
    expander_1.pinMode(P0, OUTPUT);
    expander_1.pinMode(P1, OUTPUT);
    expander_1.pinMode(P2, OUTPUT);
    expander_1.pinMode(P3, OUTPUT);

    expander_1.pinMode(P4, OUTPUT);
    expander_1.pinMode(P5, OUTPUT);
    expander_1.pinMode(P6, OUTPUT);
    expander_1.pinMode(P7, OUTPUT);

    expander_1.digitalWrite(P0, LOW);
    expander_1.digitalWrite(P1, LOW);
    expander_1.digitalWrite(P2, LOW);
    // expander_1.digitalWrite(P3, LOW);
    // expander_1.digitalWrite(P4, LOW);
    // expander_1.digitalWrite(P5, LOW);
    // expander_1.digitalWrite(P6, LOW);
    // expander_1.digitalWrite(P7, LOW);

    // Relay Test
    // delay(1000);
    // expander_1.digitalWrite(P0, HIGH);
    // expander_1.digitalWrite(P1, HIGH);
    // expander_1.digitalWrite(P2, HIGH);
    expander_1.digitalWrite(P3, HIGH);
    expander_1.digitalWrite(P4, HIGH);
    expander_1.digitalWrite(P5, HIGH);
    expander_1.digitalWrite(P6, HIGH);
    expander_1.digitalWrite(P7, HIGH);
    delay(1000);
}

void loop()
{
    char key;
    boolean stat = true;
    key = getPressedKey();

    delay(100);

    if (key == 'U')
    {
        mainMenuPage++;
        if (mainMenuPage > mainMenuTotal)
            mainMenuPage = 1;
    }
    else if (key == 'D')
    {
        mainMenuPage--;
        if (mainMenuPage == 0)
            mainMenuPage = mainMenuTotal;
    }

    if (key == 'X') //enter selected menu
    {
        Serial.println("TRUE");
        Serial.println(mainMenuPage);
    }

    if (key == 'X') //enter selected menu
    {
        switch (mainMenuPage)
        {
        case 1:
            MenuAmpere();
            break;
        case 2:
            CurrentGainCT1 = MenuSet(CurrentGainCT1, "OUT A", 1);
            break;
        case 3:
            CurrentGainCT2 = MenuSet(CurrentGainCT2, "OUT B", 1);
            break;
        case 4:
            CurrentGainCT3 = MenuSet(CurrentGainCT3, "OUT C", 1);
            break;
        case 5:
            CurrentGainCT4 = MenuSet(CurrentGainCT4, "IN A", 1);
            break;
        case 6:
            CurrentGainCT5 = MenuSet(CurrentGainCT5, "IN B", 1);
            break;
        case 7:
            CurrentGainCT6 = MenuSet(CurrentGainCT6, "IN C", 1);
            break;
        // case 8:
        //     CurrentGainCT7 = MenuSet(CurrentGainCT7, "A CT", 0);
        //     break;
        // case 7:
        //     CurrentGainCT8 = MenuSet(CurrentGainCT8, "B CT", 0);
        //     break;
        case 8:
            RC1 = MenuSet(RC1, "Relay 1", 0);
            break;
        case 9:
            RC2 = MenuSet(RC2, "Relay 2", 0);
            break;
        case 10:
            RC3 = MenuSet(RC3, "Relay 3", 0);
            break;
        case 11:
            RC4 = MenuSet(RC4, "Relay 4", 0);
            break;
            // case 14:
            //     RC1_SECOND = MenuSet(RC1_SECOND, "SECOND Relay 1", 0);
            //     break;
            // case 15:
            //     RC2_SECOND = MenuSet(RC2_SECOND, "SECOND Relay 2", 0);
            //     break;
            // case 16:
            //     RC3_SECOND = MenuSet(RC3_SECOND, "SECOND Relay 3", 0);
            //     break;
            // case 17:
            //     RC4_SECOND = MenuSet(RC4_SECOND, "SECOND Relay 4 ", 0);
            //     break;
        }
    }

    if (key != mainMenuPageOld)
    {
        MainMenuDisplay();
        mainMenuPageOld = mainMenuPage;
    }
}

void MenuAmpere()
{
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
    u8g2.drawStr(0, 10, "Initializing");
    u8g2.sendBuffer();
    /*Initialise the ATM90E32 & Pass CS pin and calibrations to its library
    the 2nd (B) current channel is not used with the split phase meter */
    Serial.println("Start ATM90E32");
    eic_first.begin(CS_pin, LineFreq, PGAGain, VoltageGain, CurrentGainCT1, CurrentGainCT2, CurrentGainCT3);
    eic_second.begin(CS_pin_second, LineFreq, PGAGain, VoltageGain, CurrentGainCT4, CurrentGainCT5, CurrentGainCT6);

    delay(500);

    //Get From ATM90E32
    /*Repeatedly fetch some values from the ATM90E32 */
    float voltageA_first, voltageB_first, voltageC_first;
    float voltageA_second, voltageB_second, voltageC_second;
    float totalVoltage;
    float currentCT1, currentCT2, currentCT3;
    float currentCT4, currentCT5, currentCT6;
    float currentCT7, currentCT8;
    float totalCurrent, realPower, powerFactor, temp, freq, totalWatts;
    float tCurrentA = 0;
    float tCurrentB = 0;
    float tCurrentC = 0;
    float testCurrent, testTotal;
    long lastMillis = millis();
    char key;
    while (key != 'B')
    {
        key = getPressedKey();
        unsigned short sys0 = eic_first.GetSysStatus0();  //EMMState0
        unsigned short sys1 = eic_second.GetSysStatus0(); //EMMState0

        //if true the MCU is not getting data from the energy meter
        if (sys0 == 65535 || sys0 == 0)
            Serial.println("Error: Not receiving data from energy meter 1 - check your connections");
        if (sys1 == 65535 || sys1 == 0)
            Serial.println("Error: Not receiving data from energy meter 2 - check your connections");

        //get voltage
        voltageA_first = eic_first.GetLineVoltageA();
        voltageA_second = eic_second.GetLineVoltageA();

        if (LineFreq = 4485)
        {
            totalVoltage = voltageA_first + voltageB_first; //is split single phase, so only 120v per leg
        }
        else
        {
            totalVoltage = voltageA_first; //voltage should be 220-240 at the AC transformer
        }

        //get current first
        currentCT1 = eic_first.GetLineCurrentA();
        currentCT2 = eic_first.GetLineCurrentB();
        currentCT3 = eic_first.GetLineCurrentC();
        tCurrentA = currentCT1 + currentCT2 + currentCT3;

        //get current second
        currentCT4 = eic_second.GetLineCurrentA();
        currentCT5 = eic_second.GetLineCurrentB();
        currentCT6 = eic_second.GetLineCurrentC();
        tCurrentB = currentCT4 + currentCT5 + currentCT6;

        //get current from ADS1115
        // currentCT7 = getIRMS(CurrentGainCT7, 1);
        // currentCT8 = getIRMS(CurrentGainCT8, 2);
        // tCurrentC = currentCT7 + currentCT8;

        totalCurrent = tCurrentA + tCurrentB;
        // totalCurrent = tCurrentA + tCurrentB + tCurrentC;

        Serial.println("Voltage 1: " + String(voltageA_first) + "V");
        Serial.println("Voltage 2: " + String(voltageA_second) + "V");

        Serial.println("Current 1: " + String(currentCT1) + "A");
        Serial.println("Current 2: " + String(currentCT2) + "A");
        Serial.println("Current 3: " + String(currentCT3) + "A");
        Serial.println("Current 4: " + String(currentCT4) + "A");
        Serial.println("Current 5: " + String(currentCT5) + "A");
        Serial.println("Current 6: " + String(currentCT6) + "A");
        // Serial.println("Current 7: " + String(currentCT7) + "A");
        // Serial.println("Current 8: " + String(currentCT8) + "A");
        Serial.println();
        Serial.println("Total Current: " + String(totalCurrent) + "A");
        Serial.println();
        Serial.println();

        // //for testing relay
        // if (millis() - lastMillis >= 2000)
        // {
        //     lastMillis = millis(); //get ready for the next iteration
        //     testCurrent += 5;
        //     testTotal += 5;
        // }

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
        u8g2.setCursor(3, 10);
        u8g2.print("O1:");
        u8g2.print(currentCT1);
        u8g2.setCursor(3, 19);
        u8g2.print("O2:");
        u8g2.print(currentCT2);
        u8g2.setCursor(3, 28);
        u8g2.print("O3:");
        u8g2.print(currentCT3);
        u8g2.setCursor(72, 10);
        u8g2.print("I4:");
        u8g2.print(currentCT4);
        u8g2.setCursor(72, 19);
        u8g2.print("I5:");
        u8g2.print(currentCT5);
        u8g2.setCursor(72, 28);
        u8g2.print("I6:");
        u8g2.print(currentCT6);
        // u8g2.setCursor(3, 37);
        // u8g2.print("C7:");
        // u8g2.print(currentCT7);
        // u8g2.setCursor(72, 37);
        // u8g2.print("C8:");
        // u8g2.print(currentCT8);
        // u8g2.drawFrame(0, 0, 128, 64);
        u8g2.sendBuffer();

        relaySwitch(tCurrentA, tCurrentB, tCurrentC, testTotal);
    }
}

//Custom Callculation
float getIRMS(float factor, unsigned int phase)
{
    float voltage;
    float corriente;
    float sum = 0;
    long tiempo = millis();
    int counter = 0;

    while (millis() - tiempo < 1000)
    {
        switch (phase)
        {
        case 1:
        {
            voltage = ADC_1.readADC_Differential_0_1() * multiplier;
            break;
        }
        case 2:
        {
            voltage = ADC_1.readADC_Differential_2_3() * multiplier;
            break;
        }
        default:
            voltage = ADC_1.readADC_Differential_0_1() * multiplier;
            break;
        }

        corriente = voltage * factor;
        corriente /= 1000.0;
        sum += sq(corriente);
        counter = counter + 1;
    }
    corriente = sqrt(sum / counter);
    return corriente;
}

void relaySwitch(float CurrentInput, float tCurrentB, float tCurrentC, float totalCurrent)
{
    //OUTPUT Relay -  1 2 3 BYPASS
    if (CurrentInput >= 0 && CurrentInput <= 65)
    {
        expander_1.digitalWrite(P0, LOW);
        expander_1.digitalWrite(P1, LOW);
        expander_1.digitalWrite(P2, LOW);
    }
    else if (CurrentInput > 65)
    {
        expander_1.digitalWrite(P0, HIGH);
        expander_1.digitalWrite(P1, HIGH);
        expander_1.digitalWrite(P2, HIGH);
    }

    //OUTPUT Relay -  5 6 7 8 GEL
    if (CurrentInput >= 0 && CurrentInput < RC1) //R1
    {
        //all OFF
        expander_1.digitalWrite(P4, HIGH);
        expander_1.digitalWrite(P5, HIGH);
        expander_1.digitalWrite(P6, HIGH);
        expander_1.digitalWrite(P7, HIGH);
    }
    else if (CurrentInput >= RC1 && CurrentInput < RC2) //R1 - R2
    {
        //5 ON
        // else OFF
        expander_1.digitalWrite(P4, LOW);
        expander_1.digitalWrite(P5, HIGH);
        expander_1.digitalWrite(P6, HIGH);
        expander_1.digitalWrite(P7, HIGH);
    }
    else if (CurrentInput >= RC2 && CurrentInput < RC3) //R2 - R3
    {
        //5 ON
        //6 ON
        // else OFF
        expander_1.digitalWrite(P4, LOW);
        expander_1.digitalWrite(P5, LOW);
        expander_1.digitalWrite(P6, HIGH);
        expander_1.digitalWrite(P7, HIGH);
    }
    else if (CurrentInput >= RC3 && CurrentInput < RC4) //R3 - R4
    {
        //5 ON
        //6 ON
        //7 ON
        // else OFF
        expander_1.digitalWrite(P4, LOW);
        expander_1.digitalWrite(P5, LOW);
        expander_1.digitalWrite(P6, LOW);
        expander_1.digitalWrite(P7, HIGH);
    }
    else if (CurrentInput > RC4) //R4
    {
        //All On
        expander_1.digitalWrite(P4, LOW);
        expander_1.digitalWrite(P5, LOW);
        expander_1.digitalWrite(P6, LOW);
        expander_1.digitalWrite(P7, LOW);
    }
}

float MenuSet(float value, String name, int status)
{
    char key = 'X';
    while (key != 'B')
    {
        key = getPressedKey();
        Serial.println(name);
        Serial.println(value);
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
        u8g2.setCursor(0, 10);
        u8g2.print(value);

        if (key == 'D')
        {
            if (status == 1)
            {
                value = value - 50;
            }
            else
            {
                value = value - 1;
            }

            if (value < 0)
            {
                value = 0;
            }
            u8g2.setCursor(0, 10);
            u8g2.print("                ");
            u8g2.setCursor(0, 10);
            u8g2.print(value);
            Serial.println(name);
            Serial.println(value);
        }
        else if (key == 'U')
        {
            if (status == 1)
            {
                value = value + 50;
            }
            else
            {
                value = value + 1;
            }

            u8g2.setCursor(0, 10);
            u8g2.print("                ");
            u8g2.setCursor(0, 10);
            u8g2.print(value);
            Serial.println(name);
            Serial.println(value);
        }
        u8g2.sendBuffer();
    }
    return value;
}

void MainMenuDisplay()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
    u8g2.setCursor(0, 10);
    switch (mainMenuPage)
    {
    case 1:
        u8g2.print("1.Monitor Arus");
        Serial.println("1.Monitor Arus");
        break;
    case 2:
        u8g2.print("2.Cal OUT A");
        Serial.println("2.Cal OUT A");
        break;
    case 3:
        u8g2.print("3.Cal OUT B");
        Serial.println("3.Cal OUT B");
        break;
    case 4:
        u8g2.print("4.Cal OUT C");
        Serial.println("4.Cal OUT C");
        break;
    case 5:
        u8g2.print("5.Cal OUT A");
        Serial.println("5.Cal OUT A");
        break;
    case 6:
        u8g2.print("6.Cal IN B");
        Serial.println("6.Cal IN B");
        break;
    case 7:
        u8g2.print("7.Cal IN C");
        Serial.println("7.Cal IN C");
        break;
    // case 8:
    //     u8g2.print("8.Cal A CT");
    //     Serial.println("8.Cal A CT");
    //     break;
    // case 7:
    //     u8g2.print("9.Cal IN C");
    //     Serial.println("9.Cal IN C");
    //     break;
    case 8:
        u8g2.print("10.Set RL C1");
        Serial.println("10.Set RL C1");
        break;
    case 9:
        u8g2.print("11.Set RL C2");
        Serial.println("11.Set RL C2");
        break;
    case 10:
        u8g2.print("12.Set RL C3");
        Serial.println("12.Set RL C3");
        break;
    case 11:
        u8g2.print("13.Set RL C4");
        Serial.println("13.Set RL C4");
        break;
        // case 14:
        //     u8g2.print("14.Set Relay_2 C1");
        //     Serial.println("14.Set Relay_2 C1");
        //     break;
        // case 15:
        //     u8g2.print("15.Set Relay_2 C2");
        //     Serial.println("15.Set Relay_2 C2");
        //     break;
        // case 16:
        //     u8g2.print("16.Set Relay_2 C3");
        //     Serial.println("16.Set Relay_2 C3");
        //     break;
        // case 17:
        //     u8g2.print("17.Set Relay_2 C4");
        //     Serial.println("17.Set Relay_2 C4");
        //     break;
    }
    u8g2.sendBuffer();
}

char getPressedKey()
{
    char key = NULL;

    if (digitalRead(UP) == 0)
    {
        key = 'U';
    }
    if (digitalRead(DOWN) == 0)
    {
        key = 'D';
    }
    if (digitalRead(OK) == 0)
    {
        key = 'X';
    }
    if (digitalRead(BACK) == 0)
    {
        key = 'B';
    }
    return key;
}

void setupSpiffs()
{
    //clean FS, for testing
    //SPIFFS.format();

    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonBuffer jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success())
                {
                    Serial.println("\nparsed json");

                    CurrentGainCT1 = json["CurrentGainCT1"];
                    CurrentGainCT2 = json["CurrentGainCT2"];
                    CurrentGainCT3 = json["CurrentGainCT3"];
                    CurrentGainCT4 = json["CurrentGainCT4"];
                    CurrentGainCT5 = json["CurrentGainCT5"];
                    CurrentGainCT6 = json["CurrentGainCT6"];
                    CurrentGainCT7 = json["CurrentGainCT7"];
                    CurrentGainCT8 = json["CurrentGainCT8"];
                    RC1 = json["RC1"];
                    RC2 = json["RC2"];
                    RC3 = json["RC3"];
                    RC4 = json["RC4"];

                    // strcpy(mqtt_server, json["mqtt_server"]);
                    // strcpy(mqtt_port, json["mqtt_port"]);
                    // strcpy(api_token, json["api_token"]);
                }
                else
                {
                    Serial.println("failed to load json config");
                }
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    //end read
}