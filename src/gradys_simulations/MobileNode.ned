package gradys_simulations;

import inet.node.inet.AdhocHost;
import inet.mobility.base.MovingMobilityBase;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.mobility.contract.IMobility;

import gradys_simulations.protocols.base.IProtocol;
import gradys_simulations.mobility.failures.base.IFailureGenerator;

module MobileNode extends AdhocHost
{
    parameters:
        @class(MobileNode);
        int numFailures = default(0);

    submodules:
        protocol: <default("CommunicationProtocolBase")> like IProtocol {
            @display("p=237.825,75.6");
        }
        failures[numFailures]: <default("FailureGeneratorBase")> like IFailureGenerator {
            @display("p=40.949997,159.075");
        }

    connections:
        app[0].protocolGate <--> protocol.communicationGate++;
        protocol.mobilityGate++ <--> mobility.protocolGate++;

        for i=0..numFailures-1 {
            failures[i].mobilityGate <--> mobility.protocolGate++;
        }


}
