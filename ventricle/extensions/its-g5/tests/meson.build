runtime_test = executable(
  'runtime-test',
  'runtime_test.cpp',
  dependencies: [ventricle_itsg5_dep],
)

address_conversion_test = executable(
	'address-conversion-test',
	'address_conversion_test.cpp',
	dependencies: [ventricle_itsg5_dep],
)

packet_conversion_test = executable(
  'packet-conversion-test',
  'packet_conversion_test.cpp',
  dependencies: [ventricle_itsg5_dep],
)

position_conversion_test = executable(
  'position-conversion-test',
  'position_conversion_test.cpp',
  dependencies: [ventricle_itsg5_dep],
)

geographic_coordinate_system_test = executable(
	'geographic-coordinate-system-test',
	'geographic_coordinate_system_test.cpp',
	dependencies: [ventricle_itsg5_dep],
)

stack_test = executable(
	'stack-test',
	'stack_test.cpp',
	dependencies: [ventricle_itsg5_dep],
)

test('its-g5-runtime', runtime_test,
  args: ['--suite=ventricle-its-g5-runtime'],
  suite: light,
  priority: critical)

test('its-g5-address-conversion', address_conversion_test,
	args: ['--suite=ventricle-its-g5-address'],
  suite: light,
  priority: normal)

test('its-g5-packet-conversion', packet_conversion_test,
  args: ['--suite=ventricle-its-g5-packet'],
  suite: light,
  priority: normal)

test('its-g5-position-conversion', position_conversion_test,
  args: ['--suite=ventricle-its-g5-position'],
  suite: light,
  priority: normal)

test('geographic-coordinate-system', geographic_coordinate_system_test,
	args: ['--suite=ventricle-geographic-coordinate-system'],
  suite: light,
  priority: normal)

test('its-g5-stack', stack_test,
  suite: light,
  priority: critical)
