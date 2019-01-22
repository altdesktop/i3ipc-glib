#!/usr/bin/env python

import subprocess
from subprocess import Popen
import os
from os import listdir, path
from os.path import isfile, join
import sys
import re
import time
try:
    from shutil import which
except ImportError:

    def which(cmd):
        path = os.getenv('PATH')
        for p in path.split(os.path.pathsep):
            p = os.path.join(p, cmd)
            if os.path.exists(p) and os.access(p, os.X_OK):
                return p


if not hasattr(Popen, '__enter__'):

    def backported_enter(self):
        return self

    def backported_exit(self, type, value, traceback):
        if self.stdout:
            self.stdout.close()
        if self.stderr:
            self.stderr.close()
        try:  # Flushing a BufferedWriter may raise an error
            if self.stdin:
                self.stdin.close()
        finally:
            # Wait for the process to terminate, to avoid zombies.
            return
            self.wait()

    Popen.__enter__ = backported_enter
    Popen.__exit__ = backported_exit

PYTEST = 'pytest'
XVFB = 'Xvfb'
I3_BINARY = 'i3'
MESON = 'meson'
NINJA = 'ninja'
SOCKETDIR = '/tmp/.X11-unix'
FILEDIR = os.path.dirname(os.path.realpath(__file__))
BUILDDIR = os.path.join(FILEDIR, 'mesonbuild-test')


def check_dependencies():
    if not which(MESON):
        print('Meson is required to run tests')
        print('Command "%s" not found in PATH' % XVFB)
        sys.exit(127)

    if not which(NINJA):
        print('Ninja build is required to run tests')
        print('Command "%s" not found in PATH' % XVFB)
        sys.exit(127)

    if not which(XVFB):
        print('Xvfb is required to run tests')
        print('Command "%s" not found in PATH' % XVFB)
        sys.exit(127)

    if not which(I3_BINARY):
        print('i3 binary is required to run tests')
        print('Command "%s" not found in PATH' % I3_BINARY)
        sys.exit(127)

    if not which(PYTEST):
        print('pytest is required to run tests')
        print('Command %s not found in PATH' % PYTEST)
        sys.exit(127)


def get_open_display():
    if not os.path.isdir(SOCKETDIR):
        sys.stderr.write(
            'warning: could not find the X11 socket directory at {}. Using display 0.\n'
            .format(SOCKETDIR))
        sys.stderr.flush()
        return 0
    socket_re = re.compile(r'^X([0-9]+)$')
    socket_files = [f for f in listdir(SOCKETDIR) if socket_re.match(f)]
    displays = [int(socket_re.search(f).group(1)) for f in socket_files]
    open_display = min(
        [i for i in range(0,
                          max(displays or [0]) + 2) if i not in displays])
    return open_display


def start_server(display):
    xvfb = Popen([XVFB, ':%d' % display])
    # wait for the socket to make sure the server is running
    socketfile = path.join(SOCKETDIR, 'X%d' % display)
    tries = 0
    while True:
        if path.exists(socketfile):
            break
        else:
            tries += 1

            if tries > 100:
                print('could not start x server')
                xvfb.kill()
                sys.exit(1)

            time.sleep(0.1)

    return xvfb


def run_pytest(display):
    env = os.environ.copy()
    env['DISPLAY'] = ':%d' % display
    env['_I3IPC_TEST'] = '1'
    env['GI_TYPELIB_PATH'] = os.path.join(BUILDDIR, 'i3ipc-glib')
    env['LD_LIBRARY_PATH'] = os.path.join(BUILDDIR, 'i3ipc-glib')
    return subprocess.call(['valgrind', PYTEST], env=env)


def build_project():
    ret = subprocess.call([MESON, BUILDDIR])
    if ret != 0:
        return ret

    # TODO make incremental builds work
    ret = subprocess.call([NINJA, '-C', BUILDDIR, 'clean'])
    if ret != 0:
        return ret

    ret = subprocess.call([NINJA, '-C', BUILDDIR])
    return ret


def main():
    check_dependencies()
    ret = build_project()
    if ret != 0:
        sys.exit(ret)
    display = get_open_display()

    with start_server(display) as server:
        ret = run_pytest(display)
        server.terminate()

    sys.exit(ret)


if __name__ == '__main__':
    main()
