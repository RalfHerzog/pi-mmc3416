//
// Created by ralf on 26.10.21.
//

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>

#include "mmc3416.h"

int main(void) {
    char *topic = "/compass/orientation/";

    // Connect to zmq proxy with pub socket
    void *context = zmq_ctx_new();
    void *publisher = zmq_socket(context, ZMQ_PUB);

    int rc = zmq_connect(publisher, "tcp://127.0.0.1:5560");
    if (rc != 0) {
        printf("Error: connect");
        return -1;
    }

    get_i2cbus(I2CBUS, I2C_ADDR);

    // Read sample frequency of mmc3416
    struct mmc3416inf mmc3416i = {0};
    mmc3416_info(&mmc3416i);

    /* cont read frequency mode from reg 0x07 bit-2 and 3 */
    int cmfreq_mode = ((mmc3416i.ctl_0_mode >> 2) & 0x03);
    printf("Continuous Read Freq. = 0x%02X\n", cmfreq_mode);

    int sample_freq = 0; // in ms
    if (cmfreq_mode == 0x00) sample_freq = 1500; // 1.5 Hz
    else if (cmfreq_mode == 0x01) sample_freq = 77; // 13 Hz
    else if (cmfreq_mode == 0x02) sample_freq = 40; // 25 Hz
    else if (cmfreq_mode == 0x03) sample_freq = 20; // 50 Hz
    else sample_freq = 1500; // 1.5 Hz

    while (1) {
        // Sleep for the sample time
        usleep(sample_freq * 1000);

        // Perform a read to query the angle
        struct mmc3416data mmc3416d;
        mmc3416_init(&mmc3416d);
        int res = mmc3416_read(&mmc3416d);
        if (res != 0) {
            printf("Error: could not read data from the sensor.\n");
            continue;
        }
        float angle = get_heading(&mmc3416d);
        /* ----------------------------------------------------------- *
         * print the formatted output string to stdout (Example below) *
         * 1584280335 Heading=337.2 degrees                            *
         * Note the sensor has a accuracy of +/-1 degree, fractions    *
         * don't make much sense. Consider taking them off...          *
         * ----------------------------------------------------------- */
        printf("Angle=%3.1f degrees\n", angle);

        // Format a string to send
        char *angle_str;
        angle_str = (char *) malloc(6);
        sprintf(angle_str, "%3.1f", angle);

        // Send angle via zmq
        zmq_send(publisher, topic, strlen(topic), ZMQ_SNDMORE);
        zmq_send(publisher, angle_str, strlen(angle_str), 0);

        free(angle_str);
    }

    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}