#define UP 12
#define OK 10
#define DOWN 11
#define BACK 9

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 7;

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

    if (key == 'O')
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
void MenuSetR()
{
    getPressedKey();
    Serial.println("Menu R");
}
void MenuSetS()
{
    Serial.println("Menu S");
}
void MenuSetT()
{
    Serial.println("Menu T");
}
void MenuSetRC1()
{
    Serial.println("Menu RC1");
}
void MenuSetRC2()
{
    Serial.println("Menu RC2");
}
void MenuSetRC3()
{
    Serial.println("Menu RC3");
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