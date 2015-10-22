/* 
 * File:   res-motion.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on September 28, 2015, 6:31 PM
 */


#include "contiki.h"

#if PLATFORM_HAS_MOTION

#include <string.h>
#include "rest-engine.h"
#include "platform/z1/dev/z1-phidgets.h"
#include "core/lib/sensors.h"

#define MAX_TEMPERATURE 30
#define MIN_TEMPERATURE 10


static uint8_t thermostat_T=15;
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* Get Method Example. Returns the reading from temperature and humidity sensors. */
RESOURCE(res_motion,
        "title=\"Temperature and Humidity\";rt=\"Sht11\"",
        res_get_handler,
        NULL,
        NULL,
        NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    /* Temperature in Celsius (t in 14 bits resolution at 3 Volts)
     * T = -39.60 + 0.01*t
     */
   uint16_t  status = phidgets.value(PHIDGET5V_1);

    unsigned int accept = -1;
    REST.get_header_accept(request, &accept);

    if (accept == -1 || accept == REST.type.TEXT_PLAIN) {
        REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "%u;", status);

        REST.set_response_payload(response, (uint8_t *) buffer, strlen((char *) buffer));
    } else if (accept == REST.type.APPLICATION_XML) {
        REST.set_header_content_type(response, REST.type.APPLICATION_XML);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "<Temperature =\"%u\" />", status);

        REST.set_response_payload(response, buffer, strlen((char *) buffer));
    } else if (accept == REST.type.APPLICATION_JSON) {
        REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "{'Sht11':{'Temperature':%u}}", status);

        REST.set_response_payload(response, buffer, strlen((char *) buffer));
    } else {
        REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
        const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
        REST.set_response_payload(response, msg, strlen(msg));
    }
}
#endif /* PLATFORM_HAS_SHT11 */
