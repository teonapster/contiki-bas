/**
 * \author
 * Kouroutzidis Theodoros - Nikolaos <kourtheo@csd.auth.gr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "ipv6parser.h"

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

extern resource_t res_energy_logger;
rtimer_clock_t start_count, end_count;

PROCESS(building_server, "Erbium Example Server");
AUTOSTART_PROCESSES(&building_server);

PROCESS_THREAD(building_server, ev, data)
{    
    
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  PRINTF("Starting Building Server\n");

#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif
  
  /* Initialize the REST engine. */
  rest_init_engine();
  rest_activate_resource(&res_energy_logger, "energy/logger");
  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }                      
  PROCESS_END();
}
