/* 
 * File:   node-state.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on November 14, 2015, 7:24 PM
 */

#ifndef NODE_STATE_H
#define	NODE_STATE_H
#include "contiki.h"
#include "contiki-net.h"
    

    
    
    extern int atWork;
    extern uint8_t alarm;

    typedef struct {
        uint8_t id, cold, hot;
    } temperature_state;

    typedef struct {
        coap_observee_t *obs;
        uint8_t light, motion;
    } motion_state;
    
    typedef struct {
        coap_observee_t *obs;
        uip_ipaddr_t addr;
    } room_state;
    
    


#endif	/* NODE_STATE_H */

