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

#include "i3ipc-con.h"
#include "i3ipc-event-types.h"
#include "i3ipc-reply-types.h"

#define I3IPC_MAGIC "i3-ipc"

/**
 * SECTION: i3ipc-connection
 * @short_description: A connection to the i3 ipc to query i3 for the state of
 * containers and to subscribe to window manager events.
 *
 * Use this class to query information from the window manager about the state
 * of the workspaces, windows, etc. You can also subscribe to events such as
 * when certain window or workspace properties change.
 *
 */

#define I3IPC_TYPE_CONNECTION (i3ipc_connection_get_type())
#define I3IPC_CONNECTION(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), I3IPC_TYPE_CONNECTION, i3ipcConnection))
#define I3IPC_IS_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), I3IPC_TYPE_CONNECTION))
#define I3IPC_CONNECTION_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), I3IPC_TYPE_CONNECTION, i3ipcConnectionClass))
#define I3IPC_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I3IPC_TYPE_CONNECTION))
#define I3IPC_CONNECTION_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), I3IPC_TYPE_CONNECTION, i3ipcConnectionClass))

typedef struct _i3ipcConnection i3ipcConnection;
typedef struct _i3ipcConnectionClass i3ipcConnectionClass;
typedef struct _i3ipcConnectionPrivate i3ipcConnectionPrivate;

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
 * @I3IPC_MESSAGE_TYPE_GET_BINDING_MODES:
 * @I3IPC_MESSAGE_TYPE_GET_CONFIG:
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
               I3IPC_MESSAGE_TYPE_GET_BINDING_MODES,
               I3IPC_MESSAGE_TYPE_GET_CONFIG,
} i3ipcMessageType;

struct _i3ipcConnection {
    GObject parent_instance;

    i3ipcConnectionPrivate *priv;
};

struct _i3ipcConnectionClass {
    GObjectClass parent_class;

    /* class members */
};

/* used by I3IPC_TYPE_CONNECTION */
GType i3ipc_connection_get_type(void);

i3ipcConnection *i3ipc_connection_new(const gchar *socket_path, GError **err);

/* Method definitions */

gchar *i3ipc_connection_message(i3ipcConnection *self, i3ipcMessageType message_type,
                                const gchar *payload, GError **err);

GSList *i3ipc_connection_command(i3ipcConnection *self, const gchar *command, GError **err);

i3ipcCommandReply *i3ipc_connection_subscribe(i3ipcConnection *self, i3ipcEvent events,
                                              GError **err);

i3ipcConnection *i3ipc_connection_on(i3ipcConnection *self, const gchar *event, GClosure *callback,
                                     GError **err);

GSList *i3ipc_connection_get_workspaces(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_outputs(i3ipcConnection *self, GError **err);

i3ipcCon *i3ipc_connection_get_tree(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_marks(i3ipcConnection *self, GError **err);

GSList *i3ipc_connection_get_bar_config_list(i3ipcConnection *self, GError **err);

i3ipcBarConfigReply *i3ipc_connection_get_bar_config(i3ipcConnection *self, const gchar *bar_id,
                                                     GError **err);

i3ipcVersionReply *i3ipc_connection_get_version(i3ipcConnection *self, GError **err);

gchar *i3ipc_connection_get_config(i3ipcConnection *self, GError **err);

void i3ipc_connection_main(i3ipcConnection *self);

void i3ipc_connection_main_with_context(i3ipcConnection *self, GMainContext *context);

void i3ipc_connection_main_quit(i3ipcConnection *self);

#endif
