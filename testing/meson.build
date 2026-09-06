test(
  'python-testing-facilities',
  python,
  args: [
    '-m', 'pytest',
    '-c', pytest_config,
    files('tests/test_async_executor.py'),
    '-q',
    '--basetemp=' + (meson.current_build_dir() / 'pytest-tmp'),
  ],
  workdir: meson.current_build_dir(),
  suite: ['python', 'testing'],
  timeout: 60,
)
