from typing import Tuple, Any
from re import fullmatch
from dataclasses import dataclass
import numpy.typing as npt
import numpy as np


class Utilities:
    @staticmethod
    def getRange() -> Tuple[int, int]:
        """Parses user input to return a range."""
        pattern: str = r"\s?(\d+)\s?->\s?(\d+)\s?"
        while True:
            user_input: str = input("Enter Range [2 -> N]: ")
            if user_input.lower() == "q":
                return (-1, -1)
            if fullmatch(pattern, user_input):
                split_input: Tuple[str, ...] = tuple(user_input.split("->"))
                range: Tuple[int, int] = (int(split_input[0]), int(split_input[1]))
                if 2 <= range[0] and 2 <= range[1] and range[0] <= range[1]:
                    return range


@dataclass
class ImageData:
    """Holds the required data to draw an image."""

    segment_count: np.uint32
    background_color: npt.NDArray[np.uint8]
    image_bytes: npt.NDArray[Any]
