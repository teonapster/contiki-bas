/* 
 * File:   time-service-client.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on October 26, 2015, 9:42 AM
 */

#ifndef TIME_SERVICE_CLIENT_H
#define	TIME_SERVICE_CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "contiki.h"
extern char  *current_time;
PROCESS(er_example_temperature_client, "Temperature process Client");
PROCESS(time_service_client, "Time process ClienT");
#ifdef	__cplusplus
}
#endif

#endif	/* TIME_SERVICE_CLIENT_H */

