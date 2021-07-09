package projeto;

import inet.mobility.single.VehicleMobility;

simple DroneMobility extends VehicleMobility
{
    parameters:
        @class(DroneMobility);
        double verticalSpeed @unit(mps)=default(speed);
        double yawSpeed @unit(rad) = default(5deg); // Yaw speed in radians per second
        double startTime @unit(s) = default(0s);
        
	gates:
	    input orderGate @directIn;
}

