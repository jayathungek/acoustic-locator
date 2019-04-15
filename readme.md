# acoustic-locator

The work presented in this project concerns the implementation of an acoustic locator system. As its name implies, its main function is the detection of the direction of a sound source. Once the direction of the sound source is determined, a positioning system is activated to point to the direction of the sound source. In the case of a multiple speaker setting, the system will control the positioning mechanism to point towards the speaker currently speaking. In the case of a single moving speaker, the system will control positioning unitin order to track the speaker as he moves. 

The acoustic locator can be used in teleconferencing settings, where more than one participants arelocated in a room talking with other participants located elsewhere. Often, these participants seat around a table with a web camera and a microphone in front of them (Figure 1), therefore, it is difficult for the camera to capture all of them. In this case the camera and the microphone can be attached on the positioning mechanism of the acoustic locator. The acoustic locator will determine the position of the person speaking and instruct the propositioning mechanism to point towards him. 

![tablepc](https://user-images.githubusercontent.com/45922282/56137082-b81dac00-5f9c-11e9-8559-82c04bebfaf5.jpg)

                         Figure 1

A similar case occurs in conferences when people from the audience wish to intervene or ask a question. If the conference is video recorded or streamed, it is necessary to move the camera so that it points to the person speaking. This operation can be achieved automatically with the help of the acoustic locator. In some cases the chairs in the conference room are not placed on the same level, as shown in figure 2. Therefore, the acoustic locator must also be able to detect the vertical direction of the audio source and the positioning mechanism must be able to provide both horizontal and vertical rotation movements.  

![theater](https://user-images.githubusercontent.com/45922282/56137264-19457f80-5f9d-11e9-9bad-949ed61c656d.jpg)

                        Figure 2

## Principle of Operation

The principle of operation of the acoustic locator is based on sound triangulation, where the direction of the location of the sound source is achieved with the use of two microphones. More specifically, the acoustic locator is using the Time Difference Of Arrival (TDOA) to determine the direction of the location of the sound source. As shown in figure 3(a), two microphones are placed next to each other with a predefined distance between them. If the distance from the sound source to the left microphone is greater from the distance of the sound source to the right microphone, then the right microphone will receive the sound wave earlier than the left microphone. This is due to the fact that sound waves travel in a spherical way at a predefined speed. The position of the sound source affects the distance between the two microphones, and therefore the time delay between the arrival of the sound wave to the left and the right microphones. The distance between the sound source and the left microphone is identified as RL, while the distance between the source and the right microphone is identified as RR.

The waveform at the lower part of figure 3(a) shows the reception of a typical sound wave by the two microphones. In this case the waveform with red colour corresponds to the wave received by the left microphone, and the waveform with blue colour corresponds to the wave received by the right microphone. The time difference (Δt) between the two waveforms is proportional to the distance difference RL– RR.

![Offset1](https://user-images.githubusercontent.com/45922282/56137420-7b9e8000-5f9d-11e9-8717-84bb7a37706f.jpg)

                         Figure 3

The direction of the position of the sound source can be determined using the distances RL, RR, the distance between the two microphones (Lm), the time difference Δt and the speed of sound.((For the implementation of the acoustic locator it is not necessary to calculate the exact direction of the position of the sound source. The acoustic locator needs to determine whether the sound source is at the left or at the right side of the microphones for the horizontal positioning, (or higher or lower for the vertical positioning) and signal the positioning motors to rotate the positioning mechanism (a) to the left or to the right for the horizontal direction and (b) up or down, for the vertical direction. The time difference (Δt) can be used in order to set the speed of the motors. The motors will keep running until the time difference (Δt) becomes zero. This operation is shown in figure 3(b). ))  

Figure 4 shows the case where the sound source has moved to the right. In this case it can be seen that the distance RL – RR and consequently the time difference (Δt) have been increased.


![Offset2(1)](https://user-images.githubusercontent.com/45922282/56137528-be605800-5f9d-11e9-84e8-2da892b74d46.jpg)

                           Figure 4
                           
## Accuracy Limitations and Aliasing

To determine the direction of the position of the sound source, the signal from the microphones is digitized at a given sampling rate. For the implementation of the acoustic locator the sampling frequency (Fs) is set at 44,100 Hz. Therefore, the sampling time (ts) is equal to 1/Fs (1/44,100 = 22.68 μs).

The velocity of sound (Vs) at 20°C is 343m/s. The minimum distance change (dm) that can be detected is the distance travelled by a sound wave in 22.68 μs. Therefore the minimum distance change detected is equal to 7.78mm, since 
dm = Vs x ts = 343m/s x 22.68μs = 7.78mm 

In the developed acoustic locator the distance between the microphones (Lm) is set to 10cm. The distance difference between the distances from the sound source to each microphone is dx = |RL – RRI. From figure 5, it can be observed that the maximum distance difference occurs when the source is in line with the two microphones, thus it is 10 cm.Therefore the number of distinct sound source directions detected is 12.9 for positive values of dx and 12.9 for negative values of dx, since 10cm/7.78mm = 12.9. This implies that for a 180 degree angle there are 25 distinct directions detected. As shown in figure 6, these directions are denser if the sound source is closer to the line formed by the two microphones, and less dense when the sound closed to perpendicular with the line formed by the microphones.

![offset3](https://user-images.githubusercontent.com/45922282/56137659-00899980-5f9e-11e9-9fd3-c370e86d0b7b.jpg)

                          Figure 5
                          
![Positions](https://user-images.githubusercontent.com/45922282/56137707-16975a00-5f9e-11e9-92a0-4de11a5218f5.jpg)

                          Figure 6

A way to determine the time difference between the arrivals of a sound signal to the microphones is by detecting either the peak values of two adjacent signal cycles, or by detecting the zero crossings of the two signals. If the number of signal samples between the two adjacent zero crossings is ‘n’ then the time difference is ‘n x ts’. This time difference corresponds to a distance difference equal to ‘n x 7.78mm ‘. An effect than can result in wrong detection results is aliasing. Aliasing occurs in cases where it is clear which microphone received the sound signal first. If we assume that the sound signals are pure sine waves, then as shown in figure 7(a) it is possible to have more than one zero crossings for each microphone in the samples examined. From the waveforms in figure 7(a) it can be seen that during the time frame (Δt) for which the signals are examined, there are two zero crossings for the red waveform and one for the blue. Therefore, there are two possible results: one by considering the first zero crossing of the red waveform, and one by considering the second. From the diagram in figure 7(a) it is clear that the correct result is achieved by considering the second zero crossing, however it is difficult to decide on this if the only information available is a number of samples. 

From the waveforms in figure 7(b) it can be seen that if the frequency of the signal is reduced, then there is no possibility to get more than one zero crossings within a given time limit. This time limit is determined by the distance between the microphones. More specifically, if the maximum signal frequency corresponds to a period that has a duration less than the time needed by the sound to travel a distance equal to the distance between the microphones, then it is ensured that aliasing will not happen. In the diagram in figure 7(b) the time (Δt) corresponds the time needed by the sound to travel a distance equal to the distance between the microphones. Thus if the frequency is low enough, there is no possibility of having two zero crossings within this time frame. The maximum sound frequency that ensures no aliasing Fmax is the frequency with period equal to the time (Δt). Therefore (Fmax = Vs/dm = 343(m/s) / 10cm/343m/s = 3.43KHz). To ensure aliasing-free operation, the signals from the microphones should be low-passed with a low pass filter with a cut-off frequency at 3.43KHz. From the above, it can be set that increasing the distance between the microphones will improve the accuracy of the acoustic locator, but it will also reduce the maximum frequency for aliasing-free operation.

![Aliasing](https://user-images.githubusercontent.com/45922282/56137827-4cd4d980-5f9e-11e9-9a3b-ca13d26a241e.jpg)

                            Figure 7
                          
## Hardware setup
Description of hardware used, with pictures and explanation of what each component does

## How to use
Description of how to clone source code, any dependancies, how to power up components etc.

### Social media links(if any)
Facebook: https://www.facebook.com/AcousticLocator/?modal=admin_todo_tour 

Youtube: https://www.youtube.com/channel/UC-Mpuqiijxo4YobSjKvtcCA/featured?view_as=subscriber

![20190313_165918](https://user-images.githubusercontent.com/45922282/55901264-ae2d2f00-5bd1-11e9-95b4-42b640f174c5.jpg)
