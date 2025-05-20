from application import Application
from display import Display
from utilities import Utilities
from pathlib import Path
import os

# Program entry point.
if __name__ == "__main__":
    display: Display = Display()
    utilities: Utilities = Utilities()

    # TODO: DEVELOPMENT PATHS - MUST CHANGE TO DIRECT PATHS WHEN FINISHED
    execution_path: Path = Path(os.path.abspath(os.path.dirname(__file__)))
    config_path: Path = Path(execution_path.parent.parent / "build" / "config.yaml")
    images_path: Path = Path(execution_path.parent.parent / "images")
    subproc_path: Path = Path(
        execution_path.parent.parent / "build", "collatz_subprocess.exe"
    )
    try:
        if not config_path.exists():
            utilities.set_config(config_path)
        if not images_path.exists():
            utilities.set_images_path(images_path)
        if not subproc_path.exists():
            raise FileNotFoundError("Subprocess executable cannot be found.")
    except Exception as e:
        print(f"{e.args}\nA fatal error has occurred in setup. Aborting operation.")
        exit(-1)
    Application(
        display, utilities, execution_path, config_path, images_path, subproc_path
    ).start()
