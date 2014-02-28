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

#ifndef __I3IPC_CONNECTION_H__
#define __I3IPC_CONNECTION_H__

#include <json-glib/json-glib.h>
#include <glib-object.h>

#include "i3ipc-con.h"

#define I3IPC_MAGIC "i3-ipc"

/**
  * SECTION: i3ipc-connection
  * @short_description: A connection to the i3 IPC.
  *
  * The #i3ipcConnection is a class to send messages to i3.
  */

#define I3IPC_TYPE_CONNECTION             (i3ipc_connection_get_type ())
#define I3IPC_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), I3IPC_TYPE_CONNECTION, i3ipcConnection))
#define I3IPC_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), I3IPC_TYPE_CONNECTION))
#define I3IPC_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), I3IPC_TYPE_CONNECTION, i3ipcConnectionClass))
#define I3IPC_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), I3IPC_TYPE_CONNECTION))
#define I3IPC_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), I3IPC_TYPE_CONNECTION, i3ipcConnectionClass))

#define I3IPC_TYPE_COMMAND_REPLY          (i3ipc_command_reply_get_type ())
#define I3IPC_TYPE_VERSION_REPLY          (i3ipc_version_reply_get_type ())
#define I3IPC_TYPE_BAR_CONFIG_REPLY       (i3ipc_bar_config_reply_get_type ())
#define I3IPC_TYPE_OUTPUT_REPLY           (i3ipc_output_reply_get_type ())
#define I3IPC_TYPE_WORKSPACE_REPLY        (i3ipc_workspace_reply_get_type ())

#define I3IPC_TYPE_WORKSPACE_EVENT        (i3ipc_workspace_event_get_type ())
#define I3IPC_TYPE_GENERIC_EVENT          (i3ipc_generic_event_get_type ())
#define I3IPC_TYPE_WINDOW_EVENT           (i3ipc_window_event_get_type ())
#define I3IPC_TYPE_BARCONFIG_UPDATE_EVENT (i3ipc_barconfig_update_event_get_type ())

typedef struct _i3ipcCommandReply            i3ipcCommandReply;
typedef struct _i3ipcVersionReply            i3ipcVersionReply;
typedef struct _i3ipcBarConfigReply          i3ipcBarConfigReply;
typedef struct _i3ipcOutputReply             i3ipcOutputReply;
typedef struct _i3ipcWorkspaceReply          i3ipcWorkspaceReply;

typedef struct _i3ipcWorkspaceEvent          i3ipcWorkspaceEvent;
typedef struct _i3ipcGenericEvent            i3ipcGenericEvent;
typedef struct _i3ipcWindowEvent             i3ipcWindowEvent;
typedef struct _i3ipcBarconfigUpdateEvent    i3ipcBarconfigUpdateEvent;

typedef struct _i3ipcConnection        i3ipcConnection;
typedef struct _i3ipcConnectionClass   i3ipcConnectionClass;
typedef struct _i3ipcConnectionPrivate i3ipcConnectionPrivate;

/**
 * i3ipcCommandReply:
 * @success: whether or not the command succeeded
 * @parse_error: whether or not this was a parse error
 * @error: An error message
 *
 * The #i3ipcCommandReply is the primary structure for accessing the reply of
 * an ipc command.
 */
struct _i3ipcCommandReply
{
  gboolean success;
  gboolean parse_error;
  gchar *error;
};

i3ipcCommandReply *i3ipc_command_reply_copy(i3ipcCommandReply *reply);
void i3ipc_command_reply_free(i3ipcCommandReply *reply);
GType i3ipc_command_reply_get_type(void);

/**
 * i3ipcVersionReply:
 * @major: The major version of i3, such as 4.
 * @minor: The minor version of i3, such as 2.
 * @patch: The patch version of i3, such as 1.
 * @human_readable: A human-readable version of i3.
 *
 * The #i3ipcVersionReply is the primary structure for accessing the reply of
 * an ipc version command.
 */
struct _i3ipcVersionReply
{
  gint major;
  gint minor;
  gint patch;
  gchar *human_readable;
};

i3ipcVersionReply *i3ipc_version_reply_copy(i3ipcVersionReply *version);
void i3ipc_version_reply_free(i3ipcVersionReply *version);
GType i3ipc_version_reply_get_type(void);

/**
 * i3ipcBarConfigReply:
 * @id:
 * @mode:
 * @position:
 * @status_command:
 * @font:
 * @workspace_buttons:
 * @binding_mode_indicator:
 * @verbose:
 * @colors: (element-type utf8 utf8):
 *
 * The #i3ipcBarConfigReply is the primary structure for accessing the reply of
 * an ipc bar config command.
 */
struct _i3ipcBarConfigReply
{
  gchar *id;
  gchar *mode;
  gchar *position;
  gchar *status_command;
  gchar *font;
  gboolean workspace_buttons;
  gboolean binding_mode_indicator;
  gboolean verbose;
  GHashTable *colors;
};

i3ipcBarConfigReply *i3ipc_bar_config_reply_copy(i3ipcBarConfigReply *config);
void i3ipc_bar_config_reply_free(i3ipcBarConfigReply *config);
GType i3ipc_bar_config_reply_get_type(void);

/**
 * i3ipcOutputReply:
 * @name:
 * @active:
 * @current_workspace:
 * @rect:
 *
 * The #i3ipcOutputReply is the primary structure for accessing the reply of an ipc output command.
 */
struct _i3ipcOutputReply
{
  gchar *name;
  gboolean active;
  gchar *current_workspace;
  i3ipcRect *rect;
};

i3ipcOutputReply *i3ipc_output_reply_copy(i3ipcOutputReply *output);
void i3ipc_output_reply_free(i3ipcOutputReply *output);
GType i3ipc_output_reply_get_type(void);

/**
 * i3ipcWorkspaceReply:
 * @num:
 * @name:
 * @visible:
 * @focused:
 * @urgent:
 * @output:
 * @rect:
 *
 * The #i3ipcWorkspaceReply is the primary structure for accessing the reply of
 * an ipc workspace command.
 */
struct _i3ipcWorkspaceReply
{
  gint num;
  gchar *name;
  gboolean visible;
  gboolean focused;
  gboolean urgent;
  gchar *output;
  i3ipcRect *rect;
};

i3ipcWorkspaceReply *i3ipc_workspace_reply_copy(i3ipcWorkspaceReply *workspace);
void i3ipc_workspace_reply_free(i3ipcWorkspaceReply *workspace);
GType i3ipc_workspace_reply_get_type(void);

/**
 * i3ipcWorkspaceEvent:
 * @change: indicates the type of change ("focus", "init", "empty", "urgent")
 * @current: when the change is "focus", the currently focused workspace
 * @old: when the change is "focus", the previously focused workspace
 *
 * The #i3ipcWorkspaceEvent contains data about a workspace event.
 */
struct _i3ipcWorkspaceEvent
{
  gchar *change;
  i3ipcCon *current;
  i3ipcCon *old;
};

i3ipcWorkspaceEvent *i3ipc_workspace_event_copy(i3ipcWorkspaceEvent *event);
void i3ipc_workspace_event_free(i3ipcWorkspaceEvent *event);
GType i3ipc_workspace_event_get_type(void);

/**
 * i3ipcGenericEvent:
 * @change: details about what changed
 *
 * A #i3ipcGenericEvent contains a description of what has changed.
 */
struct _i3ipcGenericEvent
{
  gchar *change;
};

i3ipcGenericEvent *i3ipc_generic_event_copy(i3ipcGenericEvent *event);
void i3ipc_generic_event_free(i3ipcGenericEvent *event);
GType i3ipc_generic_event_get_type(void);

/**
 * i3ipcWindowEvent:
 * @change: details about what changed
 * @container: The window's parent container
 *
 * A #i3ipcWindowEvent contains data about a window event.
 */
struct _i3ipcWindowEvent
{
  gchar *change;
  i3ipcCon *container;
};

i3ipcWindowEvent *i3ipc_window_event_copy(i3ipcWindowEvent *event);
void i3ipc_window_event_free(i3ipcWindowEvent *event);
GType i3ipc_window_event_get_type(void);

/**
 * i3ipcBarconfigUpdateEvent:
 * @id: specifies which bar instance the sent config update belongs
 * @hidden_state: indicates the hidden_state of the i3bar instance
 * @mode: corresponds to the current mode
 *
 * An #i3ipcBarconfigUpdateEvent reports options from the barconfig of the
 * specified bar_id that were updated in i3.
 */
struct _i3ipcBarconfigUpdateEvent
{
  gchar *id;
  gchar *hidden_state;
  gchar *mode;
};

i3ipcBarconfigUpdateEvent *i3ipc_barconfig_update_event_copy(i3ipcBarconfigUpdateEvent *event);
void i3ipc_barconfig_update_event_free(i3ipcBarconfigUpdateEvent *event);
GType i3ipc_barconfig_update_event_get_type(void);

/**
 * i3ipcMessageType:
 * @I3IPC_MESSAGE_TYPE_COMMAND:
 * @I3IPC_MESSAGE_TYPE_GET_WORKSPACES:
 * @I3IPC_MESSAGE_TYPE_SUBSCRIBE:
 * @I3IPC_MESSAGE_TYPE_GET_OUTPUTS:
 * @I3IPC_MESSAGE_TYPE_GET_TREE:
 * @I3IPC_MESSAGE_TYPE_GET_MARKS:
 * @I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG:
 * @I3IPC_MESSAGE_TYPE_GET_VERSION:
 *
 * Message type enumeration for #i3ipcConnection
 *
 * This enumeration can be extended at later date
 */
typedef enum { /*< underscore_name=i3ipc_message_type >*/
  I3IPC_MESSAGE_TYPE_COMMAND,
  I3IPC_MESSAGE_TYPE_GET_WORKSPACES,
  I3IPC_MESSAGE_TYPE_SUBSCRIBE,
  I3IPC_MESSAGE_TYPE_GET_OUTPUTS,
  I3IPC_MESSAGE_TYPE_GET_TREE,
  I3IPC_MESSAGE_TYPE_GET_MARKS,
  I3IPC_MESSAGE_TYPE_GET_BAR_CONFIG,
  I3IPC_MESSAGE_TYPE_GET_VERSION,
} i3ipcMessageType;

/**
 * i3ipcReplyType:
 * @I3IPC_REPLY_TYPE_COMMAND:
 * @I3IPC_REPLY_TYPE_WORKSPACES:
 * @I3IPC_REPLY_TYPE_SUBSCRIBE:
 * @I3IPC_REPLY_TYPE_OUTPUTS:
 * @I3IPC_REPLY_TYPE_TREE:
 * @I3IPC_REPLY_TYPE_MARKS:
 * @I3IPC_REPLY_TYPE_BAR_CONFIG:
 * @I3IPC_REPLY_TYPE_VERSION:
 *
 * Reply type enumeration for #i3ipcConnection
 *
 * This enumeration can be extended at later date
 */
typedef enum { /*< underscore_name=i3ipc_reply_type >*/
  I3IPC_REPLY_TYPE_COMMAND,
  I3IPC_REPLY_TYPE_WORKSPACES,
  I3IPC_REPLY_TYPE_SUBSCRIBE,
  I3IPC_REPLY_TYPE_OUTPUTS,
  I3IPC_REPLY_TYPE_TREE,
  I3IPC_REPLY_TYPE_MARKS,
  I3IPC_REPLY_TYPE_BAR_CONFIG,
  I3IPC_REPLY_TYPE_VERSION,
} i3ipcReplyType;

/**
 * i3ipcEvent:
 * @I3IPC_EVENT_WORKSPACE:
 * @I3IPC_EVENT_OUTPUT:
 * @I3IPC_EVENT_MODE:
 * @I3IPC_EVENT_WINDOW:
 * @I3IPC_EVENT_BARCONFIG_UPDATE:
 *
 * Event enumeration for #i3ipcConnection
 *
 * This enumeration can be extended at later date
 */
typedef enum { /*< underscore_name=i3ipc_event >*/
  I3IPC_EVENT_WORKSPACE =         (1 << 0),
  I3IPC_EVENT_OUTPUT =            (1 << 1),
  I3IPC_EVENT_MODE =              (1 << 2),
  I3IPC_EVENT_WINDOW =            (1 << 3),
  I3IPC_EVENT_BARCONFIG_UPDATE =  (1 << 4),
} i3ipcEvent;

struct _i3ipcConnection
{
  GObject parent_instance;

  i3ipcConnectionPrivate *priv;
};

struct _i3ipcConnectionClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by I3IPC_TYPE_CONNECTION */
GType i3ipc_connection_get_type(void);

i3ipcConnection *i3ipc_connection_new(gchar *socket_path, GError **err);

/* Method definitions */

gchar *i3ipc_connection_message(i3ipcConnection *self, i3ipcMessageType message_type, gchar *payload, GError **err);

GSList *i3ipc_connection_command(i3ipcConnection *self, gchar *command, GError **err);

i3ipcCommandReply *i3ipc_connection_subscribe(i3ipcConnection *self, i3ipcEvent events, GError **err);

i3ipcConnection *i3ipc_connection_on(i3ipcConnection *self, gchar *event, GClosure *callback, GError **err);

GSList *i3ipc_connection_get_workspaces(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_outputs(i3ipcConnection *self, GError **err);

i3ipcCon *i3ipc_connection_get_tree(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_bar_config_list(i3ipcConnection *self, GError **err);

i3ipcBarConfigReply *i3ipc_connection_get_bar_config(i3ipcConnection *self, gchar *bar_id, GError **err);

i3ipcVersionReply *i3ipc_connection_get_version(i3ipcConnection *self, GError **err);

#endif
