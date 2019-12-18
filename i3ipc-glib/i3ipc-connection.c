/*
 * This file is part of i3-ipc.
 *
 * i3-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * i3-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with i3-ipc.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright © 2014, Tony Crisci
 */

#include <fcntl.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <xcb/xcb.h>

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include <gio/gio.h>

#include "i3ipc-con-private.h"
#include "i3ipc-connection.h"
#include "i3ipc-enum-types.h"
#include "i3ipc-event-types.h"
#include "i3ipc-reply-types.h"

typedef struct i3_ipc_header {
    /* 6 = strlen(I3_IPC_MAGIC) */
    char magic[6];
    uint32_t size;
    uint32_t type;
} __attribute__((packed)) i3_ipc_header_t;

enum {
    PROP_0,

    PROP_SUBSCRIPTIONS,
    PROP_SOCKET_PATH,
    PROP_CONNECTED,

    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

enum { WORKSPACE, OUTPUT, MODE, WINDOW, BARCONFIG_UPDATE, BINDING, IPC_SHUTDOWN, LAST_SIGNAL };

static guint connection_signals[LAST_SIGNAL] = {0};

struct _i3ipcConnectionPrivate {
    i3ipcEvent subscriptions;
    gchar *socket_path;
    gboolean connected;
    GError *init_error;
    GMainLoop *main_loop;
    GIOChannel *cmd_channel;
    GIOChannel *sub_channel;
};

static void i3ipc_connection_initable_iface_init(GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE(i3ipcConnection, i3ipc_connection, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,
                                              i3ipc_connection_initable_iface_init));

static void i3ipc_connection_set_property(GObject *object, guint property_id, const GValue *value,
                                          GParamSpec *pspec) {
    i3ipcConnection *self = I3IPC_CONNECTION(object);

    switch (property_id) {
    case PROP_SOCKET_PATH:
        g_free(self->priv->socket_path);
        self->priv->socket_path = g_value_dup_string(value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void i3ipc_connection_get_property(GObject *object, guint property_id, GValue *value,
                                          GParamSpec *pspec) {
    i3ipcConnection *self = I3IPC_CONNECTION(object);

    switch (property_id) {
    case PROP_SUBSCRIPTIONS:
        g_value_set_flags(value, self->priv->subscriptions);
        break;

    case PROP_SOCKET_PATH:
        g_value_set_string(value, self->priv->socket_path);
        break;

    case PROP_CONNECTED:
        g_value_set_boolean(value, self->priv->connected);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void i3ipc_connection_dispose(GObject *gobject) {
    i3ipcConnection *self = I3IPC_CONNECTION(gobject);

    g_clear_error(&self->priv->init_error);

    if (self->priv->connected) {
        g_io_channel_shutdown(self->priv->cmd_channel, TRUE, NULL);
        g_io_channel_shutdown(self->priv->sub_channel, TRUE, NULL);
        self->priv->sub_channel = (g_io_channel_unref(self->priv->sub_channel), NULL);
        self->priv->cmd_channel = (g_io_channel_unref(self->priv->cmd_channel), NULL);
    }

    G_OBJECT_CLASS(i3ipc_connection_parent_class)->dispose(gobject);
}

static void i3ipc_connection_finalize(GObject *gobject) {
    i3ipcConnection *self = I3IPC_CONNECTION(gobject);

    g_free(self->priv->socket_path);

    G_OBJECT_CLASS(i3ipc_connection_parent_class)->finalize(gobject);
}

static void i3ipc_connected_constructed(GObject *gobject) {
    i3ipcConnection *self = I3IPC_CONNECTION(gobject);

    self->priv->init_error = NULL;

    g_initable_init((GInitable *)self, NULL, &self->priv->init_error);

    G_OBJECT_CLASS(i3ipc_connection_parent_class)->constructed(gobject);
}

static void i3ipc_connection_class_init(i3ipcConnectionClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->set_property = i3ipc_connection_set_property;
    gobject_class->get_property = i3ipc_connection_get_property;
    gobject_class->dispose = i3ipc_connection_dispose;
    gobject_class->finalize = i3ipc_connection_finalize;
    gobject_class->constructed = i3ipc_connected_constructed;

    obj_properties[PROP_SUBSCRIPTIONS] =
        g_param_spec_flags("subscriptions", "Connection subscriptions",
                           "The subscriptions this connection is subscribed to", I3IPC_TYPE_EVENT,
                           0, G_PARAM_READABLE);

    obj_properties[PROP_SOCKET_PATH] = g_param_spec_string(
        "socket-path", "Connection socket path",
        "The path of the unix socket the connection is connected to", NULL, /* default */
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_CONNECTED] = g_param_spec_boolean(
        "connected", "Connection connected",
        "Whether or not a connection has been established to the ipc", FALSE, G_PARAM_READABLE);

    g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);

    /**
     * i3ipcConnection::workspace:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: The workspace event object
     *
     * Sent when the user switches to a different workspace, when a new workspace
     * is initialized or when a workspace is removed (because the last client
     * vanished).
     */
    connection_signals[WORKSPACE] =
        g_signal_new("workspace",                           /* signal_name */
                     I3IPC_TYPE_CONNECTION,                 /* itype */
                     G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, /* signal_flags */
                     0,                                     /* class_offset */
                     NULL,                                  /* accumulator */
                     NULL,                                  /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,        /* c_marshaller */
                     G_TYPE_NONE,                           /* return_type */
                     1, I3IPC_TYPE_WORKSPACE_EVENT);        /* n_params */

    /**
     * i3ipcConnection::output:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: The output event object
     *
     * Sent when RandR issues a change notification (of either screens, outputs,
     * CRTCs or output properties).
     */
    connection_signals[OUTPUT] =
        g_signal_new("output",                               /* signal_name */
                     I3IPC_TYPE_CONNECTION,                  /* itype */
                     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                     0,                                      /* class_offset */
                     NULL,                                   /* accumulator */
                     NULL,                                   /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,         /* c_marshaller */
                     G_TYPE_NONE,                            /* return_type */
                     1, I3IPC_TYPE_GENERIC_EVENT);           /* n_params */

    /**
     * i3ipcConnection::mode:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: The mode event object
     *
     * Sent whenever i3 changes its binding mode.
     */
    connection_signals[MODE] =
        g_signal_new("mode",                                 /* signal_name */
                     I3IPC_TYPE_CONNECTION,                  /* itype */
                     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                     0,                                      /* class_offset */
                     NULL,                                   /* accumulator */
                     NULL,                                   /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,         /* c_marshaller */
                     G_TYPE_NONE,                            /* return_type */
                     1, I3IPC_TYPE_GENERIC_EVENT);           /* n_params */

    /**
     * i3ipcConnection::window:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: The window event object
     *
     * Sent when a client’s window is successfully reparented (that is when i3
     * has finished fitting it into a container).
     */
    connection_signals[WINDOW] =
        g_signal_new("window",                               /* signal_name */
                     I3IPC_TYPE_CONNECTION,                  /* itype */
                     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                     0,                                      /* class_offset */
                     NULL,                                   /* accumulator */
                     NULL,                                   /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,         /* c_marshaller */
                     G_TYPE_NONE,                            /* return_type */
                     1, I3IPC_TYPE_WINDOW_EVENT);            /* n_params */

    /**
     * i3ipcConnection::barconfig_update:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: The barconfig_update event object
     *
     * Sent when the hidden_state or mode field in the barconfig of any bar
     * instance was updated.
     */
    connection_signals[BARCONFIG_UPDATE] =
        g_signal_new("barconfig_update",                    /* signal_name */
                     I3IPC_TYPE_CONNECTION,                 /* itype */
                     G_SIGNAL_RUN_FIRST,                    /* signal_flags */
                     0,                                     /* class_offset */
                     NULL,                                  /* accumulator */
                     NULL,                                  /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,        /* c_marshaller */
                     G_TYPE_NONE,                           /* return_type */
                     1, I3IPC_TYPE_BARCONFIG_UPDATE_EVENT); /* n_params */

    /**
     * i3ipcConnection::binding:
     * @self: the #i3ipcConnection on which the signal was emitted
     * @e: the binding event object
     *
     * Sent when a binding was triggered with the keyboard or mouse because of
     * some user input.
     */
    connection_signals[BINDING] =
        g_signal_new("binding",                              /* signal_name */
                     I3IPC_TYPE_CONNECTION,                  /* itype */
                     G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                     0,                                      /* class_offset */
                     NULL,                                   /* accumulator */
                     NULL,                                   /* accu_data */
                     g_cclosure_marshal_VOID__BOXED,         /* c_marshaller */
                     G_TYPE_NONE,                            /* return_type */
                     1,                                      /* n_params */
                     I3IPC_TYPE_BINDING_EVENT);

    /**
     * i3ipcConnection::ipc_shutdown:
     * @self: the #i3ipcConnection on which the signal was emitted
     *
     * Sent when the Connection receives notice that the ipc has shut down.
     */
    connection_signals[IPC_SHUTDOWN] =
        g_signal_new("ipc_shutdown",                /* signal_name */
                     I3IPC_TYPE_CONNECTION,         /* itype */
                     G_SIGNAL_RUN_FIRST,            /* signal_flags */
                     0,                             /* class_offset */
                     NULL,                          /* accumulator */
                     NULL,                          /* accu_data */
                     g_cclosure_marshal_VOID__VOID, /* c_marshaller */
                     G_TYPE_NONE,                   /* return_type */
                     0);                            /* n_params */

    g_type_class_add_private(klass, sizeof(i3ipcConnectionPrivate));
}

static void i3ipc_connection_init(i3ipcConnection *self) {
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, I3IPC_TYPE_CONNECTION, i3ipcConnectionPrivate);
}

/**
 * i3ipc_connection_new:
 * @socket_path: (allow-none): the path of the socket to connect to
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Allocates a new #i3ipcConnection
 *
 * Returns: (transfer full): a new #i3ipcConnection
 */
i3ipcConnection *i3ipc_connection_new(const gchar *socket_path, GError **err) {
    i3ipcConnection *conn;
    GError *tmp_error = NULL;

    conn =
        g_initable_new(I3IPC_TYPE_CONNECTION, NULL, &tmp_error, "socket-path", socket_path, NULL);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    return conn;
}

static gchar *i3ipc_connection_get_socket_path(i3ipcConnection *self, GError **err) {
    GError *tmp_error = NULL;
    xcb_intern_atom_reply_t *atom_reply;
    gchar *socket_path;
    const char *atomname = "I3_SOCKET_PATH";
    size_t content_max_words = 256;
    xcb_screen_t *screen;
    xcb_intern_atom_cookie_t atom_cookie;
    xcb_window_t root;
    xcb_get_property_cookie_t prop_cookie;
    xcb_get_property_reply_t *prop_reply;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    if (self->priv->socket_path != NULL)
        return self->priv->socket_path;

    xcb_connection_t *conn = xcb_connect(NULL, NULL);

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    root = screen->root;

    atom_cookie = xcb_intern_atom(conn, 0, strlen(atomname), atomname);
    atom_reply = xcb_intern_atom_reply(conn, atom_cookie, NULL);

    if (atom_reply == NULL) {
        /* TODO i3ipc custom errors */
        g_free(atom_reply);
        tmp_error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "socket path atom reply is null");
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    prop_cookie = xcb_get_property_unchecked(conn, FALSE,               /* _delete */
                                             root,                      /* window */
                                             atom_reply->atom,          /* property */
                                             XCB_GET_PROPERTY_TYPE_ANY, /* type */
                                             0,                         /* long_offset */
                                             content_max_words          /* long_length */
    );

    prop_reply = xcb_get_property_reply(conn, prop_cookie, NULL);
    int len = xcb_get_property_value_length(prop_reply);

    if (prop_reply == NULL || len == 0) {
        /* TODO i3ipc custom errors */
        g_free(atom_reply);
        g_free(prop_reply);
        tmp_error =
            g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "socket path property reply is null");
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    socket_path = malloc(len + 1);

    strncpy(socket_path, (char *)xcb_get_property_value(prop_reply), len);
    socket_path[len] = '\0';

    g_free(atom_reply);
    g_free(prop_reply);
    xcb_disconnect(conn);

    return socket_path;
}

/*
 * Connects to the i3 IPC socket and returns the file descriptor for the
 * socket.
 */
static int get_file_descriptor(const char *socket_path, GError **err) {
    GError *tmp_error = NULL;

    int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

    if (sockfd == -1) {
        tmp_error = g_error_new(G_IO_ERROR, g_io_error_from_errno(errno),
                                "Could not create socket (%s)\n", strerror(errno));
        g_propagate_error(err, tmp_error);
        return -1;
    }

    (void)fcntl(sockfd, F_SETFD, FD_CLOEXEC);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        tmp_error = g_error_new(G_IO_ERROR, g_io_error_from_errno(errno),
                                "Could not connect to i3 (%s)\n", strerror(errno));
        g_propagate_error(err, tmp_error);
        return -1;
    }

    return sockfd;
}

/*
 * Blockingly receives a message from the ipc socket. Returns the status of the last read.
 */
static GIOStatus ipc_recv_message(GIOChannel *channel, uint32_t *message_type,
                                  uint32_t *reply_length, gchar **reply, GError **err) {
    /* Read the message header first */
    GError *tmp_error = NULL;
    const uint32_t to_read = strlen(I3IPC_MAGIC) + sizeof(uint32_t) + sizeof(uint32_t);
    char msg[to_read];
    char *walk = msg;
    GIOStatus status;

    status = g_io_channel_flush(channel, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return status;
    }

    gsize read_bytes = 0;
    while (read_bytes < to_read) {
        status = g_io_channel_read_chars(channel, msg + read_bytes, to_read - read_bytes,
                                         &read_bytes, &tmp_error);

        if (tmp_error != NULL) {
            g_propagate_error(err, tmp_error);
            return status;
        }

        if (status == G_IO_STATUS_EOF)
            return status;
    }

    if (memcmp(walk, I3IPC_MAGIC, strlen(I3IPC_MAGIC)) != 0) {
        /* TODO i3ipc custom errors */
        tmp_error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "Invalid magic in reply");
        g_propagate_error(err, tmp_error);
        return status;
    }

    walk += strlen(I3IPC_MAGIC);
    memcpy(reply_length, walk, sizeof(uint32_t));
    walk += sizeof(uint32_t);
    if (message_type != NULL)
        memcpy(message_type, walk, sizeof(uint32_t));

    *reply = malloc(*reply_length + 1);

    read_bytes = 0;
    while (read_bytes < *reply_length) {
        status = g_io_channel_read_chars(channel, *reply + read_bytes, *reply_length - read_bytes,
                                         &read_bytes, &tmp_error);

        if (tmp_error != NULL) {
            g_propagate_error(err, tmp_error);
            return status;
        }

        if (status == G_IO_STATUS_EOF)
            return status;
    }

    return status;
}

/*
 * Callback function for when a channel receives data from the ipc socket.
 * Emits the corresponding signal with the reply.
 */
static gboolean ipc_on_data(GIOChannel *channel, GIOCondition condition, i3ipcConnection *conn) {
    if (condition != G_IO_IN)
        return TRUE;

    GIOStatus status;
    uint32_t reply_length;
    uint32_t reply_type;
    gchar *reply;
    GError *err = NULL;
    JsonParser *parser;
    JsonObject *json_reply;

    status = ipc_recv_message(channel, &reply_type, &reply_length, &reply, &err);

    if (status == G_IO_STATUS_EOF) {
        g_signal_emit(conn, connection_signals[IPC_SHUTDOWN], 0);

        if (conn->priv->main_loop != NULL)
            i3ipc_connection_main_quit(conn);

        return FALSE;
    }

    if (err) {
        g_warning("could not get event reply\n");
        g_error_free(err);
        g_free(reply);
        return TRUE;
    }

    reply[reply_length] = '\0';

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &err);

    if (err) {
        g_warning("could not parse event reply json (%s)\n", err->message);
        g_error_free(err);
        g_free(reply);
        g_object_unref(parser);
        return TRUE;
    }

    json_reply = json_node_get_object(json_parser_get_root(parser));

    switch (1 << (reply_type & 0x7F)) {
    case I3IPC_EVENT_WORKSPACE: {
        i3ipcWorkspaceEvent *e = g_slice_new0(i3ipcWorkspaceEvent);

        e->change = g_strdup(json_object_get_string_member(json_reply, "change"));

        if (json_object_has_member(json_reply, "current") &&
            !json_object_get_null_member(json_reply, "current"))
            e->current =
                i3ipc_con_new(NULL, json_object_get_object_member(json_reply, "current"), conn);

        if (json_object_has_member(json_reply, "old") &&
            !json_object_get_null_member(json_reply, "old"))
            e->old = i3ipc_con_new(NULL, json_object_get_object_member(json_reply, "old"), conn);

        g_signal_emit(conn, connection_signals[WORKSPACE], g_quark_from_string(e->change), e);
        break;
    }

    case I3IPC_EVENT_OUTPUT: {
        i3ipcGenericEvent *e = g_slice_new0(i3ipcGenericEvent);

        e->change = g_strdup(json_object_get_string_member(json_reply, "change"));

        g_signal_emit(conn, connection_signals[OUTPUT], g_quark_from_string(e->change), e);
        break;
    }

    case I3IPC_EVENT_MODE: {
        i3ipcGenericEvent *e = g_slice_new0(i3ipcGenericEvent);

        e->change = g_strdup(json_object_get_string_member(json_reply, "change"));

        g_signal_emit(conn, connection_signals[MODE], g_quark_from_string(e->change), e);
        break;
    }

    case I3IPC_EVENT_WINDOW: {
        i3ipcWindowEvent *e = g_slice_new0(i3ipcWindowEvent);

        e->change = g_strdup(json_object_get_string_member(json_reply, "change"));

        if (json_object_has_member(json_reply, "container") &&
            !json_object_get_null_member(json_reply, "container"))
            e->container =
                i3ipc_con_new(NULL, json_object_get_object_member(json_reply, "container"), conn);

        g_signal_emit(conn, connection_signals[WINDOW], g_quark_from_string(e->change), e);
        break;
    }

    case I3IPC_EVENT_BARCONFIG_UPDATE: {
        i3ipcBarconfigUpdateEvent *e = g_slice_new0(i3ipcBarconfigUpdateEvent);

        e->id = g_strdup(json_object_get_string_member(json_reply, "id"));
        e->hidden_state = g_strdup(json_object_get_string_member(json_reply, "hidden_state"));
        e->mode = g_strdup(json_object_get_string_member(json_reply, "mode"));

        g_signal_emit(conn, connection_signals[BARCONFIG_UPDATE], 0, e);
        break;
    }
    case I3IPC_EVENT_BINDING: {
        i3ipcBindingEvent *e = g_slice_new0(i3ipcBindingEvent);

        e->change = g_strdup(json_object_get_string_member(json_reply, "change"));

        JsonObject *json_binding_info = json_object_get_object_member(json_reply, "binding");
        e->binding = g_slice_new0(i3ipcBindingInfo);
        e->binding->command = g_strdup(json_object_get_string_member(json_binding_info, "command"));
        e->binding->input_code = json_object_get_int_member(json_binding_info, "input_code");
        e->binding->input_type =
            g_strdup(json_object_get_string_member(json_binding_info, "input_type"));
        e->binding->symbol = g_strdup(json_object_get_string_member(json_binding_info, "symbol"));

        JsonArray *mods = json_object_get_array_member(json_binding_info, "mods");
        gint mods_len = json_array_get_length(mods);

        for (int i = 0; i < mods_len; i += 1) {
            e->binding->mods =
                g_slist_append(e->binding->mods, g_strdup(json_array_get_string_element(mods, i)));
        }

        g_signal_emit(conn, connection_signals[BINDING], g_quark_from_string(e->change), e);
        break;
    }

    default:
        g_warning("got unknown event\n");
        break;
    }

    g_object_unref(parser);
    g_free(reply);

    return TRUE;
}

static gboolean i3ipc_connection_initable_init(GInitable *initable, GCancellable *cancellable,
                                               GError **err) {
    i3ipcConnection *self = I3IPC_CONNECTION(initable);
    GError *tmp_error = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

    self->priv->socket_path = i3ipc_connection_get_socket_path(self, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return FALSE;
    }

    int cmd_fd = get_file_descriptor(self->priv->socket_path, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return FALSE;
    }

    self->priv->cmd_channel = g_io_channel_unix_new(cmd_fd);

    int sub_fd = get_file_descriptor(self->priv->socket_path, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return FALSE;
    }

    self->priv->sub_channel = g_io_channel_unix_new(sub_fd);

    g_io_channel_set_encoding(self->priv->cmd_channel, NULL, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return FALSE;
    }

    g_io_channel_set_encoding(self->priv->sub_channel, NULL, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return FALSE;
    }

    g_io_add_watch(self->priv->sub_channel, G_IO_IN, (GIOFunc)ipc_on_data, self);

    self->priv->connected = TRUE;

    return TRUE;
}

static void i3ipc_connection_initable_iface_init(GInitableIface *iface) {
    iface->init = i3ipc_connection_initable_init;
}

/*
 * Sends a message to the ipc.
 */
static GIOStatus ipc_send_message(GIOChannel *channel, const uint32_t message_size,
                                  const uint32_t message_type, const gchar *command, GError **err) {
    GError *tmp_error = NULL;
    GIOStatus status;
    gsize sent_bytes;

    const i3_ipc_header_t header = {
        .magic = {'i', '3', '-', 'i', 'p', 'c'}, .size = message_size, .type = message_type};

    sent_bytes = 0;

    while (sent_bytes < sizeof(i3_ipc_header_t)) {
        status =
            g_io_channel_write_chars(channel, ((void *)&header) + sent_bytes,
                                     sizeof(i3_ipc_header_t) - sent_bytes, &sent_bytes, &tmp_error);

        if (tmp_error != NULL) {
            g_propagate_error(err, tmp_error);
            return status;
        }
    }

    sent_bytes = 0;

    while (sent_bytes < message_size) {
        status = g_io_channel_write_chars(channel, command + sent_bytes, message_size - sent_bytes,
                                          &sent_bytes, &tmp_error);

        if (tmp_error != NULL) {
            g_propagate_error(err, tmp_error);
            return status;
        }
    }

    return status;
}

/**
 * i3ipc_connection_message:
 * @self: A #i3ipcConnection
 * @message_type: The type of message to send to i3
 * @payload: (allow-none): The body of the command
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Sends a command to the ipc synchronously.
 *
 * Returns: (transfer full): The reply of the ipc as a string
 */
gchar *i3ipc_connection_message(i3ipcConnection *self, i3ipcMessageType message_type,
                                const gchar *payload, GError **err) {
    GError *tmp_error = NULL;
    GIOStatus status;
    uint32_t reply_length;
    uint32_t reply_type;
    gchar *reply = NULL;

    if (self->priv->init_error != NULL) {
        g_propagate_error(err, self->priv->init_error);
        return NULL;
    }

    g_return_val_if_fail(!self->priv->connected || err == NULL || *err == NULL, NULL);

    if (payload == NULL)
        payload = "";

    GIOChannel *channel = (message_type == I3IPC_MESSAGE_TYPE_SUBSCRIBE ? self->priv->sub_channel
                                                                        : self->priv->cmd_channel);

    ipc_send_message(channel, strlen(payload), message_type, payload, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    status = ipc_recv_message(channel, &reply_type, &reply_length, &reply, &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    if (status == G_IO_STATUS_NORMAL)
        reply[reply_length] = '\0';

    return reply;
}

/**
 * i3ipc_connection_command:
 * @self: A #i3ipcConnection
 * @command: The command to send to i3
 * @err: (allow-none): return location of a GError, or NULL
 *
 * Sends a command to the ipc synchronously.
 *
 * Returns: (transfer full) (element-type i3ipcCommandReply): a list of #i3ipcCommandReply structs
 * for each command that was parsed
 */
GSList *i3ipc_connection_command(i3ipcConnection *self, const gchar *command, GError **err) {
    JsonParser *parser;
    GError *tmp_error = NULL;
    GSList *retval = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_COMMAND, command, &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    JsonArray *json_replies = json_node_get_array(json_parser_get_root(parser));

    guint reply_count = json_array_get_length(json_replies);

    for (int i = 0; i < reply_count; i += 1) {
        JsonObject *json_reply = json_array_get_object_element(json_replies, i);
        i3ipcCommandReply *cmd_reply = g_slice_new0(i3ipcCommandReply);

        cmd_reply->success = json_object_get_boolean_member(json_reply, "success");

        cmd_reply->parse_error = (json_object_has_member(json_reply, "parse_error")
                                      ? json_object_get_boolean_member(json_reply, "parse_error")
                                      : FALSE);

        cmd_reply->error = (json_object_has_member(json_reply, "error")
                                ? g_strdup(json_object_get_string_member(json_reply, "error"))
                                : NULL);

        cmd_reply->_id = (json_object_has_member(json_reply, "id")
                                ? g_strdup(json_object_get_string_member(json_reply, "id"))
                                : 0);

        retval = g_slist_append(retval, cmd_reply);
    }

    g_object_unref(parser);
    g_free(reply);

    return retval;
}

/**
 * i3ipc_connection_subscribe:
 * @self: A #i3ipcConnection
 * @events: The name of an IPC event
 * @err: (allow-none): The location of a GError or NULL
 *
 * Subscribes to the events given by the flags
 *
 * Returns: (transfer full): The ipc reply
 */
i3ipcCommandReply *i3ipc_connection_subscribe(i3ipcConnection *self, i3ipcEvent events,
                                              GError **err) {
    JsonParser *parser;
    JsonGenerator *generator;
    JsonBuilder *builder;
    JsonNode *tmp_node;
    gchar *reply;
    gchar *payload;
    GError *tmp_error = NULL;
    i3ipcCommandReply *retval;

    if (!(events & ~self->priv->subscriptions)) {
        /* No new events */
        retval = g_slice_new0(i3ipcCommandReply);
        retval->success = TRUE;
        return retval;
    }

    builder = json_builder_new();
    json_builder_begin_array(builder);

    if (events & (I3IPC_EVENT_WINDOW & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "window");

    if (events & (I3IPC_EVENT_BARCONFIG_UPDATE & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "barconfig_update");

    if (events & (I3IPC_EVENT_MODE & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "mode");

    if (events & (I3IPC_EVENT_OUTPUT & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "output");

    if (events & (I3IPC_EVENT_WORKSPACE & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "workspace");

    if (events & (I3IPC_EVENT_BINDING & ~self->priv->subscriptions))
        json_builder_add_string_value(builder, "binding");

    json_builder_end_array(builder);

    generator = json_generator_new();
    tmp_node = json_builder_get_root(builder);
    json_generator_set_root(generator, tmp_node);
    payload = json_generator_to_data(generator, NULL);
    json_node_free(tmp_node);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_SUBSCRIBE, payload, &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_free(payload);
        g_object_unref(generator);
        g_object_unref(builder);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_free(payload);
        g_object_unref(generator);
        g_object_unref(builder);
        g_object_unref(parser);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    JsonObject *json_reply = json_node_get_object(json_parser_get_root(parser));

    retval = g_slice_new0(i3ipcCommandReply);
    retval->success = json_object_get_boolean_member(json_reply, "success");

    g_free(reply);
    g_free(payload);
    g_object_unref(builder);
    g_object_unref(generator);
    g_object_unref(parser);

    if (retval->success)
        self->priv->subscriptions |= events;

    return retval;
}

/**
 * i3ipc_connection_on:
 * @self: an #i3ipcConnection
 * @event: the event to subscribe to
 * @callback: (scope notified): the callback to run on the event
 * @err: (allow-none): return location of a GError, or NULL
 *
 * A convenience function for bindings to subscribe an event with a callback
 *
 * Returns: (transfer none): the #i3ipcConnection for chaining
 */
i3ipcConnection *i3ipc_connection_on(i3ipcConnection *self, const gchar *event, GClosure *callback,
                                     GError **err) {
    GError *tmp_error = NULL;
    i3ipcCommandReply *cmd_reply;
    gchar **event_details;
    i3ipcEvent flags = 0;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    g_closure_ref(callback);
    g_closure_sink(callback);

    event_details = g_strsplit(event, "::", 0);

    if (strcmp(event_details[0], "workspace") == 0)
        flags = I3IPC_EVENT_WORKSPACE;
    else if (strcmp(event_details[0], "output") == 0)
        flags = I3IPC_EVENT_OUTPUT;
    else if (strcmp(event_details[0], "window") == 0)
        flags = I3IPC_EVENT_WINDOW;
    else if (strcmp(event_details[0], "mode") == 0)
        flags = I3IPC_EVENT_MODE;
    else if (strcmp(event_details[0], "barconfig_update") == 0)
        flags = I3IPC_EVENT_BARCONFIG_UPDATE;
    else if (strcmp(event_details[0], "binding") == 0)
        flags = I3IPC_EVENT_BINDING;

    if (flags) {
        cmd_reply = i3ipc_connection_subscribe(self, flags, &tmp_error);
        i3ipc_command_reply_free(cmd_reply);

        if (tmp_error != NULL) {
            g_strfreev(event_details);
            g_propagate_error(err, tmp_error);
            return NULL;
        }
    }

    g_signal_connect_closure(self, event, callback, TRUE);

    g_strfreev(event_details);

    return self;
}

/**
 * i3ipc_connection_get_workspaces:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location of a GError, or NULL
 *
 * Gets the current workspaces. The reply will be list workspaces
 *
 * Returns: (transfer full) (element-type i3ipcWorkspaceReply): a list of workspaces
 */
GSList *i3ipc_connection_get_workspaces(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    GError *tmp_error = NULL;
    GSList *retval = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_WORKSPACES, "", &tmp_error);

    if (tmp_error != NULL) {
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));

    int num_workspaces = json_reader_count_elements(reader);

    for (int i = 0; i < num_workspaces; i += 1) {
        i3ipcWorkspaceReply *workspace = g_slice_new0(i3ipcWorkspaceReply);
        workspace->rect = g_slice_new0(i3ipcRect);

        json_reader_read_element(reader, i);

        json_reader_read_member(reader, "name");
        workspace->name = g_strdup(json_reader_get_string_value(reader));
        json_reader_end_member(reader);

        json_reader_read_member(reader, "num");
        workspace->num =
            (json_reader_get_null_value(reader) ? -1 : json_reader_get_int_value(reader));
        json_reader_end_member(reader);

        json_reader_read_member(reader, "visible");
        workspace->visible = json_reader_get_boolean_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "focused");
        workspace->focused = json_reader_get_boolean_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "urgent");
        workspace->urgent = json_reader_get_boolean_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "output");
        workspace->output = g_strdup(json_reader_get_string_value(reader));
        json_reader_end_member(reader);

        json_reader_read_member(reader, "rect");
        /* begin rect */
        json_reader_read_member(reader, "x");
        workspace->rect->x = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "y");
        workspace->rect->y = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "width");
        workspace->rect->width = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "height");
        workspace->rect->height = json_reader_get_int_value(reader);
        json_reader_end_member(reader);
        /* end rect */

        json_reader_end_member(reader);

        json_reader_end_element(reader);

        retval = g_slist_prepend(retval, workspace);
    }

    g_free(reply);
    g_object_unref(reader);
    g_object_unref(parser);

    return retval;
}

/**
 * i3ipc_connection_get_outputs:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location of a GError, or NULL
 *
 * Gets the current outputs. The reply will be a list of outputs
 *
 * Returns: (transfer full) (element-type i3ipcOutputReply): a list of outputs
 */
GSList *i3ipc_connection_get_outputs(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    GError *tmp_error = NULL;
    GSList *retval = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_OUTPUTS, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));

    int num_outputs = json_reader_count_elements(reader);

    for (int i = 0; i < num_outputs; i += 1) {
        i3ipcOutputReply *output = g_slice_new(i3ipcOutputReply);
        output->rect = g_slice_new0(i3ipcRect);

        json_reader_read_element(reader, i);

        json_reader_read_member(reader, "name");
        output->name = g_strdup(json_reader_get_string_value(reader));
        json_reader_end_member(reader);

        json_reader_read_member(reader, "active");
        output->active = json_reader_get_boolean_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "current_workspace");
        output->current_workspace = g_strdup(json_reader_get_string_value(reader));
        json_reader_end_member(reader);

        json_reader_read_member(reader, "rect");
        /* begin rect */
        json_reader_read_member(reader, "x");
        output->rect->x = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "y");
        output->rect->y = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "width");
        output->rect->width = json_reader_get_int_value(reader);
        json_reader_end_member(reader);

        json_reader_read_member(reader, "height");
        output->rect->height = json_reader_get_int_value(reader);
        json_reader_end_member(reader);
        /* end rect */

        json_reader_end_member(reader);

        json_reader_end_element(reader);

        retval = g_slist_prepend(retval, output);
    }

    g_free(reply);
    g_object_unref(reader);
    g_object_unref(parser);

    return retval;
}
/**
 * i3ipc_connection_get_tree:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets the layout tree. i3 uses a tree as data structure which includes every
 * container.
 *
 * Returns: (transfer full): the root container
 */
i3ipcCon *i3ipc_connection_get_tree(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    GError *tmp_error = NULL;
    i3ipcCon *retval;
    gchar *reply;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_TREE, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    retval = i3ipc_con_new(NULL, json_node_get_object(json_parser_get_root(parser)), self);

    g_object_unref(parser);
    g_free(reply);

    return retval;
}

/**
 * i3ipc_connection_get_marks:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets a list of marks (identifiers for containers to easily jump to them
 * later). The reply will be a list of window marks.
 *
 * Returns: (transfer full) (element-type utf8): a list of strings representing marks
 */
GSList *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    GError *tmp_error = NULL;
    GSList *retval = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_MARKS, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));

    int num_elements = json_reader_count_elements(reader);

    for (int i = 0; i < num_elements; i += 1) {
        json_reader_read_element(reader, i);
        retval = g_slist_prepend(retval, g_strdup(json_reader_get_string_value(reader)));
        json_reader_end_element(reader);
    }

    g_free(reply);
    g_object_unref(reader);
    g_object_unref(parser);

    return retval;
}

/**
 * i3ipc_connection_get_bar_config_list:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets a list of all configured bar ids.
 *
 * Returns: (transfer full) (element-type utf8): the configured bar ids
 */
GSList *i3ipc_connection_get_bar_config_list(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    GError *tmp_error = NULL;
    GSList *retval = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));

    int num_elements = json_reader_count_elements(reader);

    for (int i = 0; i < num_elements; i += 1) {
        json_reader_read_element(reader, i);
        retval = g_slist_prepend(retval, g_strdup(json_reader_get_string_value(reader)));
        json_reader_end_element(reader);
    }

    g_free(reply);
    g_object_unref(reader);
    g_object_unref(parser);

    return retval;
}

/**
 * i3ipc_connection_get_bar_config:
 * @self: An #i3ipcConnection
 * @bar_id: The id of the particular bar
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets the configuration of the workspace bar with the given ID.
 *
 * Returns: (transfer full): the bar config reply
 */
i3ipcBarConfigReply *i3ipc_connection_get_bar_config(i3ipcConnection *self, const gchar *bar_id,
                                                     GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    gchar **colors_list;
    GError *tmp_error = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG, bar_id, &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    i3ipcBarConfigReply *retval = g_slice_new0(i3ipcBarConfigReply);
    retval->colors = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member(reader, "id");
    retval->id = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "mode");
    retval->mode = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "position");
    retval->position = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "status_command");
    retval->status_command = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "font");
    retval->font = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "workspace_buttons");
    retval->workspace_buttons = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "binding_mode_indicator");
    retval->binding_mode_indicator = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "verbose");
    retval->verbose = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "strip_workspace_numbers");
    retval->strip_workspace_numbers = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "colors");

    int num_colors = json_reader_count_members(reader);
    colors_list = json_reader_list_members(reader);

    for (int i = 0; i < num_colors; i += 1) {
        json_reader_read_member(reader, colors_list[i]);
        g_hash_table_insert(retval->colors, g_strdup(colors_list[i]),
                            g_strdup(json_reader_get_string_value(reader)));
        json_reader_end_member(reader);
    }

    g_strfreev(colors_list);
    g_object_unref(reader);
    g_object_unref(parser);
    g_free(reply);

    return retval;
}

/**
 * i3ipc_connection_get_version:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets the version of i3. The reply will be a boxed structure with the major,
 * minor, patch and human-readable version.
 *
 * Returns: (transfer full): an #i3ipcVersionReply
 */
i3ipcVersionReply *i3ipc_connection_get_version(i3ipcConnection *self, GError **err) {
    JsonParser *parser;
    JsonReader *reader;
    gchar *reply;
    GError *tmp_error = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_VERSION, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    i3ipcVersionReply *retval = g_slice_new0(i3ipcVersionReply);

    parser = json_parser_new();
    json_parser_load_from_data(parser, reply, -1, &tmp_error);

    if (tmp_error != NULL) {
        g_object_unref(parser);
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member(reader, "major");
    retval->major = json_reader_get_int_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "minor");
    retval->minor = json_reader_get_int_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "patch");
    retval->patch = json_reader_get_int_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "human_readable");
    retval->human_readable = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    g_object_unref(reader);
    g_object_unref(parser);
    g_free(reply);

    return retval;
}

/**
 * i3ipc_connection_get_config:
 * @self: An #i3ipcConnection
 * @err: (allow-none): return location for a GError, or NULL
 *
 * Gets the config of i3. 
 *
 * Returns: (transfer full): an *gchar
 */
gchar *i3ipc_connection_get_config(i3ipcConnection *self, GError **err) {
    gchar *reply;
    GError *tmp_error = NULL;

    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_CONFIG, "", &tmp_error);

    if (tmp_error != NULL) {
        g_free(reply);
        g_propagate_error(err, tmp_error);
        return NULL;
    }

    return reply;
}

/**
 * i3ipc_connection_main:
 * @self: An #i3ipcConnection
 *
 * A convenience function for scripts to run a main loop and wait for events.
 * The main loop will terminate when the connection to the ipc is lost, such as
 * when i3 shuts down or restarts.
 */
void i3ipc_connection_main(i3ipcConnection *self) {
    self->priv->main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(self->priv->main_loop);
    g_main_loop_unref(self->priv->main_loop);
    self->priv->main_loop = NULL;
}

/**
 * i3ipc_connection_main_with_context:
 * @self: An #i3ipcConnection
 * @context: A context to give g_main_loop_new
 *
 * A convenience function for scripts to run a main loop with custom context and wait for events.
 * The main loop will terminate when the connection to the ipc is lost, such as
 * when i3 shuts down or restarts.
 */
void i3ipc_connection_main_with_context(i3ipcConnection *self, GMainContext *context) {
    self->priv->main_loop = g_main_loop_new(context, FALSE);
    g_main_loop_run(self->priv->main_loop);
    g_main_loop_unref(self->priv->main_loop);
    self->priv->main_loop = NULL;
}

/**
 * i3ipc_connection_main_quit:
 * @self: An #i3ipcConnection
 *
 * A convenience function for scripts to quit a main loop that was started with
 * i3ipc_connection_main().
 */
void i3ipc_connection_main_quit(i3ipcConnection *self) {
    g_return_if_fail(self->priv->main_loop != NULL);
    g_main_loop_quit(self->priv->main_loop);
}
