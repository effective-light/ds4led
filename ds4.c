#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SONY_VID 0x054c
#define DELIM ":"
#define SIZE 32

/* 
   buf[4]  // rumble min
   buf[5]  // rumble max
   buf[6]  // r
   buf[7]  // g
   buf[8]  // b 
   buf[9]  // time to flash bright
   buf[10] // time to flash dark
*/


unsigned char parse_num(char *str) {
    char *endptr;
    unsigned char num = strtol(str, &endptr, 10);
    if (endptr[0] != '\0') {
        printf("Invalid input\n");
        exit(EXIT_FAILURE);
    }

    return num;
}

int main(int argc, char **argv) {
    int opt = 0;
    int v2 = 0;
    int ret = 0;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    while ((opt = getopt(argc, argv, "vr:g:b:")) != -1) {
        switch (opt) {
            case 'v':
                v2 = 0x01;
                break;
            case 'r':
                r = parse_num(optarg);
                break;
            case 'g':
                g = parse_num(optarg);
                break;
            case 'b':
                b = parse_num(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [-r c] [-g c] [-b c]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int pid;
    if (v2) {
        pid = 0x09cc; // ds4v2
    } else {
        pid = 0x05c4; // original ds4
    }

    struct udev *ctx = udev_new();
    if (!ctx) {
        fprintf(stderr, "error with udev!\n");
        exit(EXIT_FAILURE);
    }

    struct udev_enumerate *iter = udev_enumerate_new(ctx);

    udev_enumerate_add_match_subsystem(iter, "hidraw");
    udev_enumerate_scan_devices(iter);

    struct udev_list_entry *entry;
    struct udev_device *dev = NULL;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(iter)) {
        const char *path = udev_list_entry_get_name(entry);
        dev = udev_device_new_from_syspath(ctx, path);
        if (!dev) {
            continue;
        }

        struct udev_device *hid_parent =
            udev_device_get_parent_with_subsystem_devtype(dev, "hid", NULL);
        if (!hid_parent) {
            goto DEV_END;
        }

        char *id = strdup(udev_device_get_property_value(hid_parent, "HID_ID"));
        strtok(id, DELIM);
        int vid = strtol(strtok(NULL, DELIM), NULL, 16);
        int tmp_pid = strtol(strtok(NULL, DELIM), NULL, 16);
        free(id);

        if (vid == SONY_VID && tmp_pid == pid) {
            break;
        }

DEV_END:
        udev_device_unref(dev);
        dev = NULL;
    }

    if (!dev) {
        fprintf(stderr, "unable to find the device!\n");
        ret = 1;
        goto CLEANUP;
    }

    int handle = open(udev_device_get_devnode(dev), O_WRONLY);

    unsigned char buf[SIZE];
    memset(buf, 0, SIZE);

    buf[0] = 0x05; // report id
    buf[1] = 0xFF;
    buf[6] = r;
    buf[7] = g;
    buf[8] = b;

    int count = write(handle, buf, SIZE);
    if (count == -1) {
        perror("write");
        ret = 1;
    }

    udev_device_unref(dev);
CLEANUP:
    udev_enumerate_unref(iter);
    udev_unref(ctx);

    return ret;
}
