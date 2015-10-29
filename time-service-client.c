/* 
 * File:   res-motion.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on September 28, 2015, 6:31 PM
 */
#include "time-service-client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"



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
#define SERVER_TIME_NODE(ipaddr) uip_ip4addr(ipaddr, 88,198,40,75)

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define TOGGLE_INTERVAL 5


//AUTOSTART_PROCESSES(&time_service_client);

uip_ipaddr_t server_time_ipaddr;
static struct etimer et;

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 1
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_url="http://api.geonames.org/timezoneJSON?formatted=true&lat=40.38&lng=22.56&username=demo&style=full";
#if PLATFORM_HAS_BUTTON
static int uri_switch = 1;
#endif

void
    time_handler(void * response){
        const uint8_t *chunk;

        int len = coap_get_payload(response, &chunk);
        current_time = (char *) chunk;
         printf("|%.*s", len, (char *)chunk);
}

    

PROCESS_THREAD(time_service_client, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  SERVER_TIME_NODE(&server_time_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);


  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&et)) {
      printf("--Toggle timer--\n");

      /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
      coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
      coap_set_header_uri_path(request, service_url);

      const char msg[] = "Get current time!";

      coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);

      PRINT6ADDR(&server_time_ipaddr);
      PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

      COAP_BLOCKING_REQUEST(&server_time_ipaddr, REMOTE_PORT, request,
                            time_handler);

      printf("\n--Done--\n");

      etimer_reset(&et);
    }
  }

  PROCESS_END();
}

