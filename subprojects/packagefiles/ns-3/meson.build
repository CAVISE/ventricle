project('ns-3-source', version: '3.48')

ns3_program = find_program(meson.current_source_dir() / 'ns3')
meson.override_find_program('ns3', ns3_program)

ns3_modules = [
  'core',        # Event scheduler, time, objects, attributes, and command line.
  'network',     # Nodes, packets, channels, devices, sockets, and applications.
  'mobility',    # Position, velocity, and mobility models.
  'antenna',     # Antenna radiation patterns and phased arrays.
  'propagation', # Path loss, delay, fading, and channel condition models.
  'spectrum',    # Frequency-aware PHY signals, interference, and channels.
  'wifi',        # IEEE 802.11 MAC and PHY, including 802.11p operation.
]

# Project paths.
ns3_build_dir = meson.current_source_dir() / 'build'
ns3_include_dir = ns3_build_dir / 'include'
ns3_library_dir = ns3_build_dir / 'lib'

# Project specs.
ns3_version = meson.project_version()
ns3_profile = 'default'

# Final monolith target that ventricide will link to.
ns3_library_name = 'libns@0@-monolib-@1@.so'.format(ns3_version, ns3_profile)
ns3_library = ns3_library_dir / ns3_library_name

ns3_dep = declare_dependency(
  compile_args: ['-isystem', ns3_include_dir],
  link_args: [ns3_library, '-Wl,-rpath,' + ns3_library_dir],
)

meson.override_dependency('ns3', ns3_dep)

ns3_configure_command = [
  ns3_program,
  'configure',
  '-d', ns3_profile,
  '--enable-monolib',
  '--disable-examples',
  '--disable-tests',
  '--enable-modules=' + ';'.join(ns3_modules),
]

ns3_build_command = [ns3_program, 'build']
