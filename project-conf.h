/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Erbium (Er) example project configuration.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#ifndef __PROJECT_ERBIUM_CONF_H__
#define __PROJECT_ERBIUM_CONF_H__

/* Custom channel and PAN ID configuration for your project. */
/*
   #undef RF_CHANNEL
   #define RF_CHANNEL                     26

   #undef IEEE802154_CONF_PANID
   #define IEEE802154_CONF_PANID          0xABCD
 */

/* IP buffer size must match all other hops, in particular the border router. */

   #undef UIP_CONF_BUFFER_SIZE
   #define UIP_CONF_BUFFER_SIZE           256

/* Disabling RDC and CSMA for demo purposes. Core updates often
   require more memory. */
/* For projects, optimize memory and enable RDC and CSMA again. */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC              nullrdc_driver

/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     nullmac_driver

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE            80

/* Estimate your header size, especially when using Proxy-Uri. */
///*
   #undef COAP_MAX_HEADER_SIZE
   #define COAP_MAX_HEADER_SIZE           80
// */

/* Multiplies with chunk size, be aware of memory constraints. */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS     8

/* Must be <= open transactions, default is COAP_MAX_OPEN_TRANSACTIONS-1. */
///*
   #undef COAP_MAX_OBSERVERS
   #define COAP_MAX_OBSERVERS             2
// */

/* Filtering .well-known/core per query can be disabled to save space. */
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING     0
#undef COAP_PROXY_OPTION_PROCESSING
#define COAP_PROXY_OPTION_PROCESSING   0

#undef  PROCESS_CONF_NO_PROCESS_NAMES 
#define PROCESS_CONF_NO_PROCESS_NAMES 0

/* Enable client-side support for COAP observe */
#define COAP_OBSERVE_CLIENT 2

/* Disable platform button*/
#undef PLATFORM_HAS_BUTTON
#define PLATFORM_HAS_BUTTON 0

/* Enable alarm in current platform. */
#undef PLATFORM_HAS_ALARM
#define PLATFORM_HAS_ALARM 1

/* Enable light-bulb in current platform. */
#undef PLATFORM_HAS_LB
#define PLATFORM_HAS_LB 1

/* Enable motion in current platform*/
#undef PLATFORM_HAS_MOTION
#define PLATFORM_HAS_MOTION 1

#undef PLATFORM_HAS_SHT11
#define PLATFORM_HAS_SHT11 1

#undef DEBUG DEBUG_NONE

#undef DEBUG
#define DEBUG 1

#undef PLATFORM_HAS_LEDS
#define PLATFORM_HAS_LEDS 1

#undef PLATFORM_HAS_LIGHT
#define PLATFORM_HAS_LIGHT 1

#undef PLATFORM_HAS_ENERGY
#define PLATFORM_HAS_ENERGY 1

#undef ENERGY_CONSUMER_SIZE
#define ENERGY_CONSUMER_SIZE 50 // Number of messages to watch
#endif /* __PROJECT_ERBIUM_CONF_H__ */
