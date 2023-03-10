#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
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
    void updateAngleDelta();

private:
    //==============================================================================

    juce::Slider frequencySlider;
    juce::Slider gainSlider;
    double currentSampleRate = { 0.0 };
    double currentAngle = { 0.0 };
    double angleDelta = { 0.0 };
    float gain{ 0.0f };
    double currentFrequency = { 500 };
    double targetFrequency = { 500 };
    float currentLevel = { 0.125f };
    float targetLevel = { 0.125f };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
