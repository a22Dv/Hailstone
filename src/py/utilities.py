from pathlib import Path
from typing import Dict, Tuple
import subprocess
import yaml
import os
import re

class Utilities:
    def set_config(self, path: Path) -> Dict:
        config_string: str = ""
        with open(path, "r", encoding="utf-8") as yaml_config:
            config_string = yaml_config.read()
        config_data: Dict = yaml.safe_load(config_string)
        return config_data
    
    def set_images_path(self, path: Path) -> None:
        os.mkdir(path)

    def get_subprocess(self, subprocess_path: Path) -> subprocess.Popen:
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
        user_range: Tuple[int, int] = [int(val.strip()) for val in user_response.split("->")]
        return user_range


    
    
    
    