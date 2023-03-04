/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainTutorialAudioProcessorEditor::GainTutorialAudioProcessorEditor(GainTutorialAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)

{
    //GAIN SLIDER
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    //don't need to set range or default value when using parameters
    //mGainSlider.setRange(-60.0f, 0.0f, 0.01f);
    //mGainSlider.setValue(-18.0f);
    mGainSlider.addListener(this);
    addAndMakeVisible(mGainSlider);
    //connect gain slider parameter for gain to visual for plugin
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "GAIN", mGainSlider);

    //PHASE BUTTON
    addAndMakeVisible(mPhaseButton);
    mPhaseButton.setButtonText("Invert Phase");
    mPhaseButton.addListener(this);
    phaseButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "INVERT_PHASE", mPhaseButton);

    //WELCOME LABEL
    addAndMakeVisible(welcomeLabel);
    welcomeLabel.setText("Welcome to my gain tutorial plugin!", juce::dontSendNotification);
    welcomeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    welcomeLabel.setColour(juce::Label::textColourId, juce::Colours::blue);
    //welcomeLabel.setJustificationType(juce::Justification::centredTop);


    ////STATE LABEL
    //addAndMakeVisible(stateLabel);
    //stateLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    //stateLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (300, 300);
}

GainTutorialAudioProcessorEditor::~GainTutorialAudioProcessorEditor()
{
}

//==============================================================================
void GainTutorialAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GainTutorialAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    mGainSlider.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 75, 100, 150);

    
    mPhaseButton.setBounds(getWidth() / 2 - 50, getHeight() / 2 + 20, 100, 150);
    mPhaseButton.changeWidthToFitText();

    welcomeLabel.setBounds(getWidth() / 2 - 100, getHeight() / 2 - 120, 200, 30);
    
}

void GainTutorialAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &mGainSlider)
    {
        audioProcessor.mGain = mGainSlider.getValue();
    }

}

void GainTutorialAudioProcessorEditor::buttonClicked(juce::Button* button) 
{
    if (button == &mPhaseButton) 
    {
        audioProcessor.mPhase = mPhaseButton.getToggleState();
        
        //convert bool to string
        std::string boolText = "";
        if (audioProcessor.mPhase)
        {
            boolText = "True";
        }
        else
        {
            boolText = "False";
        }

        //stateLabel.setText(boolText, juce::dontSendNotification);
    }
}
