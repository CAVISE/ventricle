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

# Configures modules to build - regard the overlay to configure more
# if needed.
run_target(
  'ns3-configure',
  command: ns3_subproject.get_variable('ns3_configure_command'),
)

run_target(
  'ns3-build',
  command: ns3_subproject.get_variable('ns3_build_command'),
)

cmake = import('cmake')

abseil_dep = dependency(
  'absl',
  method: 'cmake',
  modules: ['absl::flat_hash_map', 'absl::flat_hash_set', 'absl::log', 'absl::log_initialize', 'absl::status', 'absl::status_builder', 'absl::statusor', 'absl::strings'],
)

sumo_cmake_args = []
sumo_root = get_option('sumo_root')

if sumo_root != ''
  sumo_cmake_args += ['-DCMAKE_PREFIX_PATH=' + sumo_root]
endif

sumo_cmake_dep = dependency(
  'SUMO',
  method: 'cmake',
  modules: ['SUMO::libtracicpp'],
  cmake_args: sumo_cmake_args,
)

sumo_dep = sumo_cmake_dep
if host_machine.system() == 'darwin' and sumo_root != ''
  # Meson's CMake bridge mistakes paths below *.framework for framework binaries.
  cpp = meson.get_compiler('cpp')
  sumo_dep = declare_dependency(
    dependencies: [
      sumo_cmake_dep.partial_dependency(compile_args: true, includes: true),
      cpp.find_library('tracicpp', dirs: [sumo_root / 'lib']),
    ],
  )
endif

vanetza_options = cmake.subproject_options()
vanetza_options.add_cmake_defines({
  'BUILD_TESTS': false,
  'BUILD_SOCKTAP': false,
  'BUILD_CERTIFY': false,
  'BUILD_BENCHMARK': false,
  'BUILD_FUZZ': false,
  'BUILD_PKI': false,
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

ventricle_include = include_directories('.')

subdir('src')
