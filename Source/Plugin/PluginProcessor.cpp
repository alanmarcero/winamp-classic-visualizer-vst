#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"

namespace {
constexpr auto kViscolorProperty = "viscolor";
constexpr auto kScaleProperty = "scale";
}

VisualizerProcessor::VisualizerProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "WinampClassicVis", params::createLayout())
{
}

void VisualizerProcessor::prepareToPlay(double, int) { }

bool VisualizerProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    const bool monoOrStereo = output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
    return monoOrStereo && output == layouts.getMainInputChannelSet();
}

void VisualizerProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int channels = getTotalNumInputChannels();
    if (channels == 0)
        return;

    const auto samples = static_cast<std::size_t>(buffer.getNumSamples());
    const std::span<const float> left(buffer.getReadPointer(0), samples);
    const std::span<const float> right(buffer.getReadPointer(channels > 1 ? 1 : 0), samples);
    analyzer_.push(left, right, [this](const wcv::SpectrumFrame& frame) { spectrumFrames.push(frame); });
    scope_.push(left, right, std::min(channels, 2), [this](const wcv::ScopeFrame& frame) { scopeFrames.push(frame); });
}

juce::AudioProcessorEditor* VisualizerProcessor::createEditor()
{
    return new VisualizerEditor(*this);
}

void VisualizerProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    {
        const juce::ScopedLock lock(paletteLock_);
        state.setProperty(kViscolorProperty, viscolor_, nullptr);
    }
    state.setProperty(kScaleProperty, editorScale.load(), nullptr);
    if (const auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void VisualizerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || !xml->hasTagName(parameters.state.getType()))
        return;

    const auto state = juce::ValueTree::fromXml(*xml);
    parameters.replaceState(state);
    setViscolor(state.getProperty(kViscolorProperty).toString());
    const int scale = state.getProperty(kScaleProperty, editorScale.load());
    editorScale = std::clamp(scale, kMinScale, kMaxScale);
}

void VisualizerProcessor::setViscolor(const juce::String& text)
{
    const auto parsed = wcv::parseViscolor(text.toStdString());
    const juce::ScopedLock lock(paletteLock_);
    viscolor_ = text;
    palette_ = parsed;
}

wcv::Palette VisualizerProcessor::palette() const
{
    const juce::ScopedLock lock(paletteLock_);
    return palette_;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VisualizerProcessor();
}
