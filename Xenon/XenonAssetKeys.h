#pragma once

// Centralized, human-readable asset keys for the Xenon game project.
//
// Why:
// - Avoid scattered string literals ("Ship2.bmp", "clone_sheet", etc.)
// - Make caching identity explicit (AssetManager caches by key)
// - Keep asset identity stable across scenes / spawns

namespace XenonAssetKeys {
	namespace Sheets {
		inline constexpr const char* Ship2 = "sheet.ship.ship2";
		inline constexpr const char* CompanionClone = "sheet.ally.clone";
		inline constexpr const char* EnemyProjectiles = "sheet.enemy.projectiles";
		inline constexpr const char* Missiles = "sheet.weapon.missiles";
	}

	namespace Textures {
		inline constexpr const char* CloneBmp = "tex.clone";
		inline constexpr const char* Ship2Bmp = "tex.ship2";
		inline constexpr const char* EnemyProjBmp = "tex.enemy_projectiles";
		inline constexpr const char* MissileBmp = "tex.missile";
		// Add more as you migrate other literals.
	}

	namespace Fonts {
		inline constexpr const char* Popup8x8 = "font.ui.popup8x8";
	}
}
