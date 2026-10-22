#pragma once

#include "ClassicVis.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

// The 76 x 16 vis area at a whole-number zoom, drawn with nearest-neighbour pixels.
// Left click cycles the mode and right click opens the options menu,
// as on Winamp's main window. The Settings button below the vis also opens the menu.
class VisualizerEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit VisualizerEditor(VisualizerProcessor& owner);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    void renderImage();
    void showOptionsMenu();
    void addChoiceMenu(juce::PopupMenu& menu, const char* parameterId, const juce::String& title);
    void setParameterIndex(const char* parameterId, int index);
    void chooseViscolor();
    void setScale(int scale);

    VisualizerProcessor& processor_;
    wcv::ClassicVis vis_;
    wcv::SpectrumFrame spectrum_ {};
    wcv::ScopeFrame scope_ {};
    std::uint32_t lastSpectrumMs_ = 0;
    std::uint32_t lastScopeMs_ = 0;
    juce::Image image_ { juce::Image::RGB, wcv::kVisWidth, wcv::kVisHeight, true };
    std::unique_ptr<juce::FileChooser> chooser_;
    juce::TextButton settingsButton_ { "Settings" };
    juce::Label versionLabel_ { {}, "v1.1.0" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerEditor)
};
