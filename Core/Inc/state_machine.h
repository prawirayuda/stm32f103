#include <stdint.h>


typedef void (*dispatcher_fn)(void);
typedef void (*state_handler)(void);

typedef struct tsm_state_machine_hw_reset_s{
    uint8_t state;
    dispatcher_fn dispatcher;
    uint32_t event_flag;
    state_handler *handlers;
}tsm_sm_hw_reset;

