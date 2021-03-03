package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;


module MobileSensorNode extends AdhocHost
{    
    parameters:
        @class(MobileSensorNode);

  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		

     types:
        //struct product {  int weight; double price; } 
        //foo double;

}
