#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    using NRange = juce::NormalisableRange<float>;

    NRange timeRange (float lo, float hi, float skew = 0.35f)
    {
        return { lo, hi, 0.0001f, skew };
    }

    std::unique_ptr<juce::AudioParameterFloat> makeFloat (const juce::String& id,
                                                          const juce::String& name,
                                                          NRange range, float def,
                                                          const juce::String& unit = {})
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, def,
            juce::AudioParameterFloatAttributes().withLabel (unit));
    }
}

//==============================================================================
juce::StringArray getModSourceNames()
{
    return { "Off", "LFO 1", "LFO 2", "Amp Env", "Filter Env", "Mod Env",
             "Velocity", "Key Track", "Mod Wheel", "Aftertouch", "Random" };
}

juce::StringArray getModDestNames()
{
    return { "Off", "Pitch", "Pitch A", "Pitch B", "WT Pos A", "WT Pos B",
             "Level A", "Level B", "Detune A", "Detune B", "Sub Level", "Noise Level",
             "Cutoff", "Resonance", "Drive", "Amplitude", "Pan",
             "LFO 1 Rate", "LFO 2 Rate" };
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
AuroraWTAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addOsc = [&layout] (const juce::String& tag, const juce::String& label,
                             float defLevel, float defPos)
    {
        layout.add (makeFloat (tag + "_pos",    label + " Position", { 0.0f, 1.0f }, defPos));
        layout.add (makeFloat (tag + "_level",  label + " Level",    { 0.0f, 1.0f }, defLevel));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { tag + "_oct", 1 },  label + " Octave", -3, 3, 0));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { tag + "_semi", 1 }, label + " Semitone", -12, 12, 0));
        layout.add (makeFloat (tag + "_fine",   label + " Fine",     { -100.0f, 100.0f }, 0.0f, "cent"));
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { tag + "_uni", 1 },  label + " Unison", 1, 7, 1));
        layout.add (makeFloat (tag + "_detune", label + " Detune",   { 0.0f, 100.0f }, 20.0f, "cent"));
        layout.add (makeFloat (tag + "_spread", label + " Spread",   { 0.0f, 1.0f }, 0.6f));
    };

    addOsc ("oscA", "Osc A", 0.75f, 0.42f);
    addOsc ("oscB", "Osc B", 0.0f,  0.65f);

    layout.add (makeFloat (ParamID::subLevel,   "Sub Level",   { 0.0f, 1.0f }, 0.0f));
    layout.add (makeFloat (ParamID::noiseLevel, "Noise Level", { 0.0f, 1.0f }, 0.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::filtType, 1 }, "Filter Type",
        juce::StringArray { "LP 12", "LP 24", "HP 12", "HP 24", "BP 12", "BP 24" }, 1));

    layout.add (makeFloat (ParamID::filtCutoff, "Cutoff",     NRange (20.0f, 20000.0f, 0.0f, 0.25f), 3000.0f, "Hz"));
    layout.add (makeFloat (ParamID::filtRes,    "Resonance",  { 0.0f, 0.95f }, 0.25f));
    layout.add (makeFloat (ParamID::filtDrive,  "Drive",      { 1.0f, 8.0f },  1.0f));
    layout.add (makeFloat (ParamID::filtEnvAmt, "Filter Env", { -6.0f, 6.0f }, 2.0f, "oct"));
    layout.add (makeFloat (ParamID::filtKeyTrk, "Key Track",  { 0.0f, 1.0f },  0.3f));

    layout.add (makeFloat (ParamID::ampAttack,  "Amp Attack",  timeRange (0.001f, 8.0f), 0.005f, "s"));
    layout.add (makeFloat (ParamID::ampDecay,   "Amp Decay",   timeRange (0.001f, 8.0f), 0.6f,   "s"));
    layout.add (makeFloat (ParamID::ampSustain, "Amp Sustain", { 0.0f, 1.0f }, 0.75f));
    layout.add (makeFloat (ParamID::ampRelease, "Amp Release", timeRange (0.005f, 12.0f), 0.35f, "s"));

    layout.add (makeFloat (ParamID::fegAttack,  "Filt Attack",  timeRange (0.001f, 8.0f), 0.01f, "s"));
    layout.add (makeFloat (ParamID::fegDecay,   "Filt Decay",   timeRange (0.001f, 8.0f), 0.5f,  "s"));
    layout.add (makeFloat (ParamID::fegSustain, "Filt Sustain", { 0.0f, 1.0f }, 0.3f));
    layout.add (makeFloat (ParamID::fegRelease, "Filt Release", timeRange (0.005f, 12.0f), 0.4f, "s"));

    layout.add (makeFloat (ParamID::megAttack,  "Mod Attack",  timeRange (0.001f, 8.0f), 0.30f, "s"));
    layout.add (makeFloat (ParamID::megDecay,   "Mod Decay",   timeRange (0.001f, 8.0f), 1.00f, "s"));
    layout.add (makeFloat (ParamID::megSustain, "Mod Sustain", { 0.0f, 1.0f }, 0.50f));
    layout.add (makeFloat (ParamID::megRelease, "Mod Release", timeRange (0.005f, 12.0f), 0.80f, "s"));

    const juce::StringArray lfoShapes { "Sine", "Triangle", "Saw Up", "Saw Down", "Square", "S&H" };
    const juce::StringArray lfoTrig   { "Retrig", "Free" };

    layout.add (makeFloat (ParamID::lfo1Rate, "LFO 1 Rate", NRange (0.02f, 30.0f, 0.0f, 0.35f), 4.0f, "Hz"));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::lfo1Shape, 1 }, "LFO 1 Shape", lfoShapes, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::lfo1Trig, 1 }, "LFO 1 Trigger", lfoTrig, 0));

    layout.add (makeFloat (ParamID::lfo2Rate, "LFO 2 Rate", NRange (0.02f, 30.0f, 0.0f, 0.35f), 0.5f, "Hz"));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::lfo2Shape, 1 }, "LFO 2 Shape", lfoShapes, 1));
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::lfo2Trig, 1 }, "LFO 2 Trigger", lfoTrig, 0));

    layout.add (makeFloat (ParamID::glide, "Glide", timeRange (0.0f, 2.0f), 0.0f, "s"));

    layout.add (makeFloat (ParamID::chorusMix,   "Chorus Mix",   { 0.0f, 1.0f }, 0.25f));
    layout.add (makeFloat (ParamID::chorusRate,  "Chorus Rate",  NRange (0.05f, 8.0f, 0.0f, 0.4f), 0.8f, "Hz"));
    layout.add (makeFloat (ParamID::chorusDepth, "Chorus Depth", { 0.0f, 1.0f }, 0.4f));

    layout.add (makeFloat (ParamID::delayTime, "Delay Time",     NRange (10.0f, 1500.0f, 0.0f, 0.4f), 320.0f, "ms"));
    layout.add (makeFloat (ParamID::delayFb,   "Delay Feedback", { 0.0f, 0.95f }, 0.35f));
    layout.add (makeFloat (ParamID::delayMix,  "Delay Mix",      { 0.0f, 1.0f },  0.2f));

    layout.add (makeFloat (ParamID::reverbSize, "Reverb Size",    { 0.0f, 1.0f }, 0.6f));
    layout.add (makeFloat (ParamID::reverbDamp, "Reverb Damping", { 0.0f, 1.0f }, 0.5f));
    layout.add (makeFloat (ParamID::reverbMix,  "Reverb Mix",     { 0.0f, 1.0f }, 0.25f));

    layout.add (makeFloat (ParamID::masterGain, "Master", { -60.0f, 6.0f }, -6.0f, "dB"));

    // --- モジュレーションマトリクス ---
    const auto srcNames = getModSourceNames();
    const auto dstNames = getModDestNames();

    for (int i = 0; i < numModSlots; ++i)
    {
        const auto n = juce::String (i + 1);

        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { ParamID::modSrc (i), 1 }, "Mod " + n + " Source", srcNames, 0));
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { ParamID::modDst (i), 1 }, "Mod " + n + " Dest", dstNames, 0));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamID::modAmt (i), 1 }, "Mod " + n + " Amount",
            NRange (-1.0f, 1.0f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [] (float v, int) { return juce::String (v, 2); })));
    }

    return layout;
}

//==============================================================================
AuroraWTAudioProcessor::AuroraWTAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    cacheParameterPointers();

    // GUI が prepareToPlay より先に開かれても波形を描けるように、先に生成しておく
    wavetableBank.prepare (44100.0);

    synth.addSound (new WTSound());

    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new WTVoice (wavetableBank, params, controlState));

    synth.setNoteStealingEnabled (true);
}

void AuroraWTAudioProcessor::cacheParameterPointers()
{
    auto get = [this] (const juce::String& id) { return apvts.getRawParameterValue (id); };

    params.oscAPos    = get (ParamID::oscAPos);
    params.oscALevel  = get (ParamID::oscALevel);
    params.oscAOct    = get (ParamID::oscAOct);
    params.oscASemi   = get (ParamID::oscASemi);
    params.oscAFine   = get (ParamID::oscAFine);
    params.oscAUni    = get (ParamID::oscAUni);
    params.oscADetune = get (ParamID::oscADetune);
    params.oscASpread = get (ParamID::oscASpread);

    params.oscBPos    = get (ParamID::oscBPos);
    params.oscBLevel  = get (ParamID::oscBLevel);
    params.oscBOct    = get (ParamID::oscBOct);
    params.oscBSemi   = get (ParamID::oscBSemi);
    params.oscBFine   = get (ParamID::oscBFine);
    params.oscBUni    = get (ParamID::oscBUni);
    params.oscBDetune = get (ParamID::oscBDetune);
    params.oscBSpread = get (ParamID::oscBSpread);

    params.subLevel   = get (ParamID::subLevel);
    params.noiseLevel = get (ParamID::noiseLevel);

    params.filtType   = get (ParamID::filtType);
    params.filtCutoff = get (ParamID::filtCutoff);
    params.filtRes    = get (ParamID::filtRes);
    params.filtDrive  = get (ParamID::filtDrive);
    params.filtEnvAmt = get (ParamID::filtEnvAmt);
    params.filtKeyTrk = get (ParamID::filtKeyTrk);

    params.ampAttack  = get (ParamID::ampAttack);
    params.ampDecay   = get (ParamID::ampDecay);
    params.ampSustain = get (ParamID::ampSustain);
    params.ampRelease = get (ParamID::ampRelease);

    params.fegAttack  = get (ParamID::fegAttack);
    params.fegDecay   = get (ParamID::fegDecay);
    params.fegSustain = get (ParamID::fegSustain);
    params.fegRelease = get (ParamID::fegRelease);

    params.megAttack  = get (ParamID::megAttack);
    params.megDecay   = get (ParamID::megDecay);
    params.megSustain = get (ParamID::megSustain);
    params.megRelease = get (ParamID::megRelease);

    params.lfoRate[0]  = get (ParamID::lfo1Rate);
    params.lfoShape[0] = get (ParamID::lfo1Shape);
    params.lfoTrig[0]  = get (ParamID::lfo1Trig);
    params.lfoRate[1]  = get (ParamID::lfo2Rate);
    params.lfoShape[1] = get (ParamID::lfo2Shape);
    params.lfoTrig[1]  = get (ParamID::lfo2Trig);

    params.glide      = get (ParamID::glide);

    for (int i = 0; i < numModSlots; ++i)
    {
        params.mod[(size_t) i].src = get (ParamID::modSrc (i));
        params.mod[(size_t) i].dst = get (ParamID::modDst (i));
        params.mod[(size_t) i].amt = get (ParamID::modAmt (i));
    }
}

//==============================================================================
void AuroraWTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    wavetableBank.prepare (sampleRate);
    synth.setCurrentPlaybackSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax (1, samplesPerBlock);
    spec.numChannels      = 2;

    chorus.prepare (spec);
    chorus.reset();

    delayLine.prepare (spec);
    maxDelaySamples = juce::jmax (16, (int) (sampleRate * 1.6));
    delayLine.setMaximumDelayInSamples (maxDelaySamples);
    delayLine.reset();

    reverb.prepare (spec);
    reverb.reset();

    const double smoothTime = 0.03;
    delayTimeSmoothed.reset (sampleRate, 0.15);
    delayFbSmoothed  .reset (sampleRate, smoothTime);
    delayMixSmoothed .reset (sampleRate, smoothTime);
    chorusMixSmoothed.reset (sampleRate, smoothTime);
    masterGainSmoothed.reset (sampleRate, smoothTime);

    delayTimeSmoothed.setCurrentAndTargetValue (apvts.getRawParameterValue (ParamID::delayTime)->load()
                                                * 0.001f * (float) sampleRate);
    masterGainSmoothed.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (apvts.getRawParameterValue (ParamID::masterGain)->load()));

    keyboardState.reset();
}

void AuroraWTAudioProcessor::releaseResources()
{
    synth.allNotesOff (0, false);
}

bool AuroraWTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

//==============================================================================
void AuroraWTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    if (buffer.getNumChannels() < 2)
        return;                                        // モノ出力時は FX を通さない簡易処理

    // ---------------- Chorus ----------------
    {
        const float mix = apvts.getRawParameterValue (ParamID::chorusMix)->load();
        chorus.setRate   (apvts.getRawParameterValue (ParamID::chorusRate)->load());
        chorus.setDepth  (apvts.getRawParameterValue (ParamID::chorusDepth)->load());
        chorus.setCentreDelay (12.0f);
        chorus.setFeedback (0.2f);
        chorus.setMix (juce::jlimit (0.0f, 1.0f, mix));

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }

    // ---------------- Delay ----------------
    {
        const float timeMs = apvts.getRawParameterValue (ParamID::delayTime)->load();
        delayTimeSmoothed.setTargetValue (timeMs * 0.001f * (float) currentSampleRate);
        delayFbSmoothed  .setTargetValue (apvts.getRawParameterValue (ParamID::delayFb)->load());
        delayMixSmoothed .setTargetValue (apvts.getRawParameterValue (ParamID::delayMix)->load());

        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float d   = delayTimeSmoothed.getNextValue();
            const float fb  = delayFbSmoothed.getNextValue();
            const float mix = delayMixSmoothed.getNextValue();

            delayLine.setDelay (juce::jlimit (1.0f, (float) maxDelaySamples - 2.0f, d));

            const float dl = delayLine.popSample (0);
            const float dr = delayLine.popSample (1);

            // 左右をたすき掛けにして広がりを出す
            delayLine.pushSample (0, l[i] + dr * fb);
            delayLine.pushSample (1, r[i] + dl * fb);

            l[i] += dl * mix;
            r[i] += dr * mix;
        }
    }

    // ---------------- Reverb ----------------
    {
        juce::Reverb::Parameters rp;
        const float mix = apvts.getRawParameterValue (ParamID::reverbMix)->load();
        rp.roomSize   = apvts.getRawParameterValue (ParamID::reverbSize)->load();
        rp.damping    = apvts.getRawParameterValue (ParamID::reverbDamp)->load();
        rp.wetLevel   = mix;
        rp.dryLevel   = 1.0f - mix;
        rp.width      = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        reverb.process (ctx);
    }

    // ---------------- Master ----------------
    {
        masterGainSmoothed.setTargetValue (
            juce::Decibels::decibelsToGain (apvts.getRawParameterValue (ParamID::masterGain)->load()));

        float sumSquares = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            const float g = masterGainSmoothed.getNextValue();

            for (int ch = 0; ch < 2; ++ch)
            {
                float s = buffer.getSample (ch, i) * g;
                s = std::tanh (s * 1.2f) * 0.85f;      // やさしいソフトクリップ
                buffer.setSample (ch, i, s);
                sumSquares += s * s;
            }
        }

        const float rms = numSamples > 0 ? std::sqrt (sumSquares / (float) (numSamples * 2)) : 0.0f;
        const float prev = outputLevel.load();
        outputLevel.store (juce::jmax (rms, prev * 0.75f));   // 少しだけ余韻を持たせる
    }
}

//==============================================================================
juce::AudioProcessorEditor* AuroraWTAudioProcessor::createEditor()
{
    return new AuroraWTAudioProcessorEditor (*this);
}

void AuroraWTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void AuroraWTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        presetManager.refreshFromState();
    }
}

void AuroraWTAudioProcessor::setCurrentProgram (int index)
{
    // ホストが読み込み直後に program 0 を投げてくることがあるので、
    // 同じ番号なら何もしない（ユーザーの設定を消さないため）
    if (index != presetManager.getCurrentFactoryIndex())
        presetManager.loadFactoryPreset (index);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AuroraWTAudioProcessor();
}
