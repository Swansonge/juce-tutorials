/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayTutorialAudioProcessor::DelayTutorialAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                          //when plugin initializes it needs list of parameters
                       ), params (*this, nullptr, "Parameters", createParameters())
#endif
{
}

DelayTutorialAudioProcessor::~DelayTutorialAudioProcessor()
{
}

//==============================================================================
const juce::String DelayTutorialAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayTutorialAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayTutorialAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayTutorialAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayTutorialAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayTutorialAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayTutorialAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayTutorialAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayTutorialAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayTutorialAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayTutorialAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //set circular buffer to hold 2 sec of audio
    auto delayBufferSize = sampleRate * 2.0;
    delayBuffer.setSize(getNumOutputChannels(), (int)delayBufferSize);
}

void DelayTutorialAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayTutorialAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DelayTutorialAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {

        //copy inpout signal to delay buffer
        fillBuffer(buffer, channel);
        //read from the past in the delay buffer
        readFromBuffer(buffer, delayBuffer, channel);
        //copy delayed signal to delay buffer again to create feedback loop
        fillBuffer(buffer, channel);
        
    }

    updateBufferPosition(buffer, delayBuffer);
}

void DelayTutorialAudioProcessor::fillBuffer(juce::AudioBuffer<float>& buffer, int channel)
{

    //auto* channelData = buffer.getWritePointer(channel);

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    //check to see if main buffer copies to delay buffer without needing to wrap
    if (delayBufferSize > bufferSize + writePosition)
    {
        //copy main buffer contents to delay buffer
        delayBuffer.copyFrom(channel, writePosition, buffer.getWritePointer(channel), bufferSize);
    }
    //if no
    else
    {
        //determine how much space is left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;

        //copy that amount of contents to end of delay buffer
        delayBuffer.copyFrom(channel, writePosition, buffer.getWritePointer(channel), numSamplesToEnd);

        //calculate how much content is remaining to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        //copy remaining amount to beginning of delay buffer
        delayBuffer.copyFrom(channel, 0, buffer.getWritePointer(channel, numSamplesToEnd), numSamplesAtStart);
    }
}

//read from delay buffer to main buffer to play delayed signal
void DelayTutorialAudioProcessor::readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, int channel)
{

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
  

    //read position is 1 sec in the past of write position (current position)
    // !!changing readPosition changes delay amount!!
    auto* delayTime = params.getRawParameterValue("DELAYMS");
    auto readPosition = writePosition - (delayTime->load());

    //feedback
    auto* gain = params.getRawParameterValue("FEEDBACK");
    auto g = gain->load();

    //if read position is < 0, pull from END indices of delay buffer
    if (readPosition < 0)
        //+= because adding a negative number
        readPosition += delayBufferSize;

    //easy situation: has room to write bufferSize amount of data to buffer (won't spill over to beginning)
    if (readPosition + bufferSize < delayBufferSize)
    {
        //addFrom() adds data to main buffer without overwriting contents
        //addFromWithRamp() because delay signal is a little quieter than main signal
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, g, g);
    }
    //if read position is towards end of delay buffer and doesn't have bufferSize amount of data to copy over to the main buffer.
    //copy some to end of buffer, then some to beginning of buffer
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, g, g);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        //destStartSample is not 0 because we've already added numSamplesToEnd of samples to main buffer
        //sampleIndex for getReadPointer() is 0 because we've wrapped back to the beginning of the delayu buffer to grab samples from
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, g, g);
    }
}

void DelayTutorialAudioProcessor::updateBufferPosition(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

//==============================================================================
bool DelayTutorialAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayTutorialAudioProcessor::createEditor()
{
    // !! GenericAudioProcessorEditor dynamically creates generic JUCE UI from listed parameters !!
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void DelayTutorialAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayTutorialAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayTutorialAudioProcessor();
}

//function to create AudioProcessorValueTreeState parameters
juce::AudioProcessorValueTreeState::ParameterLayout DelayTutorialAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("DELAYMS", "Delay ms", 0.0f, 96000.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", 0.0f, 1.0f, 0.0f));

    return { params.begin(), params.end() };
}

