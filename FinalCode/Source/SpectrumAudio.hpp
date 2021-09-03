
#pragma once

//==============================================================================
class SpectrumAudio

{
public:
    SpectrumAudio() : forwardFFT (fftOrder),
    window (fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
    }
    
    void sendToSpectrum (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() > 0)
        {
            // IF OUR PLUG-IN IS RECEIVING MONO AUDIO, THEN JUST USE ONE CHANNEL OF THE BUFFER DIRECTLY. IF OUR PLUG-IN IS RECEIVING STEREO AUDIO, THEN ONLY SEND THE MONAURAL MIX TO THE FIFO ARRAY TO BE PROCESSED BY THE FFT (SEE CHAPTER 5.1 FOR DETAILED EXPLANATION)
            if (buffer.getNumChannels() == 2)
            {
                auto* channelDataL = buffer.getReadPointer(0);
                auto* channelDataR = buffer.getReadPointer(1);

                        for (auto i = 0; i < buffer.getNumSamples(); ++i)
                            pushNextSampleIntoFifo ( (channelDataL[i] + channelDataR[i]) * 0.5);
            }
            
            else if (buffer.getNumChannels() == 1)
            {
                auto* channelData = buffer.getReadPointer(0);

                        for (auto i = 0; i < buffer.getNumSamples(); ++i)
                            pushNextSampleIntoFifo (channelData[i]);
            }
        }
    }

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                juce::zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                
                renderFFTaverage();
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
            nextFFTBlockReady = false;
        }
        fifo[fifoIndex++] = sample;
    }
    
    void renderFFTaverage()
    {
        currentFrame++;
        // first apply a windowing function to our data
        window.multiplyWithWindowingTable (fftData, fftSize);   
    
        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (fftData);
        
        for (int i = 0; i < fftSize * 2; i++)
        {
            average[i] += fftData[i] ;
            averageResults[i] = average[i] / (currentFrame);
        }
    
        // THIS WAS THE NORMALIZING ALGORITHM THAT WAS TESTED BUT NOT IMPLEMENTED ON THE FINAL VERSION - EXPLANATION ON CHAPTER 7.4 OF THE REPORT
//        maxValue = 0.0;
//        for (int i = 0; i < fftSize * 2; i++)
//        {
//            maxValue = jmax(abs(averageResults[i]), abs(maxValue));
//        }
//       // maxValue *= 0.9;
//
//        for (int i = 0; i < fftSize * 2; i++)
//        {
//            averageResults[i] /= maxValue;
//        }
        
        averageResults[0] = 0;
    }
    
    float* getCurrentAverageResults()
    {
        return averageResults;
    }
    
    void clearAverage()
    {
        currentFrame = 0;
        juce::zeromem (average, sizeof (average));
        juce::zeromem (averageResults, sizeof (averageResults));
    }
    
    enum FFTproperties
    {
        fftOrder  = 11, // 2 to the power of the ffTorder = size of FFT window
        fftSize   = 1 << fftOrder,
    };
    
    float averageResults[fftSize * 2] = {0};
    
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
private:
    
    float fifo [fftSize];     //  incoming audio data in samples. Occupies the 1st half of the fftData array
    float fftData [2 * fftSize];  //  results of our FFT calculations.
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    float average[fftSize * 2] = {0};
    
    int currentFrame = 0;
    
    float maxValue = 0.0;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudio)
};
