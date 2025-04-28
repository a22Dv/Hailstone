from core import Inputs, Calculate, Path, Display
from typing import List
import numpy as np


class MainProcess:
    def __init__(self) -> None:
        self.calc_instance = Calculate()
        self.disp_instance = Display(6, 3, 5)

    def main(self):
        try:
            while True:
                user_input: str = Inputs.get_input(
                    "Enter a range of values to evaluate the Collatz Conjecture [e.g. 6 -> 9]: ",
                    r"(-?\d+)\s*->\s*(-?\d+)",
                )
                range: List[int] = [
                    int(input.strip()) for input in user_input.split("->")
                ]
                sequences: List[List[int]] = self.calc_instance.get_collatz_sequences(
                    range[0], range[1]
                )
                path_coordinates: List[Path] = self.disp_instance.preprocess_sequences(
                    sequences
                )

        except Exception as e:
            print("An exception has occured.")
            print(e.args)


if __name__ == "__main__":
    MainProcess().main()
