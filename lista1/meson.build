project(
    'distributed-1',
    'cpp',
    version: '1.0',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
)

subdir('rpc')
main = executable('main', 'main.cpp', dependencies: [rpc_dep])
