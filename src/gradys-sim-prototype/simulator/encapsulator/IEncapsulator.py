from abc import ABC, abstractmethod
from simulator.protocols.IProtocol import IProtocol


class IEncapsulator(ABC):
    protocol: IProtocol

    @classmethod
    @abstractmethod
    def encapsulate(cls, protocol: IProtocol):
        pass

    @abstractmethod
    def initialize(self, stage: int):
        pass

    @abstractmethod
    def handle_timer(self, timer: dict):
        pass

    @abstractmethod
    def handle_packet(self, message: dict):
        pass

    @abstractmethod
    def finish(self):
        pass
