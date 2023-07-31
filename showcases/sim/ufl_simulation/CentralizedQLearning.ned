package sim.ufl_simulation;

import inet.common.geometry.common.SimpleGeographicCoordinateSystem;
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
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadioMedium;
import inet.physicallayer.wireless.ieee802154.bitlevel.Ieee802154UwbIrRadioMedium;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedMultiVisualizer;
import inet.visualizer.common.IntegratedVisualizer;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import gradys_simulations.protocols.auxiliary.CentralizedQLearning;

import gradys_simulations.MobileNode;
import gradys_simulations.MobileSensorNode;
import gradys_simulations.GroundStation;


network CentralizedQLearning
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
        coordinateSystem: SimpleGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
        
        environment: PhysicalEnvironment {
        }
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=0,200");
        }
        radioMedium: <default("Ieee802154NarrowbandScalarRadioMedium")> like IRadioMedium {
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
        
        learner: CentralizedQLearning {
        }
}