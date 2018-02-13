// debug level
// 0 = none, 1 = more, 2 = much more, 3 = even much more, 4 = disk filler
#define DCS_DIM_DEBUG 1

// default DNS node and port
#define DCS_DIM_DNS_NODE_DEFAULT "10.0.0.3"
#define DCS_DIM_DNS_PORT_DEFAULT 2505

// dimchannel usage
#define DCS_DIM_SRU_NUM_DEFAULT 0

#define DCS_DIM_ROOT_DEFAULT "ALICE/EMC/FEE"
#define DCS_DIM_SRU "SRU"
#define DCS_DIM_FEE "FEE"
#define DCS_DIM_TRU "TRU"
#define DCS_DIM_COMMAND "CMND"
#define DCS_DIM_SETTING "SET"
#define DCS_DIM_RESULT "RES"
#define DCS_DIM_REQUEST "REQ"

#define DCS_DIM_NOAUTO 0
#define DCS_DIM_IMMEDIATE 1
#define DCS_DIM_FORCE 1

#define DCS_DIM_PHYSICS_RUN 1
#define DCS_DIM_LED_RUN 2
#define DCS_DIM_PEDESTAL_RUN 3
#define DCS_DIM_STANDALONE_RUN 4

#define DCS_DIM_ALTRO "ALTRO"
#define DCS_DIM_BIAS "BIAS"
#define DCS_DIM_DETAIL "CONFIG"

#define DCS_DIM_STATE "STATE"
#define DCS_DIM_ALIVE "ALIVE"

#define DCS_DIM_SPECIAL_TYPE 0
#define DCS_DIM_SRU_TYPE 1
#define DCS_DIM_FEE_TYPE 2
#define DCS_DIM_TRU_TYPE 3
#define DCS_DIM_ALTRO_TYPE 4
#define DCS_DIM_INTERPRETED_TYPE 4

#define DCS_DIM_QUEUE_COMMAND 1
#define DCS_DIM_QUEUE_READBACK 2
#define DCS_DIM_QUEUE_CONFIG 3

#define DCS_DIM_SRU_DTC_NUM 40
#define DCS_DIM_UDP_BUF_MAX 2048

//#define DCS_DIM_FEEL_MASK 0xffbfe
//#define DCS_DIM_FEEH_MASK 0xffbfe

// values for SRU communication controll
// number of tries to read register, if no reply
#define DCS_DIM_READBACK_RETRY 3
// single readout buffer workaround, set 1 to activate 0 otherwise
#define DCS_DIM_BUFFER_SINGLE 1

// values for queue controll ----------------------------

// period in s for ping test
#define DCS_DIM_PING_PERIOD 10
// period in s for readback command construction
#define DCS_DIM_RB_PERIOD 20
// max queue size, if exceeded, no new readback commands are build
#define DCS_DIM_SEQUENCE_SIZE_LIMIT 10000
// max amount of commands to send before single cycle is exited
#define DCS_DIM_SEQUENCE_COMMAND_LIMIT 500
// max amount of read commands to send before readback buffers are read out
#define DCS_DIM_SEQUENCE_READ_LIMIT 8



#define DCS_DIM_SEQUENCE_READBACK_LIMIT 100
#define DCS_DIM_SEQUENCE_CONFIG_LIMIT 100

