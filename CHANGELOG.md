# Changelog - i3ipc-GLib

## Version 0.6.0

This release adds two new container properties.

* con "focus" property is a list of con ids that represents the focus stack of child nodes.
* con "deco_rect" property is the coordinates of the window decoration inside its container.

## Version 0.5.0

This release adds the new `binding` event on the i3ipcConnection and some other bugfixes and features.

* Add the `binding::run` event. This event is triggered when bindings run because of some user action.
* Add the con `scratchpad` method. This method finds the i3 scratchpad workspace container.
* Bugfix: fix bad leaf check in the con `descendents` method that would cause it to ignore some floating nodes.

## Version 0.4.0

This release adds some missing properties on the container and an abstraction to the GLib main loop.

* Add the con "mark" property
* Add the con "fullscreen_mode" property
* Implement con method `find_marked` - finds containers that have a mark that matches a pattern
* Add the connection `main` and `main_quit` methods for scripting

## Version 0.3.1

This minor release includes bugfixes and compatability fixes with the latest stable version of i3. Some new methods and a property on the con object have been added as well.

*This is the first version to distribute a debian package (others available by request)*

* Bugfix: con methods collect floating descendents
* Bugfix: support con "type" for i3 stable (member type has changed in dev version)
* Implement con floating-nodes property
* Implement con method find_by_id - finds a container with the i3 con id
* Implement con method find_by_window - finds a container with xcb window id

## Version 0.3.0

This release adds more useful container features for targetted use cases and minor bugfixes.

* Implement con_find_named method - finds containers via regexp
* Implement con_find_classed method - finds containers via regexp with WM_CLASS
* Implement con_leaves method - finds top-level windows
* Implement con window_class property - WM_CLASS as reported by the ipc
* Bugfix: children cons keep a reference on their parent

## Version 0.2.0

This release contains features that make scripting more expressive and powerful and major bugfixes related to language bindings.

* Bugfix: copy functions make deep copeies (fixes all gjs issues)
* GLib required version reduced to 2.32 (builds on Debian Wheezy)
* Added support for gtk-doc generation
* Detailed ipc signals allow for event filtering based on change
* con_command method - executes a command in the context of a container
* con_command_children method - executes a command in the context of container child nodes
* con_workspaces method - collects the workspaces of a tree
* con_find_focused method - finds a focused container in a container
* con_workspace method - finds the closest workspace

## Version 0.1.2

This is a minor bugfix release.

* Allow `i3ipc_connection_on` to connect to non-ipc events

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
