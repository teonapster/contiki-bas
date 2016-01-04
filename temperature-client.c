/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 *      Erbium (Er) CoAP client example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 * 
 * edit by 
 *      Kouroutzidis Theodoros - Nikolaos <kourtheo@csd.auth.gr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"
#include "node-state.h"

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

/* FIXME: This server address is hard-coded for Cooja and link-local for unconnected border router. */
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0xc30c, 0x0000, 0x0000, 0x0002)      /* cooja2 */
/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1) */

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define ASK_TEMPERATURE_EVERY 5
#define TOTAL_NODES 1
PROCESS(er_example_temperature_client, "Temperature process Client");
AUTOSTART_PROCESSES(&er_example_temperature_client);

uip_ipaddr_t server_ipaddr;
static struct etimer et, dailyTimer, workTimer;
static temperature_state node_state;
int atWork = 0;

#define NUMBER_OF_URLS 3
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_temperature_urls[NUMBER_OF_URLS] = {".well-known/core", "sensors/sht11", "error/in//path"};
#if PLATFORM_HAS_BUTTON
static int uri_switch = 1;
#endif

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_temperature_handler(void *response) {
    const uint8_t *chunk;
    //
    int len = coap_get_payload(response, &chunk);
    uint8_t curTemperature = atoi(strtok((char *) chunk, ";"));
    uint8_t curHumidity = atoi(strtok(NULL, ";"));
    uint8_t curThermostatLow = atoi(strtok(NULL, ";"));
    uint8_t curThermostatHigh = atoi(strtok(NULL, ";"));
    uint8_t curThermostatSwitch = atoi(strtok(NULL, ";"));

#if DEBUG
    printf("t=%u, h=%u, therm_l=%u, therm_h=%u, therm_sw=%u\n",
            curTemperature, curHumidity, curThermostatLow, curThermostatHigh, curThermostatSwitch);
#endif
    if (etimer_expired(&dailyTimer) && atWork == 0) {
        etimer_set(&workTimer, 8 * 10 * CLOCK_SECOND);
        atWork = 1;
#if DEBUG
        printf("Start working!\n");
#endif
        etimer_reset(&dailyTimer);
    }
    if (etimer_expired(&workTimer) && atWork == 1) {
        etimer_stop(&workTimer);
#if DEBUG
        printf("Stop working!\n");
#endif  
        atWork = 0;
    }

    if (curTemperature < curThermostatLow && curThermostatSwitch == 0) {// cold + closed thermostat
        node_state.cold = 1;
        node_state.hot = 0;
        printf("COOOOOLD! -> switch on thermostat\n");
    } else if (curTemperature > curThermostatHigh && curThermostatSwitch == 1) { //hot + open thermostat
        node_state.hot = 1;
        node_state.cold = 0;
        printf("HOOOOOOOOOT! -> switch off thermostat\n");
    } else {
        node_state.hot = 0;
        node_state.cold = 0;
    }


}

void
threshold_handler(void *response) {
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
#if DEBUG
    printf("%s\n", len, (char *) chunk);
#endif    
}

PROCESS_THREAD(er_example_temperature_client, ev, data) {
    PROCESS_BEGIN();

    static coap_packet_t request[1], putRequest[1]; /* This way the packet can be treated as pointer as usual. */

    SERVER_NODE(&server_ipaddr); // TODO Configure the rest server nodes :)

    /* receives all CoAP messages */
    coap_init_engine();

    etimer_set(&et, ASK_TEMPERATURE_EVERY * CLOCK_SECOND);
    etimer_set(&dailyTimer, 24 * 10 * CLOCK_SECOND);

    while (1) {
        PROCESS_YIELD();

        if (etimer_expired(&et)) { //TODO repeat for each server


#if DEBUG
            printf("Ask for current temperature\n");
#endif  
            /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
            coap_set_header_uri_path(request, service_temperature_urls[1]);

            const char msg[] = "Toggle!";

            coap_set_payload(request, (uint8_t *) msg, sizeof (msg) - 1);

            PRINT6ADDR(&server_ipaddr);
            PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    client_chunk_temperature_handler);
            char *url;
            if (atWork == 1) {
                if (node_state.cold)
                    url = "?thermostat_low=19&thermostat_high=25&thermostat_switch=1";
                else if (node_state.hot)
                    url = "?thermostat_low=19&thermostat_high=25&thermostat_switch=0";
            } else if (atWork == 0) {
                if (node_state.cold)
                    url = "?thermostat_low=15&thermostat_high=17&thermostat_switch=1";
                else if (node_state.hot)
                    url = "?thermostat_low=15&thermostat_high=17&thermostat_switch=0";
            }

            coap_init_message(putRequest, COAP_TYPE_CON, COAP_POST, 0);
            coap_set_header_uri_path(putRequest, service_temperature_urls[1]);
            coap_set_header_uri_query(putRequest, url);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, putRequest,
                    threshold_handler);

            etimer_reset(&et);
        }

        PROCESS_END();
    }
}
