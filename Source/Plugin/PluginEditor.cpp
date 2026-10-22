#include "PluginEditor.h"

#include "Parameters.h"

#include <ranges>

namespace {

constexpr int kModeCount = 3;

// sa_get in Src/Winamp/SA.cpp drops a frame more than 400 ms from now.
constexpr std::uint32_t kStaleMs = 400;

constexpr int kButtonHeight = 24;

template <typename Frame>
void takeLatest(FrameQueue<Frame>& queue, Frame& current, std::uint32_t& lastMs, std::uint32_t now)
{
    if (const auto frame = queue.popLatest()) {
        current = *frame;
        lastMs = now;
    }
    if (now - lastMs > kStaleMs)
        current.fill(0);
}

} // namespace

VisualizerEditor::VisualizerEditor(VisualizerProcessor& owner)
    : AudioProcessorEditor(owner)
    , processor_(owner)
{
    settingsButton_.onClick = [this] { showOptionsMenu(); };
    addAndMakeVisible(settingsButton_);

    versionLabel_.setText("v1.1.0", juce::dontSendNotification);
    versionLabel_.setJustificationType(juce::Justification::centred);
    versionLabel_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    versionLabel_.setFont(juce::FontOptions(11.0f));
    addAndMakeVisible(versionLabel_);
    setScale(owner.editorScale);
    renderImage();
    startTimerHz(wcv::kTickHz);
}

void VisualizerEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);
    g.drawImage(image_, getLocalBounds().withTrimmedBottom(kButtonHeight).toFloat());
}

void VisualizerEditor::resized()
{
    auto bottomBar = getLocalBounds().removeFromBottom(kButtonHeight);
    versionLabel_.setBounds(bottomBar.removeFromRight(60));
    settingsButton_.setBounds(bottomBar);
}

void VisualizerEditor::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu()) {
        showOptionsMenu();
        return;
    }
    const int current = static_cast<int>(params::readSettings(processor_.parameters).mode);
    setParameterIndex(params::kMode, (current + 1) % kModeCount);
}

void VisualizerEditor::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    takeLatest(processor_.spectrumFrames, spectrum_, lastSpectrumMs_, now);
    takeLatest(processor_.scopeFrames, scope_, lastScopeMs_, now);
    if (!vis_.tick(params::readSettings(processor_.parameters), spectrum_, scope_))
        return;

    renderImage();
    repaint();
}

void VisualizerEditor::renderImage()
{
    const auto palette = processor_.palette();
    const juce::Image::BitmapData pixels(image_, juce::Image::BitmapData::writeOnly);
    std::ranges::for_each(std::views::iota(0, wcv::kVisHeight), [&](int y) {
        std::ranges::for_each(std::views::iota(0, wcv::kVisWidth), [&](int x) {
            const auto& rgb = palette[vis_.pixelAt(x, y)];
            pixels.setPixelColour(x, y, juce::Colour(rgb[0], rgb[1], rgb[2]));
        });
    });
}

// Mirrors the vis section of Winamp's main-window right-click menu.
void VisualizerEditor::showOptionsMenu()
{
    const auto settings = params::readSettings(processor_.parameters);
    juce::PopupMenu menu;
    addChoiceMenu(menu, params::kMode, "Modes");

    juce::PopupMenu analyzer;
    addChoiceMenu(analyzer, params::kColoring, "Coloring");
    analyzer.addItem("Show peaks", true, settings.peaks,
                     [this, peaks = settings.peaks] { setParameterIndex(params::kPeaks, peaks ? 0 : 1); });
    addChoiceMenu(analyzer, params::kBandWidth, "Band width");
    menu.addSubMenu("Analyzer options", analyzer);

    addChoiceMenu(menu, params::kScopeStyle, "Oscilloscope options");
    addChoiceMenu(menu, params::kRefresh, "Refresh rate");
    addChoiceMenu(menu, params::kFalloff, "Analyzer falloff");
    addChoiceMenu(menu, params::kPeakFalloff, "Peaks falloff");

    juce::PopupMenu colors;
    colors.addItem("Load viscolor.txt...", [this] { chooseViscolor(); });
    colors.addItem("Default colors", [this] {
        processor_.setViscolor({});
        renderImage();
        repaint();
    });
    menu.addSubMenu("Colors", colors);

    juce::PopupMenu size;
    std::ranges::for_each(std::views::iota(VisualizerProcessor::kMinScale, VisualizerProcessor::kMaxScale + 1), [&](int scale) {
        size.addItem(juce::String(scale) + "x", true, processor_.editorScale == scale, [this, scale] { setScale(scale); });
    });
    menu.addSubMenu("Size", size);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMousePosition());
}

void VisualizerEditor::addChoiceMenu(juce::PopupMenu& menu, const char* parameterId, const juce::String& title)
{
    const auto* parameter = dynamic_cast<juce::AudioParameterChoice*>(processor_.parameters.getParameter(parameterId));
    juce::PopupMenu submenu;
    std::ranges::for_each(std::views::iota(0, parameter->choices.size()), [&](int index) {
        submenu.addItem(parameter->choices[index], true, parameter->getIndex() == index,
                        [this, parameterId, index] { setParameterIndex(parameterId, index); });
    });
    menu.addSubMenu(title, submenu);
}

void VisualizerEditor::setParameterIndex(const char* parameterId, int index)
{
    auto* parameter = processor_.parameters.getParameter(parameterId);
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>(index)));
    parameter->endChangeGesture();
}

void VisualizerEditor::chooseViscolor()
{
    chooser_ = std::make_unique<juce::FileChooser>("Load viscolor.txt", juce::File {}, "*.txt");
    chooser_->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this](const juce::FileChooser& chooser) {
                              const auto file = chooser.getResult();
                              if (file == juce::File {})
                                  return;
                              processor_.setViscolor(file.loadFileAsString());
                              renderImage();
                              repaint();
                          });
}

void VisualizerEditor::setScale(int scale)
{
    processor_.editorScale = scale;
    setSize(wcv::kVisWidth * scale, wcv::kVisHeight * scale + kButtonHeight);
}
