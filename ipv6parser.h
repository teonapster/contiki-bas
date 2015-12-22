/*
 * File:   ipv6parser.h
 * Author: Kouroutzidis Theodoros - Nikolaos<kourtheo@csd.auth.gr>
 *
 * Created on December 17, 2015, 7:24 PM
 */

#ifndef IPV6PARSER_H
#define	IPV6PARSER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "core/net/ip/uiplib.h"
#include "core/net/ip/uip.h"
#include "contiki-net.h"
    

#define SET_IP_FROM_STR(addrstr,addr)   uiplib_ip4addrconv(addrstr, addr);
#define SET_THIS_ADDR(addr) uip_ds6_addr_add(addr, 0, ADDR_TENTATIVE);


#ifdef	__cplusplus
}
#endif

#endif	/* IPV6PARSER_H */

