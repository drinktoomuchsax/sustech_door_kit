# architecture
the architecture of the system is design in the [productDesign draw](product_design/productDesign.excalidraw).

inMoudle in charge of controling relay and host WiFi server to access.

exModule in charge of password parity, face recognition, human detect(lidar and touch_pad of esp32-s3). Maybe in the future, homeassistant would be put in exModule.


# 产品设计
## 为什么外面使用esp32s3
引脚不够用，touch_pad
## 为什么有线
## 电从哪来
## 安全性
## 未来的形态
用蓝牙或者Wi-Fi协议来交互之前，需要解决外模块功耗的问题。


# 软件代码
## code
both esp32-s3 and esp32-c3 using platformIO to develop.
both of them are arduino framework.

exModule using a lib write by drinktoomuchsax and weiyoudong

## communication protocol between inModule and exModule
| comman | meaning       |
| ------ | ------------- |
| 0x11   | open the door |
| 0xcc   | lock the door |
| 0x00   | standby       |


# 硬件pcb和装配

## 引脚使用


