#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Specify the number of input and output channels that we want to open
    setAudioChannels (0, 2);
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

        //Get a pointer to the start sample in the buffer for this audio output channel
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        //Fill the required number of samples with noise between -0.125 and +0.125
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            //adjust individual levels for left and right sliders
            if (channel == 0) 
            {
                auto leftLevel = (float)leftSlider.getValue();
                auto leftLevelScale = leftLevel * 2.0f;

                //generate white noise.To get range of -x to x for noise, multiply nextFloat() * 2x then subtract x
                buffer[sample] = random.nextFloat() * leftLevelScale - leftLevel;
            }
            else if (channel == 1)
            {
                auto rightLevel = (float)rightSlider.getValue();
                auto rightLevelScale = rightLevel * 2.0f;

                //generate white noise.To get range of -x to x for noise, multiply nextFloat() * 2x then subtract x
                buffer[sample] = random.nextFloat() * rightLevelScale - rightLevel;
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

    //left channel slider
    addAndMakeVisible(leftSlider);
    leftSlider.setRange(0.0f, 1.0f);
    leftSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);
    
    //left slider label
    addAndMakeVisible(leftLevelLabel);
    leftLevelLabel.setText("Noise Level", juce::dontSendNotification);
    leftLevelLabel.attachToComponent(&leftSlider, true);

    //right channel slider
    addAndMakeVisible(rightSlider);
    rightSlider.setRange(0.0f, 1.0f);
    rightSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 75, 25);

    //right slider label
    addAndMakeVisible(rightLevelLabel);
    rightLevelLabel.setText("Noise Level", juce::dontSendNotification);
    rightLevelLabel.attachToComponent(&rightSlider, true);

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    auto sliderLeft = 120;
    leftSlider.setBounds(sliderLeft, 50, getWidth() - sliderLeft * 2, 50);
    rightSlider.setBounds(sliderLeft, 150, getWidth() - sliderLeft * 2, 50);
    
}
