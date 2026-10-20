#include "PluginProcessor.h"
#include "PluginEditor.h"

RawstackProcessor::RawstackProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "RAWSTACK", createLayout())
{
    for (const auto& d : paramDefs())
        rawParams.push_back (apvts.getRawParameterValue (d.id));
    setCurrentProgram (0); // по умолчанию: «just RAW: оригинал»
}

juce::AudioProcessorValueTreeState::ParameterLayout RawstackProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (const auto& d : paramDefs())
    {
        const juce::ParameterID pid { d.id, 1 };
        if (d.type == 'c')
        {
            juce::StringArray items;
            for (auto* c : d.choices) items.add (u8 (c));
            layout.add (std::make_unique<juce::AudioParameterChoice> (pid, u8 (d.name), items, (int) d.def));
        }
        else if (d.type == 'i')
        {
            layout.add (std::make_unique<juce::AudioParameterInt> (pid, u8 (d.name), (int) d.min, (int) d.max, (int) d.def));
        }
        else
        {
            juce::NormalisableRange<float> range (d.min, d.max);
            if (d.log)
                range = juce::NormalisableRange<float> (d.min, d.max,
                    [] (float s, float e, float n) { return s * std::pow (e / s, n); },
                    [] (float s, float e, float v) { return std::log (v / s) / std::log (e / s); });
            layout.add (std::make_unique<juce::AudioParameterFloat> (pid, u8 (d.name), range, d.def));
        }
    }
    return layout;
}

void RawstackProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
}

bool RawstackProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void RawstackProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (size_t i = 0; i < rawParams.size(); ++i)
        applyParam (engine.p, (int) i, rawParams[i]->load());

    buffer.clear();
    const int n = buffer.getNumSamples();
    float* L = buffer.getWritePointer (0);
    float* R = nullptr;
    if (buffer.getNumChannels() > 1) R = buffer.getWritePointer (1);
    else { monoTemp.setSize (1, n, false, false, true); monoTemp.clear(); R = monoTemp.getWritePointer (0); }

    int pos = 0;
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int t = juce::jlimit (0, n, metadata.samplePosition);
        if (t > pos) { engine.process (L + pos, R + pos, t - pos); pos = t; }
        if (msg.isNoteOn())                              engine.noteOn (msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isNoteOff())                        engine.noteOff (msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine.allOff();
    }
    if (pos < n) engine.process (L + pos, R + pos, n - pos);

    if (buffer.getNumChannels() == 1)
        for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + R[i]);
}

int RawstackProcessor::getNumPrograms() { return (int) factoryPresets().size(); }

void RawstackProcessor::setCurrentProgram (int index)
{
    const auto& ps = factoryPresets();
    if (index < 0 || index >= (int) ps.size()) return;
    currentProgram = index;
    const auto& defs = paramDefs();
    const auto& vals = ps[(size_t) index].values;
    for (size_t i = 0; i < defs.size() && i < vals.size(); ++i)
        if (auto* prm = apvts.getParameter (defs[i].id))
            prm->setValueNotifyingHost (prm->convertTo0to1 (vals[i]));
}

const juce::String RawstackProcessor::getProgramName (int index)
{
    const auto& ps = factoryPresets();
    if (index < 0 || index >= (int) ps.size()) return {};
    return u8 (ps[(size_t) index].name);
}

void RawstackProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void RawstackProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentProgram = (int) vt.getProperty ("program", 0);
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* RawstackProcessor::createEditor() { return new RawstackEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RawstackProcessor(); }
