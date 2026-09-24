#pragma once

#include <JuceHeader.h>
#include <array>

//==============================================================================
/** モジュレーションの「元」。 */
enum class ModSource
{
    off = 0,
    lfo1,          // -1..1
    lfo2,          // -1..1
    ampEnv,        //  0..1
    filtEnv,       //  0..1
    modEnv,        //  0..1
    velocity,      //  0..1
    keyTrack,      // -1..1（C3 を中心に ±4 オクターブ）
    modWheel,      //  0..1
    aftertouch,    //  0..1
    randomPerNote, // -1..1（発音ごとに決まる乱数）
    numSources
};

/** モジュレーションの「先」。 */
enum class ModDest
{
    off = 0,
    pitchAll,      // 両オシレータのピッチ  ±24 半音
    pitchA,        // オシレータ A のピッチ ±24 半音
    pitchB,        // オシレータ B のピッチ ±24 半音
    posA,          // ウェーブテーブル位置 A ±1.0
    posB,          // ウェーブテーブル位置 B ±1.0
    levelA,        // ±1.0
    levelB,        // ±1.0
    detuneA,       // ±100 セント
    detuneB,       // ±100 セント
    subLevel,      // ±1.0
    noiseLevel,    // ±1.0
    cutoff,        // ±6 オクターブ
    resonance,     // ±1.0
    drive,         // ±7
    amplitude,     // ±1.0（乗算）
    pan,           // ±1.0
    lfo1Rate,      // ±4 オクターブ
    lfo2Rate,      // ±4 オクターブ
    numDests
};

inline constexpr int numModSlots = 8;
inline constexpr int numModSources = (int) ModSource::numSources;
inline constexpr int numModDests   = (int) ModDest::numDests;

//==============================================================================
/** パラメータ ID を一箇所にまとめておく（processor と editor で共有）。 */
namespace ParamID
{
    // --- Oscillator A ---
    static const juce::String oscAPos     { "oscA_pos" };
    static const juce::String oscALevel   { "oscA_level" };
    static const juce::String oscAOct     { "oscA_oct" };
    static const juce::String oscASemi    { "oscA_semi" };
    static const juce::String oscAFine    { "oscA_fine" };
    static const juce::String oscAUni     { "oscA_uni" };
    static const juce::String oscADetune  { "oscA_detune" };
    static const juce::String oscASpread  { "oscA_spread" };

    // --- Oscillator B ---
    static const juce::String oscBPos     { "oscB_pos" };
    static const juce::String oscBLevel   { "oscB_level" };
    static const juce::String oscBOct     { "oscB_oct" };
    static const juce::String oscBSemi    { "oscB_semi" };
    static const juce::String oscBFine    { "oscB_fine" };
    static const juce::String oscBUni     { "oscB_uni" };
    static const juce::String oscBDetune  { "oscB_detune" };
    static const juce::String oscBSpread  { "oscB_spread" };

    // --- Sub / Noise ---
    static const juce::String subLevel    { "sub_level" };
    static const juce::String noiseLevel  { "noise_level" };

    // --- Filter ---
    static const juce::String filtType    { "filt_type" };
    static const juce::String filtCutoff  { "filt_cutoff" };
    static const juce::String filtRes     { "filt_res" };
    static const juce::String filtDrive   { "filt_drive" };
    static const juce::String filtEnvAmt  { "filt_envamt" };
    static const juce::String filtKeyTrk  { "filt_keytrack" };

    // --- Amp envelope ---
    static const juce::String ampAttack   { "amp_attack" };
    static const juce::String ampDecay    { "amp_decay" };
    static const juce::String ampSustain  { "amp_sustain" };
    static const juce::String ampRelease  { "amp_release" };

    // --- Filter envelope ---
    static const juce::String fegAttack   { "feg_attack" };
    static const juce::String fegDecay    { "feg_decay" };
    static const juce::String fegSustain  { "feg_sustain" };
    static const juce::String fegRelease  { "feg_release" };

    // --- Mod envelope ---
    static const juce::String megAttack   { "meg_attack" };
    static const juce::String megDecay    { "meg_decay" };
    static const juce::String megSustain  { "meg_sustain" };
    static const juce::String megRelease  { "meg_release" };

    // --- LFO 1 / 2 ---
    static const juce::String lfo1Rate    { "lfo1_rate" };
    static const juce::String lfo1Shape   { "lfo1_shape" };
    static const juce::String lfo1Trig    { "lfo1_trig" };
    static const juce::String lfo2Rate    { "lfo2_rate" };
    static const juce::String lfo2Shape   { "lfo2_shape" };
    static const juce::String lfo2Trig    { "lfo2_trig" };

    // --- Voice ---
    static const juce::String glide       { "glide" };

    // --- FX ---
    static const juce::String chorusMix   { "chorus_mix" };
    static const juce::String chorusRate  { "chorus_rate" };
    static const juce::String chorusDepth { "chorus_depth" };
    static const juce::String delayTime   { "delay_time" };
    static const juce::String delayFb     { "delay_fb" };
    static const juce::String delayMix    { "delay_mix" };
    static const juce::String reverbSize  { "reverb_size" };
    static const juce::String reverbDamp  { "reverb_damp" };
    static const juce::String reverbMix   { "reverb_mix" };

    // --- Master ---
    static const juce::String masterGain  { "master_gain" };

    // --- Modulation matrix（slot は 0 起点） ---
    inline juce::String modSrc (int slot) { return "mod" + juce::String (slot + 1) + "_src"; }
    inline juce::String modDst (int slot) { return "mod" + juce::String (slot + 1) + "_dst"; }
    inline juce::String modAmt (int slot) { return "mod" + juce::String (slot + 1) + "_amt"; }
}

//==============================================================================
/**
    モジュレーションホイールとチャンネルプレッシャーの現在値。

    JUCE の Synthesiser は「鳴っているボイス」にしかコントローラを配らないので、
    ホイールを上げてから鍵盤を押した場合に新しい音へ値が伝わらない。
    シンセ側で常に最新値を持っておき、発音中のボイスがここを読む。
*/
struct MidiControlState
{
    std::atomic<float> modWheel        { 0.0f };
    std::atomic<float> channelPressure { 0.0f };
};

//==============================================================================
/** モジュレーションの元・先・量を表示するための名前。 */
juce::StringArray getModSourceNames();
juce::StringArray getModDestNames();

//==============================================================================
/** 音を出すボイスが読む、パラメータへの生ポインタ束。 */
struct SynthParams
{
    struct ModSlot
    {
        std::atomic<float>* src = nullptr;
        std::atomic<float>* dst = nullptr;
        std::atomic<float>* amt = nullptr;
    };

    std::atomic<float>* oscAPos     = nullptr;
    std::atomic<float>* oscALevel   = nullptr;
    std::atomic<float>* oscAOct     = nullptr;
    std::atomic<float>* oscASemi    = nullptr;
    std::atomic<float>* oscAFine    = nullptr;
    std::atomic<float>* oscAUni     = nullptr;
    std::atomic<float>* oscADetune  = nullptr;
    std::atomic<float>* oscASpread  = nullptr;

    std::atomic<float>* oscBPos     = nullptr;
    std::atomic<float>* oscBLevel   = nullptr;
    std::atomic<float>* oscBOct     = nullptr;
    std::atomic<float>* oscBSemi    = nullptr;
    std::atomic<float>* oscBFine    = nullptr;
    std::atomic<float>* oscBUni     = nullptr;
    std::atomic<float>* oscBDetune  = nullptr;
    std::atomic<float>* oscBSpread  = nullptr;

    std::atomic<float>* subLevel    = nullptr;
    std::atomic<float>* noiseLevel  = nullptr;

    std::atomic<float>* filtType    = nullptr;
    std::atomic<float>* filtCutoff  = nullptr;
    std::atomic<float>* filtRes     = nullptr;
    std::atomic<float>* filtDrive   = nullptr;
    std::atomic<float>* filtEnvAmt  = nullptr;
    std::atomic<float>* filtKeyTrk  = nullptr;

    std::atomic<float>* ampAttack   = nullptr;
    std::atomic<float>* ampDecay    = nullptr;
    std::atomic<float>* ampSustain  = nullptr;
    std::atomic<float>* ampRelease  = nullptr;

    std::atomic<float>* fegAttack   = nullptr;
    std::atomic<float>* fegDecay    = nullptr;
    std::atomic<float>* fegSustain  = nullptr;
    std::atomic<float>* fegRelease  = nullptr;

    std::atomic<float>* megAttack   = nullptr;
    std::atomic<float>* megDecay    = nullptr;
    std::atomic<float>* megSustain  = nullptr;
    std::atomic<float>* megRelease  = nullptr;

    std::atomic<float>* lfoRate[2]  { nullptr, nullptr };
    std::atomic<float>* lfoShape[2] { nullptr, nullptr };
    std::atomic<float>* lfoTrig[2]  { nullptr, nullptr };

    std::atomic<float>* glide       = nullptr;

    std::array<ModSlot, numModSlots> mod {};
};
