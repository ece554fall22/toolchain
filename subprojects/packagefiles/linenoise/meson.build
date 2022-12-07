project('linenoise', 'c')

incdirs = include_directories('.')
install_headers('linenoise.h')

liblinenoise = library('linenoise', files('linenoise.c'), )

linenoise_dep = declare_dependency(link_with: liblinenoise, include_directories: incdirs)
