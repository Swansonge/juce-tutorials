/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicSVFAudioProcessor::BasicSVFAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

BasicSVFAudioProcessor::~BasicSVFAudioProcessor()
{
}

//==============================================================================
const juce::String BasicSVFAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicSVFAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicSVFAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicSVFAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicSVFAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicSVFAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicSVFAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicSVFAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicSVFAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicSVFAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicSVFAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //filter needs to pass in dsp processSpec object for prepare() method
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    filter.prepare(spec);
    reset();
}

void BasicSVFAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicSVFAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicSVFAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    filter.setCutoffFrequency(150.0f);

    //alias for audio buffer needed for dsp processing
    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    //ProcessContextReplacing replaces incoming audio with dsp-processed audio
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    filter.process(context);
}

//==============================================================================
bool BasicSVFAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicSVFAudioProcessor::createEditor()
{
    return new BasicSVFAudioProcessorEditor (*this);
}

//==============================================================================
void BasicSVFAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicSVFAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicSVFAudioProcessor();
}

void BasicSVFAudioProcessor::reset()
{
    filter.reset();
}

void BasicSVFAudioProcessor::setType()
{
    //so we don't have to type out the whole class structure for every filter type
    using fType = juce::dsp::StateVariableTPTFilterType;

    switch (filterType)
    {
        case FilterType::LowPass:
            filter.setType(fType::lowpass);
            break;

        case FilterType::BandPass:
            filter.setType(fType::bandpass);
            break;

        case FilterType::HighPass:
            filter.setType(fType::highpass);
            break;

        default:
            filter.setType(fType::lowpass);
            break;
    }
}
