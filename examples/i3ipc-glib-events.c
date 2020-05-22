#include <glib.h>
#include <i3ipc-glib/i3ipc-glib.h>
#include <stdio.h>

void print_change(i3ipcConnection *self, i3ipcWindowEvent *e, gpointer user_data) {
    printf("Change : %s\n", e->change);
}

int main() {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    i3ipcConnection *conn;
    i3ipcCommandReply *cr;

    conn = i3ipc_connection_new(NULL, NULL);
    cr = i3ipc_connection_subscribe(conn, I3IPC_EVENT_WINDOW, NULL);
    i3ipc_command_reply_free(cr);

    g_object_connect(conn, "signal::window", G_CALLBACK(print_change), NULL, NULL);

    g_main_loop_run(loop);
    g_object_unref(conn);
    return 0;
}
