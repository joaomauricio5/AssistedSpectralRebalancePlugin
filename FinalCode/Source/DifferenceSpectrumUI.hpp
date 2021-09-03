#pragma once
#include <JuceHeader.h>
#include "DifferenceSpectrumAudio.hpp"

//==============================================================================
class DifferenceUI   : public juce::Component
{
public:
    DifferenceUI()
    {
        setOpaque (true);
    }
    
    void setDifferenceToDisplay(DifferenceAudio* difference)
    {
        differenceAudio = difference;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::white);
        
        g.setOpacity (1.0f);
        g.setColour (juce::Colours::black);
        drawFrame(g);
        
        g.setColour(juce::Colours::red);
        g.drawLine(0, getHeight()/2, getWidth(), getHeight()/2);
        
        g.setColour(juce::Colours::grey);
        g.setOpacity(0.5);
        
        g.drawLine(0, getHeight()/2 - 80, getWidth(), getHeight()/2 - 80);
        g.drawSingleLineText("+10dB",  15,  getHeight()/2 - 80 );
        
        g.drawLine(0, getHeight()/2 + 80, getWidth(), getHeight()/2 + 80);
        g.drawSingleLineText("-10dB",  15,  getHeight()/2 + 80 );
        
        g.drawLine(15, 0, 15, getHeight());
        g.drawSingleLineText("100Hz",  15,  getHeight() - 5 );
        
        g.drawLine(137, 0, 137, getHeight());
        g.drawSingleLineText("1000Hz",  137,  getHeight() - 5 );
        
        g.drawLine(478, 0, 478, getHeight());
        g.drawSingleLineText("5000Hz",  427,  getHeight() - 5 );
        
        g.drawLine(629, 0, 629, getHeight());
        g.drawSingleLineText("10000Hz",  629-55,  getHeight() - 5 );
    }
    
    void displayDifference()
    {
        memcpy (differenceResultsCopy, differenceAudio->getCurrentDifference(), sizeof (differenceResultsCopy));
        repaint();
    }
    
    void drawFrame (juce::Graphics& g)
    { // DRAWS THE SPECTRUM THROUGH A COMBINATION OF LINES THAT GO FROM ONE POINT TO ANOTHER - EXPLAINED IN CHAPTER 5.1 OF THE REPORT
        Path plotPath;
        
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();
        
        for (int i = 0; i < scopeSize; ++i)
        {
            auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
            
            auto level = differenceResultsCopy[fftDataIndex] * intensityLevel/4.0; // MAKES THE SPECTRAL DIFFERENCE DISPLAY EXPAND DEPENDING ON THE FILTER INTENSITY SLIDER 
            scopeData[i] = level;
            
            if (i == 0)
            {
                plotPath.startNewSubPath( (float) juce::jmap (i, 0, scopeSize - 1, 0, width),
                                         juce::jmap (scopeData[i], -4.0f, 4.0f, (float) height, 0.0f));
            }
            plotPath.lineTo( (float) juce::jmap (i, 0, scopeSize - 1, 0, width),
                            juce::jmap (scopeData[i], -4.0f, 4.0f, (float) height, 0.0f ));
        }
        
        plotPath = plotPath.createPathWithRoundedCorners(50.0f);
        g.strokePath(plotPath, juce::PathStrokeType (1.0f));
    }
    
    enum
    {
        scopeSize = SpectrumUI::scopeSize // number of points in visual representaton
    };
    
    float intensityLevel = 0;

private:
    
    int fftSize = SpectrumAudio::fftSize;
    
    float scopeData[scopeSize] = {0};
    
    float differenceResultsCopy[SpectrumAudio::fftSize*2];
    DifferenceAudio* differenceAudio = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DifferenceUI)
};
