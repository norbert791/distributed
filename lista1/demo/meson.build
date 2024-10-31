demo_src = files('src/main.cpp')
demo = executable('demo', demo_src, dependencies: [filesystem_dep, rpc_dep, gtest_dep])
# Trick here: meson tests succed if they exit with 0, so succesful execution of demo is a passed test
# test('demo tests', demo) 