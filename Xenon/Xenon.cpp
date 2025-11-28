// Xenon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SleeplessEngine.h>

int main()
{
    SleeplessEngine engine;
    engine.Start("Xenon 2000 Sleepless Edition", 800, 600);
    engine.Shutdown();
}


