#include "MainComponent.h"


//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Specify the number of input and output channels that we want to open
    setAudioChannels (0, 2);

    //set range in decibels
    leftDecibelSlider.setRange(-96.0f, 0.0f);
    rightDecibelSlider.setRange(-96.0f, 0.0f);
    leftLinearSlider.setRange(0.0f, 1.0f);
    rightLinearSlider.setRange(0.0f, 1.0f);

    leftDecibelSlider.setValue(juce::Decibels::gainToDecibels(leftLevel, -96.0f));
    rightDecibelSlider.setValue(juce::Decibels::gainToDecibels(rightLevel, -96.0f));
    leftLinearSlider.setValue(0.0f);
    rightLinearSlider.setValue(0.0f);

    //lambda functions that will be called whenever slider value is changed
    //1) convert value of gain to decibels 
    leftDecibelSlider.onValueChange = [this] { leftLevel = juce::Decibels::decibelsToGain((float)leftDecibelSlider.getValue(), -96.0f); };
    rightDecibelSlider.onValueChange = [this] { rightLevel = juce::Decibels::decibelsToGain((float)rightDecibelSlider.getValue(), -96.0f); };
    //leftLinearSlider.onValueChange = [this] { leftLevel = leftLinearSlider.getValue(); leftDecibelSlider.setValue(juce::Decibels::gainToDecibels((float)leftLevel)); };
    //rightLinearSlider.onValueChange = [this] { rightLevel = rightLinearSlider.getValue(); rightDecibelSlider.setValue(juce::Decibels::gainToDecibels((float)rightLevel)); };

    leftDecibelLabel.setText("Noise Level in dB", juce::dontSendNotification);
    rightDecibelLabel.setText("Noise Level in dB", juce::dontSendNotification);
    leftLinearLabel.setText("Noise Level (linear)", juce::dontSendNotification);
    rightLinearLabel.setText("Noise Level (linear)", juce::dontSendNotification);

}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++)
    {
        //get function-local copy of level
        auto currentLeftLevel = leftLevel;
        auto leftLevelScale = currentLeftLevel * 2.0f;

        auto currentRightLevel = rightLevel;
        auto rightLevelScale = currentRightLevel * 2.0f;

        //Fill the required number of samples with noise between -0.125 and +0.125
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            //adjust individual levels for left and right sliders
            if (channel == 0) 
            {
                //Get a pointer to the start sample in the buffer for this audio output channel
                auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                //generate white noise.To get range of -x to x for noise, multiply nextFloat() * 2x then subtract x
                buffer[sample] = random.nextFloat() * leftLevelScale - currentLeftLevel;
            }
            else if (channel == 1)
            {

                //Get a pointer to the start sample in the buffer for this audio output channel
                auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                buffer[sample] = random.nextFloat() * rightLevelScale - currentRightLevel;
            }
            
            
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    juce::Logger::getCurrentLogger()->writeToLog("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //left channel decibel slider
    addAndMakeVisible(leftDecibelSlider);
    leftDecibelSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);

    ////left channel linear slider
    //addAndMakeVisible(leftLinearSlider);
    //leftLinearSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);
    
    //left slider decibel label
    addAndMakeVisible(leftDecibelLabel);
    leftDecibelLabel.attachToComponent(&leftDecibelSlider, true);

    ////left slider linear label
    //addAndMakeVisible(leftLinearLabel);
    //leftLinearLabel.attachToComponent(&leftLinearSlider, true);

    //right channel decibel slider
    addAndMakeVisible(rightDecibelSlider);
    rightDecibelSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);

    ////right channel linear slider
    //addAndMakeVisible(rightLinearSlider);
    //rightLinearSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);

    //right slider decibel label
    addAndMakeVisible(rightDecibelLabel);
    rightDecibelLabel.attachToComponent(&rightDecibelSlider, true);

    ////right slider linear label
    //addAndMakeVisible(rightLinearLabel);
    //rightLinearLabel.attachToComponent(&rightLinearSlider, true);

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    auto sliderLeft = 120;
    leftDecibelSlider.setBounds(sliderLeft, 50, getWidth() - sliderLeft * 2, 50);
    rightDecibelSlider.setBounds(sliderLeft, 100, getWidth() - sliderLeft * 2, 50); 
    leftLinearSlider.setBounds(sliderLeft, 150, getWidth() - sliderLeft * 2, 50);
    rightLinearSlider.setBounds(sliderLeft, 200, getWidth() - sliderLeft * 2, 50);
    
}
