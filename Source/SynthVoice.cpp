#include "SynthVoice.h"

namespace
{
    inline float midiToFreq (float noteWithFraction) noexcept
    {
        return 440.0f * std::pow (2.0f, (noteWithFraction - 69.0f) / 12.0f);
    }

    // 各デスティネーションの「変調量 1.0 のときの効き幅」
    constexpr float depthPitch   = 24.0f;    // 半音
    constexpr float depthDetune  = 100.0f;   // セント
    constexpr float depthCutoff  = 6.0f;     // オクターブ
    constexpr float depthDrive   = 7.0f;
    constexpr float depthLfoRate = 4.0f;     // オクターブ
}

//==============================================================================
WTVoice::WTVoice (WavetableBank& bankToUse, const SynthParams& paramsToUse,
                  const MidiControlState& controlsToUse)
    : bank (bankToUse), p (paramsToUse), controls (controlsToUse)
{
    workBuffer.setSize (2, 512);
    noiseRandom.setSeedRandomly();
}

void WTVoice::setCurrentPlaybackSampleRate (double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);

    if (newRate <= 0.0)
        return;

    ampEnv .setSampleRate (newRate);
    filtEnv.setSampleRate (newRate);
    modEnv .setSampleRate (newRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = newRate;
    spec.maximumBlockSize = (juce::uint32) controlRate;
    spec.numChannels      = 2;

    ladder.prepare (spec);
    ladder.setEnabled (true);
    ladder.reset();
}

//==============================================================================
void WTVoice::startNote (int midiNoteNumber, float velocity,
                         juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    noteNumber = midiNoteNumber;
    velocityValue = juce::jlimit (0.0f, 1.0f, velocity);
    velocityGain = 0.15f + 0.85f * std::pow (velocityValue, 1.2f);
    pitchBendSemis = ((float) currentPitchWheelPosition - 8192.0f) / 8192.0f * 2.0f;
    noteRandom = noiseRandom.nextFloat() * 2.0f - 1.0f;
    polyAftertouch = 0.0f;

    targetNoteFreq = midiToFreq ((float) noteNumber + pitchBendSemis);

    const float glideTime = p.glide != nullptr ? p.glide->load() : 0.0f;

    if (! hasPlayedBefore || glideTime <= 0.001f)
        currentNoteFreq = targetNoteFreq;

    hasPlayedBefore = true;

    // ユニゾンの位相をばらけさせると出音が太くなる
    for (int u = 0; u < maxUnison; ++u)
    {
        oscA.phase[u] = (u == 0 ? 0.0 : noiseRandom.nextDouble());
        oscB.phase[u] = (u == 0 ? 0.0 : noiseRandom.nextDouble());
    }

    subPhase = 0.0;

    for (int i = 0; i < 2; ++i)
    {
        // Trig = 0（Retrig）のときだけ位相をリセットする。1（Free）なら流し続ける
        const bool retrig = p.lfoTrig[i] == nullptr || p.lfoTrig[i]->load() < 0.5f;

        if (retrig)
            lfoPhase[i] = 0.0;

        lfoSampleHold[i] = noiseRandom.nextFloat() * 2.0f - 1.0f;
        lfoRateMul[i] = 1.0f;
    }

    ladder.reset();

    ampEnv .reset();
    filtEnv.reset();
    modEnv .reset();
    ampEnv .noteOn();
    filtEnv.noteOn();
    modEnv .noteOn();

    lastAmpEnvValue = 0.0f;
}

void WTVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnv .noteOff();
        filtEnv.noteOff();
        modEnv .noteOff();
    }
    else
    {
        ampEnv .reset();
        filtEnv.reset();
        modEnv .reset();
        clearCurrentNote();
    }
}

void WTVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchBendSemis = ((float) newPitchWheelValue - 8192.0f) / 8192.0f * 2.0f;
}

void WTVoice::controllerMoved (int, int)
{
    // モジュレーションホイールはシンセ側（MidiControlState）で一元管理している
}

void WTVoice::aftertouchChanged (int newAftertouchValue)
{
    polyAftertouch = (float) newAftertouchValue / 127.0f;
}

void WTVoice::channelPressureChanged (int)
{
    // チャンネルプレッシャーも MidiControlState 側で受ける
}

//==============================================================================
float WTVoice::lfoValueFor (int index) const
{
    const int shape = p.lfoShape[index] != nullptr ? (int) p.lfoShape[index]->load() : 0;
    const float ph  = (float) lfoPhase[index];

    switch (shape)
    {
        case 0:  return std::sin (juce::MathConstants<float>::twoPi * ph);   // サイン
        case 1:  return 1.0f - 4.0f * std::abs (ph - 0.5f);                  // 三角
        case 2:  return 2.0f * ph - 1.0f;                                    // ノコギリ上昇
        case 3:  return 1.0f - 2.0f * ph;                                    // ノコギリ下降
        case 4:  return ph < 0.5f ? 1.0f : -1.0f;                            // 矩形
        default: return lfoSampleHold[index];                                // サンプル＆ホールド
    }
}

//==============================================================================
void WTVoice::evaluateMatrix()
{
    std::fill (std::begin (modDestValue), std::end (modDestValue), 0.0f);

    for (int slot = 0; slot < numModSlots; ++slot)
    {
        const auto& m = p.mod[(size_t) slot];

        if (m.src == nullptr || m.dst == nullptr || m.amt == nullptr)
            continue;

        const int   s = (int) m.src->load();
        const int   d = (int) m.dst->load();
        const float a = m.amt->load();

        if (s <= 0 || d <= 0 || std::abs (a) < 1.0e-5f)
            continue;

        if (s >= numModSources || d >= numModDests)
            continue;

        modDestValue[d] += modSourceValue[s] * a;
    }
}

//==============================================================================
void WTVoice::configureOsc (UnisonOsc& osc, float baseFreq, int unisonCount,
                            float detuneCents, float spread)
{
    const double sr = getSampleRate();
    osc.count = juce::jlimit (1, maxUnison, unisonCount);
    osc.normalise = 1.0f / std::sqrt ((float) osc.count);

    for (int u = 0; u < osc.count; ++u)
    {
        const float offset = (osc.count > 1)
                           ? ((float) u / (float) (osc.count - 1)) * 2.0f - 1.0f
                           : 0.0f;

        // 中央寄りを少し密に、端を広くすると自然なユニゾンになる
        const float curved = offset * (0.55f + 0.45f * std::abs (offset));
        const float cents  = curved * detuneCents * 0.5f;
        const float f      = juce::jlimit (0.1f, 22000.0f,
                                           baseFreq * std::pow (2.0f, cents / 1200.0f));

        osc.increment[u] = (double) f / sr;
        osc.mip[u]       = bank.mipForFrequency (f);

        const float pan = juce::jlimit (-1.0f, 1.0f, offset * spread);
        const float ang = (pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
        osc.gainL[u] = std::cos (ang);
        osc.gainR[u] = std::sin (ang);
    }
}

//==============================================================================
void WTVoice::updateModulation (int numSamplesThisBlock)
{
    const double sr = getSampleRate();
    const float  n  = (float) numSamplesThisBlock;

    // --- エンベロープのパラメータを反映して、このブロックぶん進める ---
    ampEnvParams.attack  = p.ampAttack->load();
    ampEnvParams.decay   = p.ampDecay->load();
    ampEnvParams.sustain = p.ampSustain->load();
    ampEnvParams.release = p.ampRelease->load();
    ampEnv.setParameters (ampEnvParams);

    filtEnvParams.attack  = p.fegAttack->load();
    filtEnvParams.decay   = p.fegDecay->load();
    filtEnvParams.sustain = p.fegSustain->load();
    filtEnvParams.release = p.fegRelease->load();
    filtEnv.setParameters (filtEnvParams);

    modEnvParams.attack  = p.megAttack->load();
    modEnvParams.decay   = p.megDecay->load();
    modEnvParams.sustain = p.megSustain->load();
    modEnvParams.release = p.megRelease->load();
    modEnv.setParameters (modEnvParams);

    float fe = 0.0f, me = 0.0f;

    for (int i = 0; i < numSamplesThisBlock; ++i)
    {
        fe = filtEnv.getNextSample();
        me = modEnv .getNextSample();
    }

    // --- LFO（レート変調は 1 ブロック前の結果を使う） ---
    for (int i = 0; i < 2; ++i)
    {
        const float rate = p.lfoRate[i]->load() * lfoRateMul[i];
        lfoPhase[i] += (double) rate / sr * (double) numSamplesThisBlock;

        if (lfoPhase[i] >= 1.0)
        {
            lfoPhase[i] -= std::floor (lfoPhase[i]);
            lfoSampleHold[i] = noiseRandom.nextFloat() * 2.0f - 1.0f;   // S&H の更新
        }

        lfoOut[i] = lfoValueFor (i);
    }

    // --- モジュレーションの元をそろえる ---
    modSourceValue[(int) ModSource::off]           = 0.0f;
    modSourceValue[(int) ModSource::lfo1]          = lfoOut[0];
    modSourceValue[(int) ModSource::lfo2]          = lfoOut[1];
    modSourceValue[(int) ModSource::ampEnv]        = lastAmpEnvValue;
    modSourceValue[(int) ModSource::filtEnv]       = fe;
    modSourceValue[(int) ModSource::modEnv]        = me;
    modSourceValue[(int) ModSource::velocity]      = velocityValue;
    modSourceValue[(int) ModSource::keyTrack]      = juce::jlimit (-1.0f, 1.0f,
                                                        ((float) noteNumber - 60.0f) / 48.0f);
    modSourceValue[(int) ModSource::modWheel]      = controls.modWheel.load();
    modSourceValue[(int) ModSource::aftertouch]    = juce::jmax (polyAftertouch,
                                                        controls.channelPressure.load());
    modSourceValue[(int) ModSource::randomPerNote] = noteRandom;

    evaluateMatrix();

    // 次のブロック用に LFO のレート倍率を作っておく
    lfoRateMul[0] = std::pow (2.0f, modDestValue[(int) ModDest::lfo1Rate] * depthLfoRate);
    lfoRateMul[1] = std::pow (2.0f, modDestValue[(int) ModDest::lfo2Rate] * depthLfoRate);

    // --- ピッチとグライド ---
    targetNoteFreq = midiToFreq ((float) noteNumber + pitchBendSemis);

    const float glideTime = p.glide->load();
    const float glideCoeff = (glideTime <= 0.001f)
                           ? 1.0f
                           : 1.0f - std::exp (-n / (float) (sr * glideTime));

    currentNoteFreq += (targetNoteFreq - currentNoteFreq) * glideCoeff;

    const float semisAll = modDestValue[(int) ModDest::pitchAll] * depthPitch;
    const float semisA   = semisAll + modDestValue[(int) ModDest::pitchA] * depthPitch;
    const float semisB   = semisAll + modDestValue[(int) ModDest::pitchB] * depthPitch;

    // --- オシレータ ---
    const float freqA = currentNoteFreq
                      * std::pow (2.0f, p.oscAOct->load()
                                      + p.oscASemi->load() / 12.0f
                                      + p.oscAFine->load() / 1200.0f
                                      + semisA / 12.0f);

    const float freqB = currentNoteFreq
                      * std::pow (2.0f, p.oscBOct->load()
                                      + p.oscBSemi->load() / 12.0f
                                      + p.oscBFine->load() / 1200.0f
                                      + semisB / 12.0f);

    const float detuneA = juce::jlimit (0.0f, 200.0f,
        p.oscADetune->load() + modDestValue[(int) ModDest::detuneA] * depthDetune);
    const float detuneB = juce::jlimit (0.0f, 200.0f,
        p.oscBDetune->load() + modDestValue[(int) ModDest::detuneB] * depthDetune);

    configureOsc (oscA, freqA, (int) p.oscAUni->load(), detuneA, p.oscASpread->load());
    configureOsc (oscB, freqB, (int) p.oscBUni->load(), detuneB, p.oscBSpread->load());

    subIncrement = (double) (currentNoteFreq * 0.5f * std::pow (2.0f, semisAll / 12.0f)) / sr;

    modPosA = juce::jlimit (0.0f, 1.0f, p.oscAPos->load() + modDestValue[(int) ModDest::posA]);
    modPosB = juce::jlimit (0.0f, 1.0f, p.oscBPos->load() + modDestValue[(int) ModDest::posB]);

    levelA     = juce::jlimit (0.0f, 1.5f, p.oscALevel->load()  + modDestValue[(int) ModDest::levelA]);
    levelB     = juce::jlimit (0.0f, 1.5f, p.oscBLevel->load()  + modDestValue[(int) ModDest::levelB]);
    levelSub   = juce::jlimit (0.0f, 1.5f, p.subLevel->load()   + modDestValue[(int) ModDest::subLevel]);
    levelNoise = juce::jlimit (0.0f, 1.5f, p.noiseLevel->load() + modDestValue[(int) ModDest::noiseLevel]);

    // --- 音量とパン ---
    modAmp = juce::jlimit (0.0f, 2.0f, 1.0f + modDestValue[(int) ModDest::amplitude]);

    const float pan = juce::jlimit (-1.0f, 1.0f, modDestValue[(int) ModDest::pan]);
    const float panAng = (pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
    panGainL = std::cos (panAng) * juce::MathConstants<float>::sqrt2;
    panGainR = std::sin (panAng) * juce::MathConstants<float>::sqrt2;

    // --- フィルタ ---
    const float baseCutoff = p.filtCutoff->load();
    const float envOct     = p.filtEnvAmt->load() * fe;
    const float keyOct     = p.filtKeyTrk->load() * ((float) noteNumber - 60.0f) / 12.0f;
    const float modOct     = modDestValue[(int) ModDest::cutoff] * depthCutoff;

    modCutoff = juce::jlimit (20.0f, (float) (sr * 0.45),
                              baseCutoff * std::pow (2.0f, envOct + keyOct + modOct));

    ladder.setMode ([] (int t)
    {
        using M = juce::dsp::LadderFilterMode;
        switch (t)
        {
            case 0:  return M::LPF12;
            case 1:  return M::LPF24;
            case 2:  return M::HPF12;
            case 3:  return M::HPF24;
            case 4:  return M::BPF12;
            default: return M::BPF24;
        }
    } ((int) p.filtType->load()));

    ladder.setCutoffFrequencyHz (modCutoff);
    ladder.setResonance (juce::jlimit (0.0f, 0.95f,
        p.filtRes->load() + modDestValue[(int) ModDest::resonance]));
    ladder.setDrive (juce::jlimit (1.0f, 8.0f,
        p.filtDrive->load() + modDestValue[(int) ModDest::drive] * depthDrive));
}

//==============================================================================
void WTVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                               int startSample, int numSamples)
{
    if (! ampEnv.isActive() || getSampleRate() <= 0.0 || ! bank.isReady())
        return;

    if (workBuffer.getNumSamples() < controlRate)
        workBuffer.setSize (2, controlRate, false, false, true);

    const int numOutChannels = outputBuffer.getNumChannels();

    while (numSamples > 0)
    {
        const int n = juce::jmin (numSamples, controlRate);

        updateModulation (n);

        workBuffer.clear (0, n);
        float* wl = workBuffer.getWritePointer (0);
        float* wr = workBuffer.getWritePointer (1);

        const float gainA = levelA * oscA.normalise;
        const float gainB = levelB * oscB.normalise;

        for (int i = 0; i < n; ++i)
        {
            float l = 0.0f, r = 0.0f;

            if (gainA > 1.0e-5f)
            {
                float al = 0.0f, ar = 0.0f;

                for (int u = 0; u < oscA.count; ++u)
                {
                    const float s = bank.getSample (modPosA, (float) oscA.phase[u], oscA.mip[u]);
                    al += s * oscA.gainL[u];
                    ar += s * oscA.gainR[u];

                    oscA.phase[u] += oscA.increment[u];

                    if (oscA.phase[u] >= 1.0)
                        oscA.phase[u] -= 1.0;
                }

                l += al * gainA;
                r += ar * gainA;
            }
            else
            {
                for (int u = 0; u < oscA.count; ++u)
                {
                    oscA.phase[u] += oscA.increment[u];

                    if (oscA.phase[u] >= 1.0)
                        oscA.phase[u] -= 1.0;
                }
            }

            if (gainB > 1.0e-5f)
            {
                float bl = 0.0f, br = 0.0f;

                for (int u = 0; u < oscB.count; ++u)
                {
                    const float s = bank.getSample (modPosB, (float) oscB.phase[u], oscB.mip[u]);
                    bl += s * oscB.gainL[u];
                    br += s * oscB.gainR[u];

                    oscB.phase[u] += oscB.increment[u];

                    if (oscB.phase[u] >= 1.0)
                        oscB.phase[u] -= 1.0;
                }

                l += bl * gainB;
                r += br * gainB;
            }
            else
            {
                for (int u = 0; u < oscB.count; ++u)
                {
                    oscB.phase[u] += oscB.increment[u];

                    if (oscB.phase[u] >= 1.0)
                        oscB.phase[u] -= 1.0;
                }
            }

            if (levelSub > 1.0e-5f)
            {
                const float s = std::sin (juce::MathConstants<float>::twoPi * (float) subPhase) * levelSub;
                l += s;
                r += s;
            }

            subPhase += subIncrement;

            if (subPhase >= 1.0)
                subPhase -= 1.0;

            if (levelNoise > 1.0e-5f)
            {
                l += (noiseRandom.nextFloat() * 2.0f - 1.0f) * levelNoise;
                r += (noiseRandom.nextFloat() * 2.0f - 1.0f) * levelNoise;
            }

            wl[i] = l;
            wr[i] = r;
        }

        // --- フィルタ ---
        {
            float* chans[2] = { wl, wr };
            juce::dsp::AudioBlock<float> block (chans, 2, 0, (size_t) n);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            ladder.process (ctx);
        }

        // --- アンプ EG・パン・出力への加算 ---
        const float gl = velocityGain * modAmp * panGainL;
        const float gr = velocityGain * modAmp * panGainR;

        for (int i = 0; i < n; ++i)
        {
            const float e = ampEnv.getNextSample();
            lastAmpEnvValue = e;
            wl[i] *= e * gl;
            wr[i] *= e * gr;
        }

        if (numOutChannels >= 2)
        {
            outputBuffer.addFrom (0, startSample, wl, n);
            outputBuffer.addFrom (1, startSample, wr, n);
        }
        else if (numOutChannels == 1)
        {
            for (int i = 0; i < n; ++i)
                outputBuffer.addSample (0, startSample + i, 0.5f * (wl[i] + wr[i]));
        }

        startSample += n;
        numSamples  -= n;

        if (! ampEnv.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}
