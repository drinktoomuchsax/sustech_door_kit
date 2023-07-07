# architecture
the architecture of the system is design in the [productDesign draw](product_design/productDesign.excalidraw).

inMoudle in charge of controling relay and host WiFi server to access.

exModule in charge of password parity, face recognition, human detect(lidar and touch_pad of esp32-s3). Maybe in the future, homeassistant would be put in exModule.

# code
both esp32-s3 and esp32-c3 using platformIO to develop.
both of them are arduino framework.

exModule using a lib write by drinktoomuchsax and weiyoudong


# communication protocol between inModule and exModule
| comman | meaning       |
| ------ | ------------- |
| 0x00   | open the door |
| 0xcc   | lock the door |
| 0x11   | standby       |