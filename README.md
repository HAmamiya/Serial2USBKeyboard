# Serial2USBKeyboard
Serial to USB keybord using ATMEGA32U4

This program is inspired by [@andrewintw's 
arduino-serial-to-keyboard](https://github.com/andrewintw/arduino-serial-to-keyboard). Basically what I wanted to do is using a Raspberry pi (pi 3 in my case) to become a KVM over IP and controls my server. This program only serves Keyboard and Mouse part. I will explain at the bottom of this readme file how I deal with Monitor part.

## Hardware requirements
1. USB to UART (Serial) device (e.g. CH340 variant / CP2102)
2. ATMEGA32U4 (e.g. Pro Micro / Arduino Leonardao)

Connect both devices with 3 cables: tx->rx, rx->tx, GND->GND. Consult www if you are not sure how to do it.

## Software requirements
1. Programmer for compiling and loading codes into ATMEGA32U4 (e.g. Arduino IDE)
2. Serial user interface (e.g. PuTTY in Windows or `screen` in Linux).

### My Testing Enviroment
1. PuTTY 0.81
2. `screen` in Debian 12.5.

## Usage
1. Plug the USB to UART device into the client computer (Raspberry pi)
2. Start the Serial user interface (Serial monitor, 9600–8-N-1, (PuTTY) Linux keyboard)
3. Plug the ATMEGA32U4 into server/the computer you want to control
4. The Serial monitor shall prompt you to type `qwerty` with Enter for enabling the Serial to USB keybord.

## Mapped key
This program is based on US keyboard.

Not all keys are mapped. The following keys are mapped:
1. A-Z, a-z, 0-9
2. ~!@#$%^&*()_+-=`[]{}/<>?
3. Tab, Esc
4. Ctrl + c, Ctrl + k, Ctrl + u, Ctrl + v, Ctrl + w, Ctrl + x, Ctrl + z

For those keys that are not shown above, they may already been mapped but I did not include them in above, or it's really not mapped. You can test them in you usual computer before launching the device in production.

## Special Ctrl + key combinations
1. Ctrl + g	Disable Flood protection for 30 seconds (Useful when pasting something on Serial monitor)
2. Ctrl + h	Hide non special characters feedback (Useful when typing password)
3. Ctrl + q	OS logo, Command (⌘) on Mac
4. Ctrl + n	Toggle Mouse mode.

## Mouse mode
When you press Ctrl + n, the ATMEGA32U4 will switch from Keyboard to Mouse mode. The Serial monitor will prompt you how to use the Mouse mode. In Mouse mode, all keyboard keys are avilable, except keys that are binded by Mouse mode.

## Increase Serial RX buffer (optional)
To reduce the chance on Serial buffer full, you may refer to 
https://hackaday.com/2020/07/13/surgery-on-the-arduino-ide-makes-bigger-serial-buffers/ for additional changes.

## Monitor
This program does not handle monitor part. For monitor, you may purchase a HDMI capture card ([reference](https://blog.j2i.net/2021/04/19/hdmi-capture-on-the-raspberry-pi/)) and plug the capture card into Raspberry pi. The best streaming command so far I tested is as follows:

### Raspberry pi side
`ffmpeg -f v4l2 -video_size 1280x720 -i /dev/video1 -preset ultrafast -tune zero_latency -f mpegts -fflags nobuffer "udp://<ip of ffplay>:2000"`

### Playback side
`ffplay -fflags nobuffer -flags low_delay -probesize 20000 -analyzeduration 1 -strict experimental -framedrop -f mpegts -sync ext udp://<ip of raspberry pi>:2000`

## General stuffs
1. I am not using git control in my computer so this repository's git log may be messy. Sorry about this.


