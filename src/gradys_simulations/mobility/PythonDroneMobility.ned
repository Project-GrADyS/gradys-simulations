package gradys_simulations.mobility;

import inet.mobility.base.MovingMobilityBase;

simple PythonDroneMobility extends MovingMobilityBase
{
    parameters:
        @class(PythonDroneMobility);
        double speed @unit(mps);
        string groundModule = default("environment.ground"); // module path of the ground module
        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
    
        double initialLongitude @unit(deg) = default(0deg);
        double initialLatitude @unit(deg) = default(0deg);

    	double startTime @unit(s) = default(0s);
        double telemetryFrequency @unit(s) = default(1s);
        
    gates:
        inout protocolGate[];

}

