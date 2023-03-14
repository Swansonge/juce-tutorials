#pragma once

#include <JuceHeader.h>

//Sound class. Doesn't need to contain any data, just needs to report whether this sound should play on particular MIDI channels and specific notes or note ranges on that channel
struct SineWaveSound : public juce::SynthesiserSound
{
    //no code block for constructor
    SineWaveSound() {}

    //override function with simple one liner. Always return true
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};


//class that needs to maintain the state of one of the voices of the synthesiser
struct SineWaveVoice : public juce::SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;

    void stopNote(float /*velocity*/, bool allowTailOff) override;

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;


private:
    //first 3 vars from sine wave generator tutorial
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    //tailOff used to give each voice a slightly softer release to its amplitude envelope
    double tailOff = 0.0;
};


//SynthAudioSource: This implements a custom AudioSource class called SynthAudioSource, which contains the Synthesiser class itself. This outputs all of the audio from the synthesiser.
class SynthAudioSource : public juce::AudioSource
{
public:
    //constructor
    SynthAudioSource(juce::MidiKeyboardState& keyState);

    //public methods
    void  setUsingSineWaveSound();
    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    juce::MidiMessageCollector* getMidiCollector();

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;

    //members for handling MIDI events
};


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    juce::MidiKeyboardState keyboardState;
    SynthAudioSource synthAudioSource;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


