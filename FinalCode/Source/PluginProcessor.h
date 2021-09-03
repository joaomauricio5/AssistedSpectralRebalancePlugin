/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpectrumUI.hpp"
#include "SpectrumAudio.hpp"
#include "DifferenceSpectrumAudio.hpp"

//==============================================================================
/**
*/
class FyppluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FyppluginAudioProcessor();
    ~FyppluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    
    
    //==============================================================================
    
        enum numberOfFilters
    {
        numberOfLinearFilters = 47,
        numberOfLogFilters = 209,
        numberOfFilters = 256
    };
    
    void setupFilter();
    void updateFilterIntensity();
    
    bool startAveraging[2] = {false, false};
    
    float differenceResultsToUse[SpectrumAudio::fftSize] = {0};
    
    SpectrumAudio targetSpectrumAudio;
    SpectrumAudio currentSpectrumAudio;
    DifferenceAudio differenceAudio;
    
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* intensityParameter  = nullptr;
    
private:
    //==============================================================================
    
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> peakFilter[numberOfFilters];
    

    float linearQvalue[numberOfLinearFilters] = {0};
    float logQvalue;
    
    float centreFrequency[numberOfFilters];
    
    float lastIntensityValue = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FyppluginAudioProcessor)
};

