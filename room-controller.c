/*
 * Copyright (c) 2014, Daniele Alessandrelli.
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
 *      Erbium (Er) CoAP observe client example.
 * \author
 *      Daniele Alessandrelli <daniele.alessandrelli@gmail.com>
 * \edit-by 
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


/*----------------------------------------------------------------------------*/
//#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTFLN(format, ...) printf(format "\n", ##__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:" \
                                "%02x%02x:%02x%02x:%02x%02x:%02x%02x]", \
                                ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], \
                                ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], \
                                ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], \
                                ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], \
                                ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], \
                                ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], \
                                ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], \
                                ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTFLN(...)
#endif


/*----------------------------------------------------------------------------*/
/* FIXME: This server address is hard-coded for Cooja */
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0xc30c, \
                                          0x0000, 0x0000, 0x0002)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)
#define LOCAL_PORT 61617
/* Toggle interval in seconds */
#define TOGGLE_INTERVAL 30
/* Motion sensor observer resource */
#define OBS_RESOURCE_URI "periodic/motion"

/* Light actuator resource*/
#define LIGHT_ACTUATOR_RESOURCE_URI "actuators/light_bulb"

#define MAX_PAYLOAD_LEN 100
/* Light resource */
#define LIGHT_RESOURCE_URI "sensors/light"
#define DAY_LIGHT_LIMIT 175

#define COAP_DATA_BUFF_SIZE 1024
/*----------------------------------------------------------------------------*/
static uip_ipaddr_t server_ipaddr[1]; /* holds the server ip address */
static motion_state serv1state;
static uint8_t askForLight = 0;
uint8_t alarm = 0;


#define ASK_TEMPERATURE_EVERY 5
#define TOTAL_NODES 1

static struct etimer et, dailyTimer, workTimer, tempInterval;
static temperature_state node_state;
int atWork = 0;

#define NUMBER_OF_URLS 3
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_temperature_urls[NUMBER_OF_URLS] = {".well-known/core", "sensors/sht11", "error/in//path"};
#if PLATFORM_HAS_BUTTON
static int uri_switch = 1;
#endif
/*----------------------------------------------------------------------------*/
PROCESS(temperature_poll, "Erbium Coap Observe Motion Resource");
PROCESS(er_example_toggle_client, "Erbium Coap Toggle Resource");
AUTOSTART_PROCESSES(&temperature_poll);
/*----------------------------------------------------------------------------*/





/*----------------------------------------------------------------------------*/

/*
 * The main (proto-)thread. It starts/stops the observation of the remote
 * resource every time the timer elapses or the button (if available) is
 * pressed
 */
PROCESS_THREAD(temperature_poll, ev, data) {
    PROCESS_BEGIN();

    static coap_packet_t request[1], putRequest[1]; /* This way the packet can be treated as pointer as usual. */

    /* store server address in server_ipaddr */
    SERVER_NODE(server_ipaddr);
    /* receives all CoAP messages */
    coap_init_engine();
    /* init timer and button (if available) */
    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
    etimer_set(&tempInterval, ASK_TEMPERATURE_EVERY * CLOCK_SECOND);
    etimer_set(&dailyTimer, 24 * 10 * CLOCK_SECOND);

#if PLATFORM_HAS_BUTTON
    SENSORS_ACTIVATE(button_sensor);
    printf("Press a button to start/stop observation of remote resource\n");
#endif

    /* toggle observation every time the timer elapses or the button is pressed */
    while (1) {
        PROCESS_YIELD();
        if (etimer_expired(&et)) {
            //ASK FOR LIGHT VALUE -- TODO can we make this async?
            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
            coap_set_header_uri_path(request, LIGHT_RESOURCE_URI);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    light_response_handler);
            if (serv1state.light < DAY_LIGHT_LIMIT) {
                set_observation(1);
            } else { // natural light
                set_observation(0);
                process_start(&er_example_toggle_client, "0;0;");
                process_post_synch(&er_example_toggle_client, PROCESS_EVENT_CONTINUE, NULL);
            }
            etimer_reset(&et);
#if PLATFORM_HAS_BUTTON
            //    } else if(ev == sensors_event && data == &button_sensor) {
            //      printf("--Toggle tutton--\n");
            //      toggle_observation();
            //      printf("\n--Done--\n");
#endif
        }

        if (etimer_expired(&tempInterval)) { //TODO repeat for each server


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

            etimer_reset(&tempInterval);
        }
    }
    PROCESS_END();
}

/*
 * Handle the response to the observe request and the following notifications
 */
static void
notification_callback(coap_observee_t *obs, void *notification,
        coap_notification_flag_t flag) {
    int len = 0;
    const uint8_t *payload = NULL;

    printf("Notification handler\n");
    printf("Observee URI: %s\n", obs->url);
    if (notification) {
        len = coap_get_payload(notification, &payload);
    }
    switch (flag) {
        case NOTIFICATION_OK:
            printf("NOTIFICATION OK: %*s\n", len, (char *) payload);
            break;
        case OBSERVE_OK: /* server accepeted observation request */
            serv1state.motion = atoi((char *) payload);
            printf("OBSERVE_OK: %u\n", serv1state.motion);
            if (atWork) {
                //TODO Open lights
                printf("TOGGLE LIGHTS");
                process_start(&er_example_toggle_client, "0;1;");
                process_post_synch(&er_example_toggle_client, PROCESS_EVENT_CONTINUE, NULL);

                //TODO Close alarm
                process_start(&er_example_toggle_client, "1;0;");
                process_post_synch(&er_example_toggle_client, PROCESS_EVENT_CONTINUE, NULL);
            } else {
                //TODO Open alarm
                process_start(&er_example_toggle_client, "1;1;");
                process_post_synch(&er_example_toggle_client, PROCESS_EVENT_CONTINUE, NULL);
            }
            //TODO 
            break;
        case OBSERVE_NOT_SUPPORTED:
            printf("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *) payload);
            obs = NULL;
            break;
        case ERROR_RESPONSE_CODE:
            printf("ERROR_RESPONSE_CODE: %*s\n", len, (char *) payload);
            obs = NULL;
            break;
        case NO_REPLY_FROM_SERVER:
            printf("NO_REPLY_FROM_SERVER: "
                    "removing observe registration with token %x%x\n",
                    obs->token[0], obs->token[1]);
            obs = NULL;
            break;
    }
}
/*----------------------------------------------------------------------------*/

/*
 * Toggle the observation of the remote resource
 */
void
set_observation(uint8_t enable) {
    if (serv1state.obs && !enable) {
        printf("Stopping observation\n");
        coap_obs_remove_observee(serv1state.obs);
        serv1state.obs = NULL;
    } else if (!serv1state.obs && enable) {
        printf("Starting observation\n");
        serv1state.obs = coap_obs_request_registration(server_ipaddr, REMOTE_PORT,
                OBS_RESOURCE_URI, notification_callback, NULL);
    }
}

/**/
void light_response_handler(void *response) {
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    serv1state.light = atoi(strtok((char *) chunk, ";"));
#if DEBUG
    printf("LIGHT VALUE: %u\n", serv1state.light);
#endif

}

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










//=========================================START TOGGLE PROCESS=========================//
/***************************************************************************************
 ******************************************************************************************/




/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 2

/* Service URLs and parameters */
char *service_urls[NUMBER_OF_URLS] = {"actuators/light_bulb", "actuators/alarm"};
char *service_parameters[NUMBER_OF_URLS] = {"?light_value=", "?alarm_value="};

/*Not used yet*/
void service_handler(void *response) {
    //    const uint8_t *chunk;
    //    int len = coap_get_payload(response, &chunk);
    //#if DEBUG
    //    printf("%s\n", len, (char *) chunk);
    //#endif    
}

/*Toggle process thread*/
PROCESS_THREAD(er_example_toggle_client, ev, data) {
    PROCESS_BEGIN();
    const uint8_t *parameters;
    uint8_t serviceIndex = atoi(strtok((char *) data, ";"));
    char *value = strtok(NULL, ";");
    static coap_packet_t request[1];
    char url[100];

    /* receives all CoAP messages */
    coap_init_engine();
    /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
    strcpy(url, service_parameters[serviceIndex]);
    strcat(url, value);
#ifdef DEBUG
    printf("Toggle %s", service_urls[serviceIndex]);
    printf(" with parameters: %s\n", url);
#endif
    PROCESS_END();
}







