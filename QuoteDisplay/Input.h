#pragma once

enum InputEvent
{
    INPUT_NONE,
    INPUT_NEXT,
    INPUT_PREVIOUS,
    INPUT_PRESS,
    INPUT_LONG_PRESS
};

void initializeInput();
InputEvent readInputEvent();
