#include "er-coap-energy-consumer.h"

/**
     * WARNING: The following functions are not thread safe!!!!!!!!!!!!!
     * Start and finish msgs with your own responsibility
     */

    void new_coap_msg(){
//        ec.energy_units_spent+=1*ec.energy_transmit_multiplier;
        ec.energy_units_spent++;
    }
    
    void msg_start(){
        new_coap_msg();
        start_count = RTIMER_NOW();
    }
    
    void msg_send_end(){
        struct msg_energy_consumer msg_cons = ec.mes[ec.energy_units_spent];
        msg_cons.msg_id = ec.energy_units_spent;
        msg_cons.tx_time = RTIMER_NOW() - start_count; //Possible bug. In need of long casting?
        
    }
    
    void msg_receive_end(){
        struct msg_energy_consumer msg_cons = ec.mes[ec.energy_units_spent];
        msg_cons.msg_id = ec.energy_units_spent;
        msg_cons.rx_time = RTIMER_NOW() - start_count; //Possible bug. In need of long casting?
        
    }
