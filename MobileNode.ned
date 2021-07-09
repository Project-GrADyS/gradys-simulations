package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;
import projeto.communication.MessageListener;


module MobileNode extends AdhocHost
{   
    parameters:
  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		int nodeType = 171;
  		mobility.typename = "DroneMobility";
  		@class(MobileNode);
  	
  	submodules:
  	    sink: MessageListener;
  	
  	connections:
  		sink.orderOutput --> mobility.orderGate;
}