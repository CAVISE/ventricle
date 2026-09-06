"""Typed asynchronous execution of existing test binaries."""

import asyncio
import os
from collections.abc import Mapping
from dataclasses import dataclass
from pathlib import Path
from typing import IO, cast


@dataclass(frozen=True)
class ExecutionResult:
    """Exit status and captured text; uncaptured streams are None."""

    args: tuple[str, ...]
    returncode: int
    stdout: str | None
    stderr: str | None


class AsyncExector:
    """Base for test helpers wrapping a binary, with optional cwd and environment overrides."""

    def __init__(
        self,
        binary: Path,
        *,
        cwd: Path | None = None,
        env: Mapping[str, str] | None = None,
    ) -> None:
        self.binary = binary.resolve()
        self.cwd = cwd
        self.env = dict(env or {})

    async def run(
        self,
        *args: str | Path,
        input: str | None = None,
        stdout: int | IO[bytes] | None = asyncio.subprocess.PIPE,
        stderr: int | IO[bytes] | None = asyncio.subprocess.PIPE,
    ) -> ExecutionResult:
        """Run without a shell and return output and exit status without checking it.

        Each stream accepts PIPE (capture), DEVNULL (discard), None (inherit), or
        a file. stderr also accepts STDOUT to merge into stdout. Text uses UTF-8.
        Process timeouts and cancellation cleanup are outside this helper's scope.
        """
        command = (str(self.binary), *(str(arg) for arg in args))
        process = await asyncio.create_subprocess_exec(
            *command,
            stdin=(
                asyncio.subprocess.PIPE
                if input is not None
                else asyncio.subprocess.DEVNULL
            ),
            stdout=stdout,
            stderr=stderr,
            cwd=self.cwd,
            env={**os.environ, **self.env},
        )
        output, errors = await process.communicate(
            input.encode() if input is not None else None
        )
        return ExecutionResult(
            args=command,
            returncode=cast(int, process.returncode),
            stdout=output.decode() if output is not None else None,
            stderr=errors.decode() if errors is not None else None,
        )
