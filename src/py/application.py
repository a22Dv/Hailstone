from display import Display
from utilities import Utilities
from pathlib import Path

class Application:
    def __init__(
        self,
        display: Display,
        utilities: Utilities,
        execution_path: Path,
        config_path: Path,
        images_path: Path,
        subproc_path: Path,
    ) -> None:
        self.display: Display = display
        self.utilities: Utilities = utilities
        self.execution_path: Path = execution_path
        self.config_path: Path = config_path
        self.images_path: Path = images_path
        self.subproc_path: Path = subproc_path
        self.running: bool = True

    def start(self) -> None:
        while self.running:
            pass

    def quit(self) -> None:
        pass
    
