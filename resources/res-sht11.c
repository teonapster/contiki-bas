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
 * \edit-by
 *      Kouroutzidis Theodoros-Nikolaos <kourtheo@csd.auth.gr>
 */

#include "contiki.h"

#if PLATFORM_HAS_SHT11

#include <string.h>
#include "rest-engine.h"
#include "dev/sht11/sht11-sensor.h"
#include "core/lib/random.h"
#define MAX_TEMPERATURE 30
#define MIN_TEMPERATURE 10
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

static uint8_t thermostat_low=16, thermostat_high=18,thermostat_switch=0;
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static int temperature = 0;
static int humidity = 0;
static unsigned int lastTempOdd = 0;

/* Get Method Example. Returns the reading from temperature and humidity sensors. */
RESOURCE(res_sht11,
        "title=\"Temperature and Humidity: ?thermostat=0.. , POST/PUT \";rt=\"Sht11\"",
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
    int temperature = ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
    //Create random values
    
#ifdef DEBUG 
    int rnd = get_rand(15,12);
    if(temperature==0||rnd%2==lastTempOdd){
        lastTempOdd = lastTempOdd==0?1:0;
        temperature = rnd;
    }
#endif
    /* Relative Humidity in percent (h in 12 bits resolution)
     * RH = -4 + 0.0405*h - 2.8e-6*(h*h)
     */
    uint16_t humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
    humidity = -4 + 0.0405*humidity - 2.8e-6*(humidity*humidity);
    
#ifdef DEBUG
    rnd = get_rand(30,70);
    if(humidity==0||rnd%2==lastTempOdd){
//        lastTempOdd = lastTempOdd==0?1:0;
        humidity = rnd;
    }
#endif
    unsigned int accept = -1;
    REST.get_header_accept(request, &accept);

    if (accept == -1 || accept == REST.type.TEXT_PLAIN) {
        REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "%d;%u;%u;%u;%u",temperature, humidity,thermostat_low,thermostat_high,thermostat_switch);

        REST.set_response_payload(response, (uint8_t *) buffer, strlen((char *) buffer));
    } else if (accept == REST.type.APPLICATION_XML) {
        REST.set_header_content_type(response, REST.type.APPLICATION_XML);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "<Temperature =\"%u\" Humidity=\"%u\" Temperature_low=\"%u\" Temperature_high=\"%u\" Thermostat_switch=\"%u\" />",
                temperature, humidity,thermostat_low,thermostat_high,thermostat_switch);
        REST.set_response_payload(response, buffer, strlen((char *) buffer));
    }
}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

    size_t len = 0;
    char * t = NULL;
    const uint8_t *chunk;
    const char *msg = NULL;
    if ((len = REST.get_query_variable(request, "thermostat_low", &t))) {
        PRINTF("thermostat low %.*s\n", len, t);
        REST.set_response_status(response, REST.status.OK);
        msg = "Thermostat low value updated!!!";
        thermostat_low = atoi(t);
    }
    len =0;
    if ((len = REST.get_query_variable(request, "thermostat_high", &t))) {
        
        REST.set_response_status(response, REST.status.OK);
        msg = "Thermostat high value updated!!!";
        thermostat_high = atoi(t);
        PRINTF("thermostat high %u\n",thermostat_high);
    }
    len = 0;
    if ((len = REST.get_query_variable(request, "thermostat_switch", &t))) {
        PRINTF("thermostat switch%.*s\n", len, t);
        REST.set_response_status(response, REST.status.OK);
        msg = "Thermostat switch value updated!!!";
        thermostat_switch = atoi(t);
    }
    else{
     REST.set_response_status(response, REST.status.BAD_REQUEST);
     msg = "Thermostat value update failed!!!";
    }
    REST.set_response_payload(response, msg, strlen(msg));
}
#endif /* PLATFORM_HAS_SHT11 */
