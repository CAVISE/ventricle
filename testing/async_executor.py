"""Typed asynchronous execution of existing test binaries."""

import asyncio
import os
from asyncio import subprocess as subproc
from pathlib import Path
from typing import Iterable, Optional, TypeAlias

import loguru

_Env: TypeAlias = dict[str, str]
# You can use base args if every call to binary requires certain setting. This
# is actually quite useful for simulators especially, where you want to have headless
# mode on on every run, for example.
_Args: TypeAlias = Iterable[str]

DEFAULT_EXECUTION_TIMEOUT = 5
DEFAULT_TERMINATE_TIMEOUT = 3
DEFAULT_RETURNCODE = -1


class AsyncExector:
    """Base for test helpers wrapping a binary."""

    def __init__(
        self,
        binary: Path,
        base_args: Optional[_Args] = None,
        cwd: Optional[Path] = None,
        env: Optional[_Env] = None,
    ) -> None:
        self.binary = binary.resolve()
        self.cwd = cwd.resolve() if cwd else Path.cwd()
        self._environment = dict(os.environ) if env is None else env
        self.base_args = base_args if base_args is not None else ()
        self.logger = loguru.logger.bind(binary=self.binary_name)

    @property
    def binary_name(self) -> str:
        return self.binary.stem

    @property
    def cwd(self) -> Path:
        return self._cwd

    @cwd.setter
    def cwd(self, cwd: Path):
        self._cwd = cwd

    @property
    def environment(self) -> _Env:
        return self._environment

    @environment.setter
    def environment(self, env: _Env):
        self._environment = env

    @property
    def base_args(self) -> _Args:
        return self._base_args

    @base_args.setter
    def base_args(self, args: _Args):
        self._base_args = tuple(args)

    async def __try_kill_gracefully(
        self, process: subproc.Process, terminate_timeout: int
    ) -> int:
        async def __impl() -> int:
            self.logger.info('trying to finish the process with SIGTERM')

            process.terminate()
            # A negative timeout allows an indefinite wait for graceful exit.
            if terminate_timeout < 0:
                return await process.wait()

            if terminate_timeout > 0:
                try:
                    self.logger.info(
                        'waiting for processes to exit for {timeout} seconds',
                        timeout=terminate_timeout,
                    )
                    return await asyncio.wait_for(
                        process.wait(), timeout=terminate_timeout
                    )
                except asyncio.TimeoutError:
                    self.logger.info('process did not finish withing the timeout')

            self.logger.info('sending SIGKILL to the process')
            process.kill()

            return await process.wait()

        # Keep graceful cleanup running if its caller is cancelled.
        try:
            return await asyncio.shield(__impl())
        except asyncio.CancelledError:
            # Send SIGKILL before propagating cancellation, without waiting for exit.
            if process.returncode is None:
                process.kill()
            raise

    async def run(
        self,
        *args: str,
        input: Optional[str] = None,
        stdout: Path,
        stderr: Path,
        execution_timeout: int = DEFAULT_EXECUTION_TIMEOUT,
        terminate_timeout: int = DEFAULT_TERMINATE_TIMEOUT,
    ) -> int:
        """
        Invoke the binary.
        """

        stdin = subproc.PIPE if input else subproc.DEVNULL
        process: subproc.Process

        self.logger.info(
            'preparing to call a binary with combind arguments: {base_args} {args}',
            base_args=self.base_args,
            args=args,
        )

        with open(stdout, 'a') as out, open(stderr, 'a') as err:
            process = await asyncio.create_subprocess_exec(
                self.binary,
                *self.base_args,
                *args,
                stdin=stdin,
                stdout=out,
                stderr=err,
                cwd=self.cwd,
                env=self._environment,
            )

            future = process.communicate(input.encode() if input else None)

            try:
                self.logger.info(
                    'awaiting a process for {timeout} seconds',
                    timeout=execution_timeout,
                )
                _, _ = await asyncio.wait_for(future, execution_timeout)
            except asyncio.TimeoutError:
                self.logger.warning('timeout has been reached; stopping process')
                await self.__try_kill_gracefully(process, terminate_timeout)
            except asyncio.CancelledError:
                self.logger.warning(
                    'receiving cencellation: exiting the process immidiately'
                )
                # Escalate to kill right away.
                await self.__try_kill_gracefully(process, 0)
                raise

        if process.returncode is None:
            # I'm not sure it's possible, but I leave this log in case
            # return code could actually be None.
            self.logger.warning(
                'process did not have returncode set: assuming {returncode}',
                returncode=DEFAULT_RETURNCODE,
            )
            return DEFAULT_RETURNCODE

        return process.returncode
