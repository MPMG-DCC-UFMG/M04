from abc import ABC, abstractmethod

class GeneralPatternMiner(ABC):
    # def __init__(self, value):
    #     self.value = value
    #     super().__init__()

    @abstractmethod
    # Read the input data from an external source and store it locally
    def ingest_data(self, input_path: str):
        pass

    @abstractmethod
    # Mine the patterns and store them locally
    def mine_pattern(self):
        pass

    @abstractmethod
    # Export the data to the approppriate location in the approppriate format
    def export_patterns(self, output_path: str):
        pass