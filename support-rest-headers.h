/* 
 * File:   support-rest-headers.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on September 29, 2015, 5:40 PM
 */

#ifndef SUPPORT_REST_HEADERS_H
#define	SUPPORT_REST_HEADERS_H
#include "rest-engine.h"

#ifdef	__cplusplus
extern "C" {
#endif

 static void res_post_handler(unsigned int accept, void *request, void *response, 
         uint8_t *buffer){
     
 if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%u;", alarmStatus);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_XML) {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "<Alarm_State =\"%u\"/>", alarmStatus);
    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'Alarm':{'State':%u}}", alarmStatus);
    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
 }


#ifdef	__cplusplus
}
#endif

#endif	/* SUPPORT_REST_HEADERS_H */

