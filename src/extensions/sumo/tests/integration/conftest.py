"""Override these fixtures in child conftest files to customize SUMO startup."""

import os
from pathlib import Path

import pytest
from sumo_runner import SumoRunner, SumoSettings, SumoStatistics


@pytest.fixture
def sumo_settings() -> SumoSettings:
    return SumoSettings()


@pytest.fixture
def sumo_statistics() -> SumoStatistics:
    return SumoStatistics()


@pytest.fixture
def sumo_runner(
    tmp_path: Path, sumo_settings: SumoSettings, sumo_statistics: SumoStatistics
) -> SumoRunner:
    return SumoRunner(
        Path(os.environ["SUMO_BINARY"]),
        cwd=tmp_path,
        settings=sumo_settings,
        statistics=sumo_statistics,
    )
