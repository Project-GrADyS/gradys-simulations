package projeto;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;


module MobileNode extends AdhocHost
{    
    parameters:
        @class(MobileNode);

  		int internalMobNodeId = -1;
  		int simulationIndexOfFirstNode = 6;
  		int nodeType = 171;
  		
  		int wayPointSetSize = 4;
  		int wayPointIndex = 0;  
  		bool needUpdateTargetXYZ = false;
  		double nextX = 0.0;
  		double nextY = 0.0;
  		double nextZ = 0.0;
  		
  		
  		// TO-DO: remove!!!
  		double nextX_0 = 0.0;
  		double nextY_0 = 0.0;
  		double nextZ_0 = 0.0;
  		double nextX_1 = 0.0;
  		double nextY_1 = 0.0;
  		double nextZ_1 = 0.0;
  		double nextX_2 = 0.0;
  		double nextY_2 = 0.0;
  		double nextZ_2 = 0.0;
  		double nextX_3 = 0.0;
  		double nextY_3 = 0.0;
  		double nextZ_3 = 0.0;  		
  		//double waypointsAxisX[4];
  		//Coord teste;

     types:
        //struct product {  int weight; double price; } 
        //foo double;

}
