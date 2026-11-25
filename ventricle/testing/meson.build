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
  suite: test_suite_large,
  priority: test_priority_normal,
  timeout: 60,
)