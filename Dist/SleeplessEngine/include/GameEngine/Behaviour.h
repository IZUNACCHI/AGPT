#pragma once

#include "Component.h"

// Component that can be enabled or disabled.
class Behaviour : public Component {
public:
	// Creates a Behaviour with the given name.
	explicit Behaviour(const std::string& name = "Behaviour");
	// Destroys the Behaviour.
	~Behaviour() override = default;

	// Returns whether the component is enabled.
	bool IsEnabled() const { return m_enabled; }
	// Sets whether the component is enabled.
	void SetEnabled(bool enabled);

	// Returns true if the component is active and enabled in hierarchy.
	bool IsActiveAndEnabled() const;

protected:
	// Returns whether OnEnable has already been called.
	virtual bool HasOnEnableBeenCalled() const;
	// Responds to enabled state changes.
	virtual void OnEnabledStateChanged(bool enabled);

private:
	// Whether the component is enabled.
	bool m_enabled = true;
};