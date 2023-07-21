package gradys_simulations;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;
import gradys_simulations.protocols.base.IProtocol;


module GroundStation extends AdhocHost
{   
    parameters:
  		@class(MobileNode);
  	
  	submodules:
  	    protocol: <default("CommunicationProtocolBase")> like IProtocol {
  	    }
  	
  	connections:
  		app[0].protocolGate <--> protocol.communicationGate++;
}