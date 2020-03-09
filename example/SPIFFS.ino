#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>

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

                    // strcpy(mqtt_server, json["mqtt_server"]);
                    // strcpy(mqtt_port, json["mqtt_port"]);
                    // strcpy(api_token, json["api_token"]);

                    // if(json["ip"]) {
                    //   Serial.println("setting custom ip from config");
                    //   strcpy(static_ip, json["ip"]);
                    //   strcpy(static_gw, json["gateway"]);
                    //   strcpy(static_sn, json["subnet"]);
                    //   Serial.println(static_ip);
                    // } else {
                    //   Serial.println("no custom ip in config");
                    // }
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

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println();

    setupSpiffs();

    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["api_token"] = api_token;

        File configFile = SPIFFS.open("/config.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }

        json.prettyPrintTo(Serial);
        json.printTo(configFile);
        configFile.close();
        //end save
        shouldSaveConfig = false;
    }
}

void loop()
{
    // put your main code here, to run repeatedly:
}