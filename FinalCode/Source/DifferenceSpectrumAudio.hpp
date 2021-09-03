#pragma once
#include <JuceHeader.h>


//==============================================================================
class DifferenceAudio
{
public:
    DifferenceAudio()
    {
    }

    void setAverageTarget(float* averageResults)
    {
        for (int i = 0; i < fftSize; i++) // CREATES A COPY OF THE AVERAGE RESULTS FROM OUR TARGET
        {
            averageTarget[i] = averageResults[i];
        }
    }
    
    void setAverageCurrent(float* averageResults)
    {
        for (int i = 0; i < fftSize; i++) // CREATES A COPY OF THE AVERAGE RESULTS FROM OUR CURRENT
        {
            averageCurrent[i] = averageResults[i];
        }
    }
    
    void calculateDifferenceBetweenAverages()
    {
        for (int i = 0; i < fftSize; i++) // CALCULATES THE DIFFERENCE, IN DECIBELS, BETWEEN THE TARGET AND THE CURRENT MAGNITUDE VALUES FOR EACH FFT BIN
        {
            differenceResults[i] = Decibels::gainToDecibels(averageTarget[i]) - Decibels::gainToDecibels(averageCurrent[i]);
        }
        
    }
    
    float* getCurrentDifference()
    {
        return differenceResults;
    }
    
    void clearDifferenceResults()
    {
        juce::zeromem (differenceResults, sizeof (differenceResults));
    }
    
private:
    
    int fftSize = SpectrumAudio::fftSize;
    
    float averageTarget[SpectrumAudio::fftSize] = {0};
    float averageCurrent[SpectrumAudio::fftSize] = {0};
    float differenceResults[SpectrumAudio::fftSize] = {0};

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DifferenceAudio)
};
