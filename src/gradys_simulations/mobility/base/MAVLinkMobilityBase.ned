//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package gradys_simulations.mobility.base;

import inet.mobility.base.MovingMobilityBase;
//
// Base module for mobility modules that want to implement mobility using a simulated SITL instance.
//
// During the OMNeT++ simulation initialization phases this module will instantiate a SITL instance and connect to it. 
// This module will them be responsible for receiving telemetry and sending messages to the simulated vehicle.
//
// This is a base module and should not be used directly in a simulation. It is meant to be used as a base for other modules that
// want to implement realistic mobility using a SITL instance.
//
// This and all child modules **require** the coordinateSystemModule parameter to be set. This is because a coordinate system is
// required to translate geographical coordinates to scene coordinates. By default the coordinateSystemModule is set to "coordinateSystem"
// so all you need to do is include a IGeographicCoordinateSystem module in your simulation. The showcase simulations are already set
// up with a module like this.
// 
// This module works best paired with the inet::RealTimeScheduler. It will guarantee that OMNeT is syncronized with the SITL instances
// are syncronized with OMNeT as both simulations will run at real time. You can use this module without using the real time scheduler
// but the synchronicity guarantee will be off. If you want to run the simulation at aproximately real time you should set the playback
// time to "1 / mobility animation speed". **If you are observing errors related to simulation time you should try removing inet::RealTimeScheduler**
simple MAVLinkMobilityBase extends MovingMobilityBase
{
    // TODO: Add support for simulating real vehicles
    parameters:
        @class(MAVLinkMobilityBase);
        coordinateSystemModule = default("coordinateSystem");
        
        // System ID of this GCS.
        // You do not need to change this unless you are trying to connect to the SITL instance
        // with another GCS software that uses this ID
    	int systemId = default(235);
    	// Component ID of this GCS.
    	// You do not need to change this unless you are trying to connect to the SITL instance
        // with another GCS software that uses this ID
        int componentId = default(235);
        
        // systemId of this vehicle instance. This is a unique identifier of the simulated vehicle instance. 
        // You should take care not to repeat this ID if your simulation contains more than on MAVLinkMobility
        // vehicle.
        // By default will use -1, which sets the targetSystem to a unique ID. Be careful as this might
        // conflict with other module's custom targetSystems.
        int targetSystem = default(-1);
        // componentId of this vehicle instance. Generally can be left as is.
        int targetComponent = default(1);
        
        // MAVLink type of vehicle that this class represents
        // COPTER=1
        // PLANE=2 
        // ROVER=4
        int vehicleType = default(1); 
        
        // Path for the parameters for this vehicle. 
        // Default parameters can be found for your vehicle type [here](https://github.com/ArduPilot/ardupilot/tree/master/Tools/autotest/default_params).
        // **Be aware that incorrect parameters can prevent this vehicle from working.** 
        string paramPath;
        
        // Base port for the SITL simulators.
        // The actual PORT the simulators will be run is basePort + (targetSystem * 10) where 
        // targetSystem is the ID of the vehicle being simulated.
        // If you don't want port comflicts set the same value for ALL MAVLink mobility modules and use 
        // different targetSystem for each of them. Or don't set basePort or targetSystem at all, the modules 
        //will automatically figura out ports that don't cause conflict.
        int basePort = default(5505);
        
        // Path to the ArduCopter binary. Used to run simulator instances for this vehicle
        // A stable version can be downloaded from: 
        // https://firmware.ardupilot.org/Tools/MissionPlanner/sitl/CopterStable/ (WINDOWS)
        // https://firmware.ardupilot.org/Copter/stable/SITL_x86_64_linux_gnu/ (LINUX)
		string copterSimulatorPath;
		// Path to the ArduPlane binary. Used to run simulator instances for this vehicle
		// A stable version can be downloaded from: 
        // https://firmware.ardupilot.org/Tools/MissionPlanner/sitl/PlaneStable/ (WINDOWS)
        // https://firmware.ardupilot.org/Plane/stable/SITL_x86_64_linux_gnu/ (LINUX)
		string planeSimulatorPath;
		//// Path to the ArduRover binary. Used to run simulator instances for this vehicle
		// A stable version can be downloaded from: 
        // https://firmware.ardupilot.org/Tools/MissionPlanner/sitl/RoverStable/ (WINDOWS)
        // https://firmware.ardupilot.org/Rover/stable/SITL_x86_64_linux_gnu/ (LINUX)
		// Path to the Rover binary. Used to run simulator instances for this vehicle
		string roverSimulatorPath;
		
        // Latitude of the initial position of the drone.
        // The default value is the default home of the SITL simulator
        double initialLatitude @unit(deg) = default(-35.36326015deg);
        // Longitude of the initial position of the drone.
        // The default value is the default home of the SITL simulator
        double initialLongitude @unit(deg) = default(149.16523839deg);
        // Altitude of the initial position of the drone.
        double initialAltitude @unit(m) = default(0m);
        
        // Waits until the SITL vehicle is ready to arm before the simulation is able to start. 
        // **Warning**: If this option is set to true it may look like the simulation has crashed 
        // on startup. This is because, to my knowledge, the only way of preventing the simulation
        // from starting before the vehicle is ready to arm is to intetionally freeze the module's 
        // initialization.
        // **Warning**: This freeze may last for several minutes, depending on how many vehicles
        // are being initiated and your machine's hardware.
        bool waitUntilReady = default(false);
        
}
