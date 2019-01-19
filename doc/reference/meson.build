glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

configure_file(
  input : 'version.xml.in',
  output : 'version.xml',
  configuration : version_conf,
)

i3ipc_shared_link = declare_dependency(
  link_with: i3ipc_lib.get_shared_lib(),
  dependenies: deps
)

gnome.gtkdoc(
  'i3ipc',
  src_dir: join_paths(meson.source_root(), 'i3ipc-glib'),
  dependencies: [
    glib_dep,
    gobject_dep,
    i3ipc_shared_link
  ],
  mkdb_args: [
    '--output-format=xml',
    '--name-space=i3ipc',
  ],
  scan_args: '--deprecated-guards="I3IPC_DISABLE_DEPRECATED"',
  gobject_typesfile: join_paths(meson.current_build_dir(), 'i3ipc.types'),
  fixxref_args: [
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
    '--extra-dir=@0@'.format(glib_docpath),
  ],
  main_sgml: 'i3ipc-glib-docs.xml',
  ignore_headers: [
    'i3ipc-glib.h',
    'i3ipc-con-private.h'
  ],
  install: true,
)
