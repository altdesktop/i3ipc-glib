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
 *
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/errno.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <gobject/gmarshal.h>

#include <gio/gio.h>

#include "i3ipc-enum-types.h"
#include "i3ipc-con.h"
#include "i3ipc-connection.h"

typedef struct i3_ipc_header {
    /* 6 = strlen(I3_IPC_MAGIC) */
    char magic[6];
    uint32_t size;
    uint32_t type;
} __attribute__ ((packed)) i3_ipc_header_t;

enum {
  PROP_0,

  PROP_SUBSCRIPTIONS,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum {
  WORKSPACE,
  OUTPUT,
  MODE,
  WINDOW,
  BARCONFIG_UPDATE,
  LAST_SIGNAL
};

static guint connection_signals[LAST_SIGNAL] = {0};

/**
 * i3ipc_command_reply_copy:
 * @reply: a #i3ipcCommandReply struct
 *
 * Creates a dynamically allocated i3ipc command reply as a copy of @reply.
 */
i3ipcCommandReply *i3ipc_command_reply_copy(i3ipcCommandReply *reply) {
  i3ipcCommandReply *retval;

  g_return_val_if_fail(reply != NULL, NULL);

  retval = g_slice_new(i3ipcCommandReply);
  *retval = *reply;

  return retval;
}

/**
 * i3ipc_command_reply_free:
 * @reply:(allow-none): a #i3ipcCommandReply
 *
 * Frees @reply. If @reply is %NULL, it simply returns.
 */
void i3ipc_command_reply_free(i3ipcCommandReply *reply) {
  if (!reply)
    return;

  g_free(reply->error);

  g_slice_free(i3ipcCommandReply, reply);
}

G_DEFINE_BOXED_TYPE(i3ipcCommandReply, i3ipc_command_reply,
    i3ipc_command_reply_copy, i3ipc_command_reply_free);

/**
 * i3ipc_version_reply_copy:
 * @version: a #i3ipcVersionReply struct
 *
 * Creates a dynamically allocated i3ipc version reply as a copy of @version.
 *
 * This function is not intended for use in applications, because you can just
 * copy structs by value (`i3ipcVersionReply new_version = version;`).
 * You must free this version with i3ipc_version_reply_free().
 *
 * Return value: a newly-allocated copy of @version
 */
i3ipcVersionReply *i3ipc_version_reply_copy(i3ipcVersionReply *version) {
  i3ipcVersionReply *retval;

  g_return_val_if_fail(version != NULL, NULL);

  retval = g_slice_new(i3ipcVersionReply);
  *retval = *version;

  return retval;
}

/**
 * i3ipc_version_reply_free:
 * @version: (allow-none): a #i3ipcVersionReply
 *
 * Frees @version. If @version is %NULL, it simply returns.
 */
void i3ipc_version_reply_free(i3ipcVersionReply *version) {
  if (!version)
    return;

  g_free(version->human_readable);
  g_slice_free(i3ipcVersionReply, version);
}

G_DEFINE_BOXED_TYPE(i3ipcVersionReply, i3ipc_version_reply,
    i3ipc_version_reply_copy, i3ipc_version_reply_free);

/**
 * i3ipc_bar_config_reply_copy:
 * @config: a #i3ipcBarConfigReply struct
 *
 * Creates a dynamically allocated i3ipc version reply as a copy of @config.
 *
 * Return value: a newly-allocated copy of @config
 */
i3ipcBarConfigReply *i3ipc_bar_config_reply_copy(i3ipcBarConfigReply *config) {
  i3ipcBarConfigReply *retval;

  g_return_val_if_fail(config != NULL, NULL);

  retval = g_slice_new(i3ipcBarConfigReply);
  *retval = *config;

  return retval;
}

/**
 * i3ipc_bar_config_reply_free:
 * @config: (allow-none): a #i3ipcBarConfigReply
 *
 * Frees @config. If @config is %NULL, it simply returns.
 */
void i3ipc_bar_config_reply_free(i3ipcBarConfigReply *config) {
  if (!config)
    return;

  g_free(config->id);
  g_free(config->mode);
  g_free(config->position);
  g_free(config->status_command);
  g_free(config->font);
  g_hash_table_destroy(config->colors);
}

G_DEFINE_BOXED_TYPE(i3ipcBarConfigReply, i3ipc_bar_config_reply,
    i3ipc_bar_config_reply_copy, i3ipc_bar_config_reply_free)

/**
 * i3ipc_output_reply_copy:
 * @output: a #i3ipcOutputReply struct
 *
 * Creates a dynamically allocated i3ipc output reply as a copy of @output.
 *
 * Return value: a newly-allocated copy of @output
 */
i3ipcOutputReply *i3ipc_output_reply_copy(i3ipcOutputReply *output) {
  i3ipcOutputReply *retval;

  g_return_val_if_fail(output != NULL, NULL);

  retval = g_slice_new(i3ipcOutputReply);
  *retval = *output;

  return retval;
}

/**
 * i3ipc_output_reply_free:
 * @output: (allow-none): a #i3ipcOutputReply
 *
 * Frees @output. If @output is %NULL, it simply returns.
 */
void i3ipc_output_reply_free(i3ipcOutputReply *output) {
  if (!output)
    return;

  g_free(output->name);
  g_free(output->current_workspace);
  i3ipc_rect_free(output->rect);
}

G_DEFINE_BOXED_TYPE(i3ipcOutputReply, i3ipc_output_reply,
    i3ipc_output_reply_copy, i3ipc_output_reply_free)

/**
 * i3ipc_workspace_reply_copy:
 * @workspace: a #i3ipcWorkspaceReply
 *
 * Creates a dynamically allocated i3ipc workspace reply as a copy of
 * @workspace.
 */
i3ipcWorkspaceReply *i3ipc_workspace_reply_copy(i3ipcWorkspaceReply *workspace) {
  i3ipcWorkspaceReply *retval;

  g_return_val_if_fail(workspace != NULL, NULL);

  retval = g_slice_new(i3ipcWorkspaceReply);
  *retval = *workspace;

  return retval;
}

/**
 * i3ipc_workspace_reply_free:
 * @workspace: (allow-none): a #i3ipcWorkspaceReply
 *
 * Frees @workspace. If @workspace is %NULL, it simply returns.
 */
void i3ipc_workspace_reply_free(i3ipcWorkspaceReply *workspace) {
  if (!workspace)
    return;

  g_free(workspace->name);
  g_free(workspace->output);
  i3ipc_rect_free(workspace->rect);
}

G_DEFINE_BOXED_TYPE(i3ipcWorkspaceReply, i3ipc_workspace_reply,
    i3ipc_workspace_reply_copy, i3ipc_workspace_reply_free)

/**
 * i3ipc_barconfig_update_event_copy:
 * @event: a #i3ipcBarconfigUpdateEvent
 *
 * Creates a dynamically allocated i3ipc barconfig update event data container
 * as a copy of @event.
 */
i3ipcBarconfigUpdateEvent *i3ipc_barconfig_update_event_copy(i3ipcBarconfigUpdateEvent *event) {
  i3ipcBarconfigUpdateEvent *retval;

  g_return_val_if_fail(event != NULL, NULL);

  retval = g_slice_new(i3ipcBarconfigUpdateEvent);
  *retval = *event;

  return retval;
}

/**
 * i3ipc_barconfig_update_free:
 * @event:(allow-none): a #i3ipcBarconfigUpdateEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_barconfig_update_event_free(i3ipcBarconfigUpdateEvent *event) {
  if (!event)
    return;

  g_free(event->id);
  g_free(event->hidden_state);
  g_free(event->mode);
}

G_DEFINE_BOXED_TYPE(i3ipcBarconfigUpdateEvent, i3ipc_barconfig_update_event,
    i3ipc_barconfig_update_event_copy, i3ipc_barconfig_update_event_free);

struct _i3ipcConnectionPrivate {
  JsonParser *parser;
  i3ipcEvent subscriptions;
};

static void i3ipc_connection_initable_iface_init(GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE(i3ipcConnection, i3ipc_connection, G_TYPE_OBJECT, G_ADD_PRIVATE(i3ipcConnection) G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, i3ipc_connection_initable_iface_init))

static void i3ipc_connection_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  //i3ipcConnnection *self = I3IPC_CONNECTION(object);

  switch (property_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_connection_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  i3ipcConnection *self = I3IPC_CONNECTION(object);

  switch (property_id)
  {
    case PROP_SUBSCRIPTIONS:
      g_value_set_flags(value, self->priv->subscriptions);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_connection_class_init (i3ipcConnectionClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->set_property = i3ipc_connection_set_property;
  gobject_class->get_property = i3ipc_connection_get_property;

  obj_properties[PROP_SUBSCRIPTIONS] =
    g_param_spec_flags("subscriptions",
        "Connection subscriptions",
        "The subscriptions this connection is subscribed to",
        I3IPC_TYPE_EVENT,
        0,
        G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);

  /**
   * i3ipcConnection::workspace:
   * @self: the #i3ipcConnection on which the signal was emitted
   * @e: The workspace event object
   *
   * Sent when the user switches to a different workspace, when a new workspace
   * is initialized or when a workspace is removed (because the last client
   * vanished).
   *
   */
  connection_signals[WORKSPACE] = g_signal_new(
      "workspace",               /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_LAST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__VARIANT,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      G_TYPE_VARIANT);                        /* n_params */

  /**
   * i3ipcConnection::output:
   * @self: the #i3ipcConnection on which the signal was emitted
   * @e: The output event object
   *
   * Sent when RandR issues a change notification (of either screens, outputs,
   * CRTCs or output properties).
   *
   */
  connection_signals[OUTPUT] = g_signal_new(
      "output",                  /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_FIRST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__VARIANT,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      G_TYPE_VARIANT);                        /* n_params */

  /**
   * i3ipcConnection::mode:
   * @self: the #i3ipcConnection on which the signal was emitted
   * @e: The mode event object
   *
   * Sent whenever i3 changes its binding mode.
   *
   */
  connection_signals[MODE] = g_signal_new(
      "mode",                    /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_FIRST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__VARIANT,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      G_TYPE_VARIANT);                        /* n_params */

  /**
   * i3ipcConnection::window:
   * @self: the #i3ipcConnection on which the signal was emitted
   * @e: The window event object
   *
   * Sent when a client’s window is successfully reparented (that is when i3
   * has finished fitting it into a container).
   *
   */
  connection_signals[WINDOW] = g_signal_new(
      "window",                  /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_FIRST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__VARIANT,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      G_TYPE_VARIANT);                        /* n_params */

  /**
   * i3ipcConnection::barconfig_update:
   * @self: the #i3ipcConnection on which the signal was emitted
   * @e: The barconfig_update event object
   *
   * Sent when the hidden_state or mode field in the barconfig of any bar
   * instance was updated.
   *
   */
  connection_signals[BARCONFIG_UPDATE] = g_signal_new(
      "barconfig_update",                     /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_FIRST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__BOXED,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      I3IPC_TYPE_BARCONFIG_UPDATE_EVENT);                        /* n_params */

}

static void i3ipc_connection_init(i3ipcConnection *self) {
  self->priv = i3ipc_connection_get_instance_private (self);
  self->priv->parser = json_parser_new();
}

/**
 * i3ipc_connection_new:
 * @err: return location for a GError, or NULL
 *
 * Allocates a new #i3ipcConnection
 *
 * Returns: a new #i3ipcConnection
 *
 */
i3ipcConnection *i3ipc_connection_new(GError **err) {
  i3ipcConnection *conn;
  GError *tmp_error = NULL;

  conn = g_initable_new(I3IPC_TYPE_CONNECTION, NULL, &tmp_error, NULL);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  return conn;
}

static char *get_socket_path() {
  char *atomname = "I3_SOCKET_PATH";
  size_t content_max_words = 256;

  xcb_connection_t *conn = xcb_connect(NULL, NULL);

  xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

  xcb_window_t root = screen->root;

  xcb_intern_atom_cookie_t atom_cookie;
  xcb_intern_atom_reply_t *atom_reply;

  atom_cookie = xcb_intern_atom(conn, 0, strlen(atomname), atomname);
  atom_reply = xcb_intern_atom_reply(conn, atom_cookie, NULL);
  if (atom_reply == NULL) {
    g_error("atom reply is null\n");
    return NULL;
  }

  xcb_get_property_cookie_t prop_cookie;
  xcb_get_property_reply_t *prop_reply;

  prop_cookie = xcb_get_property_unchecked(conn,
      false, // _delete
      root, // window
      atom_reply->atom, //property
      XCB_GET_PROPERTY_TYPE_ANY, //type
      0, // long_offset
      content_max_words // long_length
      );

  prop_reply = xcb_get_property_reply(conn, prop_cookie, NULL);
  if (prop_reply == NULL) {
    g_error("property reply is null\n");
    free(atom_reply);
    return NULL;
  }

  int len = xcb_get_property_value_length(prop_reply);
  char *socket_path = malloc(len);
  strncpy(socket_path, (char *)xcb_get_property_value(prop_reply), len);
  socket_path[len] = '\0';

  free(atom_reply);
  free(prop_reply);
  xcb_disconnect(conn);

  return socket_path;
}

/*
 * Connects to the i3 IPC socket and returns the file descriptor for the
 * socket. die()s if anything goes wrong.
 */
static int get_file_descriptor(const char *socket_path) {
  int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (sockfd == -1) {
    g_error("Could not create socket");
    return -1;
  }

  (void)fcntl(sockfd, F_SETFD, FD_CLOEXEC);

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
    g_error("Could not connect to i3 (%s)\n", strerror(errno));
    return -1;
  }

  return sockfd;
}

/*
 * Blockingly receives a message from the ipc socket.
 */
static int ipc_recv_message(GIOChannel *channel, uint32_t *message_type, uint32_t *reply_length, gchar **reply, GError **err) {
  /* Read the message header first */
  const uint32_t to_read = strlen(I3IPC_MAGIC) + sizeof(uint32_t) + sizeof(uint32_t);
  char msg[to_read];
  char *walk = msg;
  GIOStatus status;

  status = g_io_channel_flush(channel, err);
  g_assert_true(status);
  g_assert_no_error(*err);

  gsize read_bytes = 0;
  while (read_bytes < to_read) {
    status = g_io_channel_read_chars(channel, msg + read_bytes, to_read - read_bytes, &read_bytes, err);
    g_assert_true(status);
    g_assert_no_error(*err);
  }

  if (memcmp(walk, I3IPC_MAGIC, strlen(I3IPC_MAGIC)) != 0) {
    g_error("IPC: invalid magic in reply\n");
    return -3;
  }

  walk += strlen(I3IPC_MAGIC);
  memcpy(reply_length, walk, sizeof(uint32_t));
  walk += sizeof(uint32_t);
  if (message_type != NULL)
    memcpy(message_type, walk, sizeof(uint32_t));

  *reply = malloc(*reply_length);

  read_bytes = 0;
  while (read_bytes < *reply_length) {
    status = g_io_channel_read_chars(channel, *reply + read_bytes, *reply_length - read_bytes, &read_bytes, err);
    g_assert_true(status);
    g_assert_no_error(*err);
  }

  return 0;
}

/*
 * Callback function for when a channel receives data from the ipc socket.
 * Emits the corresponding signal with the reply.
 */
static gboolean ipc_on_data(GIOChannel *channel, GIOCondition condition, i3ipcConnection *conn) {
  if (condition != G_IO_IN)
    return TRUE;

  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;
  GVariant *event_data;
  GError *err = NULL;

  ipc_recv_message(channel, &reply_type, &reply_length, &reply, &err);

  if (err) {
    return TRUE;
  }

  event_data = json_gvariant_deserialize_data(reply, reply_length, NULL, &err);

  if (err) {
    return TRUE;
  }

  switch (1 << (reply_type & 0x7F))
  {
    case I3IPC_EVENT_WORKSPACE:
      g_signal_emit(conn, connection_signals[WORKSPACE], 0, event_data);
      break;

    case I3IPC_EVENT_OUTPUT:
      g_signal_emit(conn, connection_signals[OUTPUT], 0, event_data);
      break;

    case I3IPC_EVENT_MODE:
      g_signal_emit(conn, connection_signals[MODE], 0, event_data);
      break;

    case I3IPC_EVENT_WINDOW:
      g_signal_emit(conn, connection_signals[WINDOW], 0, event_data);
      break;

    case I3IPC_EVENT_BARCONFIG_UPDATE:
      {
        JsonParser *parser = json_parser_new();
        json_parser_load_from_data(parser, reply, reply_length, &err);

        if (err)
          return TRUE;

        JsonObject *json_reply = json_node_get_object(json_parser_get_root(parser));
        i3ipcBarconfigUpdateEvent *e = g_new(i3ipcBarconfigUpdateEvent, 1);

        e->id = g_strdup(json_object_get_string_member(json_reply, "id"));
        e->hidden_state = g_strdup(json_object_get_string_member(json_reply, "hidden_state"));
        e->mode = g_strdup(json_object_get_string_member(json_reply, "mode"));

        g_object_unref(parser);

        g_signal_emit(conn, connection_signals[BARCONFIG_UPDATE], 0, e);
        break;
      }

    default:
      g_warning("got unknown event\n");
      break;
  }

  return TRUE;
}

static gboolean i3ipc_connection_initable_init(GInitable *initable, GCancellable *cancellable, GError **err) {
  i3ipcConnection *self = I3IPC_CONNECTION(initable);
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  char *socket_path = get_socket_path();

  int cmd_fd = get_file_descriptor(socket_path);
  self->cmd_channel = g_io_channel_unix_new(cmd_fd);

  int sub_fd = get_file_descriptor(socket_path);
  self->sub_channel = g_io_channel_unix_new(sub_fd);

  g_io_channel_set_encoding(self->cmd_channel, NULL, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return FALSE;
  }

  g_io_channel_set_encoding(self->sub_channel, NULL, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return FALSE;
  }

  g_io_add_watch(self->sub_channel, G_IO_IN, (GIOFunc)ipc_on_data, self);

  free(socket_path);

  return TRUE;
}

static void i3ipc_connection_initable_iface_init (GInitableIface *iface) {
  iface->init = i3ipc_connection_initable_init;
}


/*
 * Sends a message to the ipc.
 */
static int ipc_send_message(GIOChannel *channel, const uint32_t message_size, const uint32_t message_type, const gchar *command, GError **err) {
  const i3_ipc_header_t header = {
    /* We don’t use I3IPC_MAGIC because it’s a 0-terminated C string. */
    .magic = { 'i', '3', '-', 'i', 'p', 'c' },
    .size = message_size,
    .type = message_type
  };
  GIOStatus status;

  gsize sent_bytes = 0;

  /* This first loop is basically unnecessary. No operating system has
   * buffers which cannot fit 14 bytes into them, so the write() will only be
   * called once. */
  while (sent_bytes < sizeof(i3_ipc_header_t)) {
    status = g_io_channel_write_chars(channel, ((void*)&header) + sent_bytes, sizeof(i3_ipc_header_t) - sent_bytes, &sent_bytes, err);
    g_assert_true(status);
    g_assert_no_error(*err);
  }

  sent_bytes = 0;

  while (sent_bytes < message_size) {
    status = g_io_channel_write_chars(channel, command + sent_bytes, message_size - sent_bytes, &sent_bytes, err);
    g_assert_true(status);
    g_assert_no_error(*err);
  }

  return 0;
}

/**
  * i3ipc_connection_message:
  * @self: A #i3ipcConnection
  * @message_type: The type of message to send to i3
  * @payload: (allow-none): The body of the command
  * @err: return location for a GError, or NULL
  *
  * Sends a command to the ipc synchronously.
  *
  * Returns: The reply of the ipc as a string
  *
  */
gchar *i3ipc_connection_message(i3ipcConnection *self, i3ipcMessageType message_type, gchar *payload, GError **err) {
  GError *tmp_error = NULL;
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  if (payload == NULL)
    payload = "";

  GIOChannel *channel = (message_type == I3IPC_MESSAGE_TYPE_SUBSCRIBE ? self->sub_channel : self->cmd_channel);

  ipc_send_message(channel, strlen(payload), message_type, payload, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  ipc_recv_message(channel, &reply_type, &reply_length, &reply, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  reply[reply_length] = '\0';

  return reply;
}

/**
  * i3ipc_connection_command:
  * @self: A #i3ipcConnection
  * @command: The command to send to i3
  * @err:(allow-none): return location of a GError, or NULL
  *
  * Sends a command to the ipc synchronously.
  *
  * Returns:(transfer none): the #i3ipcCommandReply of the last command
  *
  */
i3ipcCommandReply *i3ipc_connection_command(i3ipcConnection *self, gchar *command, GError **err) {
  GError *tmp_error = NULL;
  i3ipcCommandReply *retval;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_COMMAND, command, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  retval = g_new(i3ipcCommandReply, 1);

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonArray *json_replies = json_node_get_array(json_parser_get_root(self->priv->parser));

  guint reply_count = json_array_get_length(json_replies);

  JsonObject *last_reply = json_array_get_object_element(json_replies, reply_count - 1);

  retval->success = json_object_get_boolean_member(last_reply, "success");

  retval->parse_error = (json_object_has_member(last_reply, "parse_error") ?
      json_object_get_boolean_member(last_reply, "parse_error") :
      FALSE);

  retval->error = (json_object_has_member(last_reply, "error") ?
      g_strdup(json_object_get_string_member(last_reply, "error")) :
      NULL);

  return retval;
}

/**
  * i3ipc_connection_subscribe:
  * @self: A #i3ipcConnection
  * @events: The name of an IPC event
  * @err:(allow-none): The location of a GError or NULL
  *
  * Subscribes to the events given by the flags
  *
  * Returns:(transfer none): The ipc reply
  *
  */
i3ipcCommandReply *i3ipc_connection_subscribe(i3ipcConnection *self, i3ipcEvent events, GError **err) {
  GError *tmp_error = NULL;
  i3ipcCommandReply *retval;

  if (!(events & ~self->priv->subscriptions)) {
    /* No new events */
    retval = g_new0(i3ipcCommandReply, 1);
    retval->success = TRUE;
    return retval;
  }

  JsonBuilder *builder = json_builder_new();
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

  json_builder_end_array(builder);

  JsonGenerator *generator = json_generator_new();
  json_generator_set_root(generator, json_builder_get_root(builder));
  gchar *payload = json_generator_to_data(generator, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_SUBSCRIBE, payload, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonObject *json_reply = json_node_get_object(json_parser_get_root(self->priv->parser));

  retval = g_new0(i3ipcCommandReply, 1);
  retval->success = json_object_get_boolean_member(json_reply, "success");

  g_object_unref(generator);
  g_object_unref(builder);

  if (retval->success)
    self->priv->subscriptions |= events;

  return retval;
}

/**
 * i3ipc_connection_get_workspaces:
 * @self: An #i3ipcConnection
 * @err: return location of a GError, or NULL
 *
 * Gets the current workspaces. The reply will be list workspaces
 *
 * Returns:(transfer none) (element-type i3ipcWorkspaceReply): a list of workspaces
 *
 */
GSList *i3ipc_connection_get_workspaces(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;
  GSList *retval = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_WORKSPACES, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));

  int num_workspaces = json_reader_count_elements(reader);

  for (int i = 0; i < num_workspaces; i += 1) {
    i3ipcWorkspaceReply *workspace = g_new(i3ipcWorkspaceReply, 1);
    workspace->rect = g_new(i3ipcRect, 1);

    json_reader_read_element(reader, i);

    json_reader_read_member(reader, "name");
    workspace->name = g_strdup(json_reader_get_string_value(reader));
    json_reader_end_member(reader);

    json_reader_read_member(reader, "num");
    workspace->num = json_reader_get_int_value(reader);
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

  g_object_unref(reader);

  return retval;
}

/**
 * i3ipc_connection_get_outputs:
 * @self: An #i3ipcConnection
 * @err: return location of a GError, or NULL
 *
 * Gets the current outputs. The reply will be a list of outputs
 *
 * Returns:(transfer none) (element-type i3ipcOutputReply): a list of outputs
 *
 */
GSList *i3ipc_connection_get_outputs(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;
  GSList *retval = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_OUTPUTS, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));

  int num_outputs = json_reader_count_elements(reader);

  for (int i = 0; i < num_outputs; i += 1) {
    i3ipcOutputReply *output = g_new(i3ipcOutputReply, 1);
    output->rect = g_new(i3ipcRect, 1);

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

  g_object_unref(reader);

  return retval;
}
/**
 * i3ipc_connection_get_tree:
 * @self: An #i3ipcConnection
 * @err: return location for a GError, or NULL
 *
 *  Gets the layout tree. i3 uses a tree as data structure which includes every
 *  container.
 *
 * Returns:(transfer none): the root container
 *
 */
i3ipcCon *i3ipc_connection_get_tree(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;
  i3ipcCon *retval;
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  ipc_send_message(self->cmd_channel, 1, I3IPC_MESSAGE_TYPE_GET_TREE, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  ipc_recv_message(self->cmd_channel, &reply_type, &reply_length, &reply, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, reply_length, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  retval = i3ipc_con_new(NULL, json_node_get_object(json_parser_get_root(self->priv->parser)));

  return retval;
}

/**
 * i3ipc_connection_get_marks:
 * @self: An #i3ipcConnection
 * @err: return location for a GError, or NULL
 *
 * Gets a list of marks (identifiers for containers to easily jump to them
 * later). The reply will be a list of window marks.
 *
 * Return value: (transfer none) (element-type utf8): a list of strings representing marks
 *
 */
GSList *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;
  GSList *retval = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_MARKS, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));

  int num_elements = json_reader_count_elements(reader);

  for (int i = 0; i < num_elements; i += 1) {
    json_reader_read_element(reader, i);
    retval = g_slist_prepend(retval, g_strdup(json_reader_get_string_value(reader)));
    json_reader_end_element(reader);
  }

  g_object_unref(reader);

  return retval;
}

/**
 * i3ipc_connection_get_bar_config_list:
 * @self: An #i3ipcConnection
 * @err: return location for a GError, or NULL
 *
 * Gets a list of all configured bar ids.
 *
 * Return value:(transfer none) (element-type utf8): the configured bar ids
 *
 */
GSList *i3ipc_connection_get_bar_config_list(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;
  GSList *retval = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));

  int num_elements = json_reader_count_elements(reader);

  for (int i = 0; i < num_elements; i += 1) {
    json_reader_read_element(reader, i);
    retval = g_slist_prepend(retval, g_strdup(json_reader_get_string_value(reader)));
    json_reader_end_element(reader);
  }

  g_object_unref(reader);

  return retval;
}

/**
 * i3ipc_connection_get_bar_config:
 * @self: An #i3ipcConnection
 * @bar_id: The id of the particular bar
 * @err: return location for a GError, or NULL
 *
 * Gets the configuration of the workspace bar with the given ID.
 *
 * Return value:(transfer none): the bar config reply
 *
 */
i3ipcBarConfigReply *i3ipc_connection_get_bar_config(i3ipcConnection *self, gchar *bar_id, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG, bar_id, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  i3ipcBarConfigReply *retval = g_new(i3ipcBarConfigReply, 1);
  retval->colors = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));
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

  json_reader_read_member(reader, "colors");

  int num_colors = json_reader_count_members(reader);
  gchar **colors_list = json_reader_list_members(reader);

  for (int i = 0; i < num_colors; i += 1) {
      json_reader_read_member(reader, colors_list[i]);
      g_hash_table_insert(retval->colors, g_strdup(colors_list[i]), g_strdup(json_reader_get_string_value(reader)));
      json_reader_end_member(reader);
  }

  g_object_unref(reader);

  return retval;
}

/**
 * i3ipc_connection_get_version:
 * @self: An #i3ipcConnection
 * @err: return location for a GError, or NULL
 *
 * Gets the version of i3. The reply will be a boxed structure with the major,
 * minor, patch and human-readable version.
 *
 * Return value:(transfer none): an #i3ipcVersionReply
 */
i3ipcVersionReply *i3ipc_connection_get_version(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  gchar *reply = i3ipc_connection_message(self, I3IPC_MESSAGE_TYPE_GET_VERSION, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  i3ipcVersionReply *retval = g_new(i3ipcVersionReply, 1);
  json_parser_load_from_data(self->priv->parser, reply, -1, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  JsonReader *reader = json_reader_new(json_parser_get_root(self->priv->parser));
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

  return retval;
}
