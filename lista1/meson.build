project(
    'distributed-1',
    'cpp',
    version: '1.0',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
)

subdir('rpc')
subdir('filesystem')
client = executable('client', 'client.cpp', dependencies: [rpc_dep])
server = executable('server', 'server.cpp', dependencies: [rpc_dep])

