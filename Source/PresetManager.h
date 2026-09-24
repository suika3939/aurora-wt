#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/** プリセット 1 個ぶんの「パラメータ ID → 値」。ここに書かれていない項目は初期値になる。 */
struct PresetValue
{
    const char* id;
    float value;
};

struct FactoryPreset
{
    const char* name;
    const char* category;
    std::vector<PresetValue> values;
};

//==============================================================================
/**
    PresetManager
    -------------
    ・工場出荷プリセット（コードに埋め込み）の適用
    ・ユーザープリセットのディスク保存／読み込み
    ・現在のプリセット名の管理（プラグインの状態と一緒に保存される）
*/
class PresetManager
{
public:
    static constexpr const char* fileExtension = ".awtpreset";
    static constexpr const char* presetNameProperty = "presetName";

    explicit PresetManager (juce::AudioProcessorValueTreeState& stateToUse);

    //==========================================================================
    // 工場出荷プリセット
    static const std::vector<FactoryPreset>& getFactoryPresets();
    static int getNumFactoryPresets();
    static juce::String getFactoryPresetName (int index);

    /** index 番の工場出荷プリセットを読み込む。範囲外なら何もしない。 */
    void loadFactoryPreset (int index);

    /** 現在選ばれている工場出荷プリセットの番号。ユーザープリセット中は -1。 */
    int getCurrentFactoryIndex() const noexcept { return currentFactoryIndex; }

    //==========================================================================
    // ユーザープリセット
    juce::File getUserPresetDirectory() const;
    juce::StringArray getUserPresetNames() const;
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::String& name);
    bool deleteUserPreset (const juce::String& name);

    //==========================================================================
    /** 工場出荷 → ユーザーの順に並べた一覧の中で、ひとつ前／次を読み込む。 */
    void loadPrevious();
    void loadNext();

    juce::String getCurrentPresetName() const;

    /** setStateInformation のあとに呼ぶ。保存されていた名前から工場出荷番号を復元する。 */
    void refreshFromState();

    /** プリセットが切り替わったときに呼ばれる（GUI の表示更新用）。 */
    std::function<void()> onPresetChanged;

private:
    void setCurrentPresetName (const juce::String& name);
    void applyValues (const std::vector<PresetValue>& values);
    void resetAllToDefaults();

    juce::AudioProcessorValueTreeState& apvts;
    int currentFactoryIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
