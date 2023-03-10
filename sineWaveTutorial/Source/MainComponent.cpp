#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }

    frequencySlider.onValueChange = [this]
    {
        //before adding frequency increment smoothing
        /*if (currentSampleRate > 0.0)
            updateAngleDelta();*/

        targetFrequency = frequencySlider.getValue();
    };

    gainSlider.onValueChange = [this]
    {
        targetLevel = gainSlider.getValue();
    };

    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(50.0, 5000.0);
    frequencySlider.setSkewFactorFromMidPoint(500);
    frequencySlider.setValue(currentFrequency, juce::dontSendNotification);

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0.0f, 0.25f);
    gainSlider.setValue(currentLevel);

    setSize(600, 100);

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

    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    //auto level = gain; //for not using gain smoothing
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    //local copies in case target value changes on the visual thread
    auto localTargetLevel = targetLevel;
    auto localTargetFrequency = targetFrequency;

    //need gain smoothing to remove artefacts
    if (localTargetLevel != currentLevel)
    {
        auto levelIncrement = (localTargetLevel - currentLevel) / bufferToFill.numSamples;

        if (localTargetFrequency != currentFrequency)
        {
            //add frequency smoothing if current freq != target freq
            auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;

            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {

                //gain smoothing adjustment 
                currentLevel += levelIncrement;

                //frequency smoothing adjustment 
                auto currentSample = (float)std::sin(currentAngle);
                currentFrequency += frequencyIncrement;
                updateAngleDelta();
                currentAngle += angleDelta;

                leftBuffer[sample] = currentSample * currentLevel;
                rightBuffer[sample] = currentSample * currentLevel;
            }
        }
        //target frequency equals current frequency. No increment needed for frequency
        else
        {
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {

                //gain smoothing adjustment 
                currentLevel += levelIncrement;

                auto currentSample = (float)std::sin(currentAngle);
                currentAngle += angleDelta;
                leftBuffer[sample] = currentSample * currentLevel;
                rightBuffer[sample] = currentSample * currentLevel;
            }
        }
    }

    //no gain smoothing needed
    else
    {
        if (localTargetFrequency != currentFrequency)
        {
            //add frequency smoothing if current freq != target freq
            auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;

            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto currentSample = (float)std::sin(currentAngle);
                currentFrequency += frequencyIncrement;
                updateAngleDelta();
                currentAngle += angleDelta;
                leftBuffer[sample] = currentSample * currentLevel;
                rightBuffer[sample] = currentSample * currentLevel;
            }
        }
        //target frequency equals current frequency. No increment needed for frequency
        else
        {
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto currentSample = (float)std::sin(currentAngle);
                currentAngle += angleDelta;
                leftBuffer[sample] = currentSample * currentLevel;
                rightBuffer[sample] = currentSample * currentLevel;
            }
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
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
    frequencySlider.setBounds(10, 10, getWidth() - 20, 20); 
    gainSlider.setBounds(10, 40, getWidth() - 20, 20);
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}
