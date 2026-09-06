"""Process-level tests, also discoverable by pytest without external binaries."""

import asyncio
import sys
import tempfile
import unittest
from pathlib import Path

from testing import AsyncExector


class AsyncExectorTests(unittest.IsolatedAsyncioTestCase):
    async def test_arguments_input_environment_and_working_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            executor = AsyncExector(
                Path(sys.executable), cwd=Path(directory), env={"VCLE_TEST": "value"}
            )
            result = await executor.run(
                "-c",
                "import os,sys; print(sys.argv[1]); print(os.environ['VCLE_TEST']); "
                "print(os.getcwd()); print(sys.stdin.read(), end=''); print('error', file=sys.stderr)",
                "literal argument; $NOT_EXPANDED",
                input="input text",
            )
            self.assertEqual(result.returncode, 0)
            self.assertEqual(
                result.stdout.splitlines(),
                [
                    "literal argument; $NOT_EXPANDED",
                    "value",
                    str(Path(directory).resolve()),
                    "input text",
                ],
            )
            self.assertEqual(result.stderr, "error\n")

    async def test_failure_preserves_output(self):
        executor = AsyncExector(Path(sys.executable))
        arguments = (
            "-c",
            "import sys; print('out'); print('err', file=sys.stderr); sys.exit(7)",
        )
        result = await executor.run(*arguments)
        self.assertEqual(result.returncode, 7)
        self.assertEqual(result.stdout, "out\n")
        self.assertEqual(result.stderr, "err\n")

    async def test_large_output_and_concurrent_runs(self):
        executor = AsyncExector(Path(sys.executable))
        results = await asyncio.gather(
            *(
                executor.run(
                    "-c",
                    "import sys; sys.stdout.write(sys.argv[1]*200000); "
                    "sys.stderr.write('e'*200000)",
                    value,
                )
                for value in ("a", "b")
            )
        )
        for result, value in zip(results, ("a", "b")):
            self.assertEqual(result.stdout, value * 200000)
            self.assertEqual(result.stderr, "e" * 200000)

    async def test_stream_redirection(self):
        executor = AsyncExector(Path(sys.executable))
        script = "import sys; print('out'); print('err', file=sys.stderr)"
        result = await executor.run("-c", script, stdout=asyncio.subprocess.DEVNULL)
        self.assertIsNone(result.stdout)
        self.assertEqual(result.stderr, "err\n")
        result = await executor.run("-c", script, stderr=asyncio.subprocess.DEVNULL)
        self.assertEqual(result.stdout, "out\n")
        self.assertIsNone(result.stderr)
        result = await executor.run("-c", script, stderr=asyncio.subprocess.STDOUT)
        self.assertCountEqual(result.stdout.splitlines(), ["out", "err"])
        self.assertIsNone(result.stderr)

    async def test_missing_binary(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(FileNotFoundError):
                await AsyncExector(Path(directory) / "missing").run()
