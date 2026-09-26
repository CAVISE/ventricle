import os
from pathlib import Path

import pytest

from testing import AsyncExector, ProgramIOLayout


class ExtensionProbe(AsyncExector):
    async def load(self, *libraries: Path, files: ProgramIOLayout) -> int:
        return await self.run(
            *(str(path) for path in libraries), stdout=files.stdout, stderr=files.stderr
        )


@pytest.fixture
def probe() -> ExtensionProbe:
    return ExtensionProbe(Path(os.environ['LOADER_PROBE']))


async def test_registry_cache_and_lifetime(
    probe: ExtensionProbe, files: ProgramIOLayout, tmp_path: Path
):
    library = Path(os.environ['TEST_EXTENSION'])
    alias = tmp_path / library.name
    alias.symlink_to(library)
    code = await probe.load(library, alias, files=files)

    assert code == 0, files.stderr.read_text()
    first, second, *alive = files.stdout.read_text().splitlines()
    assert first == second  # Canonical paths resolve to the same descriptor.
    assert first.split()[0] == 'loaded'
    assert first.split()[2:] == ['loader-test', 'loader-test::Component', '1']
    assert alive == ['alive loader-test loader-test::Component'] * 2


@pytest.mark.parametrize(
    ('case', 'status'),
    [
        ('missing', 'NOT_FOUND'),
        ('invalid_file', 'UNAVAILABLE'),
        ('MISSING_ENTRY', 'INVALID_ARGUMENT'),
        ('NULL_DESCRIPTOR', 'INVALID_ARGUMENT'),
        ('DUPLICATE_EXTENSION', 'ALREADY_EXISTS'),
        ('BAD_VERSION', 'FAILED_PRECONDITION'),
        ('UNKNOWN_TYPE', 'INVALID_ARGUMENT'),
    ],
)
async def test_load_errors(
    probe: ExtensionProbe,
    files: ProgramIOLayout,
    tmp_path: Path,
    case: str,
    status: str,
):
    if case == 'missing':
        paths = [tmp_path / 'missing.so']
    elif case == 'invalid_file':
        invalid = tmp_path / 'invalid.so'
        invalid.write_text('not a shared library')
        paths = [invalid]
    elif case == 'DUPLICATE_EXTENSION':
        paths = [Path(os.environ['TEST_EXTENSION']), Path(os.environ[case])]
    else:
        paths = [Path(os.environ[case])]

    assert await probe.load(*paths, files=files) == 1
    assert status in files.stderr.read_text()


async def test_c_extension(probe: ExtensionProbe, files: ProgramIOLayout):
    assert await probe.load(Path(os.environ['C_EXTENSION']), files=files) == 0, (
        files.stderr.read_text()
    )
    loaded, alive = files.stdout.read_text().splitlines()
    assert loaded.split()[2:] == ['c-extension', 'ns3::Object', '1']
    assert alive == 'alive c-extension ns3::Object'
