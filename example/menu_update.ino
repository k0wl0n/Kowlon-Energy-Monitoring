#define UP 12
#define OK 10
#define DOWN 11
#define BACK 9

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 7;

int RC1 = 15, RC2 = 35, RC3 = 80;
float CTR = 170, CTS = 190, CTT = 150, CTP = 150;

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(OK, INPUT_PULLUP);
    pinMode(BACK, INPUT_PULLUP);
}

void loop()
{
    char key;
    boolean stat = true;

    //Serial.println(digitalRead(UP));
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
    if (key != mainMenuPageOld)
    {
        MainMenuDisplay();
        mainMenuPageOld = mainMenuPage;
    }

    if (key == 'O') //enter selected menu
    {
        switch (mainMenuPage)
        {
        case 1:
            MenuAmpere();
            break;
        case 2:
            CTR = MenuSet(CTR, "CTR");
            break;
        case 3:
            MenuSet(CTS, "CTS");
            break;
        case 4:
            MenuSet(CTT, "CTT");
            break;
        case 5:
            MenuSet(CTP, "CTP");
            break;
        case 6:
            MenuSetRC1();
            break;
        case 7:
            MenuSetRC2();
            break;
        case 8:
            MenuSetRC3();
            break;
        }
    }
}

char getPressedKey()
{
    char key;
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
        key = 'O';
    }
    if (digitalRead(BACK) == 0)
    {
        key = 'B';
    }
    return key;
}

void MenuAmpere()
{
    Serial.println("Menu Ampere");
}
float MenuSet(float value, String name)
{
    char key;
    while (key != 'B')
    {
        key = getPressedKey();
        Serial.println(name);
        Serial.println(value);
        // lcd.setCursor(0, 1);
        // lcd.print(name);
        if (key == 'D')
        {
            value = value - 0.5;
            if (value < 0)
            {
                value = 0;
            }
            // lcd.setCursor(0, 1);
            // lcd.print("                ");
            // lcd.setCursor(0, 1);
            Serial.println(name);
            Serial.println(value);
        }
        else if (key == 'U')
        {
            value = value + 0.5;
            Serial.println(name);
            Serial.println(value);
            // lcd.setCursor(0, 1);
            // lcd.print("                ");
            // lcd.setCursor(0, 1);
            // Serial.println(name);
            // lcd.print(name);
        }
    }
    return value;
}
void MenuSetRC1()
{
    char key;
    while (key != 'B')
    {
        key = getPressedKey();
        Serial.println("dalem while RC1");
    }
    Serial.println("keluar Menu RC1");
}
void MenuSetRC2()
{
    char key;
    while (key != 'B')
    {
        key = getPressedKey();
        Serial.println("dalem while RC2");
    }
    Serial.println("keluar Menu RC2");
}
void MenuSetRC3()
{
    char key;
    while (key != 'B')
    {
        key = getPressedKey();
        Serial.println("dalem while RC3");
    }
    Serial.println("keluar Menu RC3");
}

void MainMenuDisplay()
{
    //lcd.clear();
    //lcd.setCursor(0, 0);
    switch (mainMenuPage)
    {
    case 1:
        Serial.println("1.Monitor Arus");
        break;
    case 2:
        Serial.println("2.Kalibrasi CT R");
        break;
    case 3:
        Serial.println("3.Kalibrasi CT S");
        break;
    case 4:
        Serial.println("4.Kalibrasi CT T");
        break;
    case 5:
        Serial.println("5.Set Range C1");
        break;
    case 6:
        Serial.println("6.Set Range C2");
        break;
    case 7:
        Serial.println("7.Set Range C3");
        break;
    }
}