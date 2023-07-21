package sim.mam_2_drone_simulation;

import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.environment.ground.FlatGround;
import inet.node.inet.AdhocHost;
import inet.visualizer.common.IntegratedVisualizer;
import inet.mobility.contract.IMobility;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.SensorNode;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.common.medium.RadioMedium; 
import inet.visualizer.common.IntegratedMultiVisualizer;
import inet.visualizer.common.IntegratedVisualizer;

import gradys_simulations.MobileNodeMam;
import gradys_simulations.GroundStation;

network GradysNetworkMAM
{
    parameters:

		int numUAVs = default(1);
		int numSensors = default(1);
		int numRelaySensors = default(3);

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
        quads[numUAVs]: MobileNodeMam {
			
        }
        
        sensors[numSensors]: AdhocHost {
            @display("i=misc/sensor");
        }
        sensors_rly[numRelaySensors]: AdhocHost {
            @display("i=misc/sensor");
        }
        groundStation: GroundStation {
            @display("i=misc/sensor3");
        }
}
