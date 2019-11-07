Table of Contents
=================

   * [acoustic-locator](#acoustic-locator)
      * [Hardware setup](#hardware-setup)
        * [The Microphone Circuit](#the-microphone-circuit)
        * [The Humidity/Temperature Sensor](#the-humiditytemperature-sensor)
        * [The Laser LED circuit](#the-laser-led-circuit)
        * [The Servo Circuit](#the-servo-circuit)
        * [Mechanical Design](#mechanical-design)
      * [Prerequisites](#prerequisites)
      * [Installation](#installation)
      * [How to use](#how-to-use)
      
# acoustic-locator

## Hardware setup

The acoustic locator hardware consists of two basic modules. The first module is the microphone module build on a PCB board that enables the placement of three MEMS microphones. The second module is the positioning module which consists basically of two servo motors, one being responsible for the horizontal movement of the mechanism and the other for the vertical. The front and back views of the acoustic locator are shown in figure 1. The sound source direction detection is achieved with the use of three microphones forming an equilateral triangle. The three holes at the edges of the acoustic locator identified on the front view are the openings for the three microphones. The opening in the middle of the acoustic locator, shown in the front view is the opening for the laser diode. The back view shows the positioning mechanism with the two servo motors.


![AL picture](https://user-images.githubusercontent.com/45922282/56164269-fdf86580-5fd8-11e9-9e56-7c92be215335.jpg)

                            Figure 1
                            
 The PCB board for the microphones is shown in figure 2. An IDC connector is used to connect the board with the Raspberry Pi board. The microphones used are Integrated Inter-IC Sound Bus (I2S) compatible digital MEMS microphones. I2S is a serial bus standard/protocol that enable the interface of one or two microphones on a microcontroller through a serial connection. Since the acoustic locator uses three microphones, a multiplexer is used to enable the connection of the microphones on the I2S bus. 

The microphones board also contains the circuit for the humidity/temperature sensor. The reason for using this sensor is because the speed of sound can vary with the temperature and the humidity. If the acoustic locator is used in normal room conditions then there is no need for the temperature/humidity sensor. If the acoustic locator is going to be used in places with extreme temperatures or humidity then the values of the temperature and humidity can be used to calculate the speed of sound and adjust the operation of the acoustic locator accordingly. 

The third module on the microphones PCB board is a laser LED circuit consisting of a LED driver and the laser LED. The laser LED is used for testing purposes, in order to identify the direction at which the acoustic locator is pointing to. On the final product, the laser LED can be replaced by a web camera, so that the camera will point to the person speaking, as explained in the introduction of this report.


![Mic_Board](https://user-images.githubusercontent.com/45922282/56164372-39932f80-5fd9-11e9-89cc-e91d4fbc784a.jpg)

                            Figure 2
                            
### The Microphone Circuit

The microphones used by the acoustic locator are the ICS-43432 digital I2S microphones from InvenSence. The ICS-43432 microphone includes all the required circuitry so that it can be interfaced directly with a microcontroller or a DSP, without the use of an audio codec. As shown in figure 3 the ICS-43432 contains in the same package the MEMS sensor with the signal conditioning circuit, the A/D converter, the anti-aliasing filter and a 24-bit I2S interface. The data sheet, handling information and application notes for the ICS-43432 microphone can be found [here]( https://www.invensense.com/products/digital/ics-43432/).

The ‘Config’ input is used in cases where it is required to modify the internal registers of the device. For normal operation, the ‘Config’ signal must be connected to the ground.  The ‘Left/Right’ (LR) channel select signal is used to specify to which one of the two I2S channels is the microphone connected to. If the L/R pin is connected to the ground (or set to Low), the microphone outputs its signal to the left channel of the I2C frame. If the L/R pin is connected to the supply voltage (or set to High), the microphone outputs its signal to the right channel of the I2C frame.

The I2C signals are the ‘SCK’ (Serial Data Clock for the I2S interface) input signal, the ‘SD’ (Serial Data Output for the I2S interface) signal and the ‘WS’ (Serial Data Word Select for the I2S interface) input signal. If the ‘WS’ signal is set Low, the left microphone sends its data on the ‘SD’ signal, otherwise the right microphone sends its data. The ‘SD’ line must be pull down to the ground using a 100K resistor. A typical circuit with two microphones connected to a DSP or a microcontroller is shown in figure 4.

![ICS43432](https://user-images.githubusercontent.com/45922282/56164522-8a0a8d00-5fd9-11e9-8737-d4fe71296900.jpg)

                              Figure 3
                              
![Stereo](https://user-images.githubusercontent.com/45922282/56164570-a5759800-5fd9-11e9-919e-54bae267b113.jpg)                              
            
                              Figure 4
                              
The circuit diagram of the acoustic locator microphone board is shown in figure 5. Because the acoustic locator uses three microphones and the I2S interface supports only two, a demultiplexer is used to allow two microphone to share the same I2S channel. The demultiplexer used is the TS3A24159 dual 2-channel SPDT bidirectional analogue switch. As shown in the circuit diagram, the select input (In1) of the analogue switch is connected to the GPIO9 signal of the Raspberry Pi, while the Com input is connected to the WS signal (signal GPIO19).

Setting the In1 pin to low, the WS signal is switched to the WS signal of the Mic1 microphone, thus reading the data from Mic1.  Setting the In1 pin to high, the WS signal is switched to the WS signal of the Mic2 microphone, thus reading the data from Mic2. Since the LR inputs for Mic1 and Mic2 are connected to the supply voltage, both microphones are treated as Right channel microphones. The LS signal of microphone Mic3 is connected to the ground, therefore, this microphone is treated as the Left channel microphone.

![Mic_board_Cct(1)](https://user-images.githubusercontent.com/45922282/56164616-c50cc080-5fd9-11e9-8555-11345f65d8f9.jpg)

                              Figure 5

### The Humidity/Temperature Sensor

The Humidity/Temperature Sensor used by the acoustic locator is the HIH6030-021-001 digital I2C humidity/temperature from Honeywell. This device combines a humidity and a temperature sensor in the same package. The data sheet, handling information and application notes for the HIH6030-021-001 digital I2C humidity/temperature can be found [here](https://sensing.honeywell.com/HIH6030-021-001-humidity-sensors). 

The pinout with the typical application circuit diagram, given in the device datasheet is shown in figure 13.The same circuit is used  by the acoustic locator as shown in the circuit diagram in figure 5.The SCL pin is connected to the I2C clock, while the SDA pin is connected to the I2C data line. Both the SCL and the SDA pins must be pulled up with a 2.2K resistor each. The supply pin must be decoupled with a 0.1µF capacitor, while the Vcore pin must be connected to the ground through a 0.22µF capacitor. The AL_L (Alarm Low) and the AL_H (Alarm High) pins are used in the case an external device such as a relay or a LED are needed to be controlled directly by the sensor.

![Hum_cct](https://user-images.githubusercontent.com/45922282/56164728-feddc700-5fd9-11e9-859e-fef8f67ae147.jpg)

                                Figure 6
                                                           
### The Laser LED circuit

The laser LED circuit, shown in figure 7, consists of the laser diode, MOSFET transistor driver and resistor. The transistor driver is used because the Raspberry Pi cannot provide the amount of current required to operate the laser LED. The resistor is used to limit the current through the laser to the value lower than the maximum LED current. The operation of the laser is achieved by controlling the state of the GPIO6 of the Raspberry Pi.

![laser](https://user-images.githubusercontent.com/45922282/56164785-1fa61c80-5fda-11e9-99d4-f243d8884c49.jpg)

                                Figure 7
                                
### The Servo Circuit

The movement of the positioning mechanism is achieved by the use of two servo motors. These motors are connected by the Raspberry Pi, through a board that is attached on the Raspberry Pi as a shield shown in figure 8. This shield also provides a connection from the Raspberry Pi to the microphones board through an IDC connector and a ribbon cable.The circuit diagram for the connection of the two servo motors to the Raspberry Pi is shown in figure 9. The speed and direction of rotation of the servo motors is achieved through the PWM signals obtained from the GPIO12 and the GPIO13 signals of the Raspberry Pi.

![RPShield](https://user-images.githubusercontent.com/45922282/56164868-482e1680-5fda-11e9-9f5e-45ad081df60a.jpg)

                                Figure 8
                                
![servocct](https://user-images.githubusercontent.com/45922282/56164883-4f552480-5fda-11e9-9099-9d830dd90f6f.jpg)

                                Figure 9
                                
### Mechanical Design

The mechanical design consists of the direction mechanism. This design features three parts: (a) the base, (b) a gimbal and (c) the head.

The base serves two functions: it supports the device as well as houses the Raspberry Pi, along with the locator interface board. The design allows the Raspberry Pi to be connected to a PC/laptop without opening the base.
The pan/tilt gimbal aids the movement of the device. It is capable of 180° rotation around the vertical axis as well as 100° up and down. The gimbal is mounts on top of the base as well as holding the head.

The head – a 3D printed part – clicks in place on top of the gimbal. The part is designed to secure the microphones in their respective places. The PCB just slides intro the head, thus no screws are required to hold the electronics on the gimbal.

## Prerequisites
The following steps are needed to configure the raspberry pi for I2S microphone support:

1. Enable I2S communication:- 
  ```
  $ sudo nano /boot/config.txt
  ```
  1.1 Navigate the file and uncomment the the line that contains `#dtparam=i2s=on`. 
  
2. Add the reference for the chip on the Raspberry Pi 3 that handles the I2S into the modules file:- 
  ```
  $ sudo nano  /etc/modules
  ``` 
  2.1 Add the line `snd-bcm2835` to this file.
  
3. Reboot:- 
  ```
  $ sudo reboot
  ```
  
4. Run this command and verify that it outputs `snd_soc_bcm2835_i2s`:-
  ```
  $ lsmod | grep snd
  ```
  
5. Install dependancies and set permissions:- 
  ```
  $ sudo apt-get -y install bc libncurses5-dev
  $ sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source`
  $ sudo chmod +x /usr/bin/rpi-source
  ```
6. Update `rpi-source` and run:-
  ```
  $ /usr/bin/rpi-source -q --tag-update`
  $ rpi-source
  ```
  
7. Mount the developent system debugger:-
  ```$ sudo mount -t debugfs debugs /sys/kernel/debug```
  
8. Check for the presence of the 3f203000.i2s module:-
  ```$ sudo cat /sys/kernel/debug/asoc/platforms```
  
9. Download Paul Creaser's I2S audio module, navigate to it and build it:-
  ```
  $ git clone https://github.com/PaulCreaser/rpi-i2s-audio
  $ cd rpi-i2s-audio
  $ make -C /lib/modules/$(uname -r )/build M=$(pwd) modules
  ```
  
10. Insert this module and verify its presence:-
  ```
  $ sudo insmod my_loader.ko
  $ lsmod | grep my_loader
  $ dmesg | tail
  ```
  
11. To load the module every time the pi is booted up:-
  ```
  $ sudo cp my_loader.ko /lib/modules/$(uname -r)
  $ echo 'my_loader' | sudo tee --append /etc/modules > /dev/null
  $ sudo depmod -a
  $ sudo modprobe my_loader
  ```


## Installation
1. Clone this repository: `$ git clone git@github.com:jayathungek/acoustic-locator.git`
2. Navigate to the main directory: `$ cd acoustic-locator`
3. Run tests: `$ make runtests`. All the tests should pass.
4. Build: `$ make`

A binary called `locator` is generated in the current working directory. Running this with `$ sudo ./locator` will start the main loop of the program and the gimbals will move in response to loud noises. Alternatively, f you wish to run `locator` on boot, edit `/etc/rc.local` so that it contains the path to the binary:
   ```
   #!/bin/sh -e
   #
   # rc.local
   #
   # This script is executed at the end of each multiuser runlevel.
   # Make sure that the script will "exit 0" on success or any other
   # value on error. 
   #
   # In order to enable or disable this script just change the execution
   # bits.
   #
   # By default this script does nothing.

   sudo /home/pi/acoustic-locator/locator &

   exit 0
   ```

## How to use
The raspberry pi is powered by a 5V micro usb cable and is perfectly capable of driving the servo motors from its 5V pins. Assuming all the steps above have been followed, simply plugging in the device to any 5V DC supply will start the location program.



