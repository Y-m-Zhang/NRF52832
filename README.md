# NRF52832
## ADS1292R
NRF SPI读写ADS1292R单导心电数据
![Image text](https://raw.github.com/Y-m-Zhang/NRF52832/master/pic/ads1292r.jpg)
|模块引脚|GPIO|
|---|---|
|SCK|26|
|MOSI|25|
|MISO|24|
|CS|22|
|START|27|
|PWDN|28|
|RDY|23|
打包数据15Byte：包头+功能位+4Bytes ECG data + 4Bytes respiration data

## MAX30102
NRF TWI读写MAX30102脉搏数据、Timer控制采样率100~1000Hz
<img src="https://raw.github.com/Y-m-Zhang/NRF52832/master/pic/MAX30102.gif" width="450" height="300" />
|模块引脚|GPIO|
|---|---|
|CLK|22|
|DATA|23|
|INT|26|

## AD8232+MAX30102
NRF同步读取 （1）SAADC采样AD8232单导心电数据（Fs=250Hz）（2）TWI读写MAX30102脉搏数据（Fs=100Hz）
双Timer分别控制速率

<img src="https://raw.github.com/Y-m-Zhang/NRF52832/master/pic/AD8232.jpg" width="450" height="300" />

OUTPUT引脚连接NRF的模拟输入