test(
  'python-testing-facilities',
  python,
  args: [
    '-m', 'pytest',
    '-c', pytest_config,
    meson.current_source_dir() / 'tests',
    '-q',
    '--basetemp=' + (meson.current_build_dir() / 'pytest-tmp'),
  ],
  workdir: meson.current_build_dir(),
  suite: large,
  priority: normal,
  timeout: 60,
)