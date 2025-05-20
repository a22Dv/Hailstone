from pathlib import Path
from typing import Dict, Tuple, Any, List
import subprocess
import yaml
import os
import re

class Utilities:
    def set_config(self, path: Path) -> Dict[str, Any]:
        config_string: str = ""
        with open(path, "r", encoding="utf-8") as yaml_config:
            config_string = yaml_config.read()
        config_data: Dict[str, Any] = yaml.safe_load(config_string)
        return config_data
    
    def set_images_path(self, path: Path) -> None:
        os.mkdir(path)

    def get_subprocess(self, subprocess_path: Path) -> subprocess.Popen[bytes]:
        return subprocess.Popen(
            args=[subprocess_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=False
        )
    def get_range(self) -> Tuple[int, int]:
        user_response: str = ""
        while True:
            instruc: str = "Enter [q] to Quit.\nRange Format: n->n (e.g. 1->10)."
            print(instruc)
            user_response = input("Enter: ")
            if user_response == "q":
                return (-1, -1)
            pattern: str = r"^(\s?\d+)\s?->\s?(\s?\d+\s?)$"
            if re.fullmatch(pattern, user_response):
                break
        values: List[str] = user_response.split("->")
        user_range: Tuple[int, int] = (int(values[0].strip()), int(values[1].strip()))
        return user_range


    
    
    
    