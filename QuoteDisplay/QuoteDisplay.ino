#include "Display.h"
#include "Input.h"
#include "UI.h"

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" CrowPanel Quote Display");
    Serial.println("==============================");

    initializeInput();
    initializeDisplay();
    initializeUI();

    Serial.println("MENU READY");
    Serial.println();
    Serial.println("Menu UP/DOWN    -> Select app");
    Serial.println("Menu PRESS      -> Open app");
    Serial.println("Quote UP/DOWN   -> Change quote");
    Serial.println("Quote PRESS     -> Refresh quote");
    Serial.println("Quote LONG PRESS-> Return to menu");
}

void loop()
{
    updateUI();
    delay(10);
}
