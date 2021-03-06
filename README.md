![](http://www.invensense.com/wp-content/uploads/2014/12/rp-mpu-9250.png)

MPU9250 9-Axis Gyro+Accel+Magn Sensor test on STM32F429
=========================
Implement the functionality of MPU9230 on STM32F429 plattform.

Features
------------
* Temperature and euler angle displayed on LCD
* Measured data uploaded by serial port and CAN bus
* Received data can be graphically displayed in PC
* Power saving mode
* Upload mode controlled by keys
* Watchdog

Function of keys and touchpad
------------
* KEY_UP: Standby mode/Wakeup
* KEY0: CAN upload ON/OFF
* KEY1: USART upload ON/OFF
* KEY2: LCD refresh ON/OFF
* Touchpad: LCD ON/OFF

Software Environment
------------

* MDK Version 5.20.0.0
* USB-CAN Tool V2.02
* ANO_TC_V4



Hardware
------------
* STM32F429 MCU Discovery Kits
* CANalyest-II

Experimental results
------------
#### Architecture
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/Architecture.png?raw=true)

#### Hardware
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/Headwear.jpg?raw=true)

#### Accelerometer update
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/Acc.png?raw=true)

#### Gyro update
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/Gyro.png?raw=true)

#### Attitude graphically displayed
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/Attitude.png?raw=true)

#### Received CAN data
![](https://github.com/VidaLay/MPU9250_Test_On_STM32F429/blob/master/Photos/CAN.png?raw=true)

