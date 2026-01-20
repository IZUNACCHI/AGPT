#include "Behaviour.h"

#include "GameObject.h"

bool Behaviour::IsEligibleForUpdate() const {
	return IsEnabled() && GetGameObject() && GetGameObject()->IsActiveInHierarchy() && !IsPendingDestroy();
}