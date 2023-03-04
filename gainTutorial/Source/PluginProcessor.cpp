/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//Don't have a wicked long list of MIDI CC parameters that do nothing
#define JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS 0

//==============================================================================
GainTutorialAudioProcessor::GainTutorialAudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       //below is initializer list
                       //apvts descrip: *this deferences pointer to AudioProcessor object, nullptr becuase we aren't using undo manager, createParameters() returns ParameterLayout object
                       ), 
                       apvts(*this, nullptr, juce::Identifier("Parameters"), createParameters())
                       
#endif
//start actual code of constructor
{
    phaseParam = apvts.getRawParameterValue("INVERT_PHASE");
    gainParam = apvts.getRawParameterValue("GAIN");
}


//~ is destructor, called when destroying object
GainTutorialAudioProcessor::~GainTutorialAudioProcessor()
{
}

//==============================================================================
const juce::String GainTutorialAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GainTutorialAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GainTutorialAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GainTutorialAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GainTutorialAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GainTutorialAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GainTutorialAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GainTutorialAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GainTutorialAudioProcessor::getProgramName (int index)
{
    return {};
}

void GainTutorialAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GainTutorialAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //get previous value of phase parameter. If value < 0.5, set phase = 1.0. Else, phase = 1.0f
    auto phase = *phaseParam < 0.5f ? 1.0f : -1.0f;
    //account for phase
    previousGain = *gainParam;
    previousGain = juce::Decibels::decibelsToGain(previousGain) * phase;
    
}

void GainTutorialAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GainTutorialAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


void GainTutorialAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //load in parameter states
    auto phase = *phaseParam < 0.5f ? 1.0f : -1.0f;
    mGain = *gainParam;
    mGain = juce::Decibels::decibelsToGain(mGain) * phase;

    if (mGain == previousGain)
    {
        buffer.applyGain(mGain);
    }
    //if gain has changed, apply gain ramp to prevent clicks and pops
    else
    {
        buffer.applyGainRamp(0, buffer.getNumSamples(), previousGain, mGain);
    }

    ////g is std atomic float pointer for gain
    //auto gainParam = apvts.getRawParameterValue("GAIN");
    ////use load to get value back from std atomic float pointer
    //gainParam->load();

    //for now I'm going to ignore looping through channels and samples.....
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //    buffer.clear (i, 0, buffer.getNumSamples());

    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
    //    auto* channelData = buffer.getWritePointer (channel);

    //    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    //    {
    //        channelData[sample] = channelData[sample] * juce::Decibels::decibelsToGain(mGain);
    //    }
    //}
}

//==============================================================================
bool GainTutorialAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GainTutorialAudioProcessor::createEditor()
{
    return new GainTutorialAudioProcessorEditor (*this);
}

//==============================================================================
void GainTutorialAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData); 
}

void GainTutorialAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    //check that the ValueTree-generated XML is of the correect ValueTree type (uses tag name)
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainTutorialAudioProcessor();
}

//method to get parameter layout object (parameter and value pairs)
juce::AudioProcessorValueTreeState::ParameterLayout GainTutorialAudioProcessor::createParameters()
{
    //vector (list) of pointers for parameter objects
    // <> donates what type of objects the vector holds
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    //push_back method appends object to vector
    //std::make_unique allocates memory for unique_ptr
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", -60.0f, 0.0f, -18.0f)); //gain parameter
    params.push_back(std::make_unique<juce::AudioParameterBool>("INVERT_PHASE", "Invert Phase", false)); //invert phase parameter

    return { params.begin(), params.end() };
}