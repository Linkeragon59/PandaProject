#pragma once

#include "Input.h"
#include <iostream>

struct GLFWwindow;

class GameWork
{
public:
    GameWork();
    ~GameWork();
    
    static void Create();
    static void Destroy();
    static GameWork* GetInstance() { return ourInstance; }
    
    void Run();
    void InitWindow();
    void Cleanup();

private:
    static GameWork* ourInstance;
    static Base::InputManager* myInputManager;
    GLFWwindow* myWindow = nullptr;
};
