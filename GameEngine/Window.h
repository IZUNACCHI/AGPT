#pragma once
#include <memory>
#include <string>

class Window
{
public:
	Window(const std::string & title, int w, int h);
	~Window();

	int Width() const;
	int Height() const;

	void* GetNative() const; 

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

