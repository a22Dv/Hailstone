import re
from typing import List, Optional, Tuple, TypeAlias
import subprocess
import atexit
import numpy as np
import math
from PIL import ImageDraw, ImageFont, Image
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.cm as cm

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
        self.even_angle: float = math.radians(even_angle)
        self.odd_angle: float = math.radians(odd_angle)
        self.line_length: float = float(line_len)

    def preprocess_sequences(self, sequences: List[List[int]]) -> np.ndarray:
        MAX_DIM: int = max([len(seq) for seq in sequences])
        seq_matrix: np.ndarray = np.zeros(shape=(MAX_DIM, len(sequences)), dtype=int)
        for i, seq in enumerate(sequences):
            seq_matrix[: len(seq), i] = seq
        curr_angles: np.ndarray = np.full(
            len(sequences), math.radians(90.0), dtype=float
        )
        coordinates: np.ndarray = np.zeros(
            shape=(MAX_DIM, len(sequences), 2), dtype=float
        )

        for i in range(1, MAX_DIM):
            curr_row: np.ndarray = seq_matrix[i, :]
            prev_row: np.ndarray = seq_matrix[i - 1, :]
            active_mask: np.ndarray = prev_row > 1
            move_mask: np.ndarray = curr_row != 0
            active_curr_row: np.ndarray = curr_row[active_mask]
            active_prev_row: np.ndarray = prev_row[active_mask]
            curr_angles[active_mask] += np.where(
                active_curr_row * 2 == active_prev_row, self.even_angle, self.odd_angle
            )
            delta_y: np.ndarray = self.line_length * np.sin(curr_angles)
            delta_x: np.ndarray = self.line_length * np.cos(curr_angles)
            coordinates[i, move_mask, 0] = (
                coordinates[i - 1, move_mask, 0] + delta_x[move_mask]
            )
            coordinates[i, move_mask, 1] = (
                coordinates[i - 1, move_mask, 1] + delta_y[move_mask]
            )
            coordinates[i, ~move_mask, :] = coordinates[i - 1, ~move_mask, :]
        return np.transpose(coordinates, axes=(1, 0, 2))

    def animate_display(
        self, path_coordinates: np.ndarray, sequences: List[List[int]]
    ) -> None:
        num_sequences, max_steps, _ = path_coordinates.shape
        actual_lengths = [len(seq) for seq in sequences]
        fig, ax = plt.subplots(figsize=(10, 8))
        fig.patch.set_facecolor("white")
        ax.patch.set_facecolor("white")
        for spine in ax.spines.values():
            spine.set_visible(False)
        ax.tick_params(
            axis="both", which="both", length=0, labelleft=False, labelbottom=False
        )
        ax.grid(False)
        cmap = cm.get_cmap("magma")
        min_len = min(actual_lengths) if actual_lengths else 1
        max_len = max(actual_lengths) if actual_lengths else 1
        if max_len == min_len:
            norm_lengths = [0.5] * num_sequences
        else:
            norm_lengths = [(l - min_len) / (max_len - min_len) for l in actual_lengths]
        colors = [cmap(norm) for norm in norm_lengths]
        lines = [
            ax.plot([], [], lw=1.5, color=colors[i])[0] for i in range(num_sequences)
        ]
        x_min, x_max = np.min(path_coordinates[:, :, 0]), np.max(
            path_coordinates[:, :, 0]
        )
        y_min, y_max = np.min(path_coordinates[:, :, 1]), np.max(
            path_coordinates[:, :, 1]
        )
        x_range = x_max - x_min if x_max > x_min else 1.0
        y_range = y_max - y_min if y_max > y_min else 1.0
        ax.set_xlim(x_min - x_range * 0.05, x_max + x_range * 0.05)
        ax.set_ylim(y_min - y_range * 0.05, y_max + y_range * 0.05)

        def init():
            for line in lines:
                line.set_data([], [])
            return lines

        def update(step):
            for i, line in enumerate(lines):
                line.set_data(
                    path_coordinates[i, : step + 1, 0],
                    path_coordinates[i, : step + 1, 1],
                )
            return lines

        ani = animation.FuncAnimation(
            fig,
            update,
            frames=max_steps,
            init_func=init,
            interval=0,
            blit=True,
            repeat=False,
        )
        plt.show()
