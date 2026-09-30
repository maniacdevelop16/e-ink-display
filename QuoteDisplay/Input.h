#pragma once

enum InputEvent
{
    INPUT_NONE,
    INPUT_NEXT,
    INPUT_PREVIOUS,
    INPUT_PRESS,
    INPUT_MENU,
    INPUT_EXIT
};

void initializeInput();
InputEvent readInputEvent();
