#pragma once

// Centralized asset keys + file names for Xenon.
// Use these instead of string literals scattered through the code.

namespace XenonAssetKeys {

	// AssetManager cache keys (mainly for SpriteSheets).
	namespace Sheets {
		inline constexpr const char* Ship2 = "sheet.ship.ship2";
		inline constexpr const char* Drone = "sheet.enemy.drone";
		inline constexpr const char* Loner = "sheet.enemy.loner";
		inline constexpr const char* Rusher = "sheet.enemy.rusher";

		inline constexpr const char* EnemyProjectiles = "sheet.enemy.projectiles";
		inline constexpr const char* Missiles = "sheet.weapon.missiles";

		inline constexpr const char* CompanionClone = "sheet.ally.clone";
	}

	// File names / relative paths (used by LoadTexture/LoadAudioClip/LoadBitmapFont/etc).
	namespace Files {
		// Player / allies
		inline constexpr const char* Ship2Bmp = "Ship2.bmp";
		inline constexpr const char* CloneBmp = "clone.bmp";

		// Enemies
		inline constexpr const char* DroneBmp = "drone.bmp";
		inline constexpr const char* LonerABmp = "LonerA.bmp";
		inline constexpr const char* RusherBmp = "rusher.bmp";

		// Projectiles / weapons
		inline constexpr const char* EnemyWeapBmp = "EnWeap6.bmp";
		inline constexpr const char* MissileBmp = "missile.bmp";

		// Background / tiles
		inline constexpr const char* GalaxyBmp = "galaxy2.bmp";
		inline constexpr const char* BlocksBmp = "Blocks.bmp";
		inline constexpr const char* LogoBmp = "Xlogo.bmp";

		// Fonts (UI)
		inline constexpr const char* Font8x8Bmp = "Font8x8.bmp";
		inline constexpr const char* Font16x16Bmp = "Font16x16.bmp";
	}

	namespace Fonts {
		inline constexpr const char* Popup8x8 = "font.popup.8x8";
		inline constexpr const char* UI8x8 = "font.ui.8x8";
		inline constexpr const char* UI16x16 = "font.ui.16x16";
	}


	// Audio file paths.
	namespace Audio {
		inline constexpr const char* GunWav = "gun.wav";
		// Add more as you migrate:
		// inline constexpr const char* ExplosionWav = "explosion.wav";
	}
}
