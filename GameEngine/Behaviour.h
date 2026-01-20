#pragma once

#include "Component.h"

// MonoBehaviour-like component. Lives in the Scene update list.
class Behaviour : public Component {
public:
	explicit Behaviour(std::string name = "")
		: Component(std::move(name)) {
	}
	virtual ~Behaviour() = default;

	bool IsEligibleForUpdate() const;
};