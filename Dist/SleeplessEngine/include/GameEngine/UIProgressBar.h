#pragma once

#include "UIElement.h"
#include "UIStyles.h"

class UIProgressBar : public UIElement {
public:
    UIProgressBar();
    ~UIProgressBar() override = default;

    void SetStyle(const UIProgressBarStyle& style) { m_style = style; }
    const UIProgressBarStyle& GetStyle() const { return m_style; }

    void SetValue(float v);
    float GetValue() const { return m_value; }

    void Render(Renderer& renderer) override;

private:
    float m_value = 1.0f; // 0..1
    UIProgressBarStyle m_style{};
};
