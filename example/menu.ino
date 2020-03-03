#include <Keypad.h>
#include <Wire.h>

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

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 7;

void setup()
{

    Serial.begin(9600);
    lcd.begin(20, 4);

    delay(1000);
    MainMenuDisplay();
    // MenuAmpere();
}

void loop()
{
    char key;
    boolean stat = true;

    while (stat)
    {
        key = keypad.getKey();
        if (key)
            stat = false;
    }
    // Serial.println(mainMenuPage);

    //draw ke atas key 2
    //draw ke bwawah key 8
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

    delay(500);
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
            if (RC1 < 0)
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