
#include <GLFW/glfw3.h>
#include "Input.h"
InputManager* InputManager::ourInstance = nullptr;

bool InputManager::PollInput(MouseInput anInput)
{
    if(anInput == 1)
        return true;
    else
        return false;
}
