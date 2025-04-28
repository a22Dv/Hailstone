from core import Inputs, Calculate, Path, Display
from typing import List
import numpy as np
import random as rand


class MainProcess:
    def __init__(self) -> None:
        self.calc_instance = Calculate()
        self.disp_instance = Display(-6.5, 12.75, 1)

    def main(self):
        try:
            while True:
                random: bool = False
                count: str = ""
                user_input: str = Inputs.get_input(
                    "Enter a range of values to evaluate the Collatz Conjecture [e.g. 6 -> 9 [R-512 - Random (512 numbers)]]: ",
                    r"(-?\d+)\s*->\s*(-?\d+)\s*(R-\d+)?",
                )
                if "R" in user_input:
                    random: bool = True
                    expr, count = user_input.split("R-")
                    val_range: List[int] = [
                        int(input.strip()) for input in expr.split("->")
                    ]
                    random_count: int = int(count)
                else:
                    val_range: List[int] = [
                        int(input.strip()) for input in user_input.split("->")
                    ]

                sequences: List[List[int]] = []
                if not random:
                    sequences = self.calc_instance.get_collatz_sequences(
                        val_range[0], val_range[1]
                    )
                else:
                    points: List[int] = [rand.randint(val_range[0], val_range[1]) for _ in range(random_count)]
                    for point in points:
                        sequences.append(self.calc_instance.get_collatz_sequences(
                            point, point
                        )[0])
                  
                path_coordinates: np.ndarray = self.disp_instance.preprocess_sequences(
                    sequences
                )
                self.disp_instance.animate_display(path_coordinates, sequences)

        except Exception as e:
            print("An exception has occured.")
            print(e.args)


if __name__ == "__main__":
    MainProcess().main()
