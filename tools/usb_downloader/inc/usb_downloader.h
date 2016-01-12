#ifndef USB_DOWNLOADER_H
#define USB_DOWNLOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <libusb-1.0/libusb.h>

#ifdef DEBUG
#define dbg_printf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define dbg_printf(fmt, ...)
#endif

#define FILE_PATH_BUFF_SIZE 100

enum USB_CLASS {
    USB_CLASS_HID = 0
};

struct ram_area {
    unsigned int start;
    unsigned int size;
};

struct sdp_dev {
	unsigned short vid;
	unsigned short pid;
	unsigned short max_packet_size;
	unsigned char name[64];
    unsigned char usb_class;
    struct ram_area ddr;
    struct ram_area ocram;
    libusb_device_handle *handle;
};

#pragma pack(1)
struct sdp_command {
    uint16_t cmd;
    uint32_t addr;
    uint8_t format;
    uint32_t cnt;
    uint32_t data;
    uint8_t rsvd;
};
#pragma pack ()

#define HAB_SECMODE_PROD 0x12343412
#define HAB_SECMODE_DEV  0x56787856

#define SDP_READ_REG     0x0101
#define SDP_WRITE_REG    0x0202
#define SDP_WRITE_FILE   0x0404
#define SDP_ERROR_STATUS 0x0505
#define SDP_WRITE_DCD    0x0a0a
#define SDP_JUMP_ADDRESS 0x0b0b



#endif
