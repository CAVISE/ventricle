from pathlib import Path

import pytest

from testing import FilesystemLayout


@pytest.fixture(scope='session')
def test_root() -> Path:
    return Path(__file__).parent


@pytest.fixture
def files(layout: FilesystemLayout, request):
    with layout.program(request.node.name) as files:
        yield files
