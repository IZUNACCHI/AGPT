#pragma once

#include "UISelectable.h"
#include "UIStyles.h"

#include <functional>
#include <string>

class UIButton : public UISelectable {
public:
    UIButton();
    ~UIButton() override = default;

    void SetText(const std::string& t) { m_text = t; }
    const std::string& GetText() const { return m_text; }

    void SetStyle(const UIButtonStyle& style) { m_style = style; }
    const UIButtonStyle& GetStyle() const { return m_style; }

    void SetOnClick(std::function<void()> fn) { m_onClick = std::move(fn); }

    // Pointer events
    void OnPointerEnter() override;
    void OnPointerExit() override;
    void OnPointerDown() override;
    void OnPointerUp() override;
    void OnSubmit() override;

    void Render(Renderer& renderer) override;

private:
    Vector4i GetBackgroundColor() const;

private:
    std::string m_text = "Button";
    UIButtonStyle m_style{};

    std::function<void()> m_onClick;

    bool m_hovered = false;
    bool m_pressed = false;
};
