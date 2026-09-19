"""Override these fixtures in child conftest files to customize SUMO startup."""

import os
from pathlib import Path
from uuid import uuid4

import pytest
from sumo_runner import SumoLayout, SumoRunner, SumoSettings

from testing import FilesystemLayout, ProgramIOLayout


@pytest.fixture(scope='session')
def test_root() -> Path:
    """Store artifacts beneath the SUMO integration suite directory."""
    return Path(__file__).parent


@pytest.fixture
def sumo_settings() -> SumoSettings:
    return SumoSettings()


@pytest.fixture
def sumo_layout(layout: FilesystemLayout) -> SumoLayout:
    """
    Provide independent SUMO output and custom artifact paths for each test.
    """
    return SumoLayout(layout.root, uuid4().hex)


@pytest.fixture
def sumo_runner(
    tmp_path: Path, sumo_settings: SumoSettings, sumo_layout: ProgramIOLayout
) -> SumoRunner:
    return SumoRunner(
        Path(os.environ['SUMO_BINARY']),
        layout=sumo_layout,
        cwd=tmp_path,
    )
