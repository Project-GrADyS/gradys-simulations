/*
 * MAVLinkInstruction.h
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlamz
 */

#ifndef PROJETO_MOBILITY_BASE_MAVLINKINSTRUCTION_H_
#define PROJETO_MOBILITY_BASE_MAVLINKINSTRUCTION_H_

#include <omnetpp.h>
#include <functional>
#include "mavlink/ardupilotmega/mavlink.h"

namespace projeto {

typedef std::function<bool(mavlink_message_t)> Condition;

//
// The MAVLinkMobility modules are responsible for sending messages to SITL simulator instances. These messages are stored in a queue
// before being sent to the simulation. To support more complex behavior like waiting for a condition to complete before moving on to
// the next message or trying to send a message several times to account for communication failures a more complex structure is nece-
// ssary. This is such structure.
struct Instruction {
    Instruction(mavlink_message_t message, Condition condition, omnetpp::simtime_t timeout, int retries, std::string label = "",bool completed=false) : message(message), condition(condition), timeout(timeout), retries(retries), label(label), completed(completed) {};
    // Message to send to the vehicle
    mavlink_message_t message = {};
    // This is a function that will be called with every telemetry message received while this instruction is active. The next
    // instruction in the queue will only be called when this function returns true. This allows you to implement conditions
    // that should be satisfied before considering this instruction as completed.
    // Ex: A takeoff message is only completed when the drone reaches a desired height
    // Useful and ready to use conditions can be found in the TelemetryConditions file.
    Condition condition;

    // The mobility module will wait this amount of time before trying to send the instruction again or failing to send it, in case
    // no retries are left. A timeout of -1 means it will wait for an infinite amount of time.
    omnetpp::simtime_t timeout;
    // This message will be resent this amount of times before the next instruction in the queue is called.
    // A retry happens when a condition is not satisfied and a timeout is reached or when a network error
    // occurs while the message is being sent.
    int retries;
    // Label of the instruction. This is only useful for debugging purposes. This label will be printed in log messages.
    std::string label = "";
    // Whether the instruction has already been completed. It is used to control when the next instruction in the queue
    // should be called. It should be set to false when the instruction is created and the mobility module is responsible
    // for setting it to true when the instruction is completed. The MAVLinkMobilityBase module already does this when
    // the condition is fulfilled.
    bool completed = false;
};


}


#endif /* PROJETO_MOBILITY_BASE_MAVLINKINSTRUCTION_H_ */
