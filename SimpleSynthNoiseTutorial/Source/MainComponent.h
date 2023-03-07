#pragma once

#include <JuceHeader.h>


//=========== custom decibel slider class =============
class DecibelSlider : public juce::Slider
{
public:
    DecibelSlider() {}

    double getValueFromText(const juce::String& text) override
    {
        //usually -100 is default -INF. Changing to -90.0 as part of tutorial exercise
        auto minusInfinitydB = -96.0;

        auto decibelText = text.upToFirstOccurrenceOf("dB", false, false).trim(); 

        //check if text = -INF. IF so, return -96 ( custom value of -inf as defined in this method). Else, return number value from text
        return decibelText.equalsIgnoreCase("-INF") ? minusInfinitydB
            : decibelText.getDoubleValue(); 
    }

    juce::String getTextFromValue(double value) override
    {
        //change default -INF from -100 to -96
        return juce::Decibels::toString(value, 2, -96.0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecibelSlider)
};

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

private:
    //==============================================================================

    DecibelSlider leftDecibelSlider;
    DecibelSlider rightDecibelSlider;
    juce::Slider leftLinearSlider;
    juce::Slider rightLinearSlider;
    juce::Label leftDecibelLabel;
    juce::Label rightDecibelLabel;
    juce::Label leftLinearLabel;
    juce::Label rightLinearLabel;
    juce::Random random;
    float leftLevel{ 0.0f};
    float rightLevel{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
