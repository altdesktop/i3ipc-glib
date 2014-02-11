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

#include <glib-object.h>

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

#define I3IPC_TYPE_VERSION_REPLY          (i3ipc_version_reply_get_type ())

typedef struct _i3ipcVersionReply      i3ipcVersionReply;
typedef struct _i3ipcConnection        i3ipcConnection;
typedef struct _i3ipcConnectionClass   i3ipcConnectionClass;
typedef struct _i3ipcConnectionPrivate i3ipcConnectionPrivate;

/**
 * i3ipcMessageType:
 * @I3IPC_MESSAGE_TYPE_COMMAND:
 * @I3IPC_MESSAGE_TYPE_GET_WORKSPACES:
 * @I3IPC_MESSAGE_TYPE_GET_SUBSCRIBE:
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

  GIOChannel *cmd_channel;
  GIOChannel *sub_channel;

  i3ipcConnectionPrivate *priv;
};

struct _i3ipcConnectionClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by I3IPC_TYPE_CONNECTION */
GType i3ipc_connection_get_type(void);

i3ipcConnection *i3ipc_connection_new(void);

/* Method definitions */

void i3ipc_connection_connect(i3ipcConnection *self);

gboolean i3ipc_connection_command(i3ipcConnection *self, gchar *command);

void i3ipc_connection_on(i3ipcConnection *self, gchar *event, GClosure *callback);

gchar *i3ipc_connection_get_workspaces(i3ipcConnection *self);

gchar *i3ipc_connection_get_outputs(i3ipcConnection *self);

gchar *i3ipc_connection_get_tree(i3ipcConnection *self);

GVariant *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err);

GVariant *i3ipc_connection_get_bar_config(i3ipcConnection *self, gchar *bar_id, GError **err);

GVariant *i3ipc_connection_get_version(i3ipcConnection *self, GError **err);

#endif
