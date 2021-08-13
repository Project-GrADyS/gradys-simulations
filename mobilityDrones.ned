package projeto;

import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.environment.ground.FlatGround;
import inet.node.inet.AdhocHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.mobility.contract.IMobility;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.physicallayer.common.packetlevel.RadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;




network mobilityDrones
{
    parameters:

		int numUAVs = default(1);
		int numSensors = default(1);

    types:

    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
        
        environment: PhysicalEnvironment {
        }
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=0,200");
        }
        radioMedium: RadioMedium {
            @display("p=200,0");
        }
        quads[numUAVs]: MobileNode {
			
        } 
        
        sensors[numSensors]: MobileSensorNode {
            @display("i=misc/sensor");
        }
        groundStation: GroundStation {
            @display("i=misc/sensor3");
        }
}
