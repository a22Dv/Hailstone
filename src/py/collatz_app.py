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
import os


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
        self.save_image(image)

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
        """Renders an image, then returns the final image as an Image object."""

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
        vbo_data: npt.NDArray[Any] = np.zeros(
            image_data.image_bytes.shape[0] * QUAD_VERTEX_COUNT, dtype=vertex_dtype
        )
        for i in range(QUAD_VERTEX_COUNT):
            indices: npt.NDArray[Any] = np.arange(
                i, vbo_data.shape[0], QUAD_VERTEX_COUNT
            )
            vbo_data["x"][indices] = image_data.image_bytes[f"x{i + 1}"]
            vbo_data["y"][indices] = image_data.image_bytes[f"y{i + 1}"]
            vbo_data["r"][indices] = image_data.image_bytes["r"]
            vbo_data["g"][indices] = image_data.image_bytes["g"]
            vbo_data["b"][indices] = image_data.image_bytes["b"]
            vbo_data["a"][indices] = image_data.image_bytes["a"]

        # Transform to NDC (Normalized Device Coordinates).
        # Get min, max, center x and y for scaling purposes.
        min_x: np.float32 = np.min(vbo_data["x"])
        min_y: np.float32 = np.min(vbo_data["y"])
        max_x: np.float32 = np.max(vbo_data["x"])
        max_y: np.float32 = np.max(vbo_data["y"])
        center_x: np.float32 = (min_x + max_x) / 2
        center_y: np.float32 = (min_y + max_y) / 2
        width: np.float32 = max_x - min_x
        height: np.float32 = max_y - min_y
        longest_side_length: np.float32 = width if height < width else height

        # To NDC.
        vbo_data["x"] = (vbo_data["x"] - center_x) * 2 / longest_side_length
        vbo_data["y"] = (vbo_data["y"] - center_y) * 2 / longest_side_length
        QUAD_COUNT: int = vbo_data.shape[0] // QUAD_VERTEX_COUNT

        ibo_data: npt.NDArray[np.uint32] = np.zeros((QUAD_COUNT) * 6, dtype=np.uint32)

        # Create IBO
        ibo_pattern: npt.NDArray[np.uint32] = np.array(
            [0, 1, 2, 0, 2, 3], dtype=np.uint32
        )
        ibo_base_index: npt.NDArray[np.uint32] = (
            np.arange(QUAD_COUNT, dtype=np.uint32) * QUAD_VERTEX_COUNT
        )
        ibo_repeated_index: npt.NDArray[np.uint32] = np.repeat(ibo_base_index, 6)
        ibo_tiled: npt.NDArray[np.uint32] = np.tile(ibo_pattern, QUAD_COUNT)
        ibo_data = ibo_repeated_index + ibo_tiled

        # Create standalone context.
        ctx: gl.Context = gl.create_standalone_context()

        # Get a vertex shader and frag_shader source
        # Pass these to prog to compile.
        prog: gl.Program = ctx.program(
            "\n".join(
                (
                    "#version 330 core",
                    "in vec2 in_pos;",
                    "in vec4 in_clr;",
                    "out vec4 vclr;",
                    "void main() {",
                    "   gl_Position = vec4(in_pos, 0.0, 1.0);",
                    "   vclr = in_clr;",
                    "}",
                )
            ),
            "\n".join(
                (
                    "#version 330 core",
                    "in vec4 vclr;",
                    "out vec4 fclr;",
                    "void main() {",
                    "   fclr = vclr;",
                    "}",
                )
            ),
        )

        PADDING_SIZE: int = 200  # Divided between both sides.

        # FBO resolution
        resolution: Tuple[int, ...] = tuple(
            [
                int(v.strip()) - PADDING_SIZE
                for v in self.config["image-size"].split("x")
            ]
        )

        vbo: gl.Buffer = ctx.buffer(data=vbo_data.tobytes())
        ibo: gl.Buffer = ctx.buffer(data=ibo_data.tobytes())
        vao: gl.VertexArray = ctx.vertex_array(prog, [(vbo, "2f 4f1", "in_pos", "in_clr")], index_buffer=ibo, index_element_size=4)  # type: ignore
        clr_attachment: gl.Texture = ctx.texture(
            (resolution[0], resolution[1]), components=4
        )
        fbo: gl.Framebuffer = ctx.framebuffer(color_attachments=[clr_attachment])
        fbo.use()

        # (x,y [center]), width, height.
        ctx.viewport = (0, 0, resolution[0], resolution[1])
        ctx.clear(
            image_data.background_color[0] / 255.0,
            image_data.background_color[1] / 255.0,
            image_data.background_color[2] / 255.0,
            image_data.background_color[3] / 255.0,
        )

        # Render.
        vao.render(mode=gl.TRIANGLES)
        raw: bytes = clr_attachment.read(alignment=1)
        rendered_image: Image.Image = Image.frombytes(
            "RGBA", (resolution[0], resolution[1]), raw
        )

        # Post processing.
        padded_image: Image.Image = Image.new(
            "RGBA",
            (resolution[0] + PADDING_SIZE, resolution[1] + PADDING_SIZE),
            tuple(image_data.background_color),
        )
        padded_image.paste(rendered_image, (PADDING_SIZE // 2, PADDING_SIZE // 2))

        # Transpose as ModernGL has Y-axis (+ upwards), with Pillow being (+ downwards)
        padded_image = padded_image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)

        # Release resources.
        vao.release()
        vbo.release()
        ibo.release()
        prog.release()
        clr_attachment.release()
        fbo.release()
        ctx.release()

        return padded_image

    def save_image(self, image: Image.Image) -> None:
        image.show()
        if not (Path(__file__).parent / "images").exists():
            os.mkdir(Path(__file__).parent / "images")

        if input("Save image? (Y/n): ").lower() == "y":
            image.save(
                Path(__file__).parent
                / "images"
                / f"{self.config["angle-if-odd"]}ODD{self.config["angle-if-even"]}EVEN_{len(os.listdir(Path(__file__).parent / "images"))}.png"
            )

        else:
            pass

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
