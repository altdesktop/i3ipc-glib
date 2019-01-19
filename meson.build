project(
  'i3ipc-glib',
  'c',
  version: '2.0.0-rc1',
  meson_version: '>=0.46.0'
)

gnome = import('gnome')
pkgconfig = import('pkgconfig')

version_conf = configuration_data()

i3ipc_version = meson.project_version().split('-')[0]
version_array = i3ipc_version.split('.')
i3ipc_major_version = version_array[0]

version_conf.set(
  'I3IPC_VERSION',
  meson.project_version(),
)
version_conf.set(
  'I3IPC_MAJOR_VERSION',
  i3ipc_major_version.to_int(),
)
version_conf.set(
  'I3IPC_MINOR_VERSION',
  version_array[1].to_int(),
)
version_conf.set(
  'I3IPC_MICRO_VERSION',
  version_array[2].to_int(),
)

xcb_dep = dependency('xcb')
json_dep = dependency('json-glib-1.0')
gobject_dep = dependency('gobject-2.0', version: '>=2.38')
gio_dep = dependency('gio-unix-2.0')
glib_dep = dependency('glib-2.0')

subdir('i3ipc-glib')
subdir('doc')
