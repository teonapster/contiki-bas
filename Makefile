all: er-example-server er-example-client-temperature er-example-client-toggle er-example-observe-motion
# use target "er-plugtest-server" explicitly when requried 
ifndef TARGET
TARGET=z1
endif
    
CONTIKI=../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -I$(CONTIKI)/net/ip/uip.h
ZOLERTIA_Z1SP=0
# automatically build RESTful resources
REST_RESOURCES_DIR = ./resources
ifndef TARGET
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
ifeq ($(TARGET), native)
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c'))
else
REST_RESOURCES_FILES = $(notdir $(shell find $(REST_RESOURCES_DIR) -name '*.c' ! -name 'res-plugtest*'))
endif
endif

#ifdef ($(SRV_IP))
CFLAGS += -DSERVER_IP=\"$(SERVER_IP)\"
CFLAGS += -DSERVER_ID=\"$(SERVER_ID)\"
CFLAGS += -DROOM_ID=\"$(ROOM_ID)\"
#ifdef ($(BUILD_IP))
CFLAGS += -DBUILDING_ID=\"$(BUILDING_ID)\"

#CFLAGS += -DMACID=$(THIS_ID)

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

#compile all client scripts
ifeq ($(CLIENT),YES)
#CLIENT_FILES = $(notdir $(shell find $(CLIENT_DIR) -name '*client*.c'))

CLIENT_FILES = ./er-example-observe-motion.c
PROJECT_SOURCEFILES += $(CLIENT_FILES)
endif

# linker optimizations
SMALL=1

# REST Engine shall use Erbium CoAP implementation
APPDIRS += $(CONTIKI)/warehouse/er-rest-git/apps
APPS += er-coap
APPS += rest-engine
APPS += coap-rest
APPS += powertrace

#Memory optimization
CFLAGS += -ffunction-sections
LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

# optional rules to get assembly
#CUSTOM_RULE_C_TO_OBJECTDIR_O = 1
#CUSTOM_RULE_S_TO_OBJECTDIR_O = 1

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
include $(CONTIKI)/tools/powertrace/Makefile.powertrace

# minimal-net target is currently broken in Contiki
ifeq ($(TARGET), minimal-net)
CFLAGS += -DHARD_CODED_ADDRESS=\"fdfd::10\"
${info INFO: er-example compiling with large buffers}
CFLAGS += -DUIP_CONF_BUFFER_SIZE=1300
CFLAGS += -DREST_MAX_CHUNK_SIZE=1024
CFLAGS += -DCOAP_MAX_HEADER_SIZE=176
CONTIKI_WITH_RPL=0
endif

# optional rules to get assembly
#$(OBJECTDIR)/%.o: asmdir/%.S
#	$(CC) $(CFLAGS) -MMD -c $< -o $@
#	@$(FINALIZE_DEPENDENCY)
#
#asmdir/%.S: %.c
#	$(CC) $(CFLAGS) -MMD -S $< -o $@

# border router rules
$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 -p 6000$(MOTE_ID) aaaa::$(MOTE_ID)/64

connect-router-cooja-z1:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -B 38400 -s /dev/ttyUSB0 -a 127.0.0.1 -p 60001 aaaa::1/64

connect-router-native:	$(CONTIKI)/examples/ipv6/native-border-router/border-router.native
	sudo $(CONTIKI)/exmples/ipv6/native-border-router/border-router.native -a 127.0.0.1 -p 60001 aaaa::1/64

connect-minimal:
	sudo ip address add fdfd::1/64 dev tap0 
