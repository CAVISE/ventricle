from pathlib import Path
from uuid import uuid4

import pytest

from testing import FilesystemLayout


@pytest.fixture(scope='session')
def run_id() -> str:
    """
    Generate a unique identifier shared by tests in this pytest session.
    """
    return uuid4().hex


@pytest.fixture(scope='session')
def test_root() -> Path:
    raise NotImplementedError(
        'Override test_root in the integration suite conftest file'
    )


@pytest.fixture(scope='session')
def layout(test_root: Path, run_id: str) -> FilesystemLayout:
    """
    Provide persistent artifact paths isolated by pytest invocation.
    """
    return FilesystemLayout(test_root / '.test-results' / run_id)
