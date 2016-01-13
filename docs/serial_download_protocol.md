# 串行下载协议

Freescale提供了一种串行下载协议(Serial Download Protocol, SDP).
通过修改引脚BOOT_MODE0和BOOT_MODE1控制芯片从Serial Downloader启动.
此时,Boot ROM支持串行下载协议,通过此协议可以从USB OTG口下载文件到芯片的指定地址上,
也可以直接访问一些内存和寄存器,下载DCD文件.

## SDP通信格式

ROM用USB OTG端口作为boot的下载端口.其余的USB端口都不能作此用.连接到PC机后被设置为一个USB HID类的设备.
有4种HID报文被用来支持串行下载协议(Serial Download Protocol, SDP).

| Report ID(first byte) | Transfer Endpoint | Direction      | Length       | 描述             |
|:---------------------:|-------------------|----------------|--------------|------------------|
| 1                     | control OUT       | Host to device | 17字节       | SDP指令          |
| 2                     | control OUT       | Host to device | 至多1025字节 | SDP指令的数据部分 |
| 3                     | interrupt         | Device to host | 5字节        | HAB安全设置. Closed mode: 0x12343412; Open mode: 0x56787856 |
| 4                     | interrupt         | Device to host | 至多65字节   | SDP指令响应       |

host通过HID Report 1向设备发送16字节的SDP命令,支持6种命令:

* 0x0101 READ_REGISTER
* 0x0202 WRITE_REGISTER
* 0x0404 WRITE_FILE
* 0x0505 ERROR_STATUS
* 0x0A0A DCD_WRITE
* 0x0B0B JUMP_ADDRESS
 
命令结构如下:

| Byte Offset | Size | Name         | 描述        |
|-------------|------|--------------|-------------|
| 0           | 2    | COMMAND TYPE | i.MX6 Quad ROM支持6种命令 |
| 2           | 4    | ADDRESS      | 适用于除了ERROR_STATUS命令外的5中命令 |
| 6           | 1    | FORMAT       | 数据格式,只对READ_REGISTER和WRITE_REGISTER命令有效<br> 0x8: 8-bit<br>  0x10: 16-bit<br>  0x20: 32-bit |
| 7           | 4    | DATA COUNT   | 读写数据大小<br>只对WRITE_FILE, READ_REGISTER, WRITE_REGISTER和DCD_WRITE有效<br>对于WRITE_FILE和DCD_WRITE命令DATA COUNT指字节数 |
| 11          | 4    | DATA         | 写数据,只对WRITE_REGISTER有效 |
| 15          | 1    | RESERVED     | 保留 |

## READ_REGISTER (0x0101)

READ_REGISTER的通信过程包括三个部分:

* Report 1. 命令 (READ_REGISTER_COMMAND, ADDRESS, FORMAT, DATA_COUNT)
* Report 3. HAB安全设置
* Report 4. 响应

首先,Host发送Report 1到i.MX6Q;

然后,设备返回Report 3,描述HAB状态;

最后,设备返回Report 4,记录了Host请求的数据,若请求的数据多于64个字节,将拆分为多个包发送.

## WRITE_REGISTER (0x0202)

WRITE_REGISTER的通信过程包括三个部分:

* Report 1. 命令 (WRITE_REGISTER_COMMAND, ADDRESS, FORMAT, DATA_COUNT, DATA)
* Report 3. HAB安全设置
* Report 4. 写完成(0x128A8A12)状态

首先, Host发送Report 1到i.MX6Q;

然后,设备返回Report 3,描述HAB状态;

最后,若写数据完成报告一个写完成(0x128A8A12)的状态,若失败报告HAB错误状态(HAB error status).

## WRITE_FILE (0x0404)

WRITE_FILE的通信过程包括4个部分:

* Report 1. 命令 (WRITE_FILE_COMMAND, ADDRESS, DATA_COUNT)
* Report 2. 数据
* Report 3. HAB安全设置
* Report 4. 完成(0x88888888)状态

首先, Host发送Report 1到i.MX6Q;

然后, 发送文件数据到设备,若文件大小超过1024个字节,则需要拆分为多个包发送;

然后, 设备报告HAB状态;

最后, 若完成则报告一个完成(0x88888888)的状态, 若失败报告HAB错误状态(HAB error status).

## ERROR_STATUS (0x0505)

ERROR_STATUS的通信过程包括三个部分:

* Report 1. 命令 (ERROR_STATUS_COMMAND)
* Report 3. HAB安全设置
* Report 4. 4字节的错误状态

首先, Host发送Report 1到i.MX6Q;

然后, 设备返回Report 3, 描述HAB状态;

最后, 设备报告错误状态.

## DCD_WRITE (0x0A0A)

DCD_WRITE指令提供了一种批量配置芯片寄存器的功能.其通信过程包括4个部分:

* Report 1. 命令 (DCD_WRITE_COMMAND, ADDRESS, DATA_COUNT)
* Report 2. 数据
* Report 3. HAB安全设置
* Report 4. 完成(0x128A8A12)状态

首先, Host发送Report 1到i.MX6Q;

然后, 发送文件数据到设备,若文件大小超过1024个字节,则需要拆分为多个包发送;

然后, 设备报告HAB状态;

最后, 若完成则报告一个完成(0x128A8A12)的状态, 若失败报告HAB错误状态(HAB error status).

* [设备配置数据, Device Configuration Data, DCD](docs/device_configuration_data.md)

## JUMP_ADDRESS (0x0B0B)

JUMP_ADDRESS指令控制芯片跳转到指定的地址并执行. 该指令应当是Serial Downloader启动过程的最后一条指令.
其通信过程包括3个部分:

* Report 1. 命令 (JUMP_ADDRESS_COMMAND, ADDRESS)
* Report 3. HAB安全设置
* Report 4. 4字节的错误状态

首先, Host发送Report 1到i.MX6Q;

然后, 设备返回Report 3, 描述HAB状态;

最后, 设备报告错误状态.

