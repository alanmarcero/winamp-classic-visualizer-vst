#pragma once

#include "FrameQueue.h"
#include "Oscilloscope.h"
#include "Palette.h"
#include "SpectrumAnalyzer.h"

#include <juce_audio_processors/juce_audio_processors.h>

// Passes audio through unchanged and feeds it to the classic vis.
class VisualizerProcessor final : public juce::AudioProcessor {
public:
    VisualizerProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override { }
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override { }
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override { }

    void getStateInformation(juce::MemoryBlock& destination) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // An empty text restores the default colours.
    void setViscolor(const juce::String& text);
    wcv::Palette palette() const;

    // Whole-number zoom of the 76 x 16 vis area, saved with the project.
    static constexpr int kMinScale = 1;
    static constexpr int kMaxScale = 5;
    std::atomic<int> editorScale { 3 };

    juce::AudioProcessorValueTreeState parameters;
    FrameQueue<wcv::SpectrumFrame> spectrumFrames;
    FrameQueue<wcv::ScopeFrame> scopeFrames;

private:
    wcv::SpectrumAnalyzer analyzer_;
    wcv::Oscilloscope scope_;

    mutable juce::CriticalSection paletteLock_;
    juce::String viscolor_;
    wcv::Palette palette_ = wcv::kDefaultPalette;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerProcessor)
};
