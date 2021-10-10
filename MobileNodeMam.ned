package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;

import projeto.communication.protocols.base.IProtocol;

module MobileNodeMam extends AdhocHost
{   
    parameters:
  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		int nodeType = 171;
  		
  		
  		@class(MobileNode);
  	
  	submodules:
  	    protocol: <default("CommunicationProtocolBase")> like IProtocol {
  	    }
  	
  	connections:
  		app[0].protocolGate <--> protocol.communicationGate++;
  		app[1].protocolGate <--> protocol.communicationGate++;
  		protocol.mobilityGate++ <--> mobility.commGate;
}