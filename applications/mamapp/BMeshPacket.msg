import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// Packet for ~AckingMac. Packet size is configurable
// in the MAC layer. 
//
class BMeshPacket extends FieldsChunk
{
    // Bluetooth Mesh network: up to 32,767 nodes with a maximum of 126 hops
    // From the Bluetooth Mesh specification:
    // The TTL field is a 7-bit field. The following values are defined:
    // 	• 0 = has not been relayed and will not be relayed
    // 	• 1 = may have been relayed, but will not be relayed
    // 	• 2 to 126 = may have been relayed and can be relayed
    // 	• 127 = has not been relayed and can be relayed
    int ttl;
    
    // 11 Bytes Chunk?
    chunkLength = B(11);
    int hops = 127;		// number of times the message was relayed

	// Internal (simulation use)    
    string srcUuid;     // source uuid (where the data was originally generated, the sensor node)
    string packetUuid;  // random uuid per data packet
    int sequence = -1;  // "id" of the generated data (src + sequence should be unique in the simulation)
    simtime_t creationTime;
}

