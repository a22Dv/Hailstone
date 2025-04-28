import re
from typing import List, Optional, Tuple, TypeAlias
import subprocess
import atexit
import numpy as np
import math
from PIL import ImageDraw, ImageFont, Image

Point: TypeAlias = Tuple[int, int]
Segment: TypeAlias = Tuple[Point, Point]
Path: TypeAlias = List[Segment]


class Inputs:
    @staticmethod
    def get_input(question: str, pattern: str) -> str:
        while True:
            user_input: str = input(question)
            if re.fullmatch(pattern, user_input):
                return user_input


class Calculate:
    def __init__(self) -> None:
        self.EXEC_PATH = "C:/repositories/hailstone/build/bin/collatz.exe"
        self.process: Optional[subprocess.Popen] = None
        self.start_process()
        atexit.register(self.close)

    def get_collatz_sequences(self, start: int, end: int) -> List[List[int]]:
        input_str: str = f"{start}|{end}\n"
        self.process.stdin.write(input_str)
        self.process.stdin.flush()
        output_str: str = self.process.stdout.readline()
        collatz_sequence: List[List[int]] = [
            [int(n) for n in seq.split("|")] for seq in output_str.strip().split("!")
        ]
        return collatz_sequence

    def start_process(self) -> None:
        self.process = subprocess.Popen(
            [self.EXEC_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            encoding="utf-8",
        )
        pass

    def close(self) -> None:
        self.process.terminate()


class Display:
    def __init__(self, even_angle: int, odd_angle: int, line_len: int):
        self.even_angle = math.radians(even_angle)
        self.odd_angle = math.radians(odd_angle)
        self.line_length = line_len

    def preprocess_sequences(self, sequences: List[List[int]]) -> List[Path]:
        MAX_DIM: int = max([len(seq) for seq in sequences])
        seq_matrix: np.ndarray = np.zeros(shape=(len(sequences), MAX_DIM), dtype=int)
        for i, seq in enumerate(sequences):
            seq_matrix[i, : len(seq)] = seq
        current_angles: np.ndarray = np.full(len(sequences), 90, dtype=int)
        coordinates: np.ndarray = np.zeros(shape=(len(sequences), MAX_DIM, 2), dtype=int)
        print(coordinates)
            

        


        

