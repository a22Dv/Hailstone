from typing import Dict, Any, Tuple
from collatz_utils import Utilities, ImageData
from subprocess import Popen, PIPE
from pathlib import Path
import struct
import numpy as np
import numpy.typing as npt
from PIL import Image
from time import sleep
import moderngl as gl


class Application:
    def __init__(self, config: Dict[str, Any], relative_subproc_path: Path) -> None:
        """Default constructor."""
        self.subproc_path: Path = relative_subproc_path
        self.config: Dict[str, Any] = config
        self.subproc: Popen[bytes] = Popen(
            [relative_subproc_path],
            text=False,
            stdin=PIPE,
            stdout=PIPE,
            stderr=PIPE,
        )

    def start(self) -> None:
        """Main entry point for the application."""

        # Test for subprocess response.
        IPC.send(IPC.IPC_CODES["test"], self.subproc)
        if (
            IPC.receive(self.subproc, False).decode("ascii")
            != IPC.IPC_CODES["test_suc"]
        ):
            raise ChildProcessError(f"Subprocess did not respond.")

        # Main loop.
        while True:
            range: Tuple[int, int] = Utilities.getRange()
            if range == (-1, -1):
                self.quit()
            IPC.send(f"{range[0]} {range[1]}", self.subproc)
            bytes_to_read: int = 0
            while True:
                subproc_log_bytes: bytes = IPC.receive(self.subproc, False)
                log_ascii_repr: str = subproc_log_bytes.decode("ascii")
                if IPC.IPC_CODES["proc_fnsh"] not in log_ascii_repr:
                    print(log_ascii_repr)
                    continue
                elif IPC.IPC_CODES["proc_fnsh"] in log_ascii_repr:
                    bytes_to_read = int(
                        log_ascii_repr.removeprefix(IPC.IPC_CODES["proc_fnsh"])
                    )
                IPC.send(IPC.IPC_CODES["send_data"], self.subproc)
                break
            subproc_data_bytes: bytes = IPC.receive(self.subproc, True, bytes_to_read)
            image_data: ImageData = self.get_data(subproc_data_bytes)
            image: Image.Image = self.render_image(image_data)

    def get_data(self, image_bytes: bytes) -> ImageData:
        """Transfers the data from the IPC to a format readable by python via NumPy."""
        segment_count: np.uint32 = np.uint32(struct.unpack("<I", image_bytes[:4])[0])
        background_color: npt.NDArray[np.uint8] = np.array(
            struct.unpack("<4B", image_bytes[4:8]), np.uint8
        )
        image_data_body: bytes = image_bytes[8:]
        data_type: np.dtype[Any] = np.dtype(
            [
                ("x1", "<f4"),
                ("x2", "<f4"),
                ("x3", "<f4"),
                ("x4", "<f4"),
                ("y1", "<f4"),
                ("y2", "<f4"),
                ("y3", "<f4"),
                ("y4", "<f4"),
                ("r", "u1"),
                ("g", "u1"),
                ("b", "u1"),
                ("a", "u1"),
            ]
        )
        image_data_np: npt.NDArray[Any] = np.frombuffer(
            image_data_body,
            dtype=data_type,
        )
        return ImageData(segment_count, background_color, image_data_np)

    def render_image(self, image_data: ImageData) -> Image.Image:
        """Renders an image, then shows the output to the user to determine if it should be saved."""

        # Transform data into vertex pair + broadcasted rgba (x,y,r,g,b,a)
        QUAD_VERTEX_COUNT: int = 4
        vertex_dtype: np.dtype[Any] = np.dtype(
            [
                ("x", np.float32),
                ("y", np.float32),
                ("r", np.uint8),
                ("g", np.uint8),
                ("b", np.uint8),
                ("a", np.uint8),
            ]
        )
        vbo_dtype: np.dtype[Any] = np.dtype((vertex_dtype, (QUAD_VERTEX_COUNT,)))
        vbo_data: npt.NDArray[Any] = np.zeros(image_data.image_bytes.shape, dtype=vbo_dtype)
        for i in range(QUAD_VERTEX_COUNT):
            vbo_data[i]["x"] = image_data.image_bytes[f"x{i + 1}"]
            vbo_data[i]["y"] = image_data.image_bytes[f"y{i + 1}"]
            vbo_data[i]["r"] = image_data.image_bytes["r"]
            vbo_data[i]["g"] = image_data.image_bytes["g"]
            vbo_data[i]["b"] = image_data.image_bytes["b"]
            vbo_data[i]["a"] = image_data.image_bytes["a"]


        
        # Data will be [[each x,y pair with their rgba]...number of vertex points]
        # Get min and max x and y for scaling purposes.

        # Create standalone context.

        # Get a vertex shader and frag_shader source
        # Pass these to prog to compile.
        # Set u_resolution to actual resolution on prog.

        # Setup VBO to pass data to GPU.
        # Setup VAO to describe VBO contents.

        # Setup FBO for off-screen rendering.
        # Texture, and set drawing target.

        # Set viewport to cover entire FBO.
        # Set background color with clear.

        # Render VAO with TRIANGLE_STRIP.

        # Post-processing, add padding then return as Image object.

        return Image.Image()

    def quit(self) -> None:
        """Gracefully terminates the process."""
        IPC.send(IPC.IPC_CODES["terminate"], self.subproc)
        sleep(0.1)
        if self.subproc.returncode:
            exit(0)
        else:
            raise ChildProcessError("Subprocess did not terminate.")


class IPC:
    """A class that holds methods to initiate IPC between it and a subprocess."""

    IPC_CODES: Dict[str, str] = {
        "send": "\n",
        "test": "/0",
        "test_suc": "/1",
        "proc_fnsh": "/2",
        "send_data": "/3",
        "terminate": "/-1",
    }

    @classmethod
    def send(cls, message: str, proc: Popen[bytes]) -> None:
        """Sends a message to a given Popen subprocess to its `stdin`."""
        if proc.stdin:
            proc.stdin.write(f"{message}{cls.IPC_CODES["send"]}".encode("ascii"))
            proc.stdin.flush()
        else:
            raise IOError("Cannot send message to subprocess.")

    @classmethod
    def receive(
        cls, proc: Popen[bytes], stdout: bool = True, bytes_to_read: int = 0
    ) -> bytes:
        """Receives bytes from either `stdout` or `stderr`, is a blocking operation."""
        byte_message: bytes = b""
        if stdout and proc.stdout:
            byte_message = (
                proc.stdout.readline()
                if bytes_to_read == 0
                else proc.stdout.read(bytes_to_read)
            )
        elif not stdout and proc.stderr:
            byte_message = (
                proc.stderr.readline()
                if bytes_to_read == 0
                else proc.stderr.read(bytes_to_read)
            )
        return byte_message.decode("latin-1").removesuffix("\n").encode("latin-1")
