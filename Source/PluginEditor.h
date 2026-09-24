#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

//==============================================================================
/** ツマミ 1 個ぶん（回転スライダー＋名前ラベル）。 */
class KnobControl : public juce::Component
{
public:
    KnobControl (const juce::String& labelText, juce::Colour accent);

    void resized() override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;

    juce::Slider slider;

private:
    void refreshText (bool showValue);

    juce::Label  nameLabel;
    juce::String name;
    bool showingValue = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobControl)
};

//==============================================================================
/** 選択式パラメータ（コンボボックス＋名前ラベル）。 */
class ChoiceControl : public juce::Component
{
public:
    explicit ChoiceControl (const juce::String& labelText);

    void resized() override;

    juce::ComboBox box;

private:
    juce::Label nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceControl)
};

//==============================================================================
/** モジュレーションマトリクスの 1 スロット（元 → 先 と量）。 */
class ModSlotControl : public juce::Component
{
public:
    explicit ModSlotControl (int slotNumber);

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::ComboBox srcBox, dstBox;
    juce::Slider   amountSlider;

    bool isActive() const;
    void refreshActive();

private:
    int number;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModSlotControl)
};

//==============================================================================
/** タイトル付きの枠。中に入れたコントロールをグリッドに自動配置する。 */
class Panel : public juce::Component
{
public:
    Panel (const juce::String& titleText, juce::Colour accent, int numColumns);

    /** 所有権を受け取る。 */
    template <typename T>
    T* add (T* c)
    {
        items.add (c);
        addAndMakeVisible (c);
        return c;
    }

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::String title;
    juce::Colour accentColour;
    int columns;
    juce::OwnedArray<juce::Component> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panel)
};

//==============================================================================
/** ウェーブテーブルの波形を表示するビジュアライザ。 */
class WavetableDisplay : public juce::Component, private juce::Timer
{
public:
    WavetableDisplay (AuroraWTAudioProcessor&);

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    void buildPath (juce::Path&, float framePos, juce::Rectangle<float> area,
                    float ampScale = 0.45f) const;

    AuroraWTAudioProcessor& proc;
    float lastPosA = -1.0f, lastPosB = -1.0f, lastLevelB = -1.0f;
    std::vector<float> scratch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableDisplay)
};

//==============================================================================
/** プリセットの選択バー（前へ／名前ボタン／次へ）。 */
class PresetBar : public juce::Component
{
public:
    explicit PresetBar (PresetManager&);

    void resized() override;
    void refresh();

private:
    void showMenu();
    void showSaveDialog();

    PresetManager& presets;
    juce::TextButton prevButton, nextButton, nameButton;
    std::unique_ptr<juce::AlertWindow> saveWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

//==============================================================================
/** 出力レベルメーター。 */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeter (AuroraWTAudioProcessor&);

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    AuroraWTAudioProcessor& proc;
    float smoothed = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

//==============================================================================
class AuroraWTAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AuroraWTAudioProcessorEditor (AuroraWTAudioProcessor&);
    ~AuroraWTAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int designWidth  = 1180;
    static constexpr int designHeight = 810;

    /** すべての UI を載せる固定サイズのコンテナ（拡大縮小は transform で行う）。 */
    class Content : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
    };

    void addKnob (Panel& panel, const juce::String& paramID,
                  const juce::String& label, juce::Colour accent);
    void addChoice (Panel& panel, const juce::String& paramID, const juce::String& label);
    void addModSlot (Panel& panel, int slot);

    AuroraWTAudioProcessor& proc;
    AuroraLookAndFeel lnf;

    Content content;

    Panel oscAPanel  { "OSCILLATOR A", Palette::accent,  4 };
    Panel oscBPanel  { "OSCILLATOR B", Palette::accent2, 4 };
    Panel filtPanel  { "FILTER",       Palette::accent3, 3 };
    Panel ampPanel   { "AMP ENV",      Palette::accent,  2 };
    Panel fegPanel   { "FILTER ENV",   Palette::accent3, 2 };
    Panel megPanel   { "MOD ENV",      Palette::accent2, 2 };
    Panel mixPanel   { "MIX / VOICE",  Palette::accent2, 3 };
    Panel lfo1Panel  { "LFO 1",        Palette::accent2, 3 };
    Panel lfo2Panel  { "LFO 2",        Palette::accent2, 3 };
    Panel chorPanel  { "CHORUS",       Palette::accent,  3 };
    Panel dlyPanel   { "DELAY",        Palette::accent,  3 };
    Panel revPanel   { "REVERB",       Palette::accent,  3 };
    Panel matrixPanel { "MODULATION MATRIX", Palette::accent3, 4 };

    WavetableDisplay display;
    LevelMeter meter;
    PresetBar presetBar;
    KnobControl masterKnob { "MASTER", Palette::accent3 };

    juce::MidiKeyboardComponent keyboard;

    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment>   sliderAttachments;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuroraWTAudioProcessorEditor)
};
