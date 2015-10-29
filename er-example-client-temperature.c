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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"
#include "http-socket.h"
#include "ip64-addr.h"
//#include "./time-service-client.h"


#define DEBUG 0
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
#define SERVER_TIME_NODE(ipaddr) uip_ip6addr(ipaddr, 0,0,0,0,0,0xffff,0x58c6,0x284b)

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define TOGGLE_INTERVAL 5

PROCESS(er_example_temperature_client, "Temperature process Client");
PROCESS(time_service_client, "Time process ClienT");

//PROCESS(time_service_client, "Time process Client");
AUTOSTART_PROCESSES(&er_example_temperature_client);

uip_ipaddr_t server_ipaddr,server_time_ipaddr;
static struct etimer et;
static struct http_socket s;
char  *current_time;
int atWork = 0;
/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 4
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_temperature_urls[NUMBER_OF_URLS] =
{ ".well-known/core", "/sensors/sht11", "battery/", "error/in//path"};

char *service_url="http://api.geonames.org/timezoneJSON?formatted=true&lat=40.38&lng=22.56&username=demo&style=full";

#if PLATFORM_HAS_BUTTON
static int uri_switch = 1;
#endif


/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_temperature_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);
//  char *firstValue = strtok((char *) chunk,";");
//  printf("%s",firstValue);
  /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
  process_start(&time_service_client, NULL);
  printf("|%.*s at (time): %s", len, (char *)chunk,current_time);
 }


    

PROCESS_THREAD(er_example_temperature_client, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  SERVER_NODE(&server_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
  

#if PLATFORM_HAS_BUTTON
  SENSORS_ACTIVATE(button_sensor);
  printf("Press a button to request %s\n", service_temperature_urls[uri_switch]);
#endif

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&et)) {
      printf("--Toggle timer--\n");

      /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_temperature_urls[1]);

      const char msg[] = "Toggle!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&server_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                            client_chunk_temperature_handler);

      printf("\n--Done--\n");

      etimer_reset(&et);

#if PLATFORM_HAS_BUTTON
    } else if(ev == sensors_event && data == &button_sensor) {

      /* send a request to notify the end of the process */

      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_temperature_urls[uri_switch]);

      printf("--Requesting %s--\n", service_temperature_urls[uri_switch]);

      PRINT6ADDR(&server_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                            client_chunk_temperature_handler);

      printf("\n--Done--\n");

      uri_switch = (uri_switch + 1) % NUMBER_OF_URLS;
#endif
    }
  }

  PROCESS_END();
}




//================GET TIME PROCESS ====================================//

void
    time_callback(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{

  if(e == HTTP_SOCKET_DATA) {
    current_time = "success";
    printf("HTTP socket received %d bytes of data\n", datalen);
  }
}

    

PROCESS_THREAD(time_service_client, ev, data)
{
  uip_ip4addr_t ip4addr;
  uip_ip6addr_t ip6addr;

  PROCESS_BEGIN();

  uip_ipaddr(&ip4addr, 8,8,8,8);
  ip64_addr_4to6(&ip4addr, &ip6addr);
  uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);
  http_socket_init(&s);
  http_socket_get(&s, service_url, 0, 0,
                  time_callback, NULL);
  PROCESS_END();
}
