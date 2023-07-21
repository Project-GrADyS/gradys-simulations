package gradys_simulations.mobility;

import gradys_simulations.mobility.base.MAVLinkMobilityBase;

simple DroneMobilityMav extends MAVLinkMobilityBase
{
    parameters:
        @class(DroneMobilityMav);
        @signal[reverse](type=bool);
        double startTime @unit(s) = default(0s);
        string waypointFile;
        double waypointProximity @unit(m);
        double speed @unit(mps);
        
        vehicleType=1;
          
        double telemetryFrequency @unit(s) = default(1s);
        
	gates:
	    inout protocolGate[];
}

