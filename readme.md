Table of Contents
=================

   * [acoustic-locator](#acoustic-locator)
      * [Principle of Operation](#principle-of-operation)
      * [Accuracy Limitations and Aliasing](#accuracy-limitations-and-aliasing)
      * [Hardware setup](#hardware-setup)
        * [The Microphone Circuit](#the-microphone-circuit)
        * [The Humidity/Temperature Sensor](#the-humiditytemperature-sensor)
        * [The Laser LED circuit](#the-laser-led-circuit)
        * [The Servo Circuit](#the-servo-circuit)
        * [Mechanical Design](#mechanical-design)
      * [Codework](#codework)
      * [Prerequisites](#prerequisites)
      * [Installation](#installation)
      * [How to use](#how-to-use)
      * [Social media links](#social-media-links)
      
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

The acoustic locator hardware consists of two basic modules. The first module is the microphone module build on a PCB board that enables the placement of three MEMS microphones. The second module is the positioning module which consists basically of two servo motors, one being responsible for the horizontal movement of the mechanism and the other for the vertical. The front and back views of the acoustic locator are shown in figure 8. The sound source direction detection is achieved with the use of three microphones forming an equilateral triangle. The three holes at the edges of the acoustic locator identified on the front view are the openings for the three microphones. The opening in the middle of the acoustic locator, shown in the front view is the opening for the laser diode. The back view shows the positioning mechanism with the two servo motors.


![AL picture](https://user-images.githubusercontent.com/45922282/56164269-fdf86580-5fd8-11e9-9e56-7c92be215335.jpg)

                            Figure 8
                            
 The PCB board for the microphones is shown in figure 9. An IDC connector is used to connect the board with the Raspberry Pi board. The microphones used are Integrated Inter-IC Sound Bus (I2S) compatible digital MEMS microphones. I2S is a serial bus standard/protocol that enable the interface of one or two microphones on a microcontroller through a serial connection. Since the acoustic locator uses three microphones, a multiplexer is used to enable the connection of the microphones on the I2S bus. 

The microphones board also contains the circuit for the humidity/temperature sensor. The reason for using this sensor is because the speed of sound can vary with the temperature and the humidity. If the acoustic locator is used in normal room conditions then there is no need for the temperature/humidity sensor. If the acoustic locator is going to be used in places with extreme temperatures or humidity then the values of the temperature and humidity can be used to calculate the speed of sound and adjust the operation of the acoustic locator accordingly. 

The third module on the microphones PCB board is a laser LED circuit consisting of a LED driver and the laser LED. The laser LED is used for testing purposes, in order to identify the direction at which the acoustic locator is pointing to. On the final product, the laser LED can be replaced by a web camera, so that the camera will point to the person speaking, as explained in the introduction of this report.


![Mic_Board](https://user-images.githubusercontent.com/45922282/56164372-39932f80-5fd9-11e9-89cc-e91d4fbc784a.jpg)

                            Figure 9
                            
### The Microphone Circuit

The microphones used by the acoustic locator are the ICS-43432 digital I2S microphones from InvenSence. The ICS-43432 microphone includes all the required circuitry so that it can be interfaced directly with a microcontroller or a DSP, without the use of an audio codec. As shown in figure 10 the ICS-43432 contains in the same package the MEMS sensor with the signal conditioning circuit, the A/D converter, the anti-aliasing filter and a 24-bit I2S interface. The data sheet, handling information and application notes for the ICS-43432 microphone can be found [here]( https://www.invensense.com/products/digital/ics-43432/).

The ‘Config’ input is used in cases where it is required to modify the internal registers of the device. For normal operation, the ‘Config’ signal must be connected to the ground.  The ‘Left/Right’ (LR) channel select signal is used to specify to which one of the two I2S channels is the microphone connected to. If the L/R pin is connected to the ground (or set to Low), the microphone outputs its signal to the left channel of the I2C frame. If the L/R pin is connected to the supply voltage (or set to High), the microphone outputs its signal to the right channel of the I2C frame.

The I2C signals are the ‘SCK’ (Serial Data Clock for the I2S interface) input signal, the ‘SD’ (Serial Data Output for the I2S interface) signal and the ‘WS’ (Serial Data Word Select for the I2S interface) input signal. If the ‘WS’ signal is set Low, the left microphone sends its data on the ‘SD’ signal, otherwise the right microphone sends its data. The ‘SD’ line must be pull down to the ground using a 100K resistor. A typical circuit with two microphones connected to a DSP or a microcontroller is shown in figure 11.

![ICS43432](https://user-images.githubusercontent.com/45922282/56164522-8a0a8d00-5fd9-11e9-8737-d4fe71296900.jpg)

                              Figure 10
                              
![Stereo](https://user-images.githubusercontent.com/45922282/56164570-a5759800-5fd9-11e9-919e-54bae267b113.jpg)                              
            
                              Figure 11
                              
The circuit diagram of the acoustic locator microphone board is shown in figure 12. Because the acoustic locator uses three microphones and the I2S interface supports only two, a demultiplexer is used to allow two microphone to share the same I2S channel. The demultiplexer used is the TS3A24159 dual 2-channel SPDT bidirectional analogue switch. As shown in the circuit diagram, the select input (In1) of the analogue switch is connected to the GPIO9 signal of the Raspberry Pi, while the Com input is connected to the WS signal (signal GPIO19).

Setting the In1 pin to low, the WS signal is switched to the WS signal of the Mic1 microphone, thus reading the data from Mic1.  Setting the In1 pin to high, the WS signal is switched to the WS signal of the Mic2 microphone, thus reading the data from Mic2. Since the LR inputs for Mic1 and Mic2 are connected to the supply voltage, both microphones are treated as Right channel microphones. The LS signal of microphone Mic3 is connected to the ground, therefore, this microphone is treated as the Left channel microphone.

![Mic_board_Cct(1)](https://user-images.githubusercontent.com/45922282/56164616-c50cc080-5fd9-11e9-8555-11345f65d8f9.jpg)

                              Figure 12

### The Humidity/Temperature Sensor

The Humidity/Temperature Sensor used by the acoustic locator is the HIH6030-021-001 digital I2C humidity/temperature from Honeywell. This device combines a humidity and a temperature sensor in the same package. The data sheet, handling information and application notes for the HIH6030-021-001 digital I2C humidity/temperature can be found [here](https://sensing.honeywell.com/HIH6030-021-001-humidity-sensors). 

The pinout with the typical application circuit diagram, given in the device datasheet is shown in figure 13.The same circuit is used  by the acoustic locator as shown in the circuit diagram in figure 12.The SCL pin is connected to the I2C clock, while the SDA pin is connected to the I2C data line. Both the SCL and the SDA pins must be pulled up with a 2.2K resistor each. The supply pin must be decoupled with a 0.1µF capacitor, while the Vcore pin must be connected to the ground through a 0.22µF capacitor. The AL_L (Alarm Low) and the AL_H (Alarm High) pins are used in the case an external device such as a relay or a LED are needed to be controlled directly by the sensor.

![Hum_cct](https://user-images.githubusercontent.com/45922282/56164728-feddc700-5fd9-11e9-859e-fef8f67ae147.jpg)

                                Figure 13
                                                           
### The Laser LED circuit

The laser LED circuit, shown in figure 14, consists of the laser diode, MOSFET transistor driver and resistor. The transistor driver is used because the Raspberry Pi cannot provide the amount of current required to operate the laser LED. The resistor is used to limit the current through the laser to the value lower than the maximum LED current. The operation of the laser is achieved by controlling the state of the GPIO6 of the Raspberry Pi.

![laser](https://user-images.githubusercontent.com/45922282/56164785-1fa61c80-5fda-11e9-99d4-f243d8884c49.jpg)

                                Figure 14
                                
### The Servo Circuit

The movement of the positioning mechanism is achieved by the use of two servo motors. These motors are connected by the Raspberry Pi, through a board that is attached on the Raspberry Pi as a shield shown in figure 15. This shield also provides a connection from the Raspberry Pi to the microphones board through an IDC connector and a ribbon cable.The circuit diagram for the connection of the two servo motors to the Raspberry Pi is shown in figure 16. The speed and direction of rotation of the servo motors is achieved through the PWM signals obtained from the GPIO12 and the GPIO13 signals of the Raspberry Pi.

![RPShield](https://user-images.githubusercontent.com/45922282/56164868-482e1680-5fda-11e9-9f5e-45ad081df60a.jpg)

                                Figure 15
                                
![servocct](https://user-images.githubusercontent.com/45922282/56164883-4f552480-5fda-11e9-9099-9d830dd90f6f.jpg)

                                Figure 16
                                
### Mechanical Design

The mechanical design consists of the direction mechanism. This design features three parts: (a) the base, (b) a gimbal and (c) the head.

The base serves two functions: it supports the device as well as houses the Raspberry Pi, along with the locator interface board. The design allows the Raspberry Pi to be connected to a PC/laptop without opening the base.
The pan/tilt gimbal aids the movement of the device. It is capable of 180° rotation around the vertical axis as well as 100° up and down. The gimbal is mounts on top of the base as well as holding the head.

The head – a 3D printed part – clicks in place on top of the gimbal. The part is designed to secure the microphones in their respective places. The PCB just slides intro the head, thus not screws are required to hold the electronics on the gimbal.

## Codework

## Prerequisites

## Installation

## How to use
Description of how to clone source code, any dependancies, how to power up components etc.

## Social media links
Facebook: https://www.facebook.com/AcousticLocator/?modal=admin_todo_tour 

Youtube: https://www.youtube.com/channel/UC-Mpuqiijxo4YobSjKvtcCA/featured?view_as=subscriber


