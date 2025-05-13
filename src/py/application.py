from display import Display
from utilities import Utilities
from pathlib import Path
from typing import Dict, Tuple
import numpy as np
import subprocess


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
        self.subprocess: subprocess.Popen = self.utilities.get_subprocess(subproc_path)
        self.config: Dict = {}
        self.PROC_FINISHED: str = "/1"
        self.SEND_DATA: str = "/2"

    def start(self) -> None:
        self.config = self.utilities.set_config(self.config_path)
        while self.running:
            range: Tuple[int, int] = self.utilities.get_range()
            image_data: np.ndarray = self.get_image_data(range)
            print(image_data)
            if range == (-1, -1):
                self.quit()

    def get_image_data(self, range: Tuple[int, int]) -> np.ndarray:
        data_dtype: np.dtype = np.dtype(
            [
                ("x1", np.float32),
                ("y1", np.float32),
                ("x2", np.float32),
                ("y2", np.float32),
                ("r", np.uint8),
                ("g", np.uint8),
                ("b", np.uint8),
                ("a", np.uint8),
            ]
        )
        self.subprocess.stdin.write(f"{range[0]} {range[1]}\n".encode("ascii"))
        self.subprocess.stdin.flush()
        while True:
            log_data: bytes = self.subprocess.stderr.readline()
            log_message: str = log_data.decode("ascii").removesuffix("\n")
            if log_message != self.PROC_FINISHED:
                print(log_message)
            else:
                break
        self.subprocess.stdin.write(f"{self.SEND_DATA}\n".encode("ascii"))
        self.subprocess.stdin.flush()
        image_data: bytes = self.subprocess.stdout.readline()
        return image_data

    def parse_bytes(self, image_data: bytes) -> np.ndarray:
        pass

    def quit(self) -> None:
        self.subprocess.terminate()
        self.running = False
