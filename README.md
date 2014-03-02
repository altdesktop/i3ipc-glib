# i3-ipc

An improved dynamic i3 ipc.

**Note:** i3-ipc is in the early stages of development and many things are likely to change.

## About i3-ipc

i3's *interprocess communication* (or [ipc](http://i3wm.org/docs/ipc.html)) is the interface [i3wm](http://i3wm.org) uses to receive [commands](http://i3wm.org/docs/userguide.html#_list_of_commands) from client applications such as `i3-msg`. It also features a publish/subscribe mechanism for notifying interested parties of window manager events.

i3-ipc is a library to communicate with i3 via the ipc. The project is written in C and exposes bindings to various high-level programming languages such as Python, Perl, Lua, Ruby, and JavaScript. This is possible through the use of Gnome's [GObject Introspection](https://wiki.gnome.org/action/show/Projects/GObjectIntrospection?action=show&redirect=GObjectIntrospection) library and utilities.

The goal of this project is to provide a basis for new projects in higher level languages for general scripting as well as an interface to i3 that fits well within a GLib-based stack.

## Installation

Building the library currently requires [autotools](https://en.wikipedia.org/wiki/GNU_build_system). Install the project with:

```shell
./autogen.sh && make
sudo make install prefix=/usr
```

The following packages are required for building i3-ipc:

* libxcb and xcb-proto (the requirements for i3 itself should be ok).
* glib >= 2.38
* json-glib >= 0.16

To use the Python bindings, you need the following packages:

* gobject-introspection >= 1.38
* pygobject >= 3 (python2 is supported, but has a slightly different api right now)

## Example

Here is an example use of the library with the Python bindings.

```python
#!/usr/bin/env python3

from gi.repository import i3ipc

# Create the Connection object that can be used to send commands and subscribe
# to events.
conn = i3ipc.Connection()

# Query the ipc for outputs. The result is a list that represents the parsed
# reply of a command like `i3-msg -t get_outputs`.
outputs = conn.get_outputs()

print('Active outputs:')

for output in filter(lambda o: o.active, outputs):
    print(output.name)

# Send a command to be executed synchronously.
conn.command('focus left')

# Define a callback to be called when you switch workspaces.
def on_workspace(self, e):
    # The first parameter is the connection to the ipc and the second is an object
    # with the data of the event sent from i3.
    if e.current:
        print('Windows on this workspace:')
        for w in e.current.descendents():
            print(w.props.name)

# Subscribe to the workspace event
conn.on('workspace', on_workspace)

# Start the main loop and wait for events to come in.
conn.main()
```

The library exposes a similar API to other scripting languages as well.

## Contributing

Development happens on Github. You can help by reporting bugs, making feature requests, and contributing patches by sending me a pull request.

### Task List

Here is a list of all the tasks that need to be done. You can pick something from this list and work on it by sending me an email or making an issue so we can coordinate.

- [X] Queries
- [X] Events
- [X] Subscriptions
- [ ] Async commands
- [ ] Container helper functions
- [ ] Language overrides

You can also help by fixing memory leaks, writing documentation, starring the repository, telling your friends, or giving to starving children in Uganda.

## License

This work is available under the GNU General Public License (See COPYING).

Copyright © 2014, Tony Crisci

All rights reserved.
