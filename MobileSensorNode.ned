package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;

import projeto.communication.protocols.base.IProtocol;

module MobileSensorNode extends AdhocHost
{    
    parameters:
        @class(MobileSensorNode);

  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		

     submodules:
  	    protocol: <default("CommunicationProtocolBase")> like IProtocol {
  	    }
  	
  	connections:
  		app[0].controllerGate <--> protocol.communicationGate;

}
