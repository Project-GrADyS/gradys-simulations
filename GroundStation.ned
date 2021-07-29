package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;


module GroundStation extends AdhocHost
{   
    parameters:
  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		int nodeType = 171;
  		
  		
  		@class(MobileNode);
}