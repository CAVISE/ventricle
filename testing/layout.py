"""Provide persistent test artifact paths grouped by program."""

from contextlib import contextmanager
from functools import cached_property, lru_cache
from pathlib import Path
from typing import Final, Generator, Literal

from loguru import logger

# This is cache size for custom_artifact call; means how many files
# will be fast to track by that function. It's not a hard limit tho :)
_MAX_PROGRAM_ARTIFACTS: Final[int] = 64


class _LayoutBase:
    def __init__(self, root: Path):
        self.root = root
        self.root.mkdir(exist_ok=True)

    @lru_cache
    def _ensure_exists(self, path: Path, kind: Literal['dir', 'file']) -> Path:
        match kind:
            case 'file':
                path.parent.mkdir(exist_ok=True, parents=True)
                path.touch(exist_ok=True)
            case 'dir':
                path.mkdir(exist_ok=True, parents=True)
        return path


class _ProgramIOLayout(_LayoutBase):
    """
    Provide paths beneath a program's directory.

    :param root: Existing root directory.
    :param program_name: Name of the program.
    """

    def __init__(self, root: Path, program_name: str):
        super().__init__(root / program_name)
        self.program = program_name

    @property
    def outputs_dir(self) -> Path:
        return self._ensure_exists(self.root / 'outputs', 'dir')

    def __standard_io(self, kind: Literal['stdout', 'stderr']) -> Path:
        match kind:
            case 'stderr':
                p = self.outputs_dir / 'stderr.txt'
            case 'stdout':
                p = self.outputs_dir / 'stdout.txt'
            case _:
                raise ValueError

        with logger.contextualize(program=self.program):
            logger.debug(
                'binary has intitialized path "{path}" for its {io}', io=kind, path=p
            )
        return p

    @cached_property
    def stdout(self) -> Path:
        """
        Provide the stdout destination.
        """
        return self.__standard_io('stdout')

    @cached_property
    def stderr(self) -> Path:
        """
        Provide the stderr destination.
        """
        return self.__standard_io('stderr')

    @lru_cache(maxsize=_MAX_PROGRAM_ARTIFACTS)
    def custom_output(self, file: Path) -> Path:
        p = self._ensure_exists(self.outputs_dir / file, 'file')
        with logger.contextualize(program=self.program):
            logger.debug(
                'binary has intitialized path "{path}" for its custom artifact {filename}',
                filename=file.stem,
                path=p,
            )
        return p


class FilesystemLayout(_LayoutBase):
    """
    Organize persistent test artifacts.

    :param root: Artifact directory to create or reuse. Its parent must exist.
    """

    def __init__(self, root: Path):
        super().__init__(root)

    @contextmanager
    def program(self, program: str) -> Generator['_ProgramIOLayout']:
        """
        Yield paths for a program name, retaining artifacts after context exit.

        :param program: Name of the program's directory beneath the layout root.
        :return: A context manager yielding the program's output path provider.
        """
        yield _ProgramIOLayout(self.root, program)
