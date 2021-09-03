/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FyppluginAudioProcessor::FyppluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
,
parameters (*this, nullptr, juce::Identifier ("APVTSTutorial"),
                      {
                          std::make_unique<juce::AudioParameterFloat> ("filtersIntensity",            // parameterID
                                                                       "Intensity",            // parameter name
                                                                       -1.0f,              // minimum value
                                                                       1.0f,              // maximum value
                                                                       0.0f)             // default value
                      }) // AUDIO PROCESSOR PARAMETER FOR OUR FILTER INTENSITY SLIDER VALUE
{
    intensityParameter = parameters.getRawParameterValue("filtersIntensity");
}

FyppluginAudioProcessor::~FyppluginAudioProcessor()
{
}

//==============================================================================
const juce::String FyppluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FyppluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FyppluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FyppluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FyppluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FyppluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int FyppluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FyppluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FyppluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void FyppluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FyppluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    logQvalue = 69.2;
    
    for (int i = 0; i < numberOfFilters; i++)
    {
        if (i < 47) // SETTING UP CENTRE FREQUENCIES AND Q VALUES OF THE FIRST 47 FILTERS - LINEAR DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
        {
            centreFrequency[i] =  ((getSampleRate()/currentSpectrumAudio.fftSize)/2) + (getSampleRate()/currentSpectrumAudio.fftSize) * i;
            
            linearQvalue[i] = centreFrequency[i] / ((getSampleRate()/currentSpectrumAudio.fftSize));
            
            *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], linearQvalue[i], 1.0);
        }
        else if (i >= 47) // SETTING UP CENTRE FREQUENCIES AND Q VALUES OF THE REST OF THE FILTERS - LOGARITHMIC DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
        {
            centreFrequency[i] = centreFrequency[i-1] * std::pow(2.0, 1.0/48.0);
            
            *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], logQvalue, 1.0);
        }
        
        peakFilter[i].reset();
        peakFilter[i].prepare(spec);
    }
}

void FyppluginAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FyppluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FyppluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    if (startAveraging[0] == true)
        targetSpectrumAudio.sendToSpectrum(buffer); // IF STARTCAPTURE BUTTON HAS BEEN CLICKED, START SENDING SAMPLES TO THE CHOSEN SPECTRUM
    
    else if (startAveraging[1] == true) // IF STARTCAPTURE BUTTON HAS BEEN CLICKED, START SENDING SAMPLES TO THE CHOSEN SPECTRUM
        currentSpectrumAudio.sendToSpectrum(buffer);
    
    // NECESSARY SETUP TO PASS OUR BUFFER THROUGH JUCE'S DSP PROCESSORS
    dsp::AudioBlock<float> block (buffer);
    dsp::ProcessContextReplacing<float> context (block);
    //
    
    updateFilterIntensity();
    
    for (int i = 0; i < numberOfFilters; i++)
    {
        peakFilter[i].process(context);
    }
}


void FyppluginAudioProcessor::setupFilter()
{
    differenceAudio.setAverageTarget(targetSpectrumAudio.averageResults);
    differenceAudio.setAverageCurrent(currentSpectrumAudio.averageResults);
    differenceAudio.calculateDifferenceBetweenAverages(); 
    
    juce::zeromem (differenceResultsToUse, sizeof (differenceResultsToUse));
    memcpy (differenceResultsToUse, differenceAudio.getCurrentDifference(), sizeof (differenceResultsToUse));
    
    for (int i = 0; i < numberOfFilters; i++)
    { // WHENEVER THE FILTERS ARE SETUP, THE MEMBERS OF THE DIFFERENCE RESULTS ARRAY IS SAVED TO THE VALUE TREE AS PROPERTIES
        String stringDifference("StringDifference");
        stringDifference += i;

        parameters.state.setProperty(stringDifference, differenceResultsToUse[i], nullptr);
    }
    
    
    double dBs[numberOfFilters];
    int fftBinToLookFor[numberOfLogFilters];
    
    for (int i = 0; i < numberOfFilters; i++)
    {
        if (i < 47) // SETTING UP CENTRE GAIN VALUES OF THE FIRST 47 FILTERS - LINEAR DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
        {
            dBs[i] = differenceResultsToUse[i] * (*intensityParameter);
            
            *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], linearQvalue[i], jlimit<float>(0.1, 4.0, Decibels::decibelsToGain(dBs[i])));
        }
        else if (i >= 47) // SETTING UP GAIN VALUES OF THE REST OF THE FILTERS - LOGARITHMIC DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
        {
            fftBinToLookFor[i] = centreFrequency[i] * SpectrumAudio::fftSize / getSampleRate();
            dBs[i] = differenceResultsToUse[fftBinToLookFor[i]] * (*intensityParameter);
            
            *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], logQvalue, jlimit<float>(0.1, 4.0, Decibels::decibelsToGain(dBs[i])));
        }
    }
}

void FyppluginAudioProcessor::updateFilterIntensity()
{
    if (lastIntensityValue != *intensityParameter)
    {
        lastIntensityValue = *intensityParameter;
        
      double dBs[numberOfFilters];
      int fftBinToLookFor[numberOfLogFilters];
      
      for (int i = 0; i < numberOfFilters; i++)
      {
          if (i < 47) // SETTING UP CENTRE GAIN VALUES OF THE FIRST 47 FILTERS - LINEAR DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
          {
              dBs[i] = differenceResultsToUse[i] * (*intensityParameter);
              
              *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], linearQvalue[i], jlimit<float>(0.1, 4.0, Decibels::decibelsToGain(dBs[i])));
          }
          else if (i >= 47) // SETTING UP GAIN VALUES OF THE REST OF THE FILTERS - LOGARITHMIC DISTRIBUTION (MENTIONED IN CHAPTER 7.3 OF THE REPORT)
          {
              fftBinToLookFor[i] = centreFrequency[i] * SpectrumAudio::fftSize / getSampleRate();
              dBs[i] = differenceResultsToUse[fftBinToLookFor[i]] * (*intensityParameter);
              
              *peakFilter[i].state = *dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), centreFrequency[i], logQvalue, jlimit<float>(0.1, 4.0, Decibels::decibelsToGain(dBs[i])));
          }
      }
    }
}

//==============================================================================
bool FyppluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FyppluginAudioProcessor::createEditor()
{
    return new FyppluginAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void FyppluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FyppluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    
    // THIS CLASS TAKES CARE OF RECALLING ALL OF THE VALUES SAVED DURING THE LAST TIME THE PLUG-IN WAS OPENED, BY EXTRACTING THEM FROM THE XML THAT WAS CREATED WHEN THEY WERE SAVED. REFERENCED FROM JUCE TUTORIALS: “Adding plug-in parameters” AND “Saving and loading your plug-in state”
    
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));

    for (int i = 0; i < numberOfFilters; i++)
    {
        String stringDifference("StringDifference");
        stringDifference += i;
        
        differenceResultsToUse[i] = (float)xmlState->getDoubleAttribute(stringDifference, 0);
    }

    for (int i = 0; i < SpectrumAudio::fftSize; i++)
    {
        String stringCurrent("StringCurrent");
        stringCurrent += i;
        
        currentSpectrumAudio.averageResults[i] = (float)xmlState->getDoubleAttribute(stringCurrent, 0);
        
        String stringTarget("StringTarget");
        stringTarget += i;
        
        targetSpectrumAudio.averageResults[i] = (float)xmlState->getDoubleAttribute(stringTarget, 0);
    }
    
    setupFilter();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FyppluginAudioProcessor();
}

