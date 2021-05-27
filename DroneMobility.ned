package projeto;

import inet.mobility.single.VehicleMobility;

simple DroneMobility extends VehicleMobility
{
    parameters:
        @class(DroneMobility);
        double verticalSpeed @unit(mps)=default(speed);
}

