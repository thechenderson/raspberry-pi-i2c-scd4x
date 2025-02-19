/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>  // printf
#include <time.h>  //Record time with each measurement
#include <string.h> //Used to remove the end of the Ctime function to prevent a new line in .CSV file

#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

/**
 * TO USE CONSOLE OUTPUT (PRINTF) IF NOT PRESENT ON YOUR PLATFORM
 */
//#define printf(...)

int main(void) {
    int16_t error = 0;

    sensirion_i2c_hal_init();

    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();

    uint16_t serial_0;
    uint16_t serial_1;
    uint16_t serial_2;
    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
    if (error) {
        printf("Error executing scd4x_get_serial_number(): %i\n", error);
    } else {
        printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
    }

    //Add headers to CSV file. (Make sure a CSV file is not already present as the header will be added to the exisitg CSV otherwise)
    FILE *fs;
    fs = fopen("../Outputs/CO2output.csv", "a");
    fprintf(fs,"Timestamp, CO2, Temperature, Humidity\n");
    fclose(fs);


    // Start Measurement

    error = scd4x_start_periodic_measurement();
    if (error) {
        printf("Error executing scd4x_start_periodic_measurement(): %i\n",
               error);
    }

    printf("Waiting for first measurement... (5 sec)\n");

    for (;;) {

        //Used to store the time for each measurement
        time_t currentTime;
        currentTime = time(NULL);
        char *measurementTime = ctime(&currentTime);
        //Remove the line ending \n from cdate output to allow for use in CSV
        measurementTime[strlen(measurementTime)-1] = '\0';

        // Read Measurement
        sensirion_i2c_hal_sleep_usec(5000000);

        uint16_t co2;
        float temperature;
        float humidity;
        error = scd4x_read_measurement(&co2, &temperature, &humidity);
        if (error) {
            printf("Error executing scd4x_read_measurement(): %i\n", error);
        } else if (co2 == 0) {
            printf("Invalid sample detected, skipping.\n");
        } else {
            printf("SCD40 MEASUREMENT TAKEN - %s\n", measurementTime);
            printf("********************************\n");
            fs = fopen("../Outputs/CO2output.csv", "a");
            fprintf(fs,"%s, %u, %0.2f, %0.2f\n", measurementTime, co2, temperature, humidity);
            fclose(fs);
        }
    }

    return 0;
}
