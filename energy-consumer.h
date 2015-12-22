/* 
 * File:   energy-consumer.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on December 12, 2015, 9:18 PM
 */

#ifndef ENERGY_CONSUMER_H
#define	ENERGY_CONSUMER_H

typedef struct {
    int energy_units_spent = 0;
    int last_energy_units_spent = 0;
    int32_t energy_transmit_multiplier = 1;
    int32_t energy_receive_multiplier = 1;
    
} energy_consumer;

#endif	/* ENERGY_CONSUMER_H */

