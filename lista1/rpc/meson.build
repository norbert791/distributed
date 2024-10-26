rpc_inc = include_directories('inc')
rpc_src = files('src/udp.cpp')

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
