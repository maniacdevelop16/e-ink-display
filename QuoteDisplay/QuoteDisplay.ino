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

    Serial.println("DISPLAY READY");
    Serial.println();
    Serial.println("Rotary UP    -> Next quote");
    Serial.println("Rotary DOWN  -> Previous quote");
    Serial.println("Rotary PRESS -> Refresh quote");
}

void loop()
{
    updateUI();
    delay(10);
}
