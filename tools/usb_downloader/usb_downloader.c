#include <usb_downloader.h>

#include <conf_parser.h>

int find_imx_device(const struct sdp_dev *conf, libusb_device_handle **phandle) {
    libusb_device **devs;
    libusb_device *dev = NULL;

    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        dev = NULL;

    for (ssize_t i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        libusb_device *tmp = devs[i];
        int r = libusb_get_device_descriptor(tmp, &desc);
        if (0 != r) {
            fprintf(stderr, "failed to get device descriptor");
            dev = NULL;
        }

        if((conf->vid == desc.idVendor) && (conf->pid == desc.idProduct)) {
            dev = tmp;
            break;
        }
    }

    if (NULL == dev) {
        fprintf(stderr, "no matching USB device found\n");
    } else {
        int err = libusb_open(dev, phandle);
        if (err) {
			printf("%s:Could not open device vid=0x%x pid=0x%x err=%d\n", __func__, conf->vid, conf->pid, err);
            dev = NULL;
        }
    }

    libusb_free_device_list(devs, 1);
    return (NULL == dev) ? 0 : 1;
}

// HID Class-Specific Requests values. See section 7.2 of the HID specifications
#define HID_GET_REPORT			0x01
#define HID_GET_IDLE			0x02
#define HID_GET_PROTOCOL		0x03
#define HID_SET_REPORT			0x09
#define HID_SET_IDLE			0x0A
#define HID_SET_PROTOCOL		0x0B
#define HID_REPORT_TYPE_INPUT		0x01
#define HID_REPORT_TYPE_OUTPUT		0x02
#define HID_REPORT_TYPE_FEATURE		0x03
#define CTRL_IN			LIBUSB_ENDPOINT_IN |LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE
#define CTRL_OUT		LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE

#define EP_IN	0x80


void dump_bytes(unsigned char *src, unsigned cnt, unsigned addr) {
	unsigned char *p = src;
	int i;
	while (cnt >= 16) {
		printf("%08x: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n", addr,
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		p += 16;
		cnt -= 16;
		addr += 16;
	}
	if (cnt) {
		printf("%08x:", addr);
		i = 0;
		while (cnt) {
			printf(" %02x", p[0]);
			p++;
			cnt--;
			i++;
			if (cnt) if (i == 4) {
				i = 0;
				printf(" ");
			}
		}
		printf("\n");
	}
}

/*
 * transfer_hid - hid发送器
 *
 * @dev: SDP 设备
 * @report: hid report
 * @p: 缓存的指针
 * @size: 缓存大小
 * @expected: 预期的数据(UART)
 * @last_trans: 实际传送的字节数
 */
int transfer_hid(struct sdp_dev *dev, int report, unsigned char *p, unsigned int cnt, unsigned int expected, int *last_trans) {
    libusb_device_handle *h = dev->handle;

    if (cnt > dev->max_packet_size)
        cnt = dev->max_packet_size;

#ifdef DEBUG
	printf("report=%i\n", report);
	if (report < 3) {
        printf("---------------------------------------------\n");
		dump_bytes(p, cnt, 0);
        printf("---------------------------------------------\n");
    }
#endif
    
    unsigned char tmp[1028];
    tmp[0] = (unsigned char)report;
    int err;

    if (report < 3) {
        memcpy(&tmp[1], p, cnt);
        err = libusb_control_transfer(h, CTRL_OUT, HID_SET_REPORT, (HID_REPORT_TYPE_OUTPUT << 8) | report, 0, tmp, cnt+1, 1000);
        last_trans[0] = (err > 0) ? err - 1 : 0;
        if (err > 0)
            err = 0;
    } else {
        last_trans[0] = 0;
        memset(&tmp[1], 0, cnt);
        err = libusb_interrupt_transfer(h, 1 + EP_IN, tmp, cnt + 1, last_trans, 1000);
        dbg_printf("libusb_interrupt_transfer, err=%d, trans=%d\n", err, *last_trans);
        if (err >= 0) {
            if (tmp[0] == (unsigned char)report) {
                if (last_trans[0] > 1) {
                    last_trans[0] -= 1;
                    memcpy(p, &tmp[1], last_trans[0]);
                }
            } else {
				printf("Unexpected report %i err=%i, cnt=%i, last_trans=%i, %02x %02x %02x %02x\n",
					tmp[0], err, cnt, *last_trans, tmp[0], tmp[1], tmp[2], tmp[3]);
				err = 0;
            }
        }
    }

#ifdef DEBUG
    if (report >= 3) {
        printf("---------------------------------------------\n");
		dump_bytes(p, cnt, 0);
        printf("---------------------------------------------\n");
    }
#endif
    
    return err;
}
/*
 * check_error_status - 查询SDP设备错误状态
 *
 * @dev: SDP设备
 */
int check_error_status(struct sdp_dev *dev) {
    struct sdp_command status_command = {
        .cmd = SDP_ERROR_STATUS,
        .addr = 0,
        .format = 0,
        .cnt = 0,
        .data = 0,
        .rsvd = 0
    };

    unsigned char tmp[64];
    int last_trans;
    int err;
    int cnt = 64;
    
    for (int i = 0; i < 5; i++) {
        err = transfer_hid(dev, 1, (char*)&status_command, sizeof(status_command), 0, &last_trans);
        dbg_printf("report 1, wrote %i bytes, err=%i\n", last_trans, err);
        memset(tmp, 0, sizeof(tmp));

        err = transfer_hid(dev, 3, tmp, cnt, 4, &last_trans);
        dbg_printf("report 3, read %i bytes, err=%i\n", last_trans, err);
        dbg_printf("read=%02x %02x %02x %02x\n", tmp[0], tmp[1], tmp[2], tmp[3]);

        if (!err)
            break;
    }

    unsigned int *hab_security = (unsigned int *)tmp;
 	printf("HAB security state: %s (0x%08x)\n", *hab_security == HAB_SECMODE_PROD ?
			"production mode" : "development mode", *hab_security);

    err = transfer_hid(dev, 4, tmp, cnt, 4, &last_trans);
	dbg_printf("report 4, read %i bytes, err=%i\n", last_trans, err);
	dbg_printf("read=%02x %02x %02x %02x\n", tmp[0], tmp[1], tmp[2], tmp[3]);
    
    return err;
}

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || defined(__BIG_ENDIAN__)
#define BE32(a) (a)
#else
#define BE32(a) (((a & 0xff000000)>>24) | ((a & 0x00ff0000)>>8) | ((a & 0x0000ff00)<<8) | ((a & 0x000000ff)<<24))
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
/*
 * read_register - 读SDP设备
 *
 * @dev: SDP设备
 * @addr: SDP设备起始地址
 * @dest: 目标地址
 * @cnt: 字节数
 */
int read_register(struct sdp_dev *dev, unsigned addr, unsigned char *dest, unsigned cnt) {
    struct sdp_command read_reg_command = {
		.cmd = SDP_READ_REG,
		.addr = BE32(addr),
		.format = 0x20,
		.cnt = BE32(cnt),
		.data = BE32(0),
		.rsvd = 0x00
    };

    int err, rem;
    unsigned char tmp[64];
    int last_trans;

    for (int i = 0; i < 5; i++) {
        err = transfer_hid(dev, 1, (char*)&read_reg_command, sizeof(read_reg_command), 0, &last_trans);
        if (!err)
            break;
    }
    err = transfer_hid(dev, 3, tmp, 4, 4, &last_trans);
    if (err) {
        printf("r3 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, tmp[0], tmp[1], tmp[2], tmp[3]);
        return err;
    }
    rem = cnt;
    while (rem) {
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = 0;
        err = transfer_hid(dev, 4, tmp, 64, (rem > 64 ? 64 : rem), &last_trans);
        if (err) {
            printf("r4 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, tmp[0], tmp[1], tmp[2], tmp[3]);
            break;
        }
        if ((last_trans > rem) || (last_trans > 64)) {
            last_trans = rem;
            if (last_trans > 64)
                last_trans = 64;
        }
        memcpy(dest, tmp, last_trans);
        dest += last_trans;
        rem -= last_trans;
    }
    dest -= cnt;
	dbg_printf("%s: %d addr=%08x, val=%02x %02x %02x %02x\n", __func__, err, addr, dest[0], dest[1], dest[2], dest[3]);
	return err;
}
/*
 * write_register - 写寄存器
 *
 * @dev: SDP设备
 * @addr: SDP设备起始地址
 * @val: 目标值
 */
int write_register(struct sdp_dev *dev, unsigned addr, unsigned val) {
    struct sdp_command write_reg_command = {
        .cmd = SDP_WRITE_REG,
        .addr = BE32(addr),
        .format = 0x20,
        .cnt = BE32(0x00000004),
        .data = BE32(val),
        .rsvd = 0x00
    };
	
    int err, rem;
    unsigned char tmp[64];
    int last_trans;

    for (int i = 0; i < 5; i++) {
        err = transfer_hid(dev, 1, (char*)&write_reg_command, sizeof(write_reg_command), 0, &last_trans);
        if (!err)
            break;
    }

    memset(tmp, 0, sizeof(tmp));
    err = transfer_hid(dev, 3, tmp, 4, 4, &last_trans);
    if (err) {
        printf("w3 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, tmp[0], tmp[1], tmp[2], tmp[3]);
		printf("addr=0x%08x, val=0x%08x\n", addr, val);
        return err;
    }
	err = transfer_hid(dev, 4, tmp, sizeof(tmp), 4, &last_trans);
    if (err) {
        printf("w4 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, tmp[0], tmp[1], tmp[2], tmp[3]);
		printf("addr=0x%08x, val=0x%08x\n", addr, val);
    }
    return err;
}
/*
 * get_file_size - 获取文件大小
 *
 * @file: 文件
 */
static long get_file_size(FILE *file) {
    long tmp = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, tmp, SEEK_SET);
    tmp = ftell(file);
    return size;
}
/*
 * write_file - 下载文件到芯片
 *
 * @dev: SDP设备
 * @addr: SDP设备起始地址
 * @file_name: 目标文件名
 */
int write_file(struct sdp_dev *dev, unsigned addr, const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    assert(NULL != file);

    unsigned fsize = get_file_size(file);
    unsigned left = fsize;
    
	struct sdp_command dl_command = {
		.cmd = SDP_WRITE_FILE,
		.addr = BE32(addr),
		.format = 0,
		.cnt = BE32(fsize),
		.data = 0,
		.rsvd = 0 
    };

    int err;
    int last_trans;
    char buff[1024];
    
    printf("\nloading file '%s'\n", file_name);
    printf("address: 0x%08x\n", addr);

    for (int i = 0; i < 5; i++) {
        err = transfer_hid(dev, 1, (char*)&dl_command, sizeof(dl_command), 0, &last_trans);
        if (!err)
            break;
    }

    while (left > 0) {
        int cnt = fread(buff, 1, min(left, 1024), file);

        for (int i = 0; i < 5; i++) {
            err = transfer_hid(dev, 2, buff, cnt, 0, &last_trans);
            if (!err) {
                left -= last_trans;
                break;
            }
        }

        if (feof(file))
            break;
    }

    err = transfer_hid(dev, 3, buff, sizeof(buff), 4, &last_trans);
    if (err) {
        printf("w3 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, buff[0], buff[1], buff[2], buff[3]);
    }
	err = transfer_hid(dev, 4, buff, sizeof(buff), 4, &last_trans);
    if (err) {
        printf("w4 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, buff[0], buff[1], buff[2], buff[3]);
    }
    unsigned int *status = (unsigned int*)buff;
    if (*status == 0x88888888UL)
		printf("succeeded (status 0x%08x)\n", *status);
    else
        printf("failed (status 0x%08x)\n", *status);
   
    return 0;
}
/*
 * write_dcd - 下载DCD
 *
 * @dev: SDP设备
 * @addr: SDP设备起始地址
 * @file_name: 目标文件名
 */
int write_dcd(struct sdp_dev *dev, unsigned addr, const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    assert(NULL != file);

    unsigned fsize = get_file_size(file);
    unsigned left = fsize;
    
	struct sdp_command dl_command = {
		.cmd = SDP_WRITE_DCD,
		.addr = BE32(addr),
		.format = 0,
		.cnt = BE32(fsize),
		.data = 0,
		.rsvd = 0 
    };

    int err;
    int last_trans;
    char buff[1024];
    
    printf("\nloading file '%s'\n", file_name);
    printf("address: 0x%08x\n", addr);

    for (int i = 0; i < 5; i++) {
        err = transfer_hid(dev, 1, (char*)&dl_command, sizeof(dl_command), 0, &last_trans);
        if (!err)
            break;
    }

    while (left > 0) {
        int cnt = fread(buff, 1, min(left, 1024), file);

        for (int i = 0; i < 5; i++) {
            err = transfer_hid(dev, 2, buff, cnt, 0, &last_trans);
            if (!err) {
                left -= last_trans;
                break;
            }
        }

        if (feof(file))
            break;
    }

    err = transfer_hid(dev, 3, buff, sizeof(buff), 4, &last_trans);
    if (err) {
        printf("w3 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, buff[0], buff[1], buff[2], buff[3]);
    }
	err = transfer_hid(dev, 4, buff, sizeof(buff), 4, &last_trans);
    if (err) {
        printf("w4 in err=%i, last_trans=%i  %02x %02x %02x %02x\n", err, last_trans, buff[0], buff[1], buff[2], buff[3]);
    }
    unsigned int *status = (unsigned int*)buff;
    if (*status == 0x128A8A12UL)
		printf("succeeded (status 0x%08x)\n", *status);
    else
        printf("failed (status 0x%08x)\n", *status);
   
    return 0;
}

int main(int argc, char *argv[]) {
    char *base_path = get_pwd();
    char *conf_file = get_conf_file("usb.conf", base_path);

    assert(NULL != conf_file);
    struct sdp_dev *sdp = parse_conf_file(conf_file);

    libusb_init(NULL);
    libusb_device_handle *h = NULL;

    if(!find_imx_device(sdp, &h))
        goto out;

    if (NULL == h)
        goto out;

    sdp->handle = h;

    int bConfigurationValue;
    libusb_get_configuration(h, &bConfigurationValue);
    printf("bConfigurationValue=%x\n", bConfigurationValue);

    if (libusb_kernel_driver_active(h, 0))
        libusb_detach_kernel_driver(h, 0);

    if (0 != libusb_claim_interface(h, 0)) {
        fprintf(stderr, "Claim interface failed\n");
        goto out;
    }

    printf("Interface 0 claimed\n");

    if (0 != check_error_status(sdp)) {
        fprintf(stderr, "Check Error Status Failed\n");
        goto out;
    }

    write_file(sdp, 0x00907000, "dcd.bin");

    unsigned char val[0x40];
    read_register(sdp, 0x00907000, val, 0x40);
    write_register(sdp, 0x00907000, 1);
    //printf("========\n");
    read_register(sdp, 0x00907000, val, 0x40);

    printf("========\n");
    write_dcd(sdp, 0x00907000, "dcd.bin");
    printf("========\n");
    write_register(sdp, 0x10000000, 1);
    read_register(sdp, 0x10000000, val, 0x40);

    
    libusb_release_interface(h, 0);

 out:
    if (h)
        libusb_close(h);
    libusb_exit(NULL);
    free(conf_file);
    free(base_path);
    return 0;
}
