"""Headless SUMO execution and persistent integration-test artifacts."""

from dataclasses import dataclass, fields
from pathlib import Path
from tempfile import mkdtemp

from testing import AsyncExector, ExecutionResult

RESULTS_DIR = Path(__file__).parent / ".test-results" / "sumo"


@dataclass
class SumoSettings:
    """Startup overrides; None leaves the scenario's own setting unchanged."""

    net_file: Path | None = None
    route_files: tuple[Path, ...] | None = None
    additional_files: tuple[Path, ...] | None = None
    begin: float | None = None
    end: float | None = None
    step_length: float | None = None
    seed: int = 1
    remote_port: int | None = None
    num_clients: int | None = None
    time_to_teleport: float | None = None
    collision_action: str | None = None
    xml_validation: str = "never"
    no_step_log: bool = True
    duration_log_disable: bool = True
    verbose: bool = False

    def arguments(self) -> list[str]:
        """Translate configured fields to SUMO command-line arguments."""
        args = []
        for field in fields(self):
            value = getattr(self, field.name)
            if value is None:
                continue
            option = field.name.replace("_", "-")
            if field.name == "duration_log_disable":
                option = "duration-log.disable"
            if isinstance(value, Path):
                value = value.resolve()
            elif isinstance(value, tuple):
                value = ",".join(str(path.resolve()) for path in value)
            elif isinstance(value, bool):
                value = str(value).lower()
            args.extend((f"--{option}", str(value)))
        return args


@dataclass
class SumoStatistics:
    """Select XML outputs, each written to the invocation's statistics directory."""

    tripinfo: bool = True
    fcd: bool = False
    summary: bool = False
    vehroute: bool = False
    collision: bool = False
    statistic: bool = False
    write_unfinished: bool = False
    fcd_acceleration: bool = False
    fcd_signals: bool = False
    precision: int = 6

    def arguments(self, directory: Path) -> list[str]:
        args = ["--precision", str(self.precision)]
        for output in (
            "tripinfo",
            "fcd",
            "summary",
            "vehroute",
            "collision",
            "statistic",
        ):
            if getattr(self, output):
                args.extend((f"--{output}-output", str(directory / f"{output}.xml")))
        for output in ("tripinfo", "vehroute"):
            if getattr(self, output):
                args.extend(
                    (
                        f"--{output}-output.write-unfinished",
                        str(self.write_unfinished).lower(),
                    )
                )
        if self.fcd:
            args.extend(
                ("--fcd-output.acceleration", str(self.fcd_acceleration).lower())
            )
            args.extend(("--fcd-output.signals", str(self.fcd_signals).lower()))
        return args


@dataclass(frozen=True)
class SumoResult(ExecutionResult):
    """Process output plus paths to this invocation's persistent artifacts."""

    statistics_dir: Path
    stdout_path: Path
    stderr_path: Path


class SumoRunner(AsyncExector):
    """Run SUMO with fixture-provided defaults and independent artifacts per invocation."""

    def __init__(
        self,
        binary: Path,
        *,
        cwd: Path | None = None,
        settings: SumoSettings | None = None,
        statistics: SumoStatistics | None = None,
    ) -> None:
        super().__init__(binary, cwd=cwd)
        self.settings = settings if settings is not None else SumoSettings()
        self.statistics = statistics if statistics is not None else SumoStatistics()

    async def run_scenario(
        self,
        scenario: Path,
        *args: str | Path,
        settings: SumoSettings | None = None,
        statistics: SumoStatistics | None = None,
    ) -> SumoResult:
        """Run a scenario; optional settings replace fixture defaults for this invocation.

        Additional CLI arguments are passed last. Captured stdout/stderr remain
        available on the result and are also saved alongside statistics.
        """
        settings = settings if settings is not None else self.settings
        statistics = statistics if statistics is not None else self.statistics
        for directory in ("statistics", "stdout", "err"):
            (RESULTS_DIR / directory).mkdir(parents=True, exist_ok=True)
        statistics_dir = Path(
            mkdtemp(prefix=f"{scenario.stem}-", dir=RESULTS_DIR / "statistics")
        )
        stdout_path = RESULTS_DIR / "stdout" / f"{statistics_dir.name}.log"
        stderr_path = RESULTS_DIR / "err" / f"{statistics_dir.name}.log"

        result = await self.run(
            "--configuration-file",
            scenario.resolve(),
            *settings.arguments(),
            *statistics.arguments(statistics_dir),
            *args,
        )
        stdout_path.write_text(result.stdout or "")
        stderr_path.write_text(result.stderr or "")
        return SumoResult(
            args=result.args,
            returncode=result.returncode,
            stdout=result.stdout,
            stderr=result.stderr,
            statistics_dir=statistics_dir,
            stdout_path=stdout_path,
            stderr_path=stderr_path,
        )
