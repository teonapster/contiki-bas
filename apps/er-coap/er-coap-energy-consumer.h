/* 
 * File:   er-coap-energy-consumer.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on December 28, 2015, 1:17 PM
 */

#ifndef ER_COAP_ENERGY_CONSUMER_H
#define	ER_COAP_ENERGY_CONSUMER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "../../project-conf.h"
#include "sys/rtimer.h"
    struct msg_energy_consumer {
        int rx_time;
        int tx_time;
        int msg_id;
    };

    struct energy_consumer {
        int energy_units_spent;
//        int last_energy_units_spent;
        int energy_transmit_multiplier;
        int energy_receive_multiplier;
        struct msg_energy_consumer mes[ENERGY_CONSUMER_SIZE];
    }ec;
    rtimer_clock_t start_count;
    
    void msg_receive_end();
    void msg_send_end();
    void msg_start();
    void new_coap_msg();

    
    
    

    
    

#ifdef	__cplusplus
}
#endif

#endif	/* ER_COAP_ENERGY_CONSUMER_H */

