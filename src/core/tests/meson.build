dynamic_node_manager_test = executable(
  'dynamic-node-manager-test',
  ['dynamic_node_manager_test.cpp', 'main.cpp'],
  dependencies: ventricle_core_dep,
)

test(
  'dynamic-node-manager',
  dynamic_node_manager_test,
  args: ['--suite=ventricle-dynamic-node-manager'],
)
