cmake_minimum_required(VERSION 3.22)

project(WinampClassicVisualizer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WCV_BUILD_PLUGIN "Build the VST3 plugin" ON)
option(WCV_BUILD_TESTS "Build the core unit tests" ON)

include(FetchContent)

add_library(classic_vis_core STATIC
    Source/Core/ClassicVis.cpp
    Source/Core/Fft.cpp
    Source/Core/Oscilloscope.cpp
    Source/Core/Palette.cpp
    Source/Core/SpectrumAnalyzer.cpp)
target_include_directories(classic_vis_core PUBLIC Source/Core)
set_target_properties(classic_vis_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(WCV_BUILD_PLUGIN)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)

    juce_add_plugin(WinampClassicVisualizer
        COMPANY_NAME "Alan Marcero"
        PRODUCT_NAME "Winamp Classic Visualizer"
        PLUGIN_MANUFACTURER_CODE Amrc
        PLUGIN_CODE Wcvz
        FORMATS VST3
        VST3_CATEGORIES Fx Analyzer
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE)

    target_sources(WinampClassicVisualizer PRIVATE
        Source/Plugin/Parameters.cpp
        Source/Plugin/PluginEditor.cpp
        Source/Plugin/PluginProcessor.cpp)

    target_compile_definitions(WinampClassicVisualizer PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

    target_link_libraries(WinampClassicVisualizer
        PRIVATE
            classic_vis_core
            juce::juce_audio_processors
            juce::juce_audio_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
endif()

if(WCV_BUILD_TESTS)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.9.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(Catch2)

    add_executable(core_tests
        Tests/ClassicVisTests.cpp
        Tests/FftTests.cpp
        Tests/OscilloscopeTests.cpp
        Tests/PaletteTests.cpp
        Tests/SpectrumAnalyzerTests.cpp)
    target_link_libraries(core_tests PRIVATE classic_vis_core Catch2::Catch2WithMain)

    enable_testing()
    include(Catch)
    catch_discover_tests(core_tests)
endif()
