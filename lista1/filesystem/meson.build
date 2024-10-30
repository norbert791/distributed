filesystem_inc = include_directories('inc')
filesystem_src = files('src/filesystem.cpp')

filsystem_dep = static_library(
    'filesystem_lib',
    filesystem_src,
    include_directories: [filesystem_inc],
    dependencies: [rpc_dep]
)
# filsystem_dep = declare_dependency(
#     include_directories: [filesystem_inc],
#     link_with: rpc_dep,
# )
