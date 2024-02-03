from gradysim.simulator.handler.communication import CommunicationHandler, CommunicationMedium
from gradysim.simulator.handler.mobility import MobilityHandler
from gradysim.simulator.handler.timer import TimerHandler
from gradysim.simulator.handler.visualization import VisualizationHandler, VisualizationConfiguration
from gradysim.simulator.simulation import SimulationBuilder, SimulationConfiguration

from protocol_mobile import ZigZagProtocolMobile
from protocol_sensor import ZigZagProtocolSensor
from protocol_ground import ZigZagProtocolGround


def run_simulation(real_time: bool):
    builder = SimulationBuilder(SimulationConfiguration(duration=28800, real_time=False))
    builder.add_handler(CommunicationHandler(CommunicationMedium(transmission_range=50)))
    builder.add_handler(TimerHandler())
    builder.add_handler(MobilityHandler())

    # builder.add_handler(VisualizationHandler(VisualizationConfiguration(open_browser=True,
    #                                                                     x_range=(-1000, 1000),
    #                                                                     y_range=(-1000, 1000),
    #                                                                     z_range=(-1000, 1000))))

# Drone locations 
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolMobile, (-600.0, -600.0, 0.0))

    # Ground location
    builder.add_node(ZigZagProtocolGround, (-600.0, -600.0, 0.0))

    # Sensor locations 
    builder.add_node(ZigZagProtocolSensor, (-600.0, -300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-600.0, 0.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-600.0, 300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-600.0, 600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-300.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-300.0, -300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-300.0, 0.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-300.0, 300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (-300.0, 600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (0.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (0.0, -300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (0.0, 0.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (0.0, 300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (0.0, 600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (300.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (300.0, -300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (300.0, 0.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (300.0, 300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (300.0, 600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (600.0, -600.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (600.0, -300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (600.0, 0.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (600.0, 300.0, 0.0))
    builder.add_node(ZigZagProtocolSensor, (600.0, 600.0, 0.0))


    # Simulation
    simulation = builder.build()
    simulation.start_simulation()


if __name__ == '__main__':

    # for i in range():
    run_simulation(True)
