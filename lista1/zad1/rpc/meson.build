rpc_inc = include_directories('inc')
rpc_src = files('src/marshalling.cpp', 'src/server.cpp', 'src/udp.cpp', 'src/client.cpp')

asio_dep = dependency('asio', required: true)
rpc_dep = static_library(
    'rpc_lib',
    rpc_src,
    include_directories: [rpc_inc],
    dependencies: [asio_dep],
)
rpc_dep = declare_dependency(
    include_directories: [rpc_inc],
    link_with: rpc_dep,
    dependencies: [asio_dep.partial_dependency(includes: true)],
)

gtest_dep = dependency('gtest', main: true, required: true)
rpc_test = files('test/marshalling.cpp')
tests = executable('marshalling', rpc_test, dependencies: [rpc_dep, gtest_dep])
test('RPC marshalling tests', tests)