sumo_sources = files(
  'helpers/traci_helper.cpp',
  'model/controller.cpp',
  'model/mobility/traci_mobility_manager.cpp',
  'model/mobility/traci_mobility_model.cpp',
  'model/traci_node_manager.cpp',
  'model/traci_port.cpp',
  'model/traci_subscription_manager.cpp',
)

ventricle_sumo = static_library(
  'ventricle-sumo',
  sumo_sources,
  dependencies: [ventricle_core_dep, sumo_dep],
  include_directories: ventricle_include,
)

ventricle_sumo_dep = declare_dependency(
  link_with: ventricle_sumo,
  dependencies: [ventricle_core_dep, sumo_dep],
  include_directories: ventricle_include,
)

subdir('tests')
