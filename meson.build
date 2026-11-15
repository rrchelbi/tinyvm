project(
  'tinyvm',
  'c',
  version: '0.0.1',
  meson_version: '>=1.3.0',
  default_options: [
    'c_std=c23',
    'warning_level=3',
    'buildtype=debugoptimized',
  ],
)

inc = include_directories('include')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

tinyvm_core = static_library(
  'tinyvm_core',
  'src/kvm.c',
  include_directories: inc,
)

core = declare_dependency(
  link_with: tinyvm_core,
  include_directories: inc,
)

executable(
  'tinyvm',
  'main.c',
  dependencies: core,
)

subdir('tools')
subdir('tests')

