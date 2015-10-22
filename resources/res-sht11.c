/*
 * Copyright (c) 2014, Nimbus Centre for Embedded Systems Research, Cork Institute of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      SHT11 Sensor Resource
 *
 *      This is a simple GET resource that returns the temperature in Celsius
 *      and the humidity reading from the SHT11.
 * \author
 *      Pablo Corbalan <paul.corbalan@gmail.com>
 */

#include "contiki.h"

#if PLATFORM_HAS_SHT11

#include <string.h>
#include "rest-engine.h"
#include "dev/sht11/sht11-sensor.h"
#include "core/lib/random.h"
#define MAX_TEMPERATURE 30
#define MIN_TEMPERATURE 10


static uint8_t thermostat_T=15;
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static int temperature = 0;
static int humidity = 0;
static unsigned int lastTempOdd = 0;

/* Get Method Example. Returns the reading from temperature and humidity sensors. */
RESOURCE(res_sht11,
        "title=\"Temperature and Humidity\";rt=\"Sht11\"",
        res_get_handler,
        res_put_handler,
        res_put_handler,
        NULL);
static int get_rand(int interval,int min){
    return (int) (((random_rand()/(RANDOM_RAND_MAX*1.0))*interval)+min);
}
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    /* Temperature in Celsius (t in 14 bits resolution at 3 Volts)
     * T = -39.60 + 0.01*t
     */
    
//    int temperature = ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
    //Create random values
    int rnd = get_rand(15,12);
    if(temperature==0||rnd%2==lastTempOdd){
        lastTempOdd = lastTempOdd==0?1:0;
        temperature = rnd;
    }
    /* Relative Humidity in percent (h in 12 bits resolution)
     * RH = -4 + 0.0405*h - 2.8e-6*(h*h)
     */
//    uint16_t humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
//    humidity = -4 + 0.0405*humidity - 2.8e-6*(humidity*humidity);
    rnd = get_rand(30,70);
    if(humidity==0||rnd%2==lastTempOdd){
//        lastTempOdd = lastTempOdd==0?1:0;
        humidity = rnd;
    }
    unsigned int accept = -1;
    REST.get_header_accept(request, &accept);

    if (accept == -1 || accept == REST.type.TEXT_PLAIN) {
        REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "%d;%u;%u", temperature, humidity,thermostat_T);

        REST.set_response_payload(response, (uint8_t *) buffer, strlen((char *) buffer));
    } else if (accept == REST.type.APPLICATION_XML) {
        REST.set_header_content_type(response, REST.type.APPLICATION_XML);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "<Temperature =\"%u\" Humidity=\"%u\" Temperature_T=\"%u\" />", temperature, humidity,thermostat_T);

        REST.set_response_payload(response, buffer, strlen((char *) buffer));
    } else if (accept == REST.type.APPLICATION_JSON) {
        REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "{'Sht11':{'Temperature':%u,'Humidity':%u,'Temperature_T':%u}}", temperature, humidity,thermostat_T);

        REST.set_response_payload(response, buffer, strlen((char *) buffer));
    } else {
        REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
        const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
        REST.set_response_payload(response, msg, strlen(msg));
    }
}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

    size_t len = 0;
    uint8_t *t = NULL;

    if ((len = REST.get_query_variable(request, "threshold", &t))) {
        printf("Thermostat threshold %u\n", t);

        if (t < MIN_TEMPERATURE || t > MAX_TEMPERATURE) {
            REST.set_response_status(response, REST.status.FORBIDDEN);
            snprintf((char *) buffer, REST_MAX_CHUNK_SIZE,
                    "Please select temperature between %d and %d celsius degrees", 
                    MIN_TEMPERATURE, MAX_TEMPERATURE);
        } else
            thermostat_T = MIN_TEMPERATURE;

        REST.set_response_status(response, REST.status.OK);
    }
    REST.set_response_status(response, REST.status.BAD_REQUEST);
}
#endif /* PLATFORM_HAS_SHT11 */
