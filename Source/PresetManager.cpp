#include "PresetManager.h"
#include "Parameters.h"

//==============================================================================
namespace
{
    // ウェーブテーブルのフレーム位置（0..1 を 8 等分した値）
    constexpr float WT_SINE    = 0.0000f;
    constexpr float WT_TRI     = 0.1429f;
    constexpr float WT_SOFTSAW = 0.2857f;
    constexpr float WT_SAW     = 0.4286f;
    constexpr float WT_SQUARE  = 0.5714f;
    constexpr float WT_PULSE   = 0.7143f;
    constexpr float WT_FORMANT = 0.8571f;
    constexpr float WT_BRIGHT  = 1.0000f;

    // フィルタタイプ
    constexpr float LP12 = 0.0f, LP24 = 1.0f, HP12 = 2.0f, BP12 = 4.0f;

    // LFO 波形
    constexpr float LFO_SINE = 0.0f, LFO_TRI = 1.0f, LFO_SH = 5.0f;

    // LFO のトリガー
    constexpr float TRIG_FREE = 1.0f;

    // モジュレーションの「元」（getModSourceNames() の並び）
    constexpr float SRC_LFO1 = 1.0f, SRC_LFO2 = 2.0f, SRC_FILTEG = 4.0f, SRC_MODEG = 5.0f,
                    SRC_VEL  = 6.0f, SRC_KEY  = 7.0f, SRC_WHEEL  = 8.0f, SRC_AT = 9.0f,
                    SRC_RND  = 10.0f;

    // モジュレーションの「先」（getModDestNames() の並び）
    constexpr float DST_PITCH = 1.0f,  DST_POSA  = 4.0f,  DST_POSB   = 5.0f,
                    DST_LEVB  = 7.0f,  DST_DETA  = 8.0f,  DST_SUB    = 10.0f,
                    DST_NOISE = 11.0f, DST_CUT   = 12.0f, DST_RES    = 13.0f,
                    DST_DRIVE = 14.0f, DST_AMP   = 15.0f, DST_PAN    = 16.0f,
                    DST_LFO1RATE = 17.0f;
}

//==============================================================================
const std::vector<FactoryPreset>& PresetManager::getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets =
    {
        //----------------------------------------------------------------------
        { "Init", "Init", { } },

        //----------------------------------------------------------------------
        { "Aurora Pad", "Pad", {
            { "oscA_pos", WT_SAW * 0.93f }, { "oscA_level", 0.70f }, { "oscA_uni", 7 },
            { "oscA_detune", 32.0f }, { "oscA_spread", 0.90f },
            { "oscB_pos", WT_PULSE }, { "oscB_level", 0.45f }, { "oscB_uni", 5 },
            { "oscB_detune", 22.0f }, { "oscB_oct", -1 }, { "oscB_spread", 0.70f },
            { "sub_level", 0.18f },
            { "filt_type", LP24 }, { "filt_cutoff", 700.0f }, { "filt_res", 0.30f },
            { "filt_envamt", 2.6f }, { "filt_keytrack", 0.40f },
            { "amp_attack", 0.50f }, { "amp_decay", 1.50f }, { "amp_sustain", 0.75f }, { "amp_release", 2.20f },
            { "feg_attack", 0.80f }, { "feg_decay", 2.50f }, { "feg_sustain", 0.45f }, { "feg_release", 2.00f },
            { "lfo1_rate", 0.22f }, { "lfo1_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_POSA }, { "mod1_amt", 0.20f },
            { "mod2_src", SRC_LFO1 }, { "mod2_dst", DST_POSB }, { "mod2_amt", 0.20f },
            { "mod3_src", SRC_VEL },  { "mod3_dst", DST_CUT },  { "mod3_amt", 0.18f },
            { "mod4_src", SRC_WHEEL },{ "mod4_dst", DST_CUT },  { "mod4_amt", 0.30f },
            { "chorus_mix", 0.45f }, { "chorus_rate", 0.45f },
            { "delay_time", 420.0f }, { "delay_fb", 0.42f }, { "delay_mix", 0.22f },
            { "reverb_size", 0.88f }, { "reverb_damp", 0.35f }, { "reverb_mix", 0.42f },
            { "master_gain", -4.5f } } },

        //----------------------------------------------------------------------
        { "Glass Choir", "Pad", {
            { "oscA_pos", WT_FORMANT }, { "oscA_level", 0.70f }, { "oscA_uni", 5 },
            { "oscA_detune", 18.0f }, { "oscA_spread", 0.85f },
            { "oscB_pos", WT_SINE }, { "oscB_level", 0.35f }, { "oscB_oct", 1 },
            { "oscB_uni", 3 }, { "oscB_detune", 10.0f },
            { "filt_type", LP24 }, { "filt_cutoff", 2200.0f }, { "filt_res", 0.20f },
            { "filt_envamt", 1.5f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.90f }, { "amp_decay", 2.00f }, { "amp_sustain", 0.80f }, { "amp_release", 3.00f },
            { "feg_attack", 1.20f }, { "feg_decay", 2.00f }, { "feg_sustain", 0.60f }, { "feg_release", 2.50f },
            { "lfo1_rate", 0.18f }, { "lfo1_trig", TRIG_FREE },
            { "lfo2_rate", 0.11f }, { "lfo2_shape", LFO_TRI }, { "lfo2_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_POSA }, { "mod1_amt", 0.15f },
            { "mod2_src", SRC_LFO2 }, { "mod2_dst", DST_PAN },  { "mod2_amt", 0.35f },
            { "mod3_src", SRC_WHEEL },{ "mod3_dst", DST_POSA }, { "mod3_amt", 0.30f },
            { "chorus_mix", 0.50f }, { "chorus_rate", 0.30f }, { "chorus_depth", 0.50f },
            { "delay_time", 500.0f }, { "delay_fb", 0.35f }, { "delay_mix", 0.18f },
            { "reverb_size", 0.92f }, { "reverb_damp", 0.25f }, { "reverb_mix", 0.55f },
            { "master_gain", -4.0f } } },

        //----------------------------------------------------------------------
        { "Warm Analog Pad", "Pad", {
            { "oscA_pos", WT_SOFTSAW }, { "oscA_level", 0.70f }, { "oscA_uni", 5 },
            { "oscA_detune", 26.0f }, { "oscA_spread", 0.80f },
            { "oscB_pos", WT_TRI }, { "oscB_level", 0.50f }, { "oscB_oct", -1 },
            { "oscB_uni", 3 }, { "oscB_detune", 14.0f },
            { "sub_level", 0.12f },
            { "filt_type", LP24 }, { "filt_cutoff", 900.0f }, { "filt_res", 0.22f },
            { "filt_drive", 1.6f }, { "filt_envamt", 2.0f }, { "filt_keytrack", 0.35f },
            { "amp_attack", 0.35f }, { "amp_decay", 1.20f }, { "amp_sustain", 0.80f }, { "amp_release", 1.60f },
            { "feg_attack", 0.50f }, { "feg_decay", 1.80f }, { "feg_sustain", 0.50f }, { "feg_release", 1.50f },
            { "lfo1_rate", 0.30f }, { "lfo1_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_CUT },  { "mod1_amt", 0.17f },
            { "mod2_src", SRC_VEL },  { "mod2_dst", DST_CUT },  { "mod2_amt", 0.20f },
            { "mod3_src", SRC_RND },  { "mod3_dst", DST_DETA }, { "mod3_amt", 0.06f },
            { "chorus_mix", 0.35f }, { "chorus_rate", 0.50f },
            { "delay_mix", 0.12f },
            { "reverb_size", 0.75f }, { "reverb_mix", 0.35f },
            { "master_gain", -3.0f } } },

        //----------------------------------------------------------------------
        { "Wide Strings", "Pad", {
            { "oscA_pos", WT_SOFTSAW }, { "oscA_level", 0.70f }, { "oscA_uni", 7 },
            { "oscA_detune", 22.0f }, { "oscA_spread", 1.00f },
            { "oscB_pos", WT_SAW }, { "oscB_level", 0.45f }, { "oscB_oct", -1 },
            { "oscB_uni", 5 }, { "oscB_detune", 18.0f }, { "oscB_spread", 0.80f },
            { "filt_type", LP24 }, { "filt_cutoff", 1600.0f }, { "filt_res", 0.18f },
            { "filt_envamt", 1.4f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.25f }, { "amp_decay", 1.40f }, { "amp_sustain", 0.85f }, { "amp_release", 1.20f },
            { "feg_attack", 0.35f }, { "feg_decay", 1.20f }, { "feg_sustain", 0.60f }, { "feg_release", 1.00f },
            { "lfo1_rate", 4.50f },
            { "lfo2_rate", 0.16f }, { "lfo2_shape", LFO_TRI }, { "lfo2_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 },  { "mod1_dst", DST_PITCH }, { "mod1_amt", 0.030f },
            { "mod2_src", SRC_LFO2 },  { "mod2_dst", DST_POSA },  { "mod2_amt", 0.12f },
            { "mod3_src", SRC_WHEEL }, { "mod3_dst", DST_LFO1RATE }, { "mod3_amt", 0.20f },
            { "mod4_src", SRC_KEY },   { "mod4_dst", DST_PAN },   { "mod4_amt", 0.30f },
            { "chorus_mix", 0.40f }, { "chorus_rate", 0.55f }, { "chorus_depth", 0.45f },
            { "delay_mix", 0.15f },
            { "reverb_size", 0.80f }, { "reverb_damp", 0.40f }, { "reverb_mix", 0.38f },
            { "master_gain", -4.0f } } },

        //----------------------------------------------------------------------
        { "Super Saw Lead", "Lead", {
            { "oscA_pos", WT_SAW }, { "oscA_level", 0.80f }, { "oscA_uni", 7 },
            { "oscA_detune", 45.0f }, { "oscA_spread", 1.00f },
            { "oscB_pos", WT_SAW }, { "oscB_level", 0.50f }, { "oscB_oct", -1 },
            { "oscB_uni", 7 }, { "oscB_detune", 38.0f }, { "oscB_spread", 0.80f },
            { "filt_type", LP24 }, { "filt_cutoff", 4500.0f }, { "filt_res", 0.18f },
            { "filt_envamt", 1.2f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.010f }, { "amp_decay", 1.00f }, { "amp_sustain", 0.85f }, { "amp_release", 0.50f },
            { "feg_attack", 0.020f }, { "feg_decay", 0.60f }, { "feg_sustain", 0.60f }, { "feg_release", 0.40f },
            { "chorus_mix", 0.30f },
            { "delay_time", 375.0f }, { "delay_fb", 0.38f }, { "delay_mix", 0.28f },
            { "reverb_size", 0.60f }, { "reverb_mix", 0.22f },
            { "master_gain", -9.0f } } },

        //----------------------------------------------------------------------
        { "Formant Lead", "Lead", {
            { "oscA_pos", WT_FORMANT }, { "oscA_level", 0.85f }, { "oscA_uni", 3 },
            { "oscA_detune", 12.0f }, { "oscA_spread", 0.40f },
            { "oscB_level", 0.0f },
            { "filt_type", LP24 }, { "filt_cutoff", 1800.0f }, { "filt_res", 0.45f },
            { "filt_drive", 2.2f }, { "filt_envamt", 2.2f }, { "filt_keytrack", 0.60f },
            { "amp_attack", 0.020f }, { "amp_decay", 0.50f }, { "amp_sustain", 0.80f }, { "amp_release", 0.35f },
            { "feg_attack", 0.010f }, { "feg_decay", 0.35f }, { "feg_sustain", 0.35f }, { "feg_release", 0.30f },
            { "lfo1_rate", 5.50f },
            { "mod1_src", SRC_WHEEL }, { "mod1_dst", DST_PITCH }, { "mod1_amt", 0.055f },
            { "mod2_src", SRC_MODEG }, { "mod2_dst", DST_POSA },  { "mod2_amt", -0.22f },
            { "mod3_src", SRC_AT },    { "mod3_dst", DST_CUT },   { "mod3_amt", 0.35f },
            { "meg_attack", 0.30f }, { "meg_decay", 1.20f }, { "meg_sustain", 0.0f },
            { "glide", 0.06f },
            { "delay_time", 300.0f }, { "delay_fb", 0.30f }, { "delay_mix", 0.22f },
            { "reverb_size", 0.50f }, { "reverb_mix", 0.20f },
            { "master_gain", -6.5f } } },

        //----------------------------------------------------------------------
        { "Vibrato Square Lead", "Lead", {
            { "oscA_pos", WT_SQUARE }, { "oscA_level", 0.80f }, { "oscA_uni", 3 },
            { "oscA_detune", 10.0f }, { "oscA_spread", 0.35f },
            { "oscB_pos", WT_PULSE }, { "oscB_level", 0.35f }, { "oscB_fine", 8.0f },
            { "filt_type", LP12 }, { "filt_cutoff", 3200.0f }, { "filt_res", 0.30f },
            { "filt_envamt", 1.5f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.015f }, { "amp_decay", 0.40f }, { "amp_sustain", 0.85f }, { "amp_release", 0.30f },
            { "feg_attack", 0.010f }, { "feg_decay", 0.30f }, { "feg_sustain", 0.50f }, { "feg_release", 0.30f },
            { "lfo1_rate", 5.20f }, { "lfo1_shape", LFO_SINE },
            { "mod1_src", SRC_WHEEL }, { "mod1_dst", DST_PITCH }, { "mod1_amt", 0.040f },
            { "mod2_src", SRC_VEL },   { "mod2_dst", DST_CUT },   { "mod2_amt", 0.25f },
            { "glide", 0.04f },
            { "delay_time", 250.0f }, { "delay_fb", 0.35f }, { "delay_mix", 0.20f },
            { "reverb_size", 0.45f }, { "reverb_mix", 0.18f },
            { "master_gain", -10.0f } } },

        //----------------------------------------------------------------------
        { "Deep Sub Bass", "Bass", {
            { "oscA_pos", WT_SINE }, { "oscA_level", 0.85f }, { "oscA_uni", 1 },
            { "oscB_level", 0.0f },
            { "sub_level", 0.55f },
            { "filt_type", LP24 }, { "filt_cutoff", 220.0f }, { "filt_res", 0.10f },
            { "filt_drive", 1.4f }, { "filt_envamt", 0.8f }, { "filt_keytrack", 0.20f },
            { "amp_attack", 0.005f }, { "amp_decay", 0.60f }, { "amp_sustain", 0.90f }, { "amp_release", 0.15f },
            { "feg_attack", 0.005f }, { "feg_decay", 0.25f }, { "feg_sustain", 0.40f }, { "feg_release", 0.20f },
            { "chorus_mix", 0.0f }, { "delay_mix", 0.0f }, { "reverb_mix", 0.05f },
            { "master_gain", -7.0f } } },

        //----------------------------------------------------------------------
        { "Reese Bass", "Bass", {
            { "oscA_pos", WT_SAW }, { "oscA_level", 0.80f }, { "oscA_uni", 3 },
            { "oscA_detune", 28.0f }, { "oscA_spread", 0.30f },
            { "oscB_pos", WT_SAW }, { "oscB_level", 0.70f }, { "oscB_fine", -18.0f },
            { "oscB_uni", 3 }, { "oscB_detune", 24.0f }, { "oscB_spread", 0.30f },
            { "sub_level", 0.25f },
            { "filt_type", LP24 }, { "filt_cutoff", 420.0f }, { "filt_res", 0.35f },
            { "filt_drive", 2.8f }, { "filt_envamt", 1.6f }, { "filt_keytrack", 0.30f },
            { "amp_attack", 0.010f }, { "amp_decay", 0.80f }, { "amp_sustain", 0.90f }, { "amp_release", 0.25f },
            { "feg_attack", 0.010f }, { "feg_decay", 0.50f }, { "feg_sustain", 0.45f }, { "feg_release", 0.25f },
            { "lfo1_rate", 0.80f }, { "lfo1_trig", TRIG_FREE },
            { "lfo2_rate", 0.23f }, { "lfo2_shape", LFO_TRI }, { "lfo2_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_CUT },  { "mod1_amt", 0.21f },
            { "mod2_src", SRC_LFO2 }, { "mod2_dst", DST_DETA }, { "mod2_amt", 0.18f },
            { "mod3_src", SRC_VEL },  { "mod3_dst", DST_DRIVE },{ "mod3_amt", 0.25f },
            { "chorus_mix", 0.15f }, { "delay_mix", 0.08f }, { "reverb_mix", 0.08f },
            { "master_gain", -7.0f } } },

        //----------------------------------------------------------------------
        { "Acid Bass", "Bass", {
            { "oscA_pos", WT_SAW }, { "oscA_level", 0.85f }, { "oscA_uni", 1 },
            { "oscB_level", 0.0f },
            { "filt_type", LP24 }, { "filt_cutoff", 180.0f }, { "filt_res", 0.85f },
            { "filt_drive", 3.5f }, { "filt_envamt", 4.5f }, { "filt_keytrack", 0.80f },
            { "amp_attack", 0.003f }, { "amp_decay", 0.35f }, { "amp_sustain", 0.40f }, { "amp_release", 0.12f },
            { "feg_attack", 0.002f }, { "feg_decay", 0.28f }, { "feg_sustain", 0.05f }, { "feg_release", 0.15f },
            { "glide", 0.05f },
            { "delay_time", 190.0f }, { "delay_fb", 0.30f }, { "delay_mix", 0.18f },
            { "reverb_size", 0.40f }, { "reverb_mix", 0.12f },
            { "master_gain", -7.0f } } },

        //----------------------------------------------------------------------
        { "Crystal Pluck", "Pluck", {
            { "oscA_pos", WT_BRIGHT }, { "oscA_level", 0.75f }, { "oscA_uni", 3 },
            { "oscA_detune", 16.0f }, { "oscA_spread", 0.60f },
            { "oscB_pos", WT_SINE }, { "oscB_level", 0.30f }, { "oscB_oct", 1 },
            { "filt_type", LP24 }, { "filt_cutoff", 600.0f }, { "filt_res", 0.40f },
            { "filt_envamt", 4.0f }, { "filt_keytrack", 0.70f },
            { "amp_attack", 0.002f }, { "amp_decay", 0.35f }, { "amp_sustain", 0.0f }, { "amp_release", 0.35f },
            { "feg_attack", 0.001f }, { "feg_decay", 0.25f }, { "feg_sustain", 0.0f }, { "feg_release", 0.25f },
            { "delay_time", 280.0f }, { "delay_fb", 0.45f }, { "delay_mix", 0.38f },
            { "reverb_size", 0.80f }, { "reverb_damp", 0.40f }, { "reverb_mix", 0.40f },
            { "master_gain", -3.0f } } },

        //----------------------------------------------------------------------
        { "Bell Keys", "Keys", {
            { "oscA_pos", WT_SINE }, { "oscA_level", 0.70f }, { "oscA_uni", 1 },
            { "oscB_pos", WT_FORMANT }, { "oscB_level", 0.35f },
            { "oscB_semi", 7 }, { "oscB_oct", 1 },
            { "filt_type", LP24 }, { "filt_cutoff", 5000.0f }, { "filt_res", 0.10f },
            { "filt_envamt", 2.5f }, { "filt_keytrack", 0.60f },
            { "amp_attack", 0.002f }, { "amp_decay", 1.60f }, { "amp_sustain", 0.0f }, { "amp_release", 1.20f },
            { "feg_attack", 0.001f }, { "feg_decay", 0.50f }, { "feg_sustain", 0.0f }, { "feg_release", 0.50f },
            { "chorus_mix", 0.20f },
            { "delay_time", 340.0f }, { "delay_fb", 0.30f }, { "delay_mix", 0.22f },
            { "reverb_size", 0.82f }, { "reverb_damp", 0.35f }, { "reverb_mix", 0.42f },
            { "master_gain", -7.5f } } },

        //----------------------------------------------------------------------
        { "Soft E-Piano", "Keys", {
            { "oscA_pos", WT_TRI }, { "oscA_level", 0.75f }, { "oscA_uni", 1 },
            { "oscB_pos", WT_SINE }, { "oscB_level", 0.40f }, { "oscB_oct", 1 }, { "oscB_fine", 4.0f },
            { "filt_type", LP24 }, { "filt_cutoff", 2200.0f }, { "filt_res", 0.12f },
            { "filt_envamt", 2.2f }, { "filt_keytrack", 0.55f },
            { "amp_attack", 0.003f }, { "amp_decay", 1.10f }, { "amp_sustain", 0.25f }, { "amp_release", 0.60f },
            { "feg_attack", 0.002f }, { "feg_decay", 0.35f }, { "feg_sustain", 0.15f }, { "feg_release", 0.40f },
            { "chorus_mix", 0.30f }, { "chorus_rate", 0.60f },
            { "delay_mix", 0.10f },
            { "reverb_size", 0.60f }, { "reverb_mix", 0.25f },
            { "master_gain", -8.0f } } },

        //----------------------------------------------------------------------
        { "Wavetable Motion", "Motion", {
            { "oscA_pos", 0.35f }, { "oscA_level", 0.70f }, { "oscA_uni", 5 },
            { "oscA_detune", 24.0f }, { "oscA_spread", 0.90f },
            { "oscB_pos", 0.60f }, { "oscB_level", 0.50f }, { "oscB_oct", -1 },
            { "oscB_uni", 3 }, { "oscB_detune", 16.0f },
            { "filt_type", LP24 }, { "filt_cutoff", 2500.0f }, { "filt_res", 0.25f },
            { "filt_envamt", 1.0f }, { "filt_keytrack", 0.40f },
            { "amp_attack", 0.30f }, { "amp_decay", 1.50f }, { "amp_sustain", 0.80f }, { "amp_release", 1.50f },
            { "feg_attack", 0.40f }, { "feg_decay", 1.50f }, { "feg_sustain", 0.60f }, { "feg_release", 1.20f },
            { "lfo1_rate", 0.12f }, { "lfo1_shape", LFO_TRI }, { "lfo1_trig", TRIG_FREE },
            { "lfo2_rate", 0.07f }, { "lfo2_shape", LFO_SINE }, { "lfo2_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_POSA }, { "mod1_amt",  0.48f },
            { "mod2_src", SRC_LFO1 }, { "mod2_dst", DST_POSB }, { "mod2_amt", -0.48f },
            { "mod3_src", SRC_LFO2 }, { "mod3_dst", DST_PAN },  { "mod3_amt",  0.45f },
            { "mod4_src", SRC_LFO2 }, { "mod4_dst", DST_CUT },  { "mod4_amt",  0.20f },
            { "chorus_mix", 0.30f },
            { "delay_time", 480.0f }, { "delay_fb", 0.45f }, { "delay_mix", 0.25f },
            { "reverb_size", 0.85f }, { "reverb_damp", 0.40f }, { "reverb_mix", 0.40f },
            { "master_gain", -9.0f } } },

        //----------------------------------------------------------------------
        { "S&H Sequence", "Motion", {
            { "oscA_pos", WT_SQUARE }, { "oscA_level", 0.80f }, { "oscA_uni", 3 },
            { "oscA_detune", 14.0f }, { "oscA_spread", 0.50f },
            { "filt_type", LP24 }, { "filt_cutoff", 380.0f }, { "filt_res", 0.70f },
            { "filt_drive", 2.0f }, { "filt_envamt", 2.0f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.003f }, { "amp_decay", 0.28f }, { "amp_sustain", 0.0f }, { "amp_release", 0.25f },
            { "feg_attack", 0.002f }, { "feg_decay", 0.20f }, { "feg_sustain", 0.0f }, { "feg_release", 0.20f },
            { "lfo1_rate", 11.0f }, { "lfo1_shape", LFO_SH }, { "lfo1_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 }, { "mod1_dst", DST_CUT }, { "mod1_amt", 0.67f },
            { "mod2_src", SRC_RND },  { "mod2_dst", DST_PAN }, { "mod2_amt", 0.55f },
            { "mod3_src", SRC_VEL },  { "mod3_dst", DST_RES }, { "mod3_amt", 0.15f },
            { "delay_time", 210.0f }, { "delay_fb", 0.50f }, { "delay_mix", 0.35f },
            { "reverb_size", 0.70f }, { "reverb_mix", 0.30f },
            { "master_gain", -6.5f } } },

        //----------------------------------------------------------------------
        { "Noise Sweep FX", "FX", {
            { "oscA_pos", WT_SINE }, { "oscA_level", 0.12f },
            { "noise_level", 0.80f },
            { "filt_type", BP12 }, { "filt_cutoff", 300.0f }, { "filt_res", 0.80f },
            { "filt_drive", 1.5f }, { "filt_envamt", 5.5f }, { "filt_keytrack", 0.10f },
            { "amp_attack", 1.50f }, { "amp_decay", 3.00f }, { "amp_sustain", 0.60f }, { "amp_release", 2.50f },
            { "feg_attack", 2.50f }, { "feg_decay", 3.00f }, { "feg_sustain", 0.90f }, { "feg_release", 2.00f },
            { "lfo1_rate", 0.10f }, { "lfo1_trig", TRIG_FREE },
            { "lfo2_rate", 0.25f }, { "lfo2_shape", LFO_TRI }, { "lfo2_trig", TRIG_FREE },
            { "mod1_src", SRC_LFO1 },  { "mod1_dst", DST_CUT },   { "mod1_amt", 0.25f },
            { "mod2_src", SRC_LFO2 },  { "mod2_dst", DST_PAN },   { "mod2_amt", 0.60f },
            { "mod3_src", SRC_MODEG }, { "mod3_dst", DST_RES },   { "mod3_amt", 0.15f },
            { "meg_attack", 2.00f }, { "meg_decay", 3.00f }, { "meg_sustain", 0.8f },
            { "chorus_mix", 0.30f },
            { "delay_time", 500.0f }, { "delay_fb", 0.50f }, { "delay_mix", 0.30f },
            { "reverb_size", 0.95f }, { "reverb_damp", 0.20f }, { "reverb_mix", 0.60f },
            { "master_gain", -11.5f } } },

        //----------------------------------------------------------------------
        { "Hollow Whistle", "Lead", {
            { "oscA_pos", WT_SINE }, { "oscA_level", 0.80f }, { "oscA_uni", 2 },
            { "oscA_detune", 6.0f }, { "oscA_spread", 0.35f },
            { "oscB_pos", WT_TRI }, { "oscB_level", 0.25f }, { "oscB_oct", 1 },
            { "filt_type", HP12 }, { "filt_cutoff", 240.0f }, { "filt_res", 0.20f },
            { "filt_envamt", 0.8f }, { "filt_keytrack", 0.30f },
            { "amp_attack", 0.08f }, { "amp_decay", 0.60f }, { "amp_sustain", 0.85f }, { "amp_release", 0.60f },
            { "feg_attack", 0.05f }, { "feg_decay", 0.40f }, { "feg_sustain", 0.50f }, { "feg_release", 0.50f },
            { "lfo1_rate", 4.80f },
            { "mod1_src", SRC_MODEG }, { "mod1_dst", DST_PITCH }, { "mod1_amt", 0.042f },
            { "mod2_src", SRC_WHEEL }, { "mod2_dst", DST_PITCH }, { "mod2_amt", 0.050f },
            { "mod3_src", SRC_AT },    { "mod3_dst", DST_AMP },   { "mod3_amt", 0.20f },
            { "meg_attack", 0.60f }, { "meg_decay", 0.50f }, { "meg_sustain", 1.0f },
            { "glide", 0.10f },
            { "delay_time", 440.0f }, { "delay_fb", 0.40f }, { "delay_mix", 0.30f },
            { "reverb_size", 0.85f }, { "reverb_damp", 0.35f }, { "reverb_mix", 0.45f },
            { "master_gain", -9.0f } } },

        //----------------------------------------------------------------------
        { "Velocity Morph", "Motion", {
            { "oscA_pos", WT_TRI }, { "oscA_level", 0.75f }, { "oscA_uni", 3 },
            { "oscA_detune", 16.0f }, { "oscA_spread", 0.55f },
            { "oscB_pos", WT_SINE }, { "oscB_level", 0.0f }, { "oscB_oct", 1 }, { "oscB_uni", 2 },
            { "filt_type", LP24 }, { "filt_cutoff", 900.0f }, { "filt_res", 0.25f },
            { "filt_envamt", 1.8f }, { "filt_keytrack", 0.50f },
            { "amp_attack", 0.004f }, { "amp_decay", 0.90f }, { "amp_sustain", 0.55f }, { "amp_release", 0.60f },
            { "feg_attack", 0.003f }, { "feg_decay", 0.45f }, { "feg_sustain", 0.25f }, { "feg_release", 0.40f },
            // 強く弾くほど波形が明るくなり、B の倍音が足され、歪みも増える
            { "mod1_src", SRC_VEL }, { "mod1_dst", DST_POSA },  { "mod1_amt", 0.62f },
            { "mod2_src", SRC_VEL }, { "mod2_dst", DST_LEVB },  { "mod2_amt", 0.40f },
            { "mod3_src", SRC_VEL }, { "mod3_dst", DST_CUT },   { "mod3_amt", 0.45f },
            { "mod4_src", SRC_VEL }, { "mod4_dst", DST_DRIVE }, { "mod4_amt", 0.22f },
            { "mod5_src", SRC_KEY }, { "mod5_dst", DST_POSA },  { "mod5_amt", 0.15f },
            { "chorus_mix", 0.25f },
            { "delay_time", 330.0f }, { "delay_fb", 0.35f }, { "delay_mix", 0.22f },
            { "reverb_size", 0.70f }, { "reverb_mix", 0.30f },
            { "master_gain", -6.0f } } },

        //----------------------------------------------------------------------
        { "Wheel Riser", "FX", {
            { "oscA_pos", WT_SOFTSAW }, { "oscA_level", 0.70f }, { "oscA_uni", 7 },
            { "oscA_detune", 30.0f }, { "oscA_spread", 0.95f },
            { "oscB_pos", WT_SAW }, { "oscB_level", 0.35f }, { "oscB_uni", 5 },
            { "oscB_detune", 24.0f }, { "oscB_oct", -1 },
            { "noise_level", 0.10f },
            { "filt_type", LP24 }, { "filt_cutoff", 400.0f }, { "filt_res", 0.45f },
            { "filt_envamt", 0.5f }, { "filt_keytrack", 0.30f },
            { "amp_attack", 0.30f }, { "amp_decay", 2.00f }, { "amp_sustain", 0.85f }, { "amp_release", 1.50f },
            { "lfo1_rate", 3.00f }, { "lfo1_trig", TRIG_FREE },
            { "lfo2_rate", 0.20f }, { "lfo2_shape", LFO_TRI }, { "lfo2_trig", TRIG_FREE },
            // ホイールを上げるとカットオフとピッチが上がり、震えも速くなる
            { "mod1_src", SRC_WHEEL }, { "mod1_dst", DST_CUT },      { "mod1_amt", 0.85f },
            { "mod2_src", SRC_WHEEL }, { "mod2_dst", DST_PITCH },    { "mod2_amt", 0.14f },
            { "mod3_src", SRC_WHEEL }, { "mod3_dst", DST_LFO1RATE }, { "mod3_amt", 0.60f },
            { "mod4_src", SRC_WHEEL }, { "mod4_dst", DST_NOISE },    { "mod4_amt", 0.30f },
            { "mod5_src", SRC_LFO1 },  { "mod5_dst", DST_PITCH },    { "mod5_amt", 0.012f },
            { "mod6_src", SRC_LFO2 },  { "mod6_dst", DST_PAN },      { "mod6_amt", 0.50f },
            { "chorus_mix", 0.40f },
            { "delay_time", 380.0f }, { "delay_fb", 0.45f }, { "delay_mix", 0.30f },
            { "reverb_size", 0.90f }, { "reverb_damp", 0.30f }, { "reverb_mix", 0.50f },
            { "master_gain", -8.0f } } },

        //----------------------------------------------------------------------
        { "Scatter Keys", "Keys", {
            { "oscA_pos", WT_PULSE }, { "oscA_level", 0.72f }, { "oscA_uni", 2 },
            { "oscA_detune", 10.0f }, { "oscA_spread", 0.40f },
            { "oscB_pos", WT_SINE }, { "oscB_level", 0.30f }, { "oscB_oct", 1 },
            { "filt_type", LP24 }, { "filt_cutoff", 800.0f }, { "filt_res", 0.35f },
            { "filt_envamt", 3.2f }, { "filt_keytrack", 0.65f },
            { "amp_attack", 0.003f }, { "amp_decay", 0.70f }, { "amp_sustain", 0.10f }, { "amp_release", 0.55f },
            { "feg_attack", 0.002f }, { "feg_decay", 0.40f }, { "feg_sustain", 0.05f }, { "feg_release", 0.35f },
            { "meg_attack", 0.005f }, { "meg_decay", 0.45f }, { "meg_sustain", 0.0f }, { "meg_release", 0.4f },
            // 1 音ごとに定位・デチューン・波形が少しずつ散る
            { "mod1_src", SRC_RND },   { "mod1_dst", DST_PAN },   { "mod1_amt", 0.70f },
            { "mod2_src", SRC_RND },   { "mod2_dst", DST_DETA },  { "mod2_amt", 0.10f },
            { "mod3_src", SRC_RND },   { "mod3_dst", DST_POSA },  { "mod3_amt", 0.10f },
            { "mod4_src", SRC_MODEG }, { "mod4_dst", DST_POSA },  { "mod4_amt", -0.28f },
            { "mod5_src", SRC_VEL },   { "mod5_dst", DST_CUT },   { "mod5_amt", 0.30f },
            { "mod6_src", SRC_KEY },   { "mod6_dst", DST_LEVB },  { "mod6_amt", 0.20f },
            { "chorus_mix", 0.30f },
            { "delay_time", 290.0f }, { "delay_fb", 0.40f }, { "delay_mix", 0.28f },
            { "reverb_size", 0.78f }, { "reverb_damp", 0.40f }, { "reverb_mix", 0.35f },
            { "master_gain", -7.0f } } },
    };

    return presets;
}

int PresetManager::getNumFactoryPresets()
{
    return (int) getFactoryPresets().size();
}

juce::String PresetManager::getFactoryPresetName (int index)
{
    const auto& p = getFactoryPresets();
    return juce::isPositiveAndBelow (index, (int) p.size()) ? juce::String (p[(size_t) index].name)
                                                            : juce::String();
}

//==============================================================================
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToUse)
    : apvts (stateToUse)
{
    getUserPresetDirectory().createDirectory();

    if (! apvts.state.hasProperty (presetNameProperty))
        apvts.state.setProperty (presetNameProperty, "Init", nullptr);
}

//==============================================================================
void PresetManager::resetAllToDefaults()
{
    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());
}

void PresetManager::applyValues (const std::vector<PresetValue>& values)
{
    for (const auto& v : values)
        if (auto* param = apvts.getParameter (v.id))
            param->setValueNotifyingHost (param->convertTo0to1 (v.value));
}

void PresetManager::loadFactoryPreset (int index)
{
    const auto& presets = getFactoryPresets();

    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    resetAllToDefaults();
    applyValues (presets[(size_t) index].values);

    currentFactoryIndex = index;
    setCurrentPresetName (presets[(size_t) index].name);
}

//==============================================================================
juce::File PresetManager::getUserPresetDirectory() const
{
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

   #if JUCE_MAC
    base = base.getChildFile ("Application Support");
   #endif

    return base.getChildFile ("Aurora WT").getChildFile ("Presets");
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;

    for (const auto& f : getUserPresetDirectory().findChildFiles (
             juce::File::findFiles, false, juce::String ("*") + fileExtension))
        names.add (f.getFileNameWithoutExtension());

    names.sortNatural();
    return names;
}

bool PresetManager::saveUserPreset (const juce::String& rawName)
{
    const auto name = juce::File::createLegalFileName (rawName).trim();

    if (name.isEmpty())
        return false;

    auto dir = getUserPresetDirectory();
    dir.createDirectory();

    auto state = apvts.copyState();
    state.setProperty (presetNameProperty, name, nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());

    if (xml == nullptr)
        return false;

    const bool ok = xml->writeTo (dir.getChildFile (name + fileExtension));

    if (ok)
    {
        currentFactoryIndex = -1;
        setCurrentPresetName (name);
    }

    return ok;
}

bool PresetManager::loadUserPreset (const juce::String& name)
{
    auto file = getUserPresetDirectory().getChildFile (name + fileExtension);

    if (! file.existsAsFile())
        return false;

    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));

    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    currentFactoryIndex = -1;
    setCurrentPresetName (name);
    return true;
}

bool PresetManager::deleteUserPreset (const juce::String& name)
{
    return getUserPresetDirectory().getChildFile (name + fileExtension).deleteFile();
}

//==============================================================================
void PresetManager::loadPrevious()
{
    const auto users = getUserPresetNames();
    const int total = getNumFactoryPresets() + users.size();

    if (total == 0)
        return;

    int current = currentFactoryIndex;

    if (current < 0)
    {
        const int userIndex = users.indexOf (getCurrentPresetName());
        current = userIndex >= 0 ? getNumFactoryPresets() + userIndex : 0;
    }

    const int next = (current - 1 + total) % total;

    if (next < getNumFactoryPresets())
        loadFactoryPreset (next);
    else
        loadUserPreset (users[next - getNumFactoryPresets()]);
}

void PresetManager::loadNext()
{
    const auto users = getUserPresetNames();
    const int total = getNumFactoryPresets() + users.size();

    if (total == 0)
        return;

    int current = currentFactoryIndex;

    if (current < 0)
    {
        const int userIndex = users.indexOf (getCurrentPresetName());
        current = userIndex >= 0 ? getNumFactoryPresets() + userIndex : -1;
    }

    const int next = (current + 1) % total;

    if (next < getNumFactoryPresets())
        loadFactoryPreset (next);
    else
        loadUserPreset (users[next - getNumFactoryPresets()]);
}

//==============================================================================
juce::String PresetManager::getCurrentPresetName() const
{
    return apvts.state.getProperty (presetNameProperty, "Init").toString();
}

void PresetManager::refreshFromState()
{
    const auto name = getCurrentPresetName();
    const auto& presets = getFactoryPresets();

    currentFactoryIndex = -1;

    for (int i = 0; i < (int) presets.size(); ++i)
        if (name == presets[(size_t) i].name)
        {
            currentFactoryIndex = i;
            break;
        }

    if (onPresetChanged != nullptr)
        onPresetChanged();
}

void PresetManager::setCurrentPresetName (const juce::String& name)
{
    apvts.state.setProperty (presetNameProperty, name, nullptr);

    if (onPresetChanged != nullptr)
        onPresetChanged();
}
