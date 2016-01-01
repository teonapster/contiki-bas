/*
 *  \author
 *      Kouroutzidis Theodoros-Nikolaos <kourtheo@csd.auth.gr>
 */

#include "contiki.h"

#if PLATFORM_HAS_SHT11

#include <string.h>
#include "rest-engine.h"
#include "server-state.h"
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
#define TOTAL_ROOMS 4
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static room_energy rooms_consumption[TOTAL_ROOMS];

/* Get Method Example. Returns the reading from temperature and humidity sensors. */
RESOURCE(res_energy_logger,
        "title=\"Energy logger\";rt=\"<EnergyLogger\"",
        res_get_handler,
        res_put_handler,
        res_put_handler,
        NULL);
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    unsigned int accept = -1;
    REST.get_header_accept(request, &accept);
    size_t len = 0;
    char * address = NULL;
    int roomIndex = -1;
    if ((len = REST.get_query_variable(request, "address", &address))) {
        PRINTF("Get energy units for room: %s\n",address);
        roomIndex = atoi(address);
    }
    
    if (accept == -1 || accept == REST.type.TEXT_PLAIN) {
        REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
        snprintf((char *) buffer, REST_MAX_CHUNK_SIZE, "%d;%d",atoi(address), rooms_consumption[roomIndex].energy_units);
        REST.set_response_payload(response, (uint8_t *) buffer, strlen((char *) buffer));
    } 
}

static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

    size_t len = 0;
    char * param = NULL;
    const uint8_t *chunk;
    const char *msg = NULL;
    int roomIndex = -1;
    if ((len = REST.get_query_variable(request, "address", &param))) {
        PRINTF("Change energy units for room: %s\n",param);
        REST.set_response_status(response, REST.status.OK);
        roomIndex = atoi(param);
        PRINTF("INDEX: %d\n",roomIndex);
    }
    len =0;
    if ((len = REST.get_query_variable(request, "units", &param))&& roomIndex>0 && roomIndex<=TOTAL_ROOMS) {
        PRINTF("Set units to: %s\n",param);
        REST.set_response_status(response, REST.status.OK);
        rooms_consumption[roomIndex].energy_units = atoi(param);
        
    }
    else{
     REST.set_response_status(response, REST.status.BAD_REQUEST);
     msg = "Room energy units update failed\n";
    }
    REST.set_response_payload(response, msg, strlen(msg));
}


#endif /* PLATFORM_HAS_SHT11 */
