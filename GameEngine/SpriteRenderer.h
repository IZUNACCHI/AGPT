#pragma once
#include "Component.h"
#include "Texture.h"
#include "FRect.hpp"
#include "Renderer.h"

class SpriteRenderer : public Component {
public:
	Texture* texture = nullptr;
	FRect clipRect = { 0.f, 0.f, 0.f, 0.f }; 
	Flip flip = Flip::None;
	int sortingLayer = 0;     // render layer, higher value = drawn on top
	int sortingOrder = 0;     // in layer order, higher value = drawn on top
};