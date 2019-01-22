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

#ifndef __I3IPC_REPLY_TYPES_H__
#define __I3IPC_REPLY_TYPES_H__

#include <glib-object.h>

#include "i3ipc-con.h"

#define I3IPC_TYPE_COMMAND_REPLY (i3ipc_command_reply_get_type())
#define I3IPC_TYPE_VERSION_REPLY (i3ipc_version_reply_get_type())
#define I3IPC_TYPE_BAR_CONFIG_REPLY (i3ipc_bar_config_reply_get_type())
#define I3IPC_TYPE_OUTPUT_REPLY (i3ipc_output_reply_get_type())
#define I3IPC_TYPE_WORKSPACE_REPLY (i3ipc_workspace_reply_get_type())

typedef struct _i3ipcCommandReply i3ipcCommandReply;
typedef struct _i3ipcVersionReply i3ipcVersionReply;
typedef struct _i3ipcBarConfigReply i3ipcBarConfigReply;
typedef struct _i3ipcOutputReply i3ipcOutputReply;
typedef struct _i3ipcWorkspaceReply i3ipcWorkspaceReply;

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
 * i3ipcCommandReply:
 * @success: whether or not the command succeeded
 * @parse_error: whether or not this was a parse error
 * @error: An error message
 *
 * The #i3ipcCommandReply is the primary structure for accessing the reply of
 * an ipc command.
 */
struct _i3ipcCommandReply {
    gboolean success;
    gboolean parse_error;
    gchar *error;
    /* for the 'open' command used in tests */
    gint _id;
};

i3ipcCommandReply *i3ipc_command_reply_copy(i3ipcCommandReply *reply);
void i3ipc_command_reply_free(i3ipcCommandReply *reply);
GType i3ipc_command_reply_get_type(void);

/**
 * i3ipcVersionReply:
 * @major: The major version of i3.
 * @minor: The minor version of i3.
 * @patch: The patch version of i3.
 * @human_readable: A human-readable version of i3.
 *
 * The #i3ipcVersionReply is the primary structure for accessing the reply of
 * an ipc version command.
 */
struct _i3ipcVersionReply {
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
 * @strip_workspace_numbers:
 * @colors: (element-type utf8 utf8):
 *
 * The #i3ipcBarConfigReply is the primary structure for accessing the reply of
 * an ipc bar config command.
 */
struct _i3ipcBarConfigReply {
    gchar *id;
    gchar *mode;
    gchar *position;
    gchar *status_command;
    gchar *font;
    gboolean workspace_buttons;
    gboolean binding_mode_indicator;
    gboolean verbose;
    gboolean strip_workspace_numbers;
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
 * The #i3ipcOutputReply is the primary structure for accessing the reply of an
 * ipc output command.
 */
struct _i3ipcOutputReply {
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
struct _i3ipcWorkspaceReply {
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

#endif /* __I3IPC_REPLY_TYPES_H__ */
