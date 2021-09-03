
#pragma once
#include "SpectrumAudio.hpp"

//==============================================================================
class SpectrumUI   : public juce::Component,
private juce::Timer
{
public:
    SpectrumUI()
    {
        setOpaque (true);
        startTimerHz (30);
    }
    
    void setSpectrumAudioToDisplay(SpectrumAudio* spectrum)
    {
        spectrumAudio = spectrum;
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::white);
        g.setOpacity (1.0f);
        g.setColour (juce::Colours::black);
        drawFrame(g);
        
        g.setOpacity(0.3);
        
        g.drawLine(15, 0, 15, getHeight());
        g.drawSingleLineText("100Hz",  15,  getHeight() - 5 );
        
        g.drawLine(137, 0, 137, getHeight());
        g.drawSingleLineText("1000Hz",  137,  getHeight() - 5 );
        
        g.drawLine(478, 0, 478, getHeight());
        g.drawSingleLineText("5000Hz",  427,  getHeight() - 5 );
        
        g.drawLine(629, 0, 629, getHeight());
        g.drawSingleLineText("10000Hz",  629-55,  getHeight() - 5 );
    }
    
    void timerCallback() override
    {
        if (spectrumAudio != nullptr)
        {
            memcpy (averageResultsCopy, spectrumAudio->getCurrentAverageResults(), sizeof (averageResultsCopy));
            renderNextFrameOfSpectrum();
            repaint();
        }
    }
    
    
    void renderNextFrameOfSpectrum()
    {
        // CALCULATES WHICH FFTBIN EACH SPECTRUM POINT NEEDS TO REPRESENT - EXPLAINED IN CHAPTER 28 OF THE REPORT
        auto mindB = -100.0f;
        auto maxdB =    0.0f;
        
        for (int i = 0; i < scopeSize; ++i)           
        {
            auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));

            auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels                 (averageResultsCopy[fftDataIndex])
                                                   - juce::Decibels::gainToDecibels ((float) fftSize)),
                                     mindB, maxdB, 0.0f, 1.0f);
            
            scopeData[i] = level;
        }
    }
    
    void drawFrame (juce::Graphics& g)
    {
        // DRAWS THE SPECTRUM THROUGH A COMBINATION OF LINES THAT GO FROM ONE POINT TO ANOTHER
        Path plotPath;
        for (int i = 0; i < scopeSize; ++i)
        {
            auto width  = getLocalBounds().getWidth();
            auto height = getLocalBounds().getHeight();

            if (i == 0)
            {
                plotPath.startNewSubPath( (float) juce::jmap (i , 0, scopeSize - 1, 0, width),
                                         juce::jmap (scopeData[i], 0.0f, 1.0f, (float) height, 0.0f));
            }
            plotPath.lineTo( (float) juce::jmap (i, 0, scopeSize - 1, 0, width),
                            juce::jmap (scopeData[i], 0.0f, 1.0f, (float) height, 0.0f));
        }
        plotPath = plotPath.createPathWithRoundedCorners(50.0f);
        g.strokePath(plotPath, juce::PathStrokeType (1.0f));
        
    }
    
    
    enum ScopeSize
    {
        scopeSize = 128 // number of points in visual representatIon
    };
    
    float scopeData[scopeSize] = {0};   
private:
    
    int fftSize = SpectrumAudio::fftSize;
    float averageResultsCopy[SpectrumAudio::fftSize*2];  //  results of our FFT calculations.
    
    
    SpectrumAudio* spectrumAudio = nullptr;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumUI)
};
