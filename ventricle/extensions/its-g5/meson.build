itsg5_sources = files(
	'helpers/simple_geographic_coordinate_system.cpp',
  'helpers/packet_conversion.cpp',
  'helpers/position_conversion.cpp',
  'helpers/stack_helper.cpp',
  'model/access_adapter.cpp',
  'model/runtime.cpp',
  'model/stack.cpp',
)

ventricle_itsg5 = static_library(
  'ventricle-its-g5',
  itsg5_sources,
  dependencies: [ventricle_core_dep, ns3_dep, vanetza_dep],
  include_directories: ventricle_include,
)

ventricle_itsg5_dep = declare_dependency(
  link_with: ventricle_itsg5,
  dependencies: [ventricle_core_dep, ns3_dep, vanetza_dep],
  include_directories: ventricle_include,
)

subdir('tests')
