#pragma once

#include <memory>
#include <vector>

// Engine headers
#include "AssetManager.h"
#include "Audio.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "BitmapFont.h"
#include "Collider2D.h"
#include "Component.h"
#include "GameObject.h"
#include "Input.h"
#include "MonoBehaviour.h"
#include "ObjectPool.h"
#include "Renderer.h"
#include "Rigidbody2D.h"
#include "Scene.h"
#include "SleeplessEngine.h"
#include "SpriteRenderer.h"
#include "SpriteSheet.h"
#include "TextRenderer.h"
#include "Texture.h"
#include "Time.hpp"
#include "Transform.h"
#include "Types.hpp"
#include "ViewportUtils.h"
#include "Window.h"

// UI module
#include "UISystem.h"
#include "UICanvas.h"
#include "UIElement.h"
#include "UISelectable.h"
#include "UIButton.h"
#include "UIProgressBar.h"
#include "UIStyles.h"
#include "UIPanel.h"

//asset loading shortcuts
inline Texture* LoadTexture(const std::string& relativePath) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadTexture(relativePath) : nullptr;
}

inline Texture* LoadTexture(const std::string& relativePath, const Vector3i& colorKey) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadTexture(relativePath, colorKey) : nullptr;
}

// SpriteSheet loading shortcuts
inline SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(textureRelativePath, frameSize) : nullptr;
}

inline SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(textureRelativePath, frameSize, colorKey) : nullptr;
}


inline SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(textureRelativePath, frameSize, textureScaleModeOverride) : nullptr;
}

inline SpriteSheet* LoadSpriteSheet(const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(textureRelativePath, frameSize, colorKey, textureScaleModeOverride) : nullptr;
}

inline SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(sheetKey, textureRelativePath, frameSize) : nullptr;
}

inline SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(sheetKey, textureRelativePath, frameSize, colorKey) : nullptr;
}


inline SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, TextureScaleMode textureScaleModeOverride) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(sheetKey, textureRelativePath, frameSize, textureScaleModeOverride) : nullptr;
}

inline SpriteSheet* LoadSpriteSheet(const std::string& sheetKey, const std::string& textureRelativePath, const Vector2i& frameSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadSpriteSheet(sheetKey, textureRelativePath, frameSize, colorKey, textureScaleModeOverride) : nullptr;
}

// Bitmap font shortcuts 
inline BitmapFont* LoadBitmapFont(const std::string& textureRelativePath, const Vector2i& glyphSize, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(textureRelativePath, glyphSize, firstChar) : nullptr;
}

// Keyed bitmap font shortcuts
inline BitmapFont* LoadBitmapFont(const std::string& fontKey, const std::string& textureRelativePath, const Vector2i& glyphSize, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(fontKey, textureRelativePath, glyphSize, firstChar) : nullptr;
}

inline BitmapFont* LoadBitmapFont(const std::string& textureRelativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(textureRelativePath, glyphSize, colorKey, firstChar) : nullptr;
}

inline BitmapFont* LoadBitmapFont(const std::string& fontKey, const std::string& textureRelativePath, const Vector2i& glyphSize, const Vector3i& colorKey, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(fontKey, textureRelativePath, glyphSize, colorKey, firstChar) : nullptr;
}

inline BitmapFont* LoadBitmapFont(const std::string& textureRelativePath, const Vector2i& glyphSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(textureRelativePath, glyphSize, firstChar, colorKey, textureScaleModeOverride) : nullptr;
}

inline BitmapFont* LoadBitmapFont(const std::string& fontKey, const std::string& textureRelativePath, const Vector2i& glyphSize, const Vector3i& colorKey, TextureScaleMode textureScaleModeOverride, unsigned char firstChar = 32) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadFont(fontKey, textureRelativePath, glyphSize, firstChar, colorKey, textureScaleModeOverride) : nullptr;
}

// Audio clip shortcut
inline AudioClip* LoadAudioClip(const std::string& relativePath) {
	auto* assetManager = SleeplessEngine::GetInstance().GetAssetManager();
	return assetManager ? assetManager->LoadAudioClip(relativePath) : nullptr;
}



// Input shortcuts
inline KeyState GetKey(Key key) { return Input::GetKey(key); }
inline bool IsKeyDown(Key key) { return Input::IsKeyDown(key); }
inline bool IsKeyPressed(Key key) { return Input::IsKeyPressed(key); }
inline bool IsKeyReleased(Key key) { return Input::IsKeyReleased(key); }

inline ButtonState GetMouseButton(MouseButton button) { return Input::GetMouseButton(button); }
inline bool IsMouseButtonDown(MouseButton button) { return Input::IsMouseButtonDown(button); }
inline bool IsMouseButtonPressed(MouseButton button) { return Input::IsMouseButtonPressed(button); }
inline bool IsMouseButtonReleased(MouseButton button) { return Input::IsMouseButtonReleased(button); }

inline float GetMouseX() { return Input::GetMouseX(); }
inline float GetMouseY() { return Input::GetMouseY(); }
inline Vector2f GetMousePosition() { return Input::GetMousePosition(); }
inline float GetMouseDeltaX() { return Input::GetMouseDeltaX(); }
inline float GetMouseDeltaY() { return Input::GetMouseDeltaY(); }
inline Vector2f GetMouseDelta() { return Input::GetMouseDelta(); }
inline float GetScrollX() { return Input::GetScrollX(); }
inline float GetScrollY() { return Input::GetScrollY(); }
inline Vector2f GetScroll() { return Input::GetScroll(); }

inline const std::vector<GamepadState>& GetGamepads() { return Input::GetGamepads(); }
inline int GetGamepadCount() { return Input::GetGamepadCount(); }
inline bool IsGamepadConnected(int index = 0) { return Input::IsGamepadConnected(index); }
inline bool IsGamepadButtonDown(GamepadButton button, int index = 0) { return Input::IsGamepadButtonDown(button, index); }
inline bool IsGamepadButtonPressed(GamepadButton button, int index = 0) { return Input::IsGamepadButtonPressed(button, index); }
inline bool IsGamepadButtonReleased(GamepadButton button, int index = 0) { return Input::IsGamepadButtonReleased(button, index); }
inline float GetGamepadAxis(GamepadAxis axis, int index = 0) { return Input::GetGamepadAxis(axis, index); }
inline Vector2f GetGamepadLeftStick(int index = 0) { return Input::GetGamepadLeftStick(index); }
inline Vector2f GetGamepadRightStick(int index = 0) { return Input::GetGamepadRightStick(index); }

//window shortcuts

inline Window* GetWindow() {
	return SleeplessEngine::GetInstance().GetWindow();
}


//renderer shortcuts
inline Renderer* GetRenderer() {
	return SleeplessEngine::GetInstance().GetRenderer();
}

//Time shortcuts
inline float DeltaTime() {
	return Time::DeltaTime();
}
inline float FixedDeltaTime() {
	return Time::FixedDeltaTime();
}