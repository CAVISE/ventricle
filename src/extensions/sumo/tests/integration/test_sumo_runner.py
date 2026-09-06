"""Exercise headless SUMO with a tiny, deterministic traffic scenario."""

from pathlib import Path
from xml.etree import ElementTree

from sumo_runner import SumoRunner, SumoSettings, SumoStatistics

SCENARIO = Path(__file__).parent / "scenarios" / "straight" / "scenario.sumocfg"


async def test_vehicle_completes_route(sumo_runner: SumoRunner, tmp_path: Path):
    result = await sumo_runner.run_scenario(SCENARIO)
    trips = result.statistics_dir / "tripinfo.xml"

    assert result.returncode == 0, result.stderr
    assert result.stderr == ""
    assert result.stdout_path.read_text() == result.stdout
    assert result.stderr_path.read_text() == result.stderr
    completed = ElementTree.parse(trips).getroot().findall("tripinfo")
    assert len(completed) == 1
    assert completed[0].attrib["id"] == "vehicle-1"
    assert float(completed[0].attrib["arrival"]) > 0
    assert float(completed[0].attrib["routeLength"]) > 90


async def test_missing_scenario_returns_error(sumo_runner: SumoRunner, tmp_path: Path):
    result = await sumo_runner.run_scenario(tmp_path / "missing.sumocfg")
    assert result.returncode != 0
    assert "missing.sumocfg" in result.stderr


async def test_startup_and_statistics_overrides(sumo_runner: SumoRunner):
    result = await sumo_runner.run_scenario(
        SCENARIO,
        settings=SumoSettings(end=1, step_length=0.5),
        statistics=SumoStatistics(summary=True, fcd=True, write_unfinished=True),
    )
    assert result.returncode == 0, result.stderr
    summary = ElementTree.parse(result.statistics_dir / "summary.xml").getroot()
    assert [float(step.attrib["time"]) for step in summary] == [0, 0.5]
    trips = ElementTree.parse(result.statistics_dir / "tripinfo.xml").getroot()
    assert len(trips.findall("tripinfo")) == 1
    assert float(trips.find("tripinfo").attrib["arrival"]) == -1
    assert (result.statistics_dir / "fcd.xml").is_file()
    assert not (result.statistics_dir / "collision.xml").exists()

    second = await sumo_runner.run_scenario(
        SCENARIO, statistics=SumoStatistics(tripinfo=False)
    )
    assert second.returncode == 0, second.stderr
    assert second.statistics_dir != result.statistics_dir
    assert not (second.statistics_dir / "tripinfo.xml").exists()
