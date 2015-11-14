/* 
 * File:   node-state.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on November 14, 2015, 7:24 PM
 */

#ifndef NODE_STATE_H
#define	NODE_STATE_H
#ifdef	__cplusplus
extern "C" {
#endif
  typedef  struct  {
        uint8_t id,cold,hot;
    } temperature_state;



#ifdef	__cplusplus
}
#endif

#endif	/* NODE_STATE_H */

