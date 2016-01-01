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
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"
#include "node-state.h"
#include "ipv6parser.h"
#include "powertrace.h"
//#include "apps/er-coap/er-coap-energy-consumer.h"


/*----------------------------------------------------------------------------*/
#define DEBUG 1
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
#define SERVER_NODE(ipaddr,id)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0xc30c, \
                                          0x0000, 0x0000, id)
#define BUILDING_SERVER_NODE(ipaddr1,id)   uip_ip6addr(ipaddr1, 0xfe80, 0, 0, 0, 0xc30c, \
                                          0x0000, 0x0000, id)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)
#define LOCAL_PORT 61617
/* Toggle interval in seconds */
#define TOGGLE_INTERVAL 30
/* Motion sensor observer resource */
#define OBS_RESOURCE_URI "periodic/motion"

#define MOTION_URI "sensors/motion"
/* Light actuator resource*/
#define LIGHT_ACTUATOR_RESOURCE_URI "actuators/light_bulb"

/* Energy consumption resource*/
#define ENERGY_RESOURCE_URI "energy/logger"

#define MAX_PAYLOAD_LEN 100
/* Light resource */
#define LIGHT_RESOURCE_URI "sensors/light"
#define DAY_LIGHT_LIMIT 175

#define COAP_DATA_BUFF_SIZE 1024
/*----------------------------------------------------------------------------*/


static motion_state serv1state;
static uint8_t askForLight = 0;
uint8_t alarm = 0;


#define ASK_TEMPERATURE_EVERY 5
#define TOTAL_NODES 1

static struct etimer et, dailyTimer, workTimer, tempInterval, energy_timer;
static temperature_state node_state;
int atWork = 0;

#define NUMBER_OF_URLS 3
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_temperature_urls[NUMBER_OF_URLS] = {".well-known/core", "sensors/sht11", "error/in//path"};

#if PLATFORM_HAS_BUTTON
static int uri_switch = 1;
#endif

static uip_ipaddr_t server_ipaddr;
static uip_ipaddr_t building_addr;

/*energy sync Event*/
static process_event_t energy_event, start_motion_observe, stop_motion_observe;
/*----------------------------------------------------------------------------*/
PROCESS(temperature_poll, "Erbium Coap Observe Motion Resource");
PROCESS(toggle_process, "Erbium Coap Toggle Resource");
AUTOSTART_PROCESSES(&temperature_poll, &toggle_process);
/*----------------------------------------------------------------------------*/



void alarm_handle(){
    printf("New motion: %d\n",serv1state.motion);
    if (atWork) {
        // Open lights
        printf("TOGGLE LIGHTS");
        process_post(&toggle_process, PROCESS_EVENT_CONTINUE, "0;1;");
        //Close alarm
        process_post(&toggle_process, PROCESS_EVENT_CONTINUE, "1;0;");
    } else {
        //Open alarm
        process_post(&toggle_process, PROCESS_EVENT_CONTINUE, "1;1;");
    }
}

/*----------------------------------------------------------------------------*/

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
            alarm_handle();
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

/**/
void light_response_handler(void *response) {
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    serv1state.light = atoi(strtok((char *) chunk, ";"));
#if DEBUG
    printf("LIGHT VALUE: %u\n", serv1state.light);
#endif

}


void manual_motion_handler(void *response) {
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    serv1state.motion = atoi((char *) chunk);
    
    alarm_handle();
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

void
energy_handler(void *response) {
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
#if DEBUG
    printf("Energy consumption logged successfully: %s\n", len, (char *) chunk);
#endif    
}

void toggle_observation(int ev) {
    if (ev == 0) {
        printf("Stopping observation\n");
        coap_obs_remove_observee(serv1state.obs);
        serv1state.obs = NULL;
    } else if (!serv1state.obs && ev == 1) {
        printf("Starting observation with");
        serv1state.obs = coap_obs_request_registration(&server_ipaddr, REMOTE_PORT,
                OBS_RESOURCE_URI, notification_callback, NULL);
    }
}

/*
 * The main (proto-)thread. It starts/stops the observation of the remote
 * resource every time the timer elapses or the button (if available) is
 * pressed
 */
PROCESS_THREAD(temperature_poll, ev, data) {
    PROCESS_BEGIN();
    static coap_packet_t request[1], putRequest[1]; /* This way the packet can be treated as pointer as usual. */
    printf("Server id: %d, Building id: %d, Room id: %d\n", atoi(SERVER_ID), atoi(BUILDING_ID), atoi(ROOM_ID));
    /* store server address in server_ipaddr */
    SERVER_NODE(&server_ipaddr, atoi(SERVER_ID));
    //    BUILDING_SERVER_NODE(&building_addr,atoi(ROOM_ID));
#ifdef ENERGY_ANALYSIS
    powertrace_start(CLOCK_SECOND * 100);
#endif
    /* receives all CoAP messages */
    coap_init_engine();

    /* init timer and button (if available) */
    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
    etimer_set(&tempInterval, ASK_TEMPERATURE_EVERY * CLOCK_SECOND);
    etimer_set(&dailyTimer, 24 * 10 * CLOCK_SECOND);
    etimer_set(&energy_timer, 5 * CLOCK_SECOND);


    energy_event = process_alloc_event();
    start_motion_observe = process_alloc_event();
    stop_motion_observe = process_alloc_event();

#if PLATFORM_HAS_BUTTON
    SENSORS_ACTIVATE(button_sensor);
    printf("Press a button to start/stop observation of remote resource\n");
#endif

    /* toggle observation every time the timer elapses or the button is pressed */
    while (1) {
        PROCESS_YIELD();
#ifdef ENERGY_ANALYSIS
        if (etimer_expired(&energy_timer)) {

            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
            coap_set_header_uri_path(request, MOTION_URI);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    manual_motion_handler);
            etimer_reset(&energy_timer);
        }
#endif
        if (etimer_expired(&et)) {
            //ASK FOR LIGHT VALUE -- TODO can we make this async?
            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
            coap_set_header_uri_path(request, LIGHT_RESOURCE_URI);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    light_response_handler);
            if (serv1state.light < DAY_LIGHT_LIMIT) {

#ifndef ENERGY_ANALYSIS
                toggle_observation(1);
                printf("Enable motion observation\n");
#endif
            } else { // natural light

#ifndef ENERGY_ANALYSIS
                toggle_observation(0);
                printf("Disable motion observation\n");
#endif
                process_post(&toggle_process, PROCESS_EVENT_CONTINUE, "0;0;");
            }

            etimer_reset(&et);
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
            //
            //            PRINT6ADDR(&server_ipaddr);
            //            PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    client_chunk_temperature_handler);
            printf("Temperature got");
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
            //            printf("Energy units spent: %u \n", ec.energy_units_spent);
            etimer_reset(&tempInterval);
        }

        //        if (etimer_expired(&energy_timer)) {
        //            const char energy_params [100];
        ////            const char * energy_params = "?address=1&units=10";
        //            const char unit_num[5];
        //            const char str_addr[5];
        //            int int_addr = atoi(ROOM_ID);
        //            sprintf(unit_num,"%d",ec.energy_units_spent);
        //            sprintf(str_addr,"%d",int_addr);
        //            
        //            strcpy(energy_params,"?address=");
        //            strcat(energy_params,str_addr);
        //            strcat(energy_params,"&units=");
        //            strcat(energy_params,unit_num);
        //            static coap_packet_t request [1];
        //#ifdef DEBUG
        //            printf("Sent energy units to building controller with params: %s and address: %s \n",energy_params,str_addr );
        //#endif
        //            coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
        //            coap_set_header_uri_path(request, ENERGY_RESOURCE_URI);
        //            coap_set_header_uri_query(request, energy_params);
        //            COAP_BLOCKING_REQUEST(&building_addr, REMOTE_PORT, request,
        //                    energy_handler);
        //            printf("Energy units spent: %u \n", ec.energy_units_spent);
        //            etimer_reset(&energy_timer);
        //        }
    }
    PROCESS_END();
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
void toggle_handler(void *response) {
#ifdef DEBUG
    printf("TOGGLE WORKS!!!\n");
#endif

}


/*Toggle process thread*/
PROCESS_THREAD(toggle_process, ev, data) {
    PROCESS_BEGIN();
    const uint8_t *parameters;
    coap_init_engine();

    while (1) {
        PROCESS_WAIT_EVENT();

        if (ev == PROCESS_EVENT_CONTINUE) {
            uint8_t serviceIndex = atoi(strtok((char *) data, ";"));
            char *value = strtok(NULL, ";");
            static coap_packet_t request[1];
            char url[100];

            /* receives all CoAP messages */

            /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
            strcpy(url, service_parameters[serviceIndex]);
            strcat(url, value);
            PRINTF("Actuator value change!!!!");
#ifdef DEBUG
            printf("Event data: %s, Selected index: %d\n", (char *) data, serviceIndex);
            printf("Toggle %s", service_urls[serviceIndex]);
            printf(" with parameters: %s\n", url);
#endif
            coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
            coap_set_header_uri_path(request, service_urls[serviceIndex]);

            coap_set_header_uri_query(request, url);
            COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                    toggle_handler);
        }
        //        set_observation(0);
    }
    PROCESS_END();
}
