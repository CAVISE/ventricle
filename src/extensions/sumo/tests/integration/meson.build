sumo_search_dirs = []
if get_option('sumo_root') != ''
  sumo_search_dirs += get_option('sumo_root') / 'bin'
endif
sumo_binary = find_program('sumo', dirs: sumo_search_dirs)

test(
  'sumo-integration',
  python,
  args: [
    '-m', 'pytest',
    '-c', pytest_config,
    '-q',
    '--basetemp=' + (meson.current_build_dir() / 'pytest-tmp'),
    meson.current_source_dir(),
  ],
  env: {'SUMO_BINARY': sumo_binary.full_path()},
  workdir: meson.current_build_dir(),
  suite: ['integration', 'sumo'],
  timeout: 120,
)
