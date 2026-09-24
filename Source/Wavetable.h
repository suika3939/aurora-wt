#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/**
    WavetableBank
    -------------
    8 個の「フレーム（波形）」を持つウェーブテーブル群。
    フレーム間を 0.0〜1.0 の位置でモーフィング（クロスフェード）できる。

    エイリアシング対策として、各フレームについてオクターブごとの
    「ミップマップ」（倍音数を制限したテーブル）を持つ。
    高い音ほど倍音の少ないテーブルを使うため、高音でもジャリつかない。
*/
class WavetableBank
{
public:
    static constexpr int tableSize = 2048;                 // 1周期のサンプル数
    static constexpr int numFrames = 8;                    // 波形フレーム数
    static constexpr int numMips   = 11;                   // オクターブ帯域数
    static constexpr float lowestFreq = 20.0f;             // ミップ 0 の下限周波数

    WavetableBank() = default;

    /** サンプルレートに応じてテーブルを生成する（同じレートなら再生成しない）。 */
    void prepare (double sampleRate);

    /** 周波数からどのミップレベルを使うべきか返す。 */
    inline int mipForFrequency (float freq) const noexcept
    {
        if (freq <= lowestFreq)
            return 0;

        const int m = (int) std::floor (std::log2 (freq / lowestFreq));
        return juce::jlimit (0, numMips - 1, m);
    }

    /** フレーム位置(0..1) と 位相(0..1) から線形補間でサンプルを取り出す。 */
    inline float getSample (float framePos, float phase01, int mip) const noexcept
    {
        const float fp   = juce::jlimit (0.0f, 1.0f, framePos) * (float) (numFrames - 1);
        const int   f0   = (int) fp;
        const int   f1   = juce::jmin (f0 + 1, numFrames - 1);
        const float fMix = fp - (float) f0;

        const float pos  = phase01 * (float) tableSize;
        const int   i0   = (int) pos;
        const float pMix = pos - (float) i0;

        const float* t0 = tablePtr (f0, mip);
        const float* t1 = tablePtr (f1, mip);

        // テーブルは末尾に 1 サンプル分の折り返しを持たせてあるので境界チェック不要
        const float a = t0[i0] + pMix * (t0[i0 + 1] - t0[i0]);
        const float b = t1[i0] + pMix * (t1[i0 + 1] - t1[i0]);

        return a + fMix * (b - a);
    }

    /** GUI 表示用：指定フレーム位置の波形を 1 周期分書き出す（最も倍音の多いミップ）。 */
    void fillDisplayWaveform (float framePos, float* dest, int numPoints) const;

    bool isReady() const noexcept { return ready; }

private:
    static constexpr int stride = tableSize + 1;           // 折り返し 1 サンプル込み

    inline const float* tablePtr (int frame, int mip) const noexcept
    {
        return data.data() + (size_t) ((frame * numMips + mip) * stride);
    }

    inline float* tablePtr (int frame, int mip) noexcept
    {
        return data.data() + (size_t) ((frame * numMips + mip) * stride);
    }

    /** frame 番目の波形の、n 次倍音の振幅を返す。 */
    static float harmonicAmplitude (int frame, int n) noexcept;

    std::vector<float> data;
    double currentSampleRate = 0.0;
    bool ready = false;
};
