add_languages('c', native: false)
c_extension = shared_module('loader-c-extension', 'c_extension.c', include_directories: ventricle_include)
bad_version = shared_module('loader-bad-version', 'c_extension.c', c_args: ['-DBAD_VERSION'], include_directories: ventricle_include)
unknown_type = shared_module('loader-unknown-type', 'c_extension.c', c_args: ['-DUNKNOWN_TYPE'], include_directories: ventricle_include)

test_extension = shared_module('loader-test-extension', 'extension.cpp', dependencies: ventricle_core_dep)
missing_entry = shared_module('loader-missing-entry', 'missing_entry.cpp')
null_descriptor = shared_module('loader-null-descriptor', 'extension.cpp', cpp_args: ['-DNULL_DESCRIPTOR'], dependencies: ventricle_core_dep)
duplicate_extension = shared_module('loader-duplicate-extension', 'extension.cpp', dependencies: ventricle_core_dep)

loader_probe = executable('loader-probe', 'loader_probe.cpp', dependencies: dynamic_loader_dep)
test(
  'dynamic-loader',
  python,
  args: ['-m', 'pytest', '-c', pytest_config, '-q', meson.current_source_dir()],
  env: {
    'LOADER_PROBE': loader_probe.full_path(),
    'TEST_EXTENSION': test_extension.full_path(),
    'MISSING_ENTRY': missing_entry.full_path(),
    'NULL_DESCRIPTOR': null_descriptor.full_path(),
    'DUPLICATE_EXTENSION': duplicate_extension.full_path(),
    'C_EXTENSION': c_extension.full_path(),
    'BAD_VERSION': bad_version.full_path(),
    'UNKNOWN_TYPE': unknown_type.full_path(),
  },
  depends: [loader_probe, test_extension, missing_entry, null_descriptor, duplicate_extension, c_extension, bad_version, unknown_type],
  suite: ['python', 'dynamic-loader'],
)
