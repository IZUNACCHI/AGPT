#pragma once

#include "../GameEngine/GameEngine.h"
#include "ProjectileObjects.hpp"

#include <cmath>

// -----------------------------------------------------------------------------
// Launchers
// - Behaviours added to shooters
// - Spawns projectiles using:
//   - a local muzzle offset (rotates with shooter)
//   - a local fire direction (rotates with shooter)
// -----------------------------------------------------------------------------

class ProjectileLauncherBase : public MonoBehaviour {
public:
	explicit ProjectileLauncherBase(const std::string& name = "ProjectileLauncherBase") {
		SetComponentName(name);
	}

	void SetCooldown(float seconds) { m_cooldown = seconds; }
	float GetCooldown() const { return m_cooldown; }

	void SetProjectileSpeed(float speed) { m_projectileSpeed = speed; }
	float GetProjectileSpeed() const { return m_projectileSpeed; }

	void SetDamage(int damage) { m_damage = damage; }
	int GetDamage() const { return m_damage; }

	// Local offset from the shooter origin.
	void SetMuzzleOffset(const Vector2f& offset) { m_muzzleOffset = offset; }
	Vector2f GetMuzzleOffset() const { return m_muzzleOffset; }

protected:
	bool CooldownReady() const {
		return (Time::Now() - m_lastFireTime) >= m_cooldown;
	}

	void MarkFired() { m_lastFireTime = Time::Now(); }

	// NOTE: Transform world matrix is now correct (see Matrix3x3f fix).
	Vector2f GetMuzzleWorldPosition() const {
		return GetTransform()->GetWorldMatrix() * m_muzzleOffset;
	}

	// Convert a LOCAL direction to a WORLD direction using the shooter's rotation.
	Vector2f LocalDirToWorldDir(Vector2f localDir) const {
		if (localDir.LengthSquared() <= 0.0001f) {
			localDir = Vector2f(1.0f, 0.0f);
		}
		localDir = localDir.Normalized();
		return (Matrix3x3f::Rotation(GetTransform()->GetWorldRotation()) * localDir).Normalized();
	}

	static float DirToAngleDeg(const Vector2f& worldDir) {
		return std::atan2(worldDir.y, worldDir.x) * 180.0f / 3.14159265358979323846f;
	}

	float m_cooldown = 0.2f;
	float m_lastFireTime = -9999.0f;
	float m_projectileSpeed = 650.0f;
	int m_damage = 20;
	// Default muzzle is "below" the shooter in local space.
	Vector2f m_muzzleOffset = Vector2f(0.0f, -28.0f);
};

class PlayerProjectileLauncher : public ProjectileLauncherBase {
public:
	PlayerProjectileLauncher()
		: ProjectileLauncherBase("PlayerProjectileLauncher") {
	}

	void SetMissileType(MissileType type) { m_missileType = type; }
	MissileType GetMissileType() const { return m_missileType; }

	// Fire a missile in the direction of the shooter's **forward**.
	// Convention: at rotation 0 the shooter faces UP (local +Y).
	bool TryFireForward() {
		return TryFireLocal(Vector2f(0.0f, 1.0f));
	}

	// Fire a missile using a direction expressed in the shooter's LOCAL space.
	// (Useful for enemies or special weapons.)
	bool TryFireLocal(const Vector2f& localDirection) {
		if (!CooldownReady()) return false;

		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return false;

		auto proj = scene->CreateGameObject<PlayerProjectile>("PlayerProjectile");
		proj->GetTransform()->SetPosition(GetMuzzleWorldPosition());

		Vector2f worldDir = LocalDirToWorldDir(localDirection);
		// Missiles should inherit the shooter's orientation (ship faces up at rot 0).
		proj->GetTransform()->SetRotation(GetTransform()->GetWorldRotation());

		auto behaviour = proj->GetComponent<MissileBehaviour>().get();
		if (behaviour) {
			behaviour->SetMissileType(m_missileType);
			behaviour->SetDirection(worldDir);
			behaviour->SetSpeed(m_projectileSpeed);
			// If you want to override missile-type damage, set m_damage > 0.
			if (m_damage > 0) behaviour->SetDamage(m_damage);
		}

		MarkFired();
		return true;
	}

private:
	MissileType m_missileType = MissileType::Light;
};

class EnemyProjectileLauncher : public ProjectileLauncherBase {
public:
	EnemyProjectileLauncher()
		: ProjectileLauncherBase("EnemyProjectileLauncher") {
		m_cooldown = 2.0f;
		m_damage = 1;
		m_projectileSpeed = 500.0f;
	}

	// Fire in shooter LOCAL space.
	bool TryFire(const Vector2f& localDirection) {
		if (!CooldownReady()) return false;

		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return false;

		auto proj = scene->CreateGameObject<EnemyProjectile>("EnemyProjectile");
		proj->GetTransform()->SetPosition(GetMuzzleWorldPosition());

		Vector2f worldDir = LocalDirToWorldDir(localDirection);
		proj->GetTransform()->SetRotation(DirToAngleDeg(worldDir));

		auto behaviour = proj->GetComponent<EnemyProjectileSheetBehaviour>().get();
		if (behaviour) {
			behaviour->SetDirection(worldDir);
			behaviour->SetSpeed(m_projectileSpeed);
			behaviour->SetDamage(m_damage);
		}

		MarkFired();
		return true;
	}

	// Convenience: fire toward a world position WITHOUT rotating the shooter.
	// (So loners don't spin forever.)
	bool TryFireToward(const Vector2f& targetWorldPos) {
		Vector2f dir = targetWorldPos - GetTransform()->GetWorldPosition();
		if (dir.LengthSquared() <= 0.0001f) dir = Vector2f(1.0f, 0.0f);
		dir = dir.Normalized();
		return TryFireWorld(dir);
	}

	// Fire in world space (ignores shooter rotation for direction, but still uses
	// muzzle offset in local space).
	bool TryFireWorld(const Vector2f& worldDirection) {
		if (!CooldownReady()) return false;

		Scene* scene = GetGameObject()->GetScene();
		if (!scene) return false;

		auto proj = scene->CreateGameObject<EnemyProjectile>("EnemyProjectile");
		proj->GetTransform()->SetPosition(GetMuzzleWorldPosition());

		Vector2f worldDir = worldDirection;
		if (worldDir.LengthSquared() <= 0.0001f) worldDir = Vector2f(1.0f, 0.0f);
		worldDir = worldDir.Normalized();
		proj->GetTransform()->SetRotation(DirToAngleDeg(worldDir));

		auto behaviour = proj->GetComponent<EnemyProjectileSheetBehaviour>().get();
		if (behaviour) {
			behaviour->SetDirection(worldDir);
			behaviour->SetSpeed(m_projectileSpeed);
			behaviour->SetDamage(m_damage);
		}

		MarkFired();
		return true;
	}
};
