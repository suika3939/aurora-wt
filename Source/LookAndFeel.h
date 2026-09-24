#pragma once

#include <JuceHeader.h>

//==============================================================================
/** 配色を一箇所にまとめたもの。 */
namespace Palette
{
    const juce::Colour bgTop      { 0xff11141d };
    const juce::Colour bgBottom   { 0xff0a0c12 };
    const juce::Colour panel      { 0xff1a1f2e };
    const juce::Colour panelEdge  { 0xff2a3145 };
    const juce::Colour text       { 0xffc8d2e6 };
    const juce::Colour textDim    { 0xff7b869c };
    const juce::Colour accent     { 0xff35e0c6 };   // ティール
    const juce::Colour accent2    { 0xff9a6bff };   // パープル
    const juce::Colour accent3    { 0xffff6ba6 };   // ピンク
    const juce::Colour track      { 0xff272e42 };
}

//==============================================================================
/** ツマミ・コンボボックス・ラベルの描画を差し替えるカスタム LookAndFeel。 */
class AuroraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AuroraLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool isHighlighted, bool isDown) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getLabelFont (juce::Label&) override;

    /** ツマミの色。スライダーの colourId で切り替える。 */
    static juce::Colour accentFor (const juce::Slider&);
};
