#include <usb_downloader.h>
#include <conf_parser.h>

/*
 * get_pwd - 获取当前工作目录
 */
char* get_pwd() {
    size_t size = FILE_PATH_BUFF_SIZE;

    while (1) {
        char *buffer = (char *)malloc(size * sizeof(char));

        if (getcwd(buffer, size) == buffer)
            return buffer;
        free(buffer);

        if (ERANGE != errno)
            return 0;
        size *= 2;
    }
}

/*
 * get_conf_file - 获取配置文件
 *
 * @file: 配置文件名
 * @path: 目录
 */
char* get_conf_file(const char *file, const char *path) {
    dbg_printf("finding '%s' in '%s'\n", file, path);

    char *re;
    asprintf(&re, "%s/%s", path, file);
    
    if (access(re, R_OK) != -1) {
        return re;
    }
    free(re);
    return NULL;
}


/*
 * parse_conf_file - 解析配置文件
 *
 * @file_name: 配置文件
 */
struct sdp_dev *parse_conf_file(const char const *file_name) {
    FILE *file = fopen(file_name, "rb");
    assert(NULL != file);

    size_t n = 40;
    char *line = (char *)malloc(n);
    int lineNum = 0;

	struct sdp_dev *re = (struct sdp_dev *)malloc(sizeof(struct sdp_dev));
    assert(NULL != re);
    memset(re, 0, sizeof(struct sdp_dev));
    
    while (-1 != getline(&line, &n, file)) {
        lineNum++;
        char *p = line;
        while (isblank(p[0])) p++;

        if (p[0] == '#' || p[0] == '\n')
            continue;

        if (0 == strncmp(p, "vid", 3)) {
            p += 3;
            assert('=' == p[0]);
            p++;
            re->vid = (unsigned short)strtol(p, &p, 16);
            continue;
        }

        if (0 == strncmp(p, "pid", 3)) {
            p += 3;
            assert('=' == p[0]);
            p++;
            re->pid = (unsigned short)strtol(p, &p, 16);
            continue;
        }

        if (0 == strncmp(p, "name", 4)) {
            p += 4;
            assert('=' == p[0]);
            p++;
            int i;
            for (i = 0; isalnum(p[i]) || '_' == p[i]; i++) {
                re->name[i] = p[i];
            }
            assert(i < 63);
            re->name[i] = '\0';
            continue;
        }

        if (0 == strncmp(p, "usb_class", strlen("usb_class"))) {
            p += strlen("usb_class");
            assert('=' == p[0]);
            p++;
            if (0 == strncmp(p, "hid", 3)) {
                re->usb_class = USB_CLASS_HID;
            } else {
                fprintf(stderr,"hid expected, file:%s, line:%d\n", file_name, lineNum);
                assert(0);
            }
            continue;
        }

        if (0 == strncmp(p, "max_packet_size", strlen("max_packet_size"))) {
            p += strlen("max_packet_size");
            assert('=' == p[0]);
            p++;
            re->max_packet_size = (unsigned short)strtol(p, &p, 10);
            continue;
        }

        if (0 == strncmp(p, "ddr_start", strlen("ddr_start"))) {
            p += strlen("ddr_start");
            assert('=' == p[0]);
            p++;
            (re->ddr).start = (unsigned int)strtol(p, &p, 16);
            continue;
        }

        if (0 == strncmp(p, "ddr_size", strlen("ddr_size"))) {
            p += strlen("ddr_size");
            assert('=' == p[0]);
            p++;
            (re->ddr).size = (unsigned int)strtol(p, &p, 10);
            if ('G' == p[0] || 'g' == p[0])
                re->ddr.size <<= 30;
            else if ('M' == p[0] || 'm' == p[0])
                re->ddr.size <<= 20;
            else if ('K' == p[0] || 'k' == p[0])
                re->ddr.size <<= 10;
            continue;
        }

        if (0 == strncmp(p, "ocram_start", strlen("ocram_start"))) {
            p += strlen("ocram_start");
            assert('=' == p[0]);
            p++;
            (re->ocram).start = (unsigned int)strtol(p, &p, 16);
            continue;
        }

        if (0 == strncmp(p, "ocram_size", strlen("ocram_size"))) {
            p += strlen("ocram_size");
            assert('=' == p[0]);
            p++;
            (re->ocram).size = (unsigned int)strtol(p, &p, 16);
            if ('G' == p[0] || 'g' == p[0])
                re->ocram.size <<= 30;
            else if ('M' == p[0] || 'm' == p[0])
                re->ocram.size <<= 20;
            else if ('K' == p[0] || 'k' == p[0])
                re->ocram.size <<= 10;
            continue;
        }
       
    }
    fclose(file);
    
    assert((0 != re->vid) && (0 != re->pid));
	printf("vid=0x%04x pid=0x%04x\n", re->vid, re->pid);
    printf("name=%s\n", re->name);
    printf("usb_class=%d\n", re->usb_class);
    printf("max_packet_size=%d\n", re->max_packet_size);
    printf("ddr_start=0x%08x\n", (re->ddr).start);
    printf("ddr_size=0x%08x\n", (re->ddr).size);
    printf("ocram_start=0x%08x\n", (re->ocram).start);
    printf("ocram_size=0x%08x\n", (re->ocram).size);


    return re;
}

