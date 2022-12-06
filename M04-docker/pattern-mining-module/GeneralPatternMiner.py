from abc import ABC, abstractmethod

class GeneralPatternMiner(ABC):
    # def __init__(self, value):
    #     self.value = value
    #     super().__init__()

    @abstractmethod
    # Read the input data from an external source and store it locally
    def _ingest_data(self):
        pass

    @abstractmethod
    # Mine the patterns and store them locally
    def _mine_patterns(self):
        pass

    @abstractmethod
    # Export the data to the approppriate location in the approppriate format
    def _export_patterns(self):
        pass

    @abstractmethod
    # Export the data to the approppriate location in the approppriate format
    def pipeline(self):
        pass