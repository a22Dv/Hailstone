from collatz_app import Application
from collatz_init_utils import config_default
from pathlib import Path
from yaml import load, SafeLoader
from typing import Any, Dict

if __name__ == "__main__":
    execution_path: Path = Path(__file__)

    # Actual paths. Uncomment once out of development environment.
    # relative_config_path: Path = execution_path.parent / "config.yaml"
    # relative_subproc_path: Path = execution_path.parent / "collatz_subprocess.exe"

    relative_config_path: Path = (
        execution_path.parent.parent.parent / "build" / "config.yaml"
    )
    relative_subproc_path: Path = (
        execution_path.parent.parent.parent / "build" / "collatz_subprocess.exe"
    )
    if not relative_subproc_path.exists():
        raise FileNotFoundError(
            f"Subprocess not found in expected path. {relative_subproc_path}"
        )
    if not relative_config_path.exists():
        with open(relative_config_path, "w") as CONFIG_NEW_W:
            CONFIG_NEW_W.write("\n".join(config_default))
        with open(relative_config_path, "r") as CONFIG_NEW_R:
            config: Dict[str, Any] = load(CONFIG_NEW_R, SafeLoader)
    else:
        with open(relative_config_path, "r") as CONFIG:
            config: Dict[str, Any] = load(CONFIG, SafeLoader)
    Application(config, relative_subproc_path).start()
