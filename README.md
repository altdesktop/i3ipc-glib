# i3-ipc

An improved dynamic i3 ipc.

**Note:** i3-ipc is in the early stages of development and many things are likely to change.

## About i3-ipc

i3's *interprocess communication* (or [ipc](http://i3wm.org/docs/ipc.html)) is the interface [i3wm](http://i3wm.org) uses to receive [commands](http://i3wm.org/docs/userguide.html#_list_of_commands) from client applications such as `i3-msg`. It also features a publish/subscribe mechanism for notifying interested parties of window manager events.

i3-ipc is a library to communicate with i3 via the ipc. The project is written in C and exposes bindings to various high-level programming languages such as Python, Perl, Lua, Ruby, and JavaScript. This is possible through the use of Gnome's [GObject Introspection](https://wiki.gnome.org/action/show/Projects/GObjectIntrospection?action=show&redirect=GObjectIntrospection) library and utilities.

The goal of this project is to provide a basis for new projects in higher level languages for general scripting as well as an interface to i3 that fits well within a GLib-based stack.

## Installation

Installation requires [autotools](https://en.wikipedia.org/wiki/GNU_build_system). Run `autogen.sh` to generate the files required for building and begin configuring the project. Then call `make` and the library will build in the `i3ipc` directory.

The following packages are required for building i3-ipc:

* libxcb and xcb-proto (the requirements for i3 itself should be ok).
* glib >= 2.38
* json-glib >= 0.16
* gobject-introspection >= 1.38 (for language bindings)

To use the bindings, set the following environment variables:

    export GI_TYPELIB_PATH=/path/to/i3-ipc/i3ipc
    export LD_LIBRARY_PATH=/path/to/i3-ipc/i3ipc/.libs:$LD_LIBRARY_PATH

This should be sufficient for creating a development environment for working on the project.

## Example

Here is an example use of the library with the Python bindings.

```python
#!/usr/bin/env python3

from gi.repository import i3ipc
from gi.repository.GLib import MainLoop

# Create the Connection object that can be used to send commands and subscribe
# to events.
ipc = i3ipc.Connection()

# Query the ipc for outputs. The result is a list that represents the parsed
# reply of a command like `i3-msg -t get_outputs`.
outputs = ipc.get_outputs()

print('Active outputs:')

for output in filter(lambda o: o.active, outputs):
    print(output.name)

# Send a command to be executed synchronously.
ipc.command('focus left')

# Define a callback to be called when you switch workspaces.
def on_workspace(self, e):
    # The first parameter is the connection to the ipc and the second is an object
    # with the data of the event sent from i3.
    if (e.current):
        print('Windows on this workspace:')
        for w in e.current.get_nodes():
            print(w.props.name)

# Subscribe to the workspace event
ipc.on('workspace', on_workspace)

# Start the main loop and wait for events to come in.
main = MainLoop()
main.run()
main.unref()
```

The library exposes the same API to many other scripting languages as well.

## Contributing

Development happens on Github. You can help by reporting bugs, making feature requests, and contributing patches by sending me a pull request.

### Task List

Here is a list of all the tasks that need to be done before releasing version 0.0.1 to the public. You can pick something from this list and work on it by sending me an email or making an issue so we can coordinate.

- [X] Queries
- [X] Events
- [X] Subscriptions
- [ ] Async commands
- [ ] Container helper functions
- [ ] Python wrapper for scripting
- [ ] Perl, JavaScript, Ruby, Lua wrappers

You can also help by fixing memory leaks, writing documentation, starring the repository, telling your friends, or giving to starving children in Uganda.

## License

This work is available under the GNU General Public License (See COPYING).

Copyright © 2014, Tony Crisci

All rights reserved.
