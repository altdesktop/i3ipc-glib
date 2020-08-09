# i3ipc-GLib

A C interface library to [i3wm](http://i3wm.org).

## About

i3's interprocess communication (or [ipc](http://i3wm.org/docs/ipc.html)) is the interface i3wm uses to receive [commands](http://i3wm.org/docs/userguide.html#_list_of_commands) from client applications such as `i3-msg`. It also features a publish/subscribe mechanism for notifying interested parties of window manager events.

i3ipc-GLib is a C library for controlling the window manager. This project is intended to be useful in applications such as status line generators, pagers, notification daemons, scripting wrappers, external controllers, dock windows, compositors, config templaters, and for debugging or testing the window manager itself.

The latest documentation can be found online [here](http://dubstepdish.com/i3ipc-glib).

[Chat](https://discord.gg/UdbXHVX)

### Projects using i3ipc-GLib

* [j4status](https://github.com/sardemff7/j4status) - shows the focused window in the status line.
* [xfce4-i3-workspaces-plugin](https://github.com/denesb/xfce4-i3-workspaces-plugin) - workspaces plugin for using xfce4 with i3wm
* [i3-easyfocus](https://github.com/cornerman/i3-easyfocus) - focus and select windows in i3

## Installation

Check the [releases](https://github.com/acrisci/i3ipc-glib/releases) page or your package manager for a package for your distro. Additional packages are available upon request.

Building the library requires [autotools](https://en.wikipedia.org/wiki/GNU_build_system). Install the project with:

```shell
./autogen.sh # --prefix=/usr might be required
make
sudo make install
```

The following packages are required for building i3-ipc:

* libxcb and xcb-proto
* glib >= 2.32
* gobject-introspection (optional for bindings)
* json-glib >= 0.14
* gtk-doc-tools

## Example

The i3ipc connection class extends from [GObject](https://developer.gnome.org/gobject/stable/).

```C
#include <glib/gprintf.h>
#include <i3ipc-glib/i3ipc-glib.h>

gint main() {
  i3ipcConnection *conn;
  gchar *reply;

  conn = i3ipc_connection_new(NULL, NULL);

  reply = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_COMMAND, "focus left", NULL);

  g_printf("Reply: %s\n", reply);

  g_free(reply);
  g_object_unref(conn);

  return 0;
}
```

Compile with `gcc -o example example.c $(pkg-config --libs --cflags i3ipc-glib-1.0)`.

## Contributing

Patches are welcome by pull request to fix bugs and add features.

### Task List

Here is a list of tasks that could be done.

- [ ] Async commands

## License

This work is available under the GNU General Public License (See COPYING).

Copyright © 2014, Tony Crisci

All rights reserved.
