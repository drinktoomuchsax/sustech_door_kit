|IO选择|对应功能|IO编号|启动项相关|ADC|DAC|UART|Touch|SPI/IIC|
|:----|:----|:----|:----|:----|:----|:----|:----|:----|
| | |GPIO0|下载：低电平/启动：高电平|ADC2_1| | |TP_1| |
| | |GPIO1| | | |TX0| | |
| | |GPIO2| |ADC2_2| | |TP_2| |
| | |GPIO3| | | |RX0| | |
|x|Touch pad|GPIO4| |ADC2_0| | |TP_0| |
| | |GPIO5|启动时必须高电平| | | | |VSPI|
| | |GPIO6|连接了FLASH(CLK)| | | | | |
| | |GPIO7|连接了FLASH(SD0)| | | | | |
| | |GPIO8|连接了FLASH(SD1)| | | | | |
| | |GPIO9|连接了FLASH(SD2)| | |RX1| | |
| | |GPIO10|连接了FLASH(SD3)| | |TX1| | |
| | |GPIO11|连接了FLASH(CMD)| | | | | |
| | |GPIO12|启动时必须低电平|ADC2_5| | |TP_5|HSPI|
| | |GPIO13| |ADC2_4| | |TP_4|HSPI|
| | |GPIO14|启动时必须高电平|ADC2_6| | |TP_6|HSPI|
| | |GPIO15|启动时必须高电平|ADC2_3| | |TP_3|HSPI|
| | |GPIO16| | | |RX2| | |
| | |GPIO17| | | |TX2| | |
| | |GPIO18| | | | | |VSPI|
|x|relay ouput|GPIO19| | | | | |VSPI|
|x|relay ouput|GPIO21| | | | | |IIC_SDA|
|x|relay ouput|GPIO22| | | | | |IIC_SCL|
|x|relay ouput|GPIO23| | | | | |VSPI|
| | |GPIO25| |ADC2_8|DAC_1| | | |
| | |GPIO26| |ADC2_9|DAC_2| | | |
| | |GPIO27| |ADC2_7| | |TP_7| |
| | |GPIO32| |ADC1_4| | |TP_9| |
| | |GPIO33| |ADC1_5| | |TP_8| |
| | |GPIO34|Input Only|ADC1_6| | | | |
| | |GPIO35|Input Only|ADC1_7| | | | |
| | |GPIO36|Input Only/SVP|ADC1_0| | | | |
| | |GPIO39|Input Only/SVN|ADC1_3| | | | |
|x|GND for relay|GND||| | | | |
|x|Powe in|GND||| | | | |
| | |3V3||| | | | |
|x|Power in|VIN||| | | | |