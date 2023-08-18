# from simulator.encapsulator.InteropEncapsulator import InteropEncapsulator
# from simulator.protocols.simple.SimpleProtocolMobile import SimpleProtocolMobile
# from simulator.protocols.simple.SimpleProtocolGround import SimpleProtocolGround
# from simulator.protocols.simple.SimpleProtocolSensor import SimpleProtocolSensor
# from simulator.protocols.simple.SimpleMessage import SenderType, SimpleMessage


# def create_protocol_mobile():
#     return InteropEncapsulator.encapsulate(SimpleProtocolMobile)

# def create_protocol_ground():
#     return InteropEncapsulator.encapsulate(SimpleProtocolGround)

# def create_protocol_sensor():
#     return InteropEncapsulator.encapsulate(SimpleProtocolSensor)

# def create_message(sender_type: int, content: int):
#     type = SenderType(sender_type)
#     message = SimpleMessage(sender=type, content=content)
#     return InteropEncapsulator.encapsulate(message)
