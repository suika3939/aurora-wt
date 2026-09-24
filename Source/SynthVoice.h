#pragma once

#include <JuceHeader.h>
#include "Wavetable.h"
#include "Parameters.h"

//==============================================================================
/** このシンセが鳴らせる音域を表すだけの Sound クラス（全ノート・全チャンネル対応）。 */
struct WTSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
/**
    WTVoice
    -------
    1 音ぶんの発音を担当するクラス。

      ・ウェーブテーブルオシレータ × 2（各最大 7 声ユニゾン／ステレオ拡がり付き）
      ・サブオシレータ（1 オクターブ下のサイン波）／ホワイトノイズ
      ・ラダーフィルタ（LP12 / LP24 / HP12 / HP24 / BP12 / BP24）
      ・エンベロープ 3 基（アンプ／フィルタ／モジュレーション専用）
      ・LFO 2 基（波形 6 種・ノートごとにリセットするか選択可）
      ・8 スロットのモジュレーションマトリクス
      ・ポルタメント（グライド）
*/
class WTVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int maxUnison   = 7;
    static constexpr int controlRate = 16;   // 何サンプルごとにモジュレーションを更新するか

    WTVoice (WavetableBank& bankToUse, const SynthParams& paramsToUse,
             const MidiControlState& controlsToUse);

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<WTSound*> (s) != nullptr;
    }

    void setCurrentPlaybackSampleRate (double newRate) override;

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    void aftertouchChanged (int newAftertouchValue) override;
    void channelPressureChanged (int newChannelPressureValue) override;

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;

private:
    struct UnisonOsc
    {
        double phase[maxUnison]   {};
        double increment[maxUnison] {};
        float  gainL[maxUnison]   {};
        float  gainR[maxUnison]   {};
        int    mip[maxUnison]     {};
        int    count = 1;
        float  normalise = 1.0f;
    };

    void updateModulation (int numSamplesThisBlock);
    void evaluateMatrix();
    void configureOsc (UnisonOsc& osc, float baseFreq, int unisonCount,
                       float detuneCents, float spread);
    float lfoValueFor (int index) const;

    WavetableBank&          bank;
    const SynthParams&      p;
    const MidiControlState& controls;

    juce::AudioBuffer<float> workBuffer;
    juce::dsp::LadderFilter<float> ladder;

    juce::ADSR ampEnv, filtEnv, modEnv;
    juce::ADSR::Parameters ampEnvParams, filtEnvParams, modEnvParams;

    UnisonOsc oscA, oscB;

    double subPhase = 0.0, subIncrement = 0.0;
    juce::Random noiseRandom;

    // --- LFO ---
    double lfoPhase[2] { 0.0, 0.0 };
    float  lfoSampleHold[2] { 0.0f, 0.0f };
    float  lfoOut[2] { 0.0f, 0.0f };
    float  lfoRateMul[2] { 1.0f, 1.0f };     // 前のブロックで算出したレート変調

    // --- モジュレーションの元と先 ---
    float modSourceValue[numModSources] {};
    float modDestValue[numModDests] {};

    float lastAmpEnvValue = 0.0f;
    float polyAftertouch = 0.0f;      // このノート個別のアフタータッチ
    float noteRandom = 0.0f;

    float currentNoteFreq = 440.0f;   // グライド適用後の周波数
    float targetNoteFreq  = 440.0f;

    int   noteNumber = 60;
    float velocityValue = 1.0f;
    float velocityGain = 1.0f;
    float pitchBendSemis = 0.0f;
    bool  hasPlayedBefore = false;

    // 制御レートで更新される値
    float modPosA = 0.0f, modPosB = 0.0f;
    float modCutoff = 1000.0f;
    float modAmp = 1.0f;
    float panGainL = 1.0f, panGainR = 1.0f;
    float levelA = 0.5f, levelB = 0.0f, levelSub = 0.0f, levelNoise = 0.0f;

    JUCE_LEAK_DETECTOR (WTVoice)
};
