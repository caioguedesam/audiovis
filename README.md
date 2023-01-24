# Audiovis
A prototype for a real-time audio visualizer in 3D, written in C++ and OpenGL.

The purpose of this project was to quickly test out some ideas and dogfood my rendering engine API, [Typheus](https://github.com/caioguedesam/typheus), which is currently in its infancy.

---

The project contains a free camera view of a 3D terrain mesh that reacts to a sound file being played by the program. I experimented with some possibilities for the effect, and the coolest one I got was a propagation of highest audio pitch along time.

:warning: **HIGH VOLUME AND FLASHING COLORS WARNING** :warning:

[Pitch Visualization](https://user-images.githubusercontent.com/36515465/214288849-93eb1776-78a5-4ef8-8bb6-4799ab69e973.mp4)

I also got some interesting results when displaying the latest audio samples as a waveform starting from the center of the mesh.

:warning: **HIGH VOLUME AND FLASHING COLORS WARNING** :warning:

[Waveform Visualization](https://user-images.githubusercontent.com/36515465/214288886-9313a06d-277e-42fa-bc9f-cd2d9c6459cf.mp4)

And finally, I also tried to display frequency values with the Fast Fourier Transform output. I like this one, and think it would look great with a filter pass, but I know very little about digital signal processing. In the future I will revisit this project and expand on this one.

:warning: **HIGH VOLUME AND FLASHING COLORS WARNING** :warning:

[Frequency Visualization](https://user-images.githubusercontent.com/36515465/214288897-875a0383-43b1-4c1d-b4b4-d7e6dd01393c.mp4)

---
### Credits

Song used on the clips is [Mata Ashita](https://youtu.be/PIxC5I8W2zI), mashup by [nakinyko](https://www.youtube.com/@nakinyko).
