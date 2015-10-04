/* 
 * File:   res-motion.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on September 28, 2015, 6:31 PM
 */
#include "contiki.h"

#if PLATFORM_HAS_MOTION
#include <stdio.h>

#include "rest-engine.h"
#include "platform/z1/dev/z1-phidgets.h"
#include "core/lib/sensors.h"

#if 1
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif

#define RELAY_INTERVAL (CLOCK_SECOND)

static struct etimer et;
static int8_t status;



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
/* Get Method Example. Returns the reading from temperature and humidity sensors. */
RESOURCE(res_motion,
         "title=\"Motion Endpoint\";rt=\"Motion\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);


static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

  int value = 0;
  
  //TODO set phidget index as parameter
  value = phidgets.value(PHIDGET5V_1);
//  printf("Phidget 5v 1:%d\n", phidgets.value(PHIDGET5V_1));
  printf("CURRENT VALUE: %u",value);

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%u;", value);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_XML) {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "<Motion_Value=\"%u\"/>", value);
    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'Motion':{'Value':%u}}", value);
    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
}

#endif /* PLATFORM_HAS_MOTION */

