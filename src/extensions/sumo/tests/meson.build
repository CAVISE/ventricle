sumo_node_manager_test = executable(
  'traci-node-manager-test',
  ['traci_node_manager_test.cpp', 'main.cpp'],
  dependencies: ventricle_sumo_dep,
)

test(
  'traci-node-manager',
  sumo_node_manager_test,
  args: ['--suite=ventricle-traci-node-manager'],
)

sumo_listener_test = executable(
  'traci-listener-test',
  ['traci_listener_test.cpp', 'main.cpp'],
  dependencies: ventricle_sumo_dep,
)

test(
  'traci-listener',
  sumo_listener_test,
  args: ['--suite=ventricle-traci-listener'],
)
