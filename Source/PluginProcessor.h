#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"
#include "Params.h"
#include "Presets.h"

inline juce::String u8 (const char* s) { return juce::String (juce::CharPointer_UTF8 (s)); }

class RawstackProcessor : public juce::AudioProcessor
{
public:
    RawstackProcessor();
    ~RawstackProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Rawstack"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    rs::Engine engine;
    std::atomic<int> currentProgram { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    std::vector<std::atomic<float>*> rawParams;
    juce::AudioBuffer<float> monoTemp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RawstackProcessor)
};
