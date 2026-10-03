import asyncio
import os
import signal
from asyncio import subprocess as subproc
from pathlib import Path
from typing import Optional

import pytest

from testing import AsyncExector, FilesystemLayout
from testing.async_executor import DEFAULT_EXECUTION_TIMEOUT


class Echo(AsyncExector):
    def __init__(self):
        super().__init__(Path('/bin/echo'))

    async def echo(self, text: str, *, stdout: Path, stderr: Path) -> int:
        return await super().run(text, stdout=stdout, stderr=stderr)


class Sleep(AsyncExector):
    def __init__(self):
        super().__init__(Path('/bin/sleep'))

    async def sleep(
        self,
        seconds: float,
        *,
        stdout: Path,
        stderr: Path,
        execution_timeout: int = DEFAULT_EXECUTION_TIMEOUT,
    ) -> int:
        return await super().run(
            str(seconds),
            stdout=stdout,
            stderr=stderr,
            execution_timeout=execution_timeout,
        )


@pytest.fixture
def layout(tmp_path: Path) -> FilesystemLayout:
    return FilesystemLayout(tmp_path / 'programs')


@pytest.fixture
def timeout() -> int:
    return 3


async def test_echo(layout: FilesystemLayout):
    executor = Echo()
    with layout.program(executor.binary_name) as files:
        assert (
            await executor.echo('hello world', stdout=files.stdout, stderr=files.stderr)
            == 0
        )
        assert files.stdout.read_text() == 'hello world\n'
        assert files.stderr.read_text() == ''


async def test_cancellation_during_execution(
    layout: FilesystemLayout, monkeypatch, timeout
):
    spawn = asyncio.create_subprocess_exec
    future: asyncio.Future[subproc.Process] = asyncio.get_running_loop().create_future()
    process: Optional[subproc.Process] = None

    async def capture_process(*args, **kwargs):
        nonlocal process
        process = await spawn(*args, **kwargs)
        future.set_result(process)
        return process

    with monkeypatch.context() as patcher:
        patcher.setattr(asyncio, 'create_subprocess_exec', capture_process)
        executor = Sleep()
        with layout.program(executor.binary_name) as files:
            task = asyncio.create_task(
                executor.sleep(timeout * 2, stdout=files.stdout, stderr=files.stderr)
            )
            try:
                process = await asyncio.wait_for(future, timeout)
                task.cancel()
                with pytest.raises(asyncio.CancelledError):
                    await asyncio.wait_for(task, timeout)
                assert process.returncode in (-signal.SIGTERM, -signal.SIGKILL)
            finally:
                if process is not None and process.returncode is None:
                    process.kill()
                task.cancel()


def test_properties():
    binary = Path('/bin/echo')
    executor = Echo()

    assert executor.binary == binary.resolve()
    assert executor.binary_name == binary.resolve().stem
    assert executor.cwd == Path.cwd()
    assert executor.environment == dict(os.environ)
    assert tuple(executor.base_args) == ()
