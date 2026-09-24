#pragma once

#include <JuceHeader.h>
#include "Wavetable.h"
#include "SynthVoice.h"
#include "Parameters.h"
#include "PresetManager.h"

//==============================================================================
/** モジュレーションホイールとチャンネルプレッシャーを常に拾う Synthesiser。 */
class WTSynthesiser : public juce::Synthesiser
{
public:
    explicit WTSynthesiser (MidiControlState& s) : controls (s) {}

    void handleController (int midiChannel, int controllerNumber, int value) override
    {
        if (controllerNumber == 1)
            controls.modWheel.store ((float) value / 127.0f);

        juce::Synthesiser::handleController (midiChannel, controllerNumber, value);
    }

    void handleChannelPressure (int midiChannel, int value) override
    {
        controls.channelPressure.store ((float) value / 127.0f);
        juce::Synthesiser::handleChannelPressure (midiChannel, value);
    }

private:
    MidiControlState& controls;
};

//==============================================================================
class AuroraWTAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numVoices = 16;

    AuroraWTAudioProcessor();
    ~AuroraWTAudioProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                        { return true; }

    const juce::String getName() const override            { return "Aurora WT"; }
    bool acceptsMidi() const override                      { return true; }
    bool producesMidi() const override                     { return false; }
    bool isMidiEffect() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 4.0; }

    // DAW 側のプリセットメニューからも工場出荷プリセットを選べるようにする
    int getNumPrograms() override                          { return PresetManager::getNumFactoryPresets(); }
    int getCurrentProgram() override                       { return juce::jmax (0, presetManager.getCurrentFactoryIndex()); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override { return PresetManager::getFactoryPresetName (index); }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    WavetableBank wavetableBank;
    PresetManager presetManager { apvts };

    /** GUI のメーター表示用（-1..1 の RMS 相当）。 */
    std::atomic<float> outputLevel { 0.0f };

    /** GUI の鍵盤表示用。 */
    juce::MidiKeyboardState keyboardState;

private:
    void cacheParameterPointers();

    MidiControlState controlState;
    WTSynthesiser synth { controlState };
    SynthParams params;

    juce::dsp::Chorus<float>    chorus;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 96000 };
    juce::dsp::Reverb           reverb;

    juce::SmoothedValue<float> delayTimeSmoothed, delayFbSmoothed, delayMixSmoothed;
    juce::SmoothedValue<float> chorusMixSmoothed, masterGainSmoothed;

    double currentSampleRate = 44100.0;
    int    maxDelaySamples   = 96000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuroraWTAudioProcessor)
};
