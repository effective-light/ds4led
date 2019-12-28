#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s <version> <r> <g> <b>\n", argv[0]);
        return 1;
    }

    if (hid_init())
        return -1;
    
    hid_device *handle;
    if (!strcmp(argv[1], "2")) {
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
    for (int i = 2; i < 4; i++) {
        char *endptr;
        unsigned char num = strtol(argv[i], &endptr, 10);
        if (argv[i] == endptr) {
            printf("Invalid input\n");
            cleanup(handle);
            return 1;
        }

        buf[0x04 + i] = num;
    }

    int count = hid_write(handle, buf, sizeof(buf));
    if (count < 0) {
        printf("Unable to write()\n");
        printf("Error: %ls\n", hid_error(handle));
    }

    cleanup(handle);
    return 0;
}
