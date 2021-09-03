/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumUI.hpp"
#include "DifferenceSpectrumUI.hpp"

//==============================================================================
/**
*/
class FyppluginAudioProcessorEditor  : public AudioProcessorEditor,
                                       public Button::Listener
{
    
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    
    
public:
    FyppluginAudioProcessorEditor (FyppluginAudioProcessor&, AudioProcessorValueTreeState& vts);
    ~FyppluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FyppluginAudioProcessor& audioProcessor;
    
    SpectrumUI targetSpectrumUI;
    Label targetSpectrumLabel;
    
    SpectrumUI currentSpectrumUI;
    Label currentSpectrumLabel;
    
    DifferenceUI differenceSpectrumUI;
    Label differenceSpectrumLabel;
    
    enum
      {
          NumberOfAnalysers = 2 // NUMBER OF LONG-TERM AVERAGE SPECTRUMS IN THE PLUG-IN
      };
    
    TextButton startOrStopAveragingButton[NumberOfAnalysers];
    TextButton clearAverageButton[NumberOfAnalysers];
    
    TextButton matchSpectrumsButton;
    
    Slider filterIntensitySlider;
    Label filterIntensityLabel;
    
    void buttonClicked (Button* button) override;
    
    
    juce::AudioProcessorValueTreeState& valueTreeStateReference; // REFERENCE TO THE VALUE TREE IN AUDIOPROCESSOR
    std::unique_ptr<SliderAttachment> filterIntensityAttachment; // ATTACHES OUR INTENSITY FILTER TO AUDIOPROCESSOR'S VALUE TREE
 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FyppluginAudioProcessorEditor)
};
