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
#include "ipc-protocol.h"

#include "i3ipc-connection.h"

enum {
  WORKSPACE,
  OUTPUT,
  MODE,
  WINDOW,
  BARCONFIG_UPDATE,
  LAST_SIGNAL
};

static const char *signal_names[LAST_SIGNAL] = {
  "workspace",
  "output",
  "mode",
  "window",
  "barconfig_update"
};

static int find_signal_number(char *name) {
  for (int i = 0; i < LAST_SIGNAL; i += 1)
    if (strcmp(signal_names[i], name) == 0)
      return i;

  /* not found */
  return -1;
}

static guint connection_signals[LAST_SIGNAL] = {0};

struct _i3ipcConnectionPrivate {
  JsonParser *parser;
  uint32_t subscriptions;
};

G_DEFINE_TYPE_WITH_PRIVATE (i3ipcConnection, i3ipc_connection, G_TYPE_OBJECT)

static void i3ipc_connection_class_init (i3ipcConnectionClass *klass) {
  //GObjectClass *g_object_class = G_OBJECT_CLASS(klass);

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
      signal_names[WORKSPACE],               /* signal_name */
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
      signal_names[OUTPUT],                  /* signal_name */
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
      signal_names[MODE],                    /* signal_name */
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
      signal_names[WINDOW],                  /* signal_name */
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
      signal_names[BARCONFIG_UPDATE],        /* signal_name */
      I3IPC_TYPE_CONNECTION,                  /* itype */
      G_SIGNAL_RUN_FIRST,                    /* signal_flags */
      0,                                     /* class_offset */
      NULL,                                  /* accumulator */
      NULL,                                  /* accu_data */
      g_cclosure_marshal_VOID__VARIANT,       /* c_marshaller */
      G_TYPE_NONE,                           /* return_type */
      1,
      G_TYPE_VARIANT);                        /* n_params */

}

static void i3ipc_connection_init (i3ipcConnection *self) {
  self->priv = i3ipc_connection_get_instance_private (self);
  self->priv->parser = json_parser_new();
}

/**
 * i3ipc_connection_new:
 *
 * Allocates a new #i3ipcConnection
 *
 * Return: a new #i3ipcConnection
 *
 */
i3ipcConnection *i3ipc_connection_new() {
  i3ipcConnection *conn;

  conn = g_object_new(I3IPC_TYPE_CONNECTION, NULL);
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
  const uint32_t to_read = strlen(I3_IPC_MAGIC) + sizeof(uint32_t) + sizeof(uint32_t);
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

  if (memcmp(walk, I3_IPC_MAGIC, strlen(I3_IPC_MAGIC)) != 0) {
    g_error("IPC: invalid magic in reply\n");
    return -3;
  }

  walk += strlen(I3_IPC_MAGIC);
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
  g_assert_no_error(err);

  event_data = json_gvariant_deserialize_data(reply, reply_length, NULL, &err);
  g_assert_no_error(err);

  switch (1 << (reply_type & 0x7F))
  {
    case I3_IPC_EVENT_WORKSPACE:
      g_signal_emit(conn, connection_signals[WORKSPACE], 0, event_data);
      break;

    case I3_IPC_EVENT_OUTPUT:
      g_signal_emit(conn, connection_signals[OUTPUT], 0, event_data);
      break;

    case I3_IPC_EVENT_MODE:
      g_signal_emit(conn, connection_signals[MODE], 0, event_data);
      break;

    case I3_IPC_EVENT_WINDOW:
      g_signal_emit(conn, connection_signals[WINDOW], 0, event_data);
      break;

    case I3_IPC_EVENT_BARCONFIG_UPDATE:
      g_signal_emit(conn, connection_signals[BARCONFIG_UPDATE], 0, event_data);
      break;

    default:
      g_warning("got unknown event\n");
      break;
  }

  return TRUE;
}

/**
 * i3ipc_connection_connect:
 * @self: A #i3ipcConnection
 *
 * Returns: nothing
 *
 * Connects to the ipc. After the connection is established, the connection is
 * ready to send commands and subscribe to events.
 *
 */
void i3ipc_connection_connect(i3ipcConnection *self) {
  char *socket_path = get_socket_path();

  int cmd_fd = get_file_descriptor(socket_path);
  self->cmd_channel = g_io_channel_unix_new(cmd_fd);

  int sub_fd = get_file_descriptor(socket_path);
  self->sub_channel = g_io_channel_unix_new(sub_fd);

  GError *err = NULL;
  g_io_channel_set_encoding(self->cmd_channel, NULL, &err);
  g_assert_no_error(err);

  g_io_channel_set_encoding(self->sub_channel, NULL, &err);
  g_assert_no_error(err);

  g_io_add_watch(self->sub_channel, G_IO_IN, (GIOFunc)ipc_on_data, self);

  free(socket_path);
}

/*
 * Sends a message to the ipc.
 */
static int ipc_send_message(GIOChannel *channel, const uint32_t message_size, const uint32_t message_type, const gchar *command, GError **err) {
  const i3_ipc_header_t header = {
    /* We don’t use I3_IPC_MAGIC because it’s a 0-terminated C string. */
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

/*
 * Synchronously sends the command to the ipc and returns the result.
 */
static gboolean ipc_command_sync(i3ipcConnection *conn, uint32_t message_type, char *command, GError **err) {
  GIOChannel *channel = (message_type == I3_IPC_MESSAGE_TYPE_SUBSCRIBE ? conn->sub_channel : conn->cmd_channel);
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;
  ipc_send_message(channel, strlen(command), message_type, command, err);
  g_assert_no_error(*err);

  ipc_recv_message(channel, &reply_type, &reply_length, &reply, err);
  g_assert_no_error(*err);

  gboolean result = FALSE;

  if (json_parser_load_from_data(conn->priv->parser, reply, reply_length, err)) {
    g_assert_no_error(*err);

    JsonReader *reader = json_reader_new(json_parser_get_root(conn->priv->parser));
    json_reader_read_element(reader, 0);
    json_reader_read_member(reader, "success");
    result = json_reader_get_boolean_value(reader);

    g_object_unref(reader);
  }

  return result;
}

/*
 * Synchronously sends the query to the ipc and returns the result as a GVariant.
 * Returns NULL if an error occurred.
 */
static GVariant *ipc_query_sync(i3ipcConnection *conn, uint32_t message_type, char *command, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;

  ipc_send_message(conn->cmd_channel, strlen(command), message_type, command, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  ipc_recv_message(conn->cmd_channel, &reply_type, &reply_length, &reply, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  GVariant *retval = json_gvariant_deserialize_data(reply, reply_length, NULL, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  return retval;
}

/*
 * Subscribes to the event with the given name.
 */
static gboolean ipc_subscribe(i3ipcConnection *conn, char *event_name, GError **err) {
  char command[strlen(event_name) + 4];

  sprintf(command, "[\"%s\"]", event_name);

  gboolean result = ipc_command_sync(conn, I3_IPC_MESSAGE_TYPE_SUBSCRIBE, command, err);
  g_assert_no_error(*err);

  return result;
}

/**
  * i3ipc_connection_command:
  * @self: A #i3ipcConnection
  * @command: The command to send to i3
  *
  * Sends a command to the ipc synchronously.
  *
  * Returns: TRUE on success, FALSE if an error occurred
  *
  */
gboolean i3ipc_connection_command(i3ipcConnection *self, gchar *command) {
  GError *err = NULL;
  gboolean result = ipc_command_sync(self, I3_IPC_MESSAGE_TYPE_COMMAND, command, &err);
  g_assert_no_error(err);

  return result;
}

/**
  * i3ipc_connection_on:
  * @self: A #i3ipcConnection
  * @event: The name of an IPC event
  * @callback: Callback for the event
  *
  * Subscribes to the event given by its name.
  *
  * Returns: nothing
  *
  */
void i3ipc_connection_on(i3ipcConnection *self, gchar *event, GClosure *callback) {
  int signal = find_signal_number(event);
  if (signal != -1) {
    g_signal_connect_closure(self, event, callback, TRUE);
    if (!(self->priv->subscriptions & (1 << signal))) {
      GError *err = NULL;
      ipc_subscribe(self, event, &err);
      g_assert_no_error(err);
      self->priv->subscriptions |= (1 << signal);
    }
  }
}

/**
 * i3ipc_connection_get_workspaces:
 * @self: An #i3ipcConnection
 *
 * Gets the current workspaces. The reply will be a JSON-encoded list of
 * workspaces
 *
 * Returns: a string reply
 *
 */
gchar *i3ipc_connection_get_workspaces(i3ipcConnection *self) {
  GError *err = NULL;
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;
  ipc_send_message(self->cmd_channel, 1, I3_IPC_MESSAGE_TYPE_GET_WORKSPACES, "", &err);
  g_assert_no_error(err);

  ipc_recv_message(self->cmd_channel, &reply_type, &reply_length, &reply, &err);
  g_assert_no_error(err);

  reply[reply_length] = '\0';

  return reply;
}

/**
 * i3ipc_connection_get_outputs:
 * @self: An #i3ipcConnection
 *
 * Gets the current outputs. The reply will be a JSON-encoded list of outputs
 * (see the reply section of docs/ipc, e.g. at
 * http://i3ipc.org/docs/ipc.html#_receiving_replies_from_i3).
 *
 * Returns: a string reply
 *
 */
gchar *i3ipc_connection_get_outputs(i3ipcConnection *self) {
  GError *err = NULL;
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;
  ipc_send_message(self->cmd_channel, 1, I3_IPC_MESSAGE_TYPE_GET_OUTPUTS, "", &err);
  g_assert_no_error(err);

  ipc_recv_message(self->cmd_channel, &reply_type, &reply_length, &reply, &err);
  g_assert_no_error(err);

  reply[reply_length] = '\0';

  return reply;
}
/**
 * i3ipc_connection_get_tree:
 * @self: An #i3ipcConnection
 *
 *  Gets the layout tree. i3 uses a tree as data structure which includes every
 *  container. The reply will be the JSON-encoded tree
 *
 * Returns: a string reply
 *
 */
gchar *i3ipc_connection_get_tree(i3ipcConnection *self) {
  GError *err = NULL;
  uint32_t reply_length;
  uint32_t reply_type;
  gchar *reply;
  ipc_send_message(self->cmd_channel, 1, I3_IPC_MESSAGE_TYPE_GET_TREE, "", &err);
  g_assert_no_error(err);

  ipc_recv_message(self->cmd_channel, &reply_type, &reply_length, &reply, &err);
  g_assert_no_error(err);

  reply[reply_length] = '\0';

  return reply;
}

/**
 * i3ipc_connection_get_marks:
 * @self: An #i3ipcConnection
 * @error: return location for a GError, or NULL
 *
 * Gets a list of marks (identifiers for containers to easily jump to them
 * later). The reply will be a JSON-encoded list of window marks.
 *
 * Return value: (transfer none) a list of window marks
 *
 */
GVariant *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  GVariant *retval = ipc_query_sync(self, I3_IPC_MESSAGE_TYPE_GET_MARKS, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  return retval;
}

/**
 * i3ipc_connection_get_bar_config:
 * @self: An #i3ipcConnection
 * @bar_id: (allow-none): The id of the particular bar
 * @error: return location for a GError, or NULL
 *
 * Gets the configuration (as JSON map) of the workspace bar with the given ID.
 * If no ID is provided, an array with all configured bar IDs is returned
 * instead.
 *
 * Return value:(transfer none) the bar config reply
 *
 */
GVariant *i3ipc_connection_get_bar_config(i3ipcConnection *self, gchar *bar_id, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  if (bar_id == NULL)
    bar_id = "";

  GVariant *retval = ipc_query_sync(self, I3_IPC_MESSAGE_TYPE_GET_BAR_CONFIG, bar_id, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  return retval;
}

/**
 * i3ipc_connection_get_version:
 * @self: An #i3ipcConnection
 * @error: return location for a GError, or NULL
 *
 * Gets the version of i3. The reply will be a JSON-encoded dictionary with the
 * major, minor, patch and human-readable version.
 *
 * Return value: (transfer none) the version reply
 */
GVariant *i3ipc_connection_get_version(i3ipcConnection *self, GError **err) {
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  GVariant *retval = ipc_query_sync(self, I3_IPC_MESSAGE_TYPE_GET_VERSION, "", &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  return retval;
}
