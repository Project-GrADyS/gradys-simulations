# Gradys Simulations
Simulations from Project GrADyS on OMNET++ and INET framework


# Installation
In order to run the simulations and use the components in this repository you need to have both OMNeT++ and the INET framework installed.

Version 5.6 of OMNeT++ is required, to install it [just follow these instructions](https://doc.omnetpp.org/omnetpp/InstallGuide.pdf). INET version 4.2 is also required, when first opening the OMNeT++ IDE you should be prompted with the option to install INET but if you need help [check out the installation instructions](https://inet.omnetpp.org/Installation.html).

After installing both OMNeT++ and INET you should be able to clone the repository to youw active OMNeT++ IDE workspace. To do this select File > Impor... then open the "git" section and select "Projects from git" then "Clone Uri". After that just fill in the URL for this repository and finish the process following the displayed instructions.

# Usage
OMNeT++ simulations are initialized by *.ini* files. The already provided **mobilityDrones-omnetpp.ini** file contains some launch configurations with 2, 3 and 4 drones following a predetermined course. 

To run a simulation simply select one of the *.ini* files and use the OMNeT++ IDE Run option. After the GUI opens select the desired launch configuration and press play on the simulation.

# Project Structure
INET offers a series of modules that control node mobility. Our objective was to create a module that was capable of simulating a very simple drone mobility model and could react to network events. This setup allows support for a wide array of possible drone coordination protocols.

The described requirement was achieved with three modules, one resposible for communication between drones (communication), one for controlling the node's movement (mobility) and the last to manage the interaction between the last two (protocol). They were made in such a way that the messages exchanged between them were generic enough that in the creation of a new protocol only the protocol module would need to be changed, levaraging these generic messages to carry out different procedures.

 ## Mobility
 The mobility module is responsible for controlling drone movement and responding to requests from the protocol module to change that movement through MobilityCommand messages. It also needs to inform the procol module about the current state of the drone's movement through Telemetry messages. 
 
 As part of the module initialization the waypoint list is attached to a Telemetry message so the protocol module has access to the tour the mobile node is following.
 
 These are the messages used:

* **MobilityCommand.msg**
```C++
// Commands that the mobility module should be capable of carrying out
enum MobilityCommandType {
    // Makes the drone reverse on its course
    // No params
    REVERSE=0; 
    
    // Makes the drone travel to a specific waypoint, following the tour pack
    // Param 1: Waypoint index
    GOTO_WAYPOINT=1;
    
    // Makes the drone go to a specific coordinate and orient itself so it can continue the tour afterwards
    // Param 1: x component of the coord
    // Param 2: y component of the coord
    // Param 3: z component of the coord
    // Param 4: Next waypoint (Waypoint the drone should go to after reaching the target)
    // Param 5: Last waypoint (Waypoint the drone used to reach the coords)
    GOTO_COORDS=2;
}

// Message declaration containing the command Id and its parameters 
message MobilityCommand {
    MobilityCommandType commandType;
    double param1=-1;
    double param2=-1;
    double param3=-1;
    double param4=-1;
    double param5=-1;
}
```
* **Telemetry.msg**
```C++
// Activity that the drone is currently carrying out
enum DroneActivity { 
    IDLE=0; 
    NAVIGATING=1;
    REACHED_EDGE=2; 
    FOLLOWING_COMMAND=3;
}

// Message declaration designed to share necessary drone information with the communication module
message Telemetry {
    int nextWaypointID=-1;
    int lastWaypointID=-1;
    int currentCommand=-1;
    bool isReversed=false;
    DroneActivity droneActivity;
}
```

The only mobility module currently implemented is DroneMobility which simulates the movement of a drone. 
 ## Communication
 INET provides built in support for the simulation of real communications protocols and the communication module takes advantage of this to simulate communication between nodes. It also has to inform the protocol module of the messages being recieved by sharing the messages themselves and listen to orders from the protocol module through CommunicationCommands. Here are the messages used:
 
 * **CommunicationCommand.msg**
 ```C++
enum CommunicationCommandType {
    // Sets the payload that the communication module sends
    SET_PAYLOAD=0;
    // Sets the target of the communication message (null means broadcast)
    SET_TARGET=1;
}

// Message declaration for the communication command
message CommunicationCommand {
    CommunicationCommandType commandType;
    
    // Template for the SET_PAYLOAD message type (message that the communication module should send)
    inet::FieldsChunk *payloadTemplate;
    
    // Target for the set target command
    string target;
}
 ```

 The message module has several implementations. The *base* folder contains several base implementations for possible communication modules, these files contain functions that interface with INET's communication capabilities but don't implement interaction with any other module.
 
 These files were used as base for the implementation of the communication modules. The following files are the implementations used in the simulations:
  * **UdpMobileNodeCommunicationApp.ned**
  
    Manages the communication between mobile nodes and between mobile nodes and sensors.
 * **UdpSensorCommunicationApp.ned**
    
    Manages communication between sensors and mobile nodes.
 ## Protocol 
The protocol module manages the interaction between the movement and communication of the mobile nodes. It makes use of the messages provided by it's two sibling modules to create node interaction strategies. It mostly reacts to messages it recieves from those modules and determines which orders to give them to achieve the desired result.

Protocools implement an IProtocol interface and extend  **CommunicationProtocolBas.nete** which provides useful stub functions to use when implementing protocols. These functions are as follows:
```C++
// Redirects message to the proper function
virtual void handleMessage(cMessage *msg);

// Handles package received from communication
// This packet is a message that was sent to the drone
virtual void handlePacket(Packet *pk) {};

// Handles telemetry received from mobility
// The mobility module exchanges mobility information in the form of telemetry
virtual void handleTelemetry(Telemetry *telemetry) {};

// Sends command to mobility
virtual void sendCommand(MobilityCommand *order);
// Sends command to communication
virtual void sendCommand(CommunicationCommand *order);

// Sets a timeout
virtual void initiateTimeout(simtime_t duration);
// Checks if the module is timed out
virtual bool isTimedout();
```

These are the currently implemented protocols:

* **ZigZagProtocol.ned** and **ZigZagProtocolSensor.ned**

     These files implement the mobile node and the sensor side of the ZigZag protocol. This prococol manages a group of drones folowwing a set path passing above several sensors from where they pick up imaginary data from those sensors. The drones also interact with each other sending several messages to coordinate their movement.

     Heartbeat messages are sent on a multicast address, if these are picked up by sensors they respond with data. If they are picked up by other drones they initiate a communication pair by sending a Pair Request message which is them confirmed by the other drone with a Pair Confirmation message. The drone furthest away from the starting point of the path sends its data to the other drone in the pair and they both reverse their movement. The objective is that over time the drones will each occupy an equally sized section of the course, picking up data on the way and sharing it at their section's extremities.

* **DadcaProtocol.ned** and **DadcaProtocolSensor.ned**
     
     This protocol is similar to the ZigZagProtocol. It also manages data collection by mobile nodes in a set path. The difference is that this method aims to speed up the process of equally spacing the drones in the course by implementing a more advanced movement protocol.

     When the Pair Confirmation message is recieved by both drones, confirming the pair, both drones take note of the number of neighours on their left (closer to the start) and their right (further from the start) and share this information with their pair. Both update their neighbour count and use it to calculate a point in the course that would represent the extremity of both their sections if their current count of neighbours is accurate. Them they both travel together to this point and revert. This behaviour is implemented with a sequence of commands that get queued on the mobility module.

# Development
To develop new protocols, you will probably be creating new protocol modules that use the current message definitions and commands to implement new behaviour and management and data collection strategies. If the current set of commands and messages is not enough you are free to add more by modifying the message definitions and the modules so that they can properly react to these new messages.

After creating a new module all you need to do to test it is modifying the desired *.ini* configuration to load your protocol. The protocol module is flexible and can be loaded with any implemented protocol by changing it's typename, for example:
```C++
*.quads[*].protocol.typename = "DadcaProtocol"
```

You can find additinal information about this developing enviroment on the [OMNeT++](https://omnetpp.org/documentation/) and [INET](https://inet.omnetpp.org/docs/) documentations. Sometimes some classes or functions are not that well documented, in that case looking at the samples and tutorials included with INET and OMNeT++ can be a useful resource and if even that doesn't help you can easily look at INET's source code as it should be included in your workspace if you installed it correctly.