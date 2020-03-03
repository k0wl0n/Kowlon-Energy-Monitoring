#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#define RELAY1 34
#define RELAY2 32
#define RELAY3 30
#define RELAY4 28

#define pin_c1 A7
#define pin_c2 A8
#define pin_c3 A9

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 3, 2};    //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// EnergyMonitor emon1; // Create an instance
// EnergyMonitor emon2; // Create an instance
// EnergyMonitor emon3; // Create an instance

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 7;

int sec = 1;
unsigned long start;
float c1 = 0, c2 = 0, c3 = 0;
int RC1 = 15, RC2 = 35, RC3 = 80;
float CTR = 170, CTS = 190, CTT = 150;

void setup()
{

    Serial.begin(9600);
    lcd.begin(20, 4);

    // emon1.current(pin_c1, CTR);
    // emon2.current(pin_c2, CTS);
    // emon3.current(pin_c3, CTT);

    Serial.print("CTR ");
    Serial.print(CTR);
    Serial.print(" CTS ");
    Serial.print(CTS);
    Serial.print(" CTT ");
    Serial.print(CTT);
    Serial.print(" RC1 ");
    Serial.print(RC1);
    Serial.print(" RC2 ");
    Serial.print(RC2);
    Serial.print(" RC3 ");
    Serial.print(RC3);

    delay(1000);
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
    //MainMenuDisplay();
    MenuAmpere();
}

void loop()
{
    char key;
    boolean stat = true;

    //MainMenuDisplay();
    while (stat)
    {
        key = keypad.getKey();
        if (key)
            stat = false;
    }

    if (key == '2')
    {
        mainMenuPage++;
        if (mainMenuPage > mainMenuTotal)
            mainMenuPage = 1;
    }
    else if (key == '8')
    {
        mainMenuPage--;
        if (mainMenuPage == 0)
            mainMenuPage = mainMenuTotal;
    }
    if (key != mainMenuPageOld)
    {
        MainMenuDisplay();
        mainMenuPageOld = mainMenuPage;
    }

    if (key == '5') //enter selected menu
    {
        switch (mainMenuPage)
        {
        case 1:
            MenuAmpere();
            break;
        case 2:
            MenuSetR();
            break;
        case 3:
            MenuSetS();
            break;
        case 4:
            MenuSetT();
            break;
        case 5:
            MenuSetRC1();
            break;
        case 6:
            MenuSetRC2();
            break;
        case 7:
            MenuSetRC3();
            break;
        }
    }
}

float femon1()
{
    float Irms = emon1.calcIrms(1480); // Calculate Irms only
    Serial.print(Irms * 220.0);        // Apparent power
    Serial.print(" ");
    Serial.print(Irms); // Irms
    Serial.print(" A  ");
    lcd.setCursor(0, 0);
    lcd.print("C1:");
    lcd.setCursor(3, 0);
    lcd.print(Irms, 1);
    return Irms;
}

float femon2()
{
    float Irms2 = emon2.calcIrms(1480); // Calculate Irms only
    Serial.print(Irms2 * 220.0);        // Apparent power
    Serial.print(" ");
    Serial.print(Irms2); // Irms
    Serial.print(" ");
    Serial.print(" A  ");

    lcd.setCursor(8, 0);
    lcd.print(" C2:");
    lcd.setCursor(12, 0);
    lcd.print(Irms2, 1);
    return Irms2;
}

float femon3()
{

    float Irms3 = emon3.calcIrms(1480); // Calculate Irms only
    Serial.print(Irms3 * 220.0);        // Apparent power
    Serial.print(" ");
    Serial.print(Irms3); // Irms
    Serial.print(" ");
    Serial.print(" A  ");

    lcd.setCursor(0, 1);
    lcd.print("C3: ");
    lcd.setCursor(3, 1);
    lcd.print(Irms3, 1);
    return Irms3;
}

void ampere()
{
    c1 = c1 + femon1();
    c2 = c2 + femon2();
    c3 = c3 + femon3();

    start = millis();
    delay(100);
    int rst = start / 1000;
    if (sec == 5)
    {
        float relay1 = c1 / sec;
        float relay2 = c2 / sec;
        float relay3 = c3 / sec;
        float total = (relay1 + relay2 + relay3) / 3;
        lcd.setCursor(8, 1);
        lcd.print(" TC:");
        lcd.setCursor(12, 1);
        lcd.print(total);

        lcd.setCursor(0, 2);
        lcd.print("R1");
        lcd.setCursor(0, 3);
        lcd.print(RC1);

        lcd.setCursor(3, 2);
        lcd.print("R2");
        lcd.setCursor(3, 3);
        lcd.print(RC2);

        lcd.setCursor(6, 2);
        lcd.print("R3");
        lcd.setCursor(6, 3);
        lcd.print(RC3);

        Serial.println("");
        Serial.println("AI : ");
        Serial.println(total);

        Serial.println("C1 : ");
        Serial.println(c1 / sec);

        if (total >= 0 && total <= RC1)
        {
            digitalWrite(RELAY1, HIGH);
            digitalWrite(RELAY2, HIGH);
            digitalWrite(RELAY3, HIGH);
            lcd.setCursor(9, 2);
            lcd.print("R1");
            lcd.setCursor(9, 3);
            lcd.print("OFF");
            lcd.setCursor(12, 2);
            lcd.print("R2");
            lcd.setCursor(12, 3);
            lcd.print("OFF");
            lcd.setCursor(15, 2);
            lcd.print("R3");
            lcd.setCursor(15, 3);
            lcd.print("OFF");
        }
        else if (total > RC1 && total <= RC2)
        {
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, HIGH);
            digitalWrite(RELAY3, HIGH);

            lcd.setCursor(9, 2);
            lcd.print("R1");
            lcd.setCursor(9, 3);
            lcd.print("ON ");
            lcd.setCursor(12, 2);
            lcd.print("R2");
            lcd.setCursor(12, 3);
            lcd.print("OFF");
            lcd.setCursor(15, 2);
            lcd.print("R3");
            lcd.setCursor(15, 3);
            lcd.print("OFF");
        }
        else if (total > RC2 && total <= RC3)
        {
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, HIGH);

            lcd.setCursor(9, 2);
            lcd.print("R1");
            lcd.setCursor(9, 3);
            lcd.print("ON ");
            lcd.setCursor(12, 2);
            lcd.print("R2");
            lcd.setCursor(12, 3);
            lcd.print("ON ");
            lcd.setCursor(15, 2);
            lcd.print("R3");
            lcd.setCursor(15, 3);
            lcd.print("OFF");
        }
        else if (total > RC3 && total <= 150)
        {
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, LOW);

            lcd.setCursor(9, 2);
            lcd.print("R1");
            lcd.setCursor(9, 3);
            lcd.print("ON ");
            lcd.setCursor(12, 2);
            lcd.print("R2");
            lcd.setCursor(12, 3);
            lcd.print("ON ");
            lcd.setCursor(15, 2);
            lcd.print("R3");
            lcd.setCursor(15, 3);
            lcd.print("ON ");
        }

        Serial.println("C2 : ");
        Serial.println(c2 / sec);
        Serial.println("C3 : ");
        Serial.println(c3 / sec);
        c1 = 0;
        c2 = 0;
        c3 = 0;
        sec = 1;
    }
    else
    {
        sec++;
    }
    Serial.println(" ");
}

void MainMenuDisplay()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    switch (mainMenuPage)
    {
    case 1:
        lcd.print("1.Monitor Arus");
        break;
    case 2:
        lcd.print("2.Kalibrasi CT R");
        break;
    case 3:
        lcd.print("3.Kalibrasi CT S");
        break;
    case 4:
        lcd.print("4.Kalibrasi CT T");
        break;
    case 5:
        lcd.print("5.Set Range C1");
        break;
    case 6:
        lcd.print("6.Set Range C2");
        break;
    case 7:
        lcd.print("7.Set Range C3");
        break;
    }
}

void MenuAmpere()
{
    char key;
    lcd.clear();
    lcd.setCursor(0, 0);

    while (key != '4')
    {
        key = keypad.getKey();
        ampere();
    }
}

void MenuSetR()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi CT R");

    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(CTR);

        if (key == '8')
        {
            CTR = CTR - 0.5;
            if (CTR < 0)
            {
                CTR = 0;
            }

            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(CTR);
        }
        else if (key == '2')
        {
            CTR = CTR + 0.5;

            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(CTR);
            lcd.print(CTR);
        }
    }
}

void MenuSetS()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi CT S");

    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(CTS);

        if (key == '8')
        {
            CTS = CTS - 0.5;
            if (CTS < 0)
            {
                CTS = 0;
            }

            lcd.setCursor(0, 1);
            lcd.print("                ");

            Serial.println(CTS);
            lcd.setCursor(0, 1);
            lcd.print(CTS);
        }
        else if (key == '2')
        {
            CTS = CTS + 0.5;

            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(CTS);
            lcd.print(CTS);
        }
    }
}

void MenuSetT()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi CT T");

    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(CTT);

        if (key == '8')
        {
            CTT = CTT - 0.5;
            if (CTT < 0)
            {
                CTT = 0;
            }
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(CTT);
            lcd.print(CTT);
        }
        else if (key == '2')
        {
            CTT = CTT + 0.5;
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(CTT);
            lcd.print(CTT);
        }
    }
}

void MenuSetRC1()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi R C1");

    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(RC1);

        if (key == '8')
        {
            RC1 = RC1 - 1;
            if (RC 1 < 0)
            {
                RC1 = 0;
            }
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC1);
            lcd.print(RC1);
        }
        else if (key == '2')
        {
            RC1 = RC1 + 1;
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC1);
            lcd.print(RC1);
        }
    }
}

void MenuSetRC2()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi R C2");
    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(RC2);

        if (key == '8')
        {
            RC2 = RC2 - 1;
            if (RC2 < 0)
            {
                RC2 = 0;
            }
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC2);
            lcd.print(RC2);
        }
        else if (key == '2')
        {
            RC2 = RC2 + 1;
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC2);
            lcd.print(RC2);
        }
    }
}

void MenuSetRC3()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kalibrasi R C3");
    char key;
    while (key != '4')
    {
        key = keypad.getKey();
        lcd.setCursor(0, 1);
        lcd.print(RC3);

        if (key == '8')
        {
            RC3 = RC3 - 1;
            if (RC3 < 0)
            {
                RC3 = 0;
            }
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC3);
            lcd.print(RC3);
        }
        else if (key == '2')
        {
            RC3 = RC3 + 1;
            lcd.setCursor(0, 1);
            lcd.print("                ");

            lcd.setCursor(0, 1);
            Serial.println(RC3);
            lcd.print(RC3);
        }
    }
}