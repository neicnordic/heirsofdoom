## SSH access:

* [Put your public key in this file](authorized_keys) and ping your botmaster of choice to give you ssh access to the bot. 

## Preparations

* [Build the bot](building.md)
* [Prep the card](sdcard-prep.md)

### Arduino stuff

* Download [Arduino for Linux ARM](https://www.arduino.cc/en/Main/Software) and untar in /opt
* Clone command line tools in /opt `git clone https://github.com/sudar/Arduino-Makefile`
* Clone makeblock libraries in /opt `git clone https://github.com/Makeblock-official/Makeblock-Libraries`

## Start hacking

* Log in to the robot using [SSH](#SSH access).
* Clone NeIC robo stuff `git clone https://github.com/neicnordic/heirsofdoom`
* Set up the arduino `cd heirsofdoom/2017/arduino; make; make upload; cd -`.
* Go play with python `cd heirsofdoom/2017/python; python`.

Example:

```python
import robot

r = robot.Robot()
r.sense(mag=True)
print r.mag
```
