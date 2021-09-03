/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FyppluginAudioProcessorEditor::FyppluginAudioProcessorEditor (FyppluginAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeStateReference(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    
    addAndMakeVisible(targetSpectrumUI);
    addAndMakeVisible(currentSpectrumUI);
    addAndMakeVisible(differenceSpectrumUI);
    
    addAndMakeVisible (targetSpectrumLabel);
    targetSpectrumLabel.setText("Target", dontSendNotification);
    targetSpectrumLabel.setAlwaysOnTop(true);
    targetSpectrumLabel.setColour(juce::Label::textColourId ,juce::Colours::black);
    targetSpectrumLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    
    addAndMakeVisible (&currentSpectrumLabel);
    currentSpectrumLabel.setText("Current", dontSendNotification);
    currentSpectrumLabel.setAlwaysOnTop(true);
    currentSpectrumLabel.setColour(juce::Label::textColourId ,juce::Colours::black);
    currentSpectrumLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    
    addAndMakeVisible (&differenceSpectrumLabel);
    differenceSpectrumLabel.setText("Difference", dontSendNotification);
    differenceSpectrumLabel.setAlwaysOnTop(true);
    differenceSpectrumLabel.setColour(juce::Label::textColourId ,juce::Colours::black);
    differenceSpectrumLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    
    addAndMakeVisible (&matchSpectrumsButton);
    matchSpectrumsButton.setButtonText ("Match");
    matchSpectrumsButton.addListener(this);
    
    filterIntensitySlider.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(filterIntensitySlider);
    filterIntensityAttachment.reset (new SliderAttachment (valueTreeStateReference, "filtersIntensity", filterIntensitySlider)); // DIRECTLY ATTACHES OUR INTENSITY SLIDER TO THE VALUE TREE IN AUDIOPROCESSOR, IN ORDER TO SAVE AND RECALL THE VALUES WHEN THE PLUG-IN IS CLOSED AND RE-OPENED
    
    addAndMakeVisible (filterIntensityLabel);
    filterIntensityLabel.setText("Intensity", dontSendNotification);
    filterIntensityLabel.setColour(juce::Label::textColourId ,juce::Colours::white);
    filterIntensityLabel.setFont (juce::Font (20.0f, juce::Font::bold));
    
    for (int i = 0; i < NumberOfAnalysers; i++)
    {
        addAndMakeVisible (startOrStopAveragingButton[i]);
        startOrStopAveragingButton[i].setButtonText ("Start capture");
        startOrStopAveragingButton[i].addListener(this);
        startOrStopAveragingButton[i].setColour (juce::TextButton::buttonColourId, juce::Colours::green);
        
        addAndMakeVisible (clearAverageButton[i]);
        clearAverageButton[i].setButtonText ("Clear average");
        clearAverageButton[i].addListener(this);
        clearAverageButton[i].setColour (juce::TextButton::buttonColourId, juce::Colours::red);
    }
 
    setResizable(false, false);
    setSize (1350 , 900);
    
    // ATTACHES EACH SPECTRUM WINDOW UI (TARGET, CURRENT AND DIFFERENCE) TO ITS RESPECTIVE AUDIO COUNTERPART
    targetSpectrumUI.setSpectrumAudioToDisplay(&audioProcessor.targetSpectrumAudio);
    currentSpectrumUI.setSpectrumAudioToDisplay(&audioProcessor.currentSpectrumAudio);
    differenceSpectrumUI.setDifferenceToDisplay(&audioProcessor.differenceAudio);
    //
    
    filterIntensitySlider.onValueChange = [this]
    {
        differenceSpectrumUI.intensityLevel = *audioProcessor.intensityParameter;
        differenceSpectrumUI.displayDifference();
    };

    
    differenceSpectrumUI.intensityLevel = *audioProcessor.intensityParameter;
    differenceSpectrumUI.displayDifference(); // WHENEVER THE UI IS RE-OPENED, DISPLAY THE SPECTRAL DIFFERENCE RESULTS
}

FyppluginAudioProcessorEditor::~FyppluginAudioProcessorEditor()
{
    
}

//==============================================================================
void FyppluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::white);
}

void FyppluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto differenceWidth = getWidth()/2 -20;
    
    targetSpectrumUI.setBounds (10, 100, getWidth()/2 - 20, getHeight() - 500);
    targetSpectrumLabel.setBounds(differenceWidth/2 - 25, 100, 80, 30);
    
    currentSpectrumUI.setBounds (10 + (getWidth()/2), 100, getWidth()/2 -20, getHeight() - 500);
    currentSpectrumLabel.setBounds((getWidth()/2) + differenceWidth/2 - 20, 100, 80, 30);
    
    startOrStopAveragingButton[0].setBounds (10, 40, getWidth()/2 - 20, 20);
    clearAverageButton[0].setBounds (10, 70, getWidth()/2 - 20, 20);
    
    startOrStopAveragingButton[1].setBounds (10 + (getWidth()/2), 40, getWidth()/2 - 20, 20);
    clearAverageButton[1].setBounds (10 + (getWidth()/2), 70, getWidth()/2 - 20, 20);
    
    differenceSpectrumUI.setBounds ( getWidth()/2 - differenceWidth/2, getHeight()/2 +70 , differenceWidth, getHeight()/2 - 80);
    differenceSpectrumLabel.setBounds(getWidth()/2 - 40, getHeight()/2 +70, 80, 30);
    
    matchSpectrumsButton.setBounds (1050, getHeight()/2 +70 , 250, 50);
    
    filterIntensitySlider.setBounds(1090, getHeight()/2 + 120, 100, 300);
    filterIntensityLabel.setBounds(1130, getHeight()/2 + 380, 100, 100);
}


void FyppluginAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &startOrStopAveragingButton[0])
    {
        if (audioProcessor.startAveraging[0] == false)
        {
            // START AVERAGING CHOSEN SPECTRUM
            audioProcessor.startAveraging[0] = true;
            startOrStopAveragingButton[0].setButtonText("Stop capture");
            startOrStopAveragingButton[0].setColour (juce::TextButton::buttonColourId, juce::Colours::red);
            
            startOrStopAveragingButton[1].setEnabled(false);
        }
        
        else if (audioProcessor.startAveraging[0] == true)
        {
            // STOP AVERAGING CHOSEN SPECTRUM
            audioProcessor.startAveraging[0] = false;
            startOrStopAveragingButton[0].setButtonText("Start capture");
            startOrStopAveragingButton[0].setColour (juce::TextButton::buttonColourId, juce::Colours::green);
            
            startOrStopAveragingButton[1].setEnabled(true);
            
            
            
            for (int i = 0; i < SpectrumAudio::fftSize; i++) // WHENEVER THE ANALYSIS STOPS, THE AVERAGE RESULTS ARRAY IS SAVED TO THE VALUE TREE IN AUDIOPROCESSOR AS PROPERTIES
            {
                String stringTarget("StringTarget");
                stringTarget += i;
                
                audioProcessor.parameters.state.setProperty(stringTarget, audioProcessor.targetSpectrumAudio.averageResults[i], nullptr);
            }
        }
    }
    
    else if (button == &clearAverageButton[0])
    {
        // CLEARS THE AVERAGE SPECTRUM SELECTED AND CLEARS THE FILTERS SETUP
        audioProcessor.targetSpectrumAudio.clearAverage();
        filterIntensitySlider.setValue(0.0);
        
        audioProcessor.differenceAudio.clearDifferenceResults();
        juce::zeromem (audioProcessor.differenceResultsToUse, sizeof (audioProcessor.differenceResultsToUse));
        audioProcessor.updateFilterIntensity();
        differenceSpectrumUI.displayDifference();
    }
    
    else if (button == &startOrStopAveragingButton[1])
    {
        if (audioProcessor.startAveraging[1] == false)
        {
            // START AVERAGING CHOSEN SPECTRUM
            audioProcessor.startAveraging[1] = true;
            startOrStopAveragingButton[1].setButtonText("Stop capture");
            startOrStopAveragingButton[1].setColour (juce::TextButton::buttonColourId, juce::Colours::red);
            
            startOrStopAveragingButton[0].setEnabled(false);
        }
        
        else if (audioProcessor.startAveraging[1] == true)
        {
            // STOP AVERAGING CHOSEN SPECTRUM
            audioProcessor.startAveraging[1] = false;
            startOrStopAveragingButton[1].setButtonText("Start capture");
            startOrStopAveragingButton[1].setColour (juce::TextButton::buttonColourId, juce::Colours::green);
            
            startOrStopAveragingButton[0].setEnabled(true);
            
            for (int i = 0; i < SpectrumAudio::fftSize; i++)// WHENEVER THE ANALYSIS STOPS, THE AVERAGE RESULTS ARRAY IS SAVED TO THE VALUE TREE IN AUDIOPROCESSOR AS PROPERTIES
            {
                String stringCurrent("StringCurrent");
                stringCurrent += i;
                
                audioProcessor.parameters.state.setProperty(stringCurrent, audioProcessor.currentSpectrumAudio.averageResults[i], nullptr);
            }
        }
    }
    
    else if (button == &clearAverageButton [1])
    {
        // CLEARS THE AVERAGE SPECTRUM SELECTED AND CLEARS THE FILTERS SETUP
        audioProcessor.currentSpectrumAudio.clearAverage();
        filterIntensitySlider.setValue(0.0);
        
        audioProcessor.differenceAudio.clearDifferenceResults();
        juce::zeromem (audioProcessor.differenceResultsToUse, sizeof (audioProcessor.differenceResultsToUse));
        audioProcessor.updateFilterIntensity();
        differenceSpectrumUI.displayDifference();
    }
    
    else if (button == &matchSpectrumsButton)
    {
        // TRIGGERS THE SPECTRAL DIFFERENCE CALCULATIONS AND REQUESTS AUDIOPROCESSOR TO SETUP THE FILTERS
        audioProcessor.differenceAudio.setAverageTarget(audioProcessor.targetSpectrumAudio.averageResults);
        audioProcessor.differenceAudio.setAverageCurrent(audioProcessor.currentSpectrumAudio.averageResults);
        audioProcessor.differenceAudio.calculateDifferenceBetweenAverages();
        
        filterIntensitySlider.setValue(0.6); // DEFAULT INTENSITY VALUE - 60 % 
        differenceSpectrumUI.displayDifference();
        audioProcessor.setupFilter();
    }
}

