fmt_dep      = dependency('fmt', required: true)
doctest_dep  = dependency('doctest', required: true)

morph_inc = include_directories('.')
morph_deps = [fmt_dep]

libmorph_dep = declare_dependency(
    include_directories: morph_inc,
    dependencies: [morph_deps])

test_instr_decoder = executable('test_instr_decoder',
    files('tests/instr_decoder.cpp'),
    dependencies: [doctest_dep, libmorph_dep])
test_varint = executable('test_varint',
    files('tests/varint.cpp'),
    dependencies: [doctest_dep, libmorph_dep])
test('instruction decoder', test_instr_decoder)
test('varint utils', test_varint)
