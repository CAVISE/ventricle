from dataclasses import dataclass, fields
from pathlib import Path
from typing import Optional

from testing import AsyncExector, ProgramIOLayout


class SumoLayout(ProgramIOLayout):
    def __init__(self, root: Path, run_id: str):
        super().__init__(root / 'sumo', run_id)
        self.program = 'sumo'


@dataclass
class SumoSettings:
    """Typed SUMO configuration"""

    end: Optional[float] = None
    step_length: Optional[float] = None
    seed: int = 1
    remote_port: Optional[int] = None

    def arguments(self, *extra_args: str) -> list[str]:
        """
        Translate configured fields to SUMO command-line arguments.

        :param extra_args: Additional arguments appended after configured options.
        :return: Arguments ready to pass to SUMO.
        """

        args = []
        for setting in fields(self):
            if (value := getattr(self, setting.name)) is None:
                continue

            option = setting.metadata.get('option', setting.name.replace('_', '-'))
            if isinstance(value, bool):
                value = str(value).lower()
            args.extend((f'--{option}', str(value)))

        args.extend(extra_args)
        return args


class SumoRunner(AsyncExector):
    """Run SUMO with settings and artifact paths provided by fixtures."""

    def __init__(
        self,
        binary: Path,
        layout: ProgramIOLayout,
        cwd: Optional[Path] = None,
    ) -> None:
        super().__init__(binary, cwd=cwd)
        self.layout = layout

    async def run_scenario(
        self,
        scenario: Path,
        *args: str,
        settings: SumoSettings,
    ) -> int:
        """
        Run a scenario; optional settings replace fixture defaults for this invocation.

        :param scenario: SUMO configuration file describing the scenario.
        :param args: Additional CLI arguments, passed after generated options.
        :param settings: Startup overrides; defaults to the runner's settings.
        """

        arguments = (
            '--configuration-file',
            str(scenario.resolve()),
            *settings.arguments(*args),
        )

        return await self.run(
            *arguments, stdout=self.layout.stdout, stderr=self.layout.stderr
        )
