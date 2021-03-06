//WiFi
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//InfluxDB
#include <influxDB_udp.h>

//Utility
#include <string.h>
#include <math.h>
#include <Ticker.h>

//Sensor Library
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SparkFun_Si7021_Breakout_Library.h>
#include <Adafruit_CCS811.h>

// HTTP
#include <Bridge.h>
#include <HttpClient.h>

// JSON
#include <ArduinoJson.h>

//-----------------------------------------------------------------------------
#define DEVICE_VERSION 0.9
#define DEVICE_COUNT 7

#define TABLE_HEIGHT DEVICE_COUNT
#define TABLE_WIDTH 4

#define INDEX_MAC 0
#define INDEX_LOCATION 1
#define INDEX_ROOM 2
#define INDEX_TYPE 3
#define INDEX_Offset 4

#define SparkFun_Si7021 Weather

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "11e76ce84e2d03e19a9a65f925e0d9b15bc1e76c";

//WiFi Information -- START ---------------------------------------------------
String config_host = "chronograf01.monitor.agora-space.com";

const char *ssid = "Agora Space";
const char *password = "getstuffdone";

bool isPowerType;
bool isQualityType;

String MacToString(const uint8_t *mac);

//InfluxDB initialisation -- START --------------------------------------------
WiFiUDP udp;
InfluxDB_Data dataPower("power_measurement");
InfluxDB_Data dataAirQuality("quality_measurement");
//InfluxDB initialisation -- END ----------------------------------------------

//Ticker stuff -- START -------------------------------------------------------
Ticker DB_Updater;
int Timer = 0;
bool tickOccured = false;

void timerCallback()
{
    tickOccured = true;
    if (Timer >= 50)
    {
        Timer = 0;
        getConfigFromPi();
    }
    else
    {
        Timer++;
    }
}
//Ticker stuff -- UDP ---------------------------------------------------------

Adafruit_ADS1115 ADC_1(0x49); //Create ADS1115 object
Adafruit_ADS1115 ADC_2(0x48); //Create ADS1115 object
SparkFun_Si7021 Si7021;       //Create Instance of Si7021 temperature and humidity sensor
Adafruit_CCS811 CCS811;       //Create CCS811 object

const float SCALEFACTOR = 0.125F;   // This is the scale factor for the default +/- 4.096V Range we will use - see ADS1X15 Library
const float BURDEN_RESISTOR = 220;  // Burden resistor where measurement voltage gets measured.
const float VOLTAGE_MAINS = 220.00; // line voltage
const float COIL_WINDING = 2000;    // ratio of sensor 100:0.05

//int16_t rawADCvalue;  // The is where we store the value we receive from the ADS1115
float MAX_CURRENT_COIL = 0.0707; // Maximum current of sensor rated at  Rms 100 A
float MAX_VOLTAGE_ADC = 15.981;  // Maximum Voltage on burden resistor calculated by burden resistor * maxAmps (33 Ohm * 0.0707 A)

float voltage_adc_1 = 0.0; // The result of applying the scale factor to the raw value
float voltage_adc_2 = 0.0;
float voltage_adc_3 = 0.0;

float current_coil = 0.0; // Calculated depending on the Voltage/Burden resistor

float power_1 = 0.0;
float power_2 = 0.0;
float power_3 = 0.0;
float power_total = 0.0;

float humidity = 0;
float temperature = 0;

float eCO2 = 0;
float TVOC = 0;
float eCO2OLD = 0;
float TVOCOLD = 0;
float LongRST = 0;

int writeflag = 0;
int Powerflag = 0;

String line;
String PowerAsString;

int n = 0;

class ArduinoConfig
{
public:
    String location = "";
    String influx_bd_host = "";
    String room = "";
    String mac = "";
    String sensor_type = "";
    int quality_port = 0;
    int power_port = 0;
    int offset_temperature = 0;
    int offset_humidity = 0;
};

ArduinoConfig *arduinoConfig;

void SWI(void)
{
    writeflag = 0;
    digitalWrite(D5, LOW); //Reset Sensor
    delay(100);
    digitalWrite(D5, HIGH);
    Serial.println("Reset Sensor");
    CCS811.begin();
    delay(5000);
    while (eCO2 == 0.00)
    {
        eCO2 = CCS811.geteCO2();
        delay(100);
    }
    Serial.println("Sensor ready");
}

void reconnectWifi(void)
{
    WiFi.softAPdisconnect(true);
    static int n = 0;
    n = 0;
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED || n > 100)
    {
        delay(500);
        Serial.print(".");
        n++;
    }
    Serial.print("reconnected");
}

void getConfigFromPi()
{
    WiFiClientSecure httpsClient; //Declare object of class WiFiClient

    Serial.println(config_host);

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(15000); // 15 Seconds
    delay(1000);

    Serial.print("HTTPS Connecting");
    int r = 0; //retry counter
    while ((!httpsClient.connect(config_host, 443)) && (r < 30))
    {
        delay(100);
        Serial.print(".");
        r++;
    }
    if (r == 30)
    {
        Serial.println("Connection failed");
    }
    else
    {
        Serial.println("Connected to web");
    }

    String ADCData, getData, Link;
    int adcvalue = analogRead(A0); //Read Analog value of LDR
    ADCData = String(adcvalue);    //String to interger conversion

    //GET Data
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String mac_address = MacToString(mac);
    Link = "/api/v1.0/config/" + mac_address;

    Serial.print("requesting URL: ");
    Serial.println(config_host + Link);

    httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
                      "Host: " + config_host + "\r\n" +
                      "Connection: close\r\n\r\n");

    Serial.println("request sent");

    String respond = "";

    while (httpsClient.connected())
    {
        String line = httpsClient.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("headers received");
            break;
        }
    }

    Serial.println("reply was:");
    Serial.println("==========");
    String line;
    while (httpsClient.available())
    {
        line = httpsClient.readStringUntil('\n'); //Read Line by Line
        Serial.println(line);
        respond = respond + line; //Print response
    }
    Serial.println("==========");
    Serial.println("closing connection");

    int i = respond.indexOf('{');
    String json = respond.substring(i);

    Serial.println("JSON: ");
    Serial.println(json);

    StaticJsonDocument<1024> jsonDoc;
    auto parseError = deserializeJson(jsonDoc, json.c_str());

    if (parseError)
    {
        Serial.println("parseObject() failed: ");
        Serial.println(parseError.c_str());
        return;
    }

    arduinoConfig = new ArduinoConfig;
    arduinoConfig->location = jsonDoc["location"].as<String>();
    arduinoConfig->influx_bd_host = jsonDoc["influx_bd_host"].as<String>();
    arduinoConfig->room = jsonDoc["room"].as<String>();
    arduinoConfig->mac = jsonDoc["mac"].as<String>();
    arduinoConfig->sensor_type = jsonDoc["sensor_type"].as<String>();
    arduinoConfig->quality_port = jsonDoc["quality_port"];
    arduinoConfig->power_port = jsonDoc["power_port"];
    arduinoConfig->offset_temperature = jsonDoc["offset_temperature"];
    arduinoConfig->offset_humidity = jsonDoc["offset_humidity"];
}

void setup(void)
{
    /*-------------------Software Interupt-----------------------------------*/
    pinMode(D5, OUTPUT);
    digitalWrite(D5, HIGH);
    //-----------------------------------------------------------------------//
    //---------------------------Wifihotspo--from--ESP--disable---------------
    WiFi.softAPdisconnect(true);
    Wire.setClock(400000);
    Serial.begin(115200);
    delay(1000);

    // WiFi Connection -- START -----------------------------------------------
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Wifi Connected to " + String(ssid));
    // WiFi Connection -- END -------------------------------------------------

    getConfigFromPi();

    // WiFi Mac Address Mapping -- START --------------------------------------
    String mac_address = arduinoConfig->mac;

    Serial.println("\n----- IMPORTANT INFORMATION -----");
    Serial.println(arduinoConfig->mac);
    Serial.println(arduinoConfig->location);
    Serial.println(arduinoConfig->room);
    Serial.println("---------------------------------\n");

    if (arduinoConfig->sensor_type == "power")
    {
        isPowerType = 1;
        isQualityType = 0;
        Serial.println("power");
        Powerflag = 1;

        dataPower.addTag("location", arduinoConfig->location);
        dataPower.addTag("room", arduinoConfig->room);
    }
    else
    {
        isPowerType = 0;
        isQualityType = 1;

        dataAirQuality.addTag("location", arduinoConfig->location);
        dataAirQuality.addTag("room", arduinoConfig->room);
    }
    // WiFi Mac Address Mapping -- END ----------------------------------------

    // Sensor Initialisation -- START -----------------------------------------
    if (isQualityType)
    {
        Si7021.begin();
        CCS811.begin();

        if (!CCS811.begin())
        {
            Serial.println("Failed to start CCS811! Please check your wiring.");
            while (!CCS811.available())
            {
                delay(200);
            }
        }
        else
        {
            Serial.println("CCS811 found");
        }
        while (!CCS811.available())
        {
            delay(200);
        }

        float temp;
        temp = CCS811.calculateTemperature();
        CCS811.setTempOffset(temp - 25.0);
    }

    if (isPowerType)
    {
        ADC_1.setGain(GAIN_ONE);
        ADC_2.setGain(GAIN_ONE);

        ADC_1.begin();
        ADC_2.begin();
    }

    // Sensor Initialisation -- END -------------------------------------------

    //activate Ticker
    DB_Updater.attach(1, timerCallback);
}

void loop(void)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        reconnectWifi();
    }

    if ((tickOccured == true) && (writeflag == 1))
    {
        writeflag = 0;
        tickOccured = false;

        InfluxDB_UDP influxPowerPort(udp, arduinoConfig->influx_bd_host, arduinoConfig->power_port);
        InfluxDB_UDP influxSensorPort(udp, arduinoConfig->influx_bd_host, arduinoConfig->quality_port);

        if (isQualityType)
        {
            dataAirQuality.clear(InfluxDB_Data::FIELD);
            dataAirQuality.addField("temperature_c", String(temperature + arduinoConfig->offset_temperature));
            dataAirQuality.addField("TVOC_ppb", String(TVOC));
            dataAirQuality.addField("eCO2_ppm", String(eCO2));
            dataAirQuality.addField("humidity_RH", String(humidity + arduinoConfig->offset_humidity));
            influxSensorPort.write(dataAirQuality);
        }

        if (isPowerType)
        {
            Serial.println(power_1);
            Serial.println(power_3);
            Serial.println(power_2);
            dataPower.clear(InfluxDB_Data::FIELD);
            dataPower.addField("power1_W", String(power_1));
            dataPower.addField("power2_W", String(power_2));
            dataPower.addField("power3_W", String(power_3));
            dataPower.addField("powerTotal_W", String(power_total));
            influxPowerPort.write(dataPower);
        }
    }
    if (isQualityType)
    {
        LongRST++;
        delay(100);
        if (LongRST >= 12000)
        {
            LongRST = 0;
            SWI();
            humidity = Si7021.getRH();
            temperature = Si7021.getTemp();
            if (CCS811.available())
            {
                if (!CCS811.readData())
                {
                    eCO2OLD = eCO2;
                    TVOCOLD = TVOC;
                    eCO2 = CCS811.geteCO2();
                    TVOC = CCS811.getTVOC();
                }
                else
                {
                    Serial.println("ERROR!");
                }
            }
            if (eCO2 - eCO2OLD >= 20)
            {
                writeflag = 0;
                SWI();
            }
            else
            {
                writeflag = 1;
            }
        }
        humidity = Si7021.getRH();
        temperature = Si7021.getTemp();
        if (CCS811.available())
        {
            if (!CCS811.readData())
            {
                eCO2OLD = eCO2;
                TVOCOLD = TVOC;
                eCO2 = CCS811.geteCO2();
                TVOC = CCS811.getTVOC();
            }
            else
            {
                Serial.println("ERROR!");
            }
        }
        if (eCO2 - eCO2OLD >= 50)
        {
            writeflag = 0;
            SWI();
        }
        else
        {
            writeflag = 1;
        }
    }
    // current_coil = ((voltage_adc_1) * MAX_CURRENT_COIL)/MAX_VOLTAGE_ADC;
    //power = current_coil * COIL_WINDING * VOLTAGE_MAINS;

    if (isPowerType)
    {
        voltage_adc_1 = (calcVrms(32, 1)) / 1000.0;
        voltage_adc_2 = (calcVrms(32, 2)) / 1000.0;
        voltage_adc_3 = (calcVrms(32, 3)) / 1000.0;

        power_1 = calcPower(voltage_adc_1);
        power_2 = calcPower(voltage_adc_2);
        power_3 = calcPower(voltage_adc_3);
        power_total = power_1 + power_2 + power_3;
    }
}

double calcVrms(unsigned int Samples, unsigned int phase)
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
            sample_voltage = ADC_2.readADC_Differential_2_3();
            break;
        }
        case 3:
        {
            sample_voltage = ADC_2.readADC_Differential_0_1();
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

String MacToString(const uint8_t *mac)
{
    String s_mac = "";
    for (int8_t index = 5; index >= 0; index--)
    {
        s_mac += String(mac[index], HEX);
        if (index != 0)
        {
            s_mac += ":";
        }
    }
    return s_mac;
}