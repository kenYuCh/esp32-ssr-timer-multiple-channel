#include "sensor_model.h"
#include "setpoint.h"


const char *conv_state_to_string(MeasurementStates_t state) {
    if(state == SM_NORMAL_STATE)              return "NORMAL_STATE";
    else if(state == SM_HIGH_SWITCHING_STATE) return "H.SW_STATE";
    else if(state == SM_LOW_SWITCHING_STATE)  return "L.SW_STATE";
    else if(state == SM_HIGH_ALARM_STATE)     return "H.ALARM_STATE";
    else if(state == SM_LOW_ALARM_STATE)      return "L.ALARM_STATE";
    else                                      return "EXCEPTION";
}

const char *conv_event_to_string(MeasurementEvent_t event) {
    if(event == NO_EVENT)                              return "NO_EVENT";
    else if(event == NORMAL_TO_HIGH_SWITCH_EVENT)      return "NORMAL -> H.SWITCH EVENT";
    else if(event == NORMAL_TO_LOW_SWITCH_EVENT)       return "NORMAL -> L.SWITCH EVENT";
    else if(event == HIGH_SWITCH_TO_HIGH_ALARM_EVENT)  return "H.SWITCH -> H.ALARM EVENT";
    else if(event == HIGH_SWITCH_TO_NORMAL_EVENT)      return "H.SWITCH -> NORMAL EVENT";
    else if(event == HIGH_ALARM_TO_NORMAL_EVENT)       return "H.ALARM -> NORMAL EVENT";
    else if(event == LOW_SWITCH_TO_LOW_ALARM_EVENT)    return "L.SWITCH -> L.ALARM EVENT";
    else if(event == LOW_SWITCH_TO_NORMAL_EVENT)       return "L.SWITCH -> NORMAL EVENT";
    else if(event == LOW_SWITCH_TO_LOW_ALARM_EVENT)    return "L.ALARM -> NORMAL EVENT";
    else                                               return "EXCEPTION";
} 