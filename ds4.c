#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hidapi.h"

#define SONY_V_ID 0x054c
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


void cleanup(hid_device *handle) {
    hid_close(handle);
    hid_exit();
}

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
    int opt, v2 = 0;
    unsigned char r, g, b = 0;
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

    if (hid_init())
        return -1;
    
    hid_device *handle;
    if (v2) {
        handle = hid_open(SONY_V_ID, 0x09cc, NULL); // ds4v2
    } else {
        handle = hid_open(SONY_V_ID, 0x05c4, NULL); // original ds4
    }

    if (!handle) {
        printf("unable to open device\n");
        return 1;
    }

    unsigned char buf[SIZE];
    memset(buf, 0, SIZE);

    buf[0] = 0x05; // report id
    buf[1] = 0xFF;
    buf[6] = r;
    buf[7] = g;
    buf[8] = b;

    int count = hid_write(handle, buf, sizeof(buf));
    if (count < 0) {
        printf("Unable to write()\n");
        printf("Error: %ls\n", hid_error(handle));
    }

    cleanup(handle);
    return 0;
}
