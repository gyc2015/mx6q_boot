# USB Downloader

PC端通过USB下载文件到i.MX6Q的指定地址中,可以用作Serial Downloader启动系统.<br>
本工具是PC端对[串行下载协议,Serial Download Protocol, SDP](/docs/serial_download_protocol.md)的实现.

* 工作环境: Ubuntu 14.04 x86_64
* 编译环境: gcc 4.8.4, GNU Make 3.81
* 依赖: libusb-1.0

## 需要注意的是:

* 通过SDP启动时镜像的ivt偏移为0
* 需要以管理员身份运行
