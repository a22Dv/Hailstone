from display import Display
from utilities import Utilities
from ipc import IPC
from pathlib import Path
from typing import Dict, Tuple, Any
import numpy.typing as npt
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
        self.execution_path: Path = execution_path
        self.config_path: Path = config_path
        self.images_path: Path = images_path
        self.subproc_path: Path = subproc_path
        self.running: bool = True
        self.config: Dict[str, Any] = {}
        self.display: Display = display
        self.utilities: Utilities = utilities
        self.subprocess: subprocess.Popen[bytes] = self.utilities.get_subprocess(subproc_path)
        self.ipc: IPC = IPC(self.subprocess, False)
        self.batch_threshold: int = 10000

    def start(self) -> None:
        "Start of process."
        self.config = self.utilities.set_config(self.config_path)
        while self.running:
            range: Tuple[int, int] = self.utilities.get_range()
            if range == (-1, -1):
                self.quit()

            effective_range: int = range[1] - range[0]

            # BUG: Doesn't run if range goes below threshold.
            for _ in range(effective_range // self.batch_threshold): # type: ignore
                image_data: npt.NDArray[Any] = self.get_image_data(range)


    # TODO: Add progress display support.
    def get_image_data(self, range: Tuple[int, int]) -> npt.NDArray[Any]:
        self.ipc.send(f"{range[0]} {range[1]}")
        while True:
            log_data: bytes | str = self.ipc.receive(False, True)
            if log_data == self.ipc.codes["processing_finished"]:
                break
        self.ipc.send(self.ipc.codes["send_data"])
        image_data: bytes | str = self.ipc.receive(True, False)

        if type(image_data) == bytes:
            self.parse_bytes(image_data)
        else:
            raise TypeError("Incorrect type. Bytes required.")

     # TODO: Add progress display support.
    def parse_bytes(self, image_data: bytes) -> npt.NDArray[Any]:
        pass


    def quit(self) -> None:
        self.ipc.send(self.ipc.codes["terminate"])
        if self.subprocess.returncode != 0:
            raise ChildProcessError("Failure to terminate gracefully.")
        self.running = False
