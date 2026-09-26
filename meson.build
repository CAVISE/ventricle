#############
# Ventricle #
#############

project(
  'ventricle',
  'cpp',
  # We use semantic versioning for our components.
  version: files('.version'),
  license: 'GPL-2.0-or-later',
  license_files: 'LICENCE',
  meson_version: '>=1.1.0',
  # We pull subprojects that otherwise would be hard to find in system.
  # Prefer locating dependency if it's more widely used than some specific
  # simulation software.
  subproject_dir: 'subprojects',
  default_options: ['cpp_std=c++20'],
)

###########
# Options #
###########

# Path to sumo root, if installed outside of discoverable location.
sumo_root = get_option('sumo_root')

py = import('python')
cmake = import('cmake')

# Includes are relative to root dir, look like src/...
ventricle_include = include_directories('.')

#############################
# Core external componenets #
#############################

# Ns3 requires a script to run to configure itself cleanly.
# Since the nature of cmake machinery behind the Ns3 configuration,
# it's easier to invoke it via a script:
# ...
# meson compile -C build ns3-configure
# meson compile -C build ns3-build
# ...

ns3_program = find_program('ns3')
ns3_dep = dependency('ns3')
ns3_subproject = subproject('ns-3')

# Configures modules to build - regard the overlay to configure more if needed.
run_target(
  'ns3-configure',
  command: ns3_subproject.get_variable('ns3_configure_command'),
)

run_target(
  'ns3-build',
  command: ns3_subproject.get_variable('ns3_build_command'),
)

abseil_dep = dependency(
  'absl',
  method: 'cmake',
  modules: [
    'absl::flat_hash_map',    # used instead of plain maps; tends to be faster
    'absl::flat_hash_set',    # same as map, but for sets
    'absl::log',              # ventricle is a complex thing that needs an appropriate logging library
    'absl::log_initialize',   # also logging stuff
    'absl::status',           # I like golang style of reporting errors; status is exactly that
    'absl::status_builder',   # see status
    'absl::statusor',         # see status
    'absl::strings',          # string manipluation lib; proves useful occasionally
    'absl::flags',            # CLI flags
    'absl::flags_parse',      # see flags
    'absl::flags_usage'       # see flags
  ],
)

sumo_cmake_args = []
if sumo_root != ''
  sumo_cmake_args += ['-DCMAKE_PREFIX_PATH=' + sumo_root]
endif

sumo_dep = dependency(
  'SUMO',
  method: 'cmake',
  modules: ['SUMO::libtracicpp'],
  cmake_args: sumo_cmake_args,
)

vanetza_options = cmake.subproject_options()
vanetza_options.add_cmake_defines({
  'CMAKE_POSITION_INDEPENDENT_CODE': true,
  'VANETZA_WITH_GEOGRAPHICLIB': false,
  'VANETZA_WITH_RPC': false,
  'VANETZA_WITH_CRYPTOPP': false,
  'VANETZA_WITH_OPENSSL': true,
  'VANETZA_NET_WITH_POSIX': false,
})

vanetza_subproject = cmake.subproject('vanetza', options: vanetza_options)

# Vanetza's umbrella INTERFACE target loses its component links when translated
# by Meson's CMake module. BTP pulls in GeoNetworking and its transitive stack.
vanetza_dep = vanetza_subproject.dependency('btp', include_type: 'system')

# Used by testsuite
python = py.find_installation(
  modules: [
    'pytest',
    'pytest_asyncio'
  ],
  required: get_option('integration_tests'),
)

pytest_config = files('pytest.ini')

if python.found()
  subdir('testing')
endif

subdir('src')
