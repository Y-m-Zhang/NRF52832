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
![Image text](https://raw.github.com/Y-m-Zhang/NRF52832/master/pic/MAX30102.gif)
|模块引脚|GPIO|
|---|---|
|CLK|22|
|DATA|23|
|INT|26|
