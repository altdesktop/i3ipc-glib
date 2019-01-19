configuration_inc = include_directories('..')

headers = [
  'i3ipc-con.h',
  'i3ipc-glib.h',
  'i3ipc-reply-types.h',
  'i3ipc-event-types.h',
  'i3ipc-connection.h'
]

i3ipc_sources = [
  'i3ipc-con.c',
  'i3ipc-connection.c',
  'i3ipc-reply-types.c',
  'i3ipc-event-types.c'
]

deps = [
  xcb_dep,
  json_dep,
  gobject_dep,
  gio_dep,
  glib_dep
]

install_headers(
 headers,
 install_dir: join_paths(get_option('includedir'), 'i3ipc-glib')
)

enums = gnome.mkenums_simple(
  'i3ipc-enum-types',
  sources: headers,
  install_header: true,
  install_dir: join_paths(get_option('includedir'), 'i3ipc-glib')
)

i3ipc_lib = both_libraries(
  'i3ipc-glib',
  i3ipc_sources, enums,
  dependencies: deps,
  version: i3ipc_version,
  include_directories: configuration_inc,
  install: true
)

if get_option('introspection')
  introspection_dep = dependency('gobject-introspection-1.0', required: false)
  if not introspection_dep.found()
    error('You need to have gobject-introspection installed to generate Gir data. Disable it with `-Dintrospection=false` if you don\'t want to build it')
  endif

  gnome.generate_gir(
    i3ipc_lib,
    sources: [
      enums,
      'i3ipc-connection.c',
      'i3ipc-connection.h',
      'i3ipc-con.c',
      'i3ipc-con.h',
      'i3ipc-reply-types.c',
      'i3ipc-reply-types.h',
      'i3ipc-event-types.c',
      'i3ipc-event-types.h'
    ],
    nsversion: i3ipc_major_version + '.0',
    namespace: 'i3ipc',
    includes: ['GObject-2.0'],
    install: true
  )
endif

pkgconfig.generate(
  libraries: i3ipc_lib,
  subdirs: 'i3ipc-glib',
  version: meson.project_version(),
  name: 'i3ipc-glib',
  filebase: 'i3ipc-glib',
  description: 'A library for controling i3wm',
  requires: ['gobject-2.0'],
  requires_private: 'gio-2.0',
)
