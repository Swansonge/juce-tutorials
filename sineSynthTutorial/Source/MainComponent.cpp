#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
//intialize synthAudioSource and keyboardComponent members
    : synthAudioSource (keyboardState),
      keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{

    addAndMakeVisible(keyboardComponent);
    setAudioChannels(0, 2);


    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);
    
    startTimer(400);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    synthAudioSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    synthAudioSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    keyboardComponent.setBounds(10, 40, getWidth() - 20, getHeight() - 50);
}

//callback for when startTimer() is called
void MainComponent::timerCallback()
{
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}


//----- SynthAudioSource methods -----

SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyState)
    : keyboardState(keyState)
{
    //We add some voices to our synthesiser.This number of voices added determines the polyphony of the synthesiser.
    //so, 4 voices in this case
    for (auto i = 0; i < 4; ++i)
        synth.addVoice(new SineWaveVoice());

    //We add the sound so that the synthesiser knows which sounds it can play.
    synth.addSound(new SineWaveSound());
}

void SynthAudioSource::setUsingSineWaveSound()
{
    synth.clearSounds();
}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    //The synthesiser needs to know the sample rate of the audio output.
    synth.setCurrentPlaybackSampleRate(sampleRate);
    //In order to process the timestamps of the MIDI data the MidiMessageCollector class needs to know the audio sample rate
    midiCollector.reset(sampleRate);
}

void SynthAudioSource::releaseResources()
{

}

void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;

    //pull any MIDI messages for each block of audio using the MidiMessageCollector::removeNextBlockOfMessages()
    midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);

    //In the getNextAudioBlock() function we pull buffers of MIDI data from the MidiKeyboardState object.
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

    //These buffers of MIDI are passed to the synthesiser which will be used to render the voices using the timestamps of the note-on and note-off messages (and other MIDI channel voice messages).
    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}

//We'll need access to this MidiMessageCollector object from outside the SynthAudioSource class, so add an accessor
juce::MidiMessageCollector* SynthAudioSource::getMidiCollector()
{
    return &midiCollector;
}



//----- SineWaveVoice methods -----
 
//return whether the voice can play a sound
bool SineWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    //use dynamic_cast to check the type of the sound class being passed in
    return dynamic_cast<SineWaveSound*> (sound) != nullptr;
}

//A voice is started by the owning synthesiser by calling our SynthesiserVoice::startNote() function
void SineWaveVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentAngle = 0.0;
    //use velocity to change level of note played
    level = velocity * 0.15;
    tailOff = 0.0;

    //convert MIDI note to frequency
    auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();

    //multiply by 2pi, length of whole sine wave
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

//generate audio
void SineWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        //When the key has been released the tailOff value will be greater than zero
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                //play in mono or stereo depending on # of output channels
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    //startSample is current index to add sample to because numSamples is decremented each loop iteration and startSample is incremented
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                ++startSample;

                //We use a simple exponential decay envelope shape.
                tailOff *= 0.99;

                //When the tailOff value is small we determine that the voice has ended
                if (tailOff <= 0.005)
                {
                    //We must call the SynthesiserVoice::clearCurrentNote() function at this point so that the voice is reset and available to be reused.
                    clearCurrentNote();

                    angleDelta = 0.0;
                    break;
                }

            }
        }

        //This loop is used for the normal state of the voice, while the key is being held down
        else
        {
            while(--numSamples >= 0)
            {
                auto currentSample = (float)(std::sin(currentAngle) * level);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
}

//voice is stopped by the owning synthersiser calling our SynthesiserVoice::stopNote()
void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }

    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }
}