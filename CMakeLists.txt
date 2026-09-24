cmake_minimum_required(VERSION 3.22)

project(AuroraWT VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS: Intel + Apple Silicon 両対応のユニバーサルバイナリを作る
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# JUCE の取得
#   1) ローカルに JUCE がある場合   : cmake -DJUCE_PATH=/path/to/JUCE ..
#   2) 無い場合                     : GitHub から自動ダウンロード
# ---------------------------------------------------------------------------
set(JUCE_PATH "" CACHE PATH "ローカルの JUCE ディレクトリ（省略時は自動取得）")

if (JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(STATUS "ローカルの JUCE を使用します: ${JUCE_PATH}")
    add_subdirectory("${JUCE_PATH}" JUCE EXCLUDE_FROM_ALL)
else()
    message(STATUS "JUCE を GitHub から取得します（初回のみ数分かかります）")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.15
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------
# プラグイン本体
# ---------------------------------------------------------------------------
juce_add_plugin(AuroraWT
    COMPANY_NAME              "KStudio"
    BUNDLE_ID                 "com.kstudio.aurorawt"
    PLUGIN_MANUFACTURER_CODE  Kstu          # 4文字・1文字目は大文字
    PLUGIN_CODE               Awt1          # 4文字・1文字目は大文字
    FORMATS                   VST3 AU Standalone
    PRODUCT_NAME              "Aurora WT"
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   TRUE          # ビルド後に自動でシステムのプラグインフォルダへコピー
    VST3_CATEGORIES           "Instrument" "Synth"
    AU_MAIN_TYPE              "kAudioUnitType_MusicDevice"
)

juce_generate_juce_header(AuroraWT)

target_sources(AuroraWT PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Wavetable.cpp
    Source/SynthVoice.cpp
    Source/LookAndFeel.cpp
    Source/PresetManager.cpp
)

target_compile_definitions(AuroraWT PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(AuroraWT PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
