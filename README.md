# i3-ipc

An improved dynamic i3 ipc.

**Note:** i3-ipc is still in the design phase and is not yet ready for general use.

## About i3-ipc

i3's *interprocess communication* (or [ipc](http://i3wm.org/docs/ipc.html)) is the interface [i3wm](http://i3wm.org) uses to receive [commands](http://i3wm.org/docs/userguide.html#_list_of_commands) from client applications such as `i3-msg`. It also features a publish/subscribe mechanism for notifying interested parties of window manager events.

i3-ipc is a library to communicate with i3 via the ipc. The project is written in C and exposes bindings to various high-level programming languages such as Python, Perl, Lua, Ruby, and JavaScript. This is possible through the use of Gnome's [GObject Introspection](https://wiki.gnome.org/action/show/Projects/GObjectIntrospection?action=show&redirect=GObjectIntrospection) library and utilities.

This project has the following goals:

* Provide a complete, object-oriented library interface to i3 for general scripting.
* Replace older unmaintained ipc libraries such as [i3-py](https://github.com/ziberna/i3-py) as well as providing an ipc library for new languages such as Lua and JavaScript.
* Provide a unified api in a single library for easier maintenance of the ipc.

## Where we are now

The project as it is now should be considered a proof-of-concept only. Many aspects of the api will change as new features are implemented.

Currently only the Python bindings have been confirmed to work correctly.

The object-oriented interface has yet to be designed. The implementation of the interface will be guided by the community. If you have an opinion, you can send me an email or get in touch with me on the [i3 irc channel](irc://irc.twice-irc.de/i3) (nick: TonyC).

## Installation

Installation requires [autotools](https://en.wikipedia.org/wiki/GNU_build_system). Run `autogen.sh` to generate the files required for building and begin configuring the project. Then call `make` and the library will build in the `i3ipc` directory.

The following packages are required for building i3-ipc:

* libxcb and xcb-proto (the requirements for i3 itself should be ok).
* glib >= 2.38
* json-glib >= 0.16
* gobject-introspection >= 1.38

You can check if you have these packages with:

    pkg-config --exists --print-errors xcb xcb-proto json-glib-1.0 gobject-2.0 gobject-introspection-1.0

To use the Python bindings, set the following environment variables:

    export GI_TYPELIB_PATH=/path/to/i3-ipc/i3ipc
    export LD_LIBRARY_PATH=/path/to/i3-ipc/i3ipc/.libs:$LD_LIBRARY_PATH

This should be sufficient for creating a development environment for working on the project. I'm not completely confident in the build system yet, so build problems may be considered bugs. The library has been tested to build correctly on ArchLinux when the required packages are installed.

## Example

Here is an example use of the library with the Python bindings.

```python
#!/usr/bin/env python3

from gi.repository import i3ipc

# Create the Connection object that can be used to send commands and subscribe
# to events.
ipc = i3ipc.Connection()

# Establish a connection to the ipc. These steps may be eliminated in the final
# version of the library.
ipc.connect()

# Query the ipc for outputs. The result is a string that can be parsed as json.
outputs = ipc.get_outputs()

print('Got outputs:')
print(outputs)

# Send a command to be executed synchronously
ipc.command('focus left')

# Define a callback to be called when you switch workspaces.
def on_workspace(conn, data):
    #The first parameter is the connection to the ipc and the second is the
    #data of the event as a string that you can parse as json.
    print('Got workspace data:')
    print(data)

# Subscribe to the workspace event
ipc.on('workspace', on_workspace)

# Start the main loop and wait for events to come in.
ipc.main()
```

## Contributing

Development happens on Github. You can help by reporting bugs, making feature requests, and contributing patches by sending me a pull request.

### Task List

Here is a list of all the tasks that need to be done before releasing version 0.0.1 to the public. You can pick something from this list and work on it by sending me an email or making an issue so we can coordinate.

- [X] Commands
- [X] Subscriptions
- [X] Queries
- [ ] WORKSPACES reply/event
- [ ] OUTPUTS reply/event
- [ ] TREE reply
- [ ] MARKS reply
- [ ] BAR_CONFIG reply/event
- [X] VERSION reply
- [ ] mode event
- [ ] window event
- [ ] error handling

You can also help by fixing memory leaks, writing documentation, starring the repository, telling your friends, or giving to starving children in Uganda.

## License

Copyright © 2014, Tony Crisci

All rights reserved.

This work is available under the GNU General Public License (See COPYING).
