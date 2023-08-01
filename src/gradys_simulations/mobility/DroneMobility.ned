package gradys_simulations.mobility;

import inet.mobility.single.VehicleMobility;

simple DroneMobility extends VehicleMobility
{
    parameters:
        @class(DroneMobility);
        @signal[reverse](type=bool);
        double verticalSpeed @unit(mps)=default(speed);
        double yawSpeed @unit(rad) = default(5deg); // Yaw speed in radians per second
        double startTime @unit(s) = default(0s);
        
        double homeLatitude @unit(degree);
        double homeLongitude @unit(degree);
        
        double telemetryFrequency @unit(s) = default(1s);
        
	gates:
	    inout protocolGate[];
}

