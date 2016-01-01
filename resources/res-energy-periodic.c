
/**
 * \file
 *      res-energy-periodic 
 * \author
 *      Kouroutzidis Theodoros - Nikolaos <kourtheo@csd.auth.gr>
 */

#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
static void res_get_energy_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_energy_handler(void);

PERIODIC_RESOURCE(res_energy_periodic,
                  "title=\"Periodic Energy Resource\";obs",
                  res_get_energy_handler,
                  NULL,
                  NULL,
                  NULL,
                  5 * CLOCK_SECOND,
                  res_periodic_energy_handler);

/*
 * Use local resource state that is accessed by res_get_handler() and altered by res_periodic_handler() or PUT or POST.
 */

static void
res_get_energy_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */
    int motion = 1;
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_max_age(response, res_energy_periodic.periodic->period / CLOCK_SECOND);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%u", motion));

  /* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}
/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_energy_handler()
{
        /* Notify the registered observers if motion is detected. */
//        if(motion==1)
            REST.notify_subscribers(&res_energy_periodic);
}


