from display import Display
from utilities import Utilities
from pathlib import Path
import os
class Application:
    def __init__(self, display: Display, utilities: Utilities) -> None:
        self.display: Display = display
        self.utilities: Utilities = utilities
        self.execution_path: Path = Path(os.path.abspath(os.path.dirname(__file__)))

        
        self.config_path: Path = Path(self.execution_path.parent.parent / "build" / "config.yaml")
        self.images_path: Path = Path(self.execution_path.parent.parent / "images")
        self.subproc_path: Path = Path(self.execution_path / "collatz_subproc.exe")




    def start(self) -> None:
        pass
