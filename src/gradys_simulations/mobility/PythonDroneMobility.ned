package gradys_simulations.mobility;

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

simple VehicleMobility extends MovingMobilityBase
{
    parameters:
        @class(PythonDroneMobility);
        double speed @unit(mps);
        string groundModule = default("environment.ground"); // module path of the ground module
        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
    gates:
        inout protocolGate[];

}

