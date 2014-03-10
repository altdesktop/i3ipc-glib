# Changelog - i3ipc-GLib

## Version 0.1.1

This release contains a number of small bugfixes and feature enhancements.

* Add `const` qualifier to `*gchar` command and query parameters and transfer-none accessors
* move `i3ipc_con_new` constructor out of the public API
* Move Json-GLib and Gio libraries `Requires` to `Requires.private` in the pkg-config and remove them from the gir/typelib
* New `ipc_shutdown` signal is emitted on the Connection when ipc shutdown (i3wm restart) is detected
* Bugfix: remove socket event source on ipc EOF status
* Bugfix: proper error handling on internal ipc message sending

Thanks to [sardemff7](https://github.com/sardemff7) for bug reports and feature requests.

*Tony Crisci - March-10-2014*

## Version 0.0.1

This initial release is a faithful and bindable representation of the ipc. The API should be considered stable enough to use in a new project. Please test thoroughly for memory issues before using it in production.

*Tony Crisci - March-02-2014*
