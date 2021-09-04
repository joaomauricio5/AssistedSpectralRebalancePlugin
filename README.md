# AssistedSpectralRebalancePlugin
<b>
This was my Final Year Project for university, and it involved 6 months of research, programming, debugging and learning. It's the first audio plug-in I created on my own and it was built with C++, using the JUCE framework. Brief introduction available here: https://vimeo.com/597503612. Full report included in the repository files.
<br><br>

The project deeply explored DSP concepts on a programming level, such as the Fast Fourier Transform and Digital Filter Design. Additionally, I was able to learn how to fully program an audio plug-in: from creating the UI, to setting up I/O with the DAW, designing the DSP, implementing save/load methods and DAW automation, and packaging an installer loaded with the AU and VST3 formats of the plug-in.
</b>

<br>
This plug-in rebalances the spectrum of a piece of audio by comparing and matching it with a suitable reference provided by the user. It can be useful in cases where there are multiple vocal takes in a mixing session, recorded in different conditions (different preamp, different mic, distance to the mic or even the tilt of the mic) but the mixing engineer needs the spectral characteristics to be consistent across the entire performance.

<p align="center">
<img width="1400" alt="PluginScreenshot" src="https://user-images.githubusercontent.com/90060036/132075517-e1593f78-ea56-466f-ac8c-8f3e9f948f5f.png">
</p>
