A Quick Introduction to the Erbium (Er) REST Engine
===================================================

EXAMPLE FILES
-------------

- room-server.c: A RESTful server that contains several resources such as sht11,motion periodic resource,alarm & light resource
- room-controller.c: A simple controller that polls the server every some seconds. More specific this controller asks frequently room's temperature,light sensor,motion sensor and he decides the next actions. For example if temperature is too low, controller decides to enable thermostat actuator.

PRELIMINARIES
-------------

- Make sure rpl-border-router has the same stack and fits into mote memory:
  You can disable RDC in border-router project-conf.h (not really required as BR keeps radio turned on).
    #undef NETSTACK_CONF_RDC
    #define NETSTACK_CONF_RDC     nullrdc_driver
- Alternatively, you can use the native-border-router together with the slip-radio.
- For convenience, define the Cooja addresses in /etc/hosts
      aaaa::0212:7401:0001:0101 cooja1
      aaaa::0212:7402:0002:0202 cooja2
      ...
- Get the Copper (Cu) CoAP user-agent from
  [https://addons.mozilla.org/en-US/firefox/addon/copper-270430](https://addons.mozilla.org/en-US/firefox/addon/copper-270430)
- Optional: Save your target as default target
      make TARGET=sky savetarget

COOJA HOWTO
-----------

###Run Room simulation

    make TARGET=cooja smart-room.csc

Open new terminal

    make connect-router-cooja

- Wait until red LED toggles on mote 2 (server)
- Choose "Click button on Sky 3" from the context menu of mote 3 (client) and
  watch serial output
