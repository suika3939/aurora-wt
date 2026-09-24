#include "Wavetable.h"

//==============================================================================
float WavetableBank::harmonicAmplitude (int frame, int n) noexcept
{
    jassert (n >= 1);

    const float fn   = (float) n;
    const bool  odd  = (n % 2) == 1;
    constexpr float pi = juce::MathConstants<float>::pi;

    switch (frame)
    {
        case 0:   // 純粋なサイン波
            return (n == 1) ? 1.0f : 0.0f;

        case 1:   // 三角波（奇数倍音・1/n^2・符号交替）
            if (! odd) return 0.0f;
            return (((n - 1) / 2) % 2 == 0 ? 1.0f : -1.0f) / (fn * fn);

        case 2:   // やわらかいノコギリ波（倍音の減衰が急）
            return 1.0f / std::pow (fn, 1.7f);

        case 3:   // ノコギリ波
            return 1.0f / fn;

        case 4:   // 矩形波
            return odd ? (1.0f / fn) : 0.0f;

        case 5:   // パルス波（デューティ 25%）
            return (2.0f / (fn * pi)) * std::sin (fn * pi * 0.25f);

        case 6:   // フォルマント風（倍音にピークが 3 か所ある声っぽい波形）
        {
            auto peak = [fn] (float centre, float width)
            {
                const float d = (fn - centre) / width;
                return std::exp (-0.5f * d * d);
            };
            const float env = peak (1.0f, 1.2f) + 0.85f * peak (6.0f, 2.2f)
                                                + 0.55f * peak (14.0f, 3.5f);
            return env / std::pow (fn, 0.25f);
        }

        case 7:   // ブライト／ハーシュ（高域が減りにくく櫛状の色付き）
        default:
        {
            const float comb = 0.62f + 0.38f * std::sin (fn * 0.73f);
            return comb / std::pow (fn, 0.5f);
        }
    }
}

//==============================================================================
void WavetableBank::prepare (double sampleRate)
{
    if (ready && juce::approximatelyEqual (sampleRate, currentSampleRate))
        return;

    currentSampleRate = sampleRate;
    data.assign ((size_t) (numFrames * numMips * stride), 0.0f);

    const double nyquist = sampleRate * 0.5;

    juce::dsp::FFT fft (11);                       // 2^11 = 2048
    jassert (fft.getSize() == tableSize);

    std::vector<juce::dsp::Complex<float>> spectrum ((size_t) tableSize);
    std::vector<juce::dsp::Complex<float>> timeDom  ((size_t) tableSize);

    for (int mip = 0; mip < numMips; ++mip)
    {
        // このミップが担当する最高の基本周波数
        const double topFreq = (double) lowestFreq * std::pow (2.0, (double) (mip + 1));
        int maxHarmonic = (int) std::floor (nyquist / topFreq);
        maxHarmonic = juce::jlimit (1, tableSize / 2 - 1, maxHarmonic);

        for (int frame = 0; frame < numFrames; ++frame)
        {
            std::fill (spectrum.begin(), spectrum.end(), juce::dsp::Complex<float> (0.0f, 0.0f));

            for (int n = 1; n <= maxHarmonic; ++n)
            {
                const float a = harmonicAmplitude (frame, n);

                if (std::abs (a) < 1.0e-7f)
                    continue;

                // sin(2πnt/N) を作るための共役対称なスペクトル
                spectrum[(size_t) n]             = { 0.0f, -a * 0.5f };
                spectrum[(size_t) (tableSize - n)] = { 0.0f,  a * 0.5f };
            }

            fft.perform (spectrum.data(), timeDom.data(), true);

            float* dst = tablePtr (frame, mip);
            float peak = 0.0f;

            for (int i = 0; i < tableSize; ++i)
            {
                dst[i] = timeDom[(size_t) i].real();
                peak = juce::jmax (peak, std::abs (dst[i]));
            }

            if (peak > 1.0e-6f)
            {
                const float g = 1.0f / peak;

                for (int i = 0; i < tableSize; ++i)
                    dst[i] *= g;
            }

            dst[tableSize] = dst[0];               // 線形補間用の折り返しサンプル
        }
    }

    ready = true;
}

//==============================================================================
void WavetableBank::fillDisplayWaveform (float framePos, float* dest, int numPoints) const
{
    if (! ready || numPoints <= 0)
        return;

    // 表示用は倍音を程よく間引いたミップを使うと形が見やすい
    constexpr int displayMip = 4;

    for (int i = 0; i < numPoints; ++i)
    {
        const float p = (float) i / (float) numPoints;
        dest[i] = getSample (framePos, p, displayMip);
    }
}
