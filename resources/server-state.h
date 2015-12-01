/* 
 * File:   server-state.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on November 16, 2015, 9:21 PM
 */

#ifndef SERVER_STATE_H
#define	SERVER_STATE_H

#ifdef	__cplusplus
extern "C" {
#endif
extern uint8_t lightBulbStatus, // light bulbs status
                motion,         // motion sensor state 0 || 1
                light_solar,    // natural light value
                alarm,          // alarm state 0 || 1
                isDay;


#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_STATE_H */

