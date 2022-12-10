argparse_dep  = dependency('argparse', required: true)
fmt_dep       = dependency('fmt', required: true)
json_dep      = dependency('nlohmann_json', version: '3.11.2', required: true)
eigen_dep     = dependency('eigen3', required: true)

host_inc = include_directories('.')
sim_exe = executable('host', files('main.cpp'),
                     include_directories: host_inc,
                     dependencies: [argparse_dep, fmt_dep, json_dep, eigen_dep])
