import subprocess
from typing import Dict


class IPC:
    def __init__(self, subproc: subprocess.Popen, text=False) -> None:
        self.subprocess: subprocess.Popen = subproc
        self.text: bool = text
        self.codes: Dict[str, str] = {
            "send": "\n",
            "processing_finished": "/1",
            "send_data": "/2",
            "terminate": "/-1",
            "failure_to_receive": "/-2"
        }

    def send(self, message: str) -> None:
        stream: str | bytes = ""
        if self.text:
            stream: str = f"{message}{self.codes['send']}"
        else:
            stream: bytes = f"{message}{self.codes['send']}".encode("ascii")
        self.subprocess.stdin.write(stream)
        self.subprocess.stdin.flush()

    def receive(self, stdout=True, as_text=False) -> bytes | str:
        stream: str | bytes = (
            self.subprocess.stdout.readline()
            if stdout
            else self.subprocess.stderr.readline()
        )
        if self.text:
            if not as_text:
                return stream.removesuffix("\n").encode("ascii")
            return stream.removesuffix("\n")
        else:
            if as_text:
                return stream.decode("ascii").removesuffix("\n")
            return stream.decode("ascii").removesuffix("\n").encode("ascii")
