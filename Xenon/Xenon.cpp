// Xenon.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SleeplessEngine.h>
#include "Level1.hpp"

int main()
{
    SleeplessEngine engine;
    engine.Start("Xenon 2000 Sleepless Edition", 800, 600);
	//make initial scene here
	auto level1 = std::make_unique<Level1>();
	SceneManager::SetActiveScene(std::move(level1));
	engine.Run();
    engine.Shutdown();
}


