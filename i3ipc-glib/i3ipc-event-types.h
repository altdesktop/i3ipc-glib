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

#include <glib-object.h>

#include "i3ipc-con.h"

#ifndef __I3IPC_EVENT_TYPES_H__
#define __I3IPC_EVENT_TYPES_H__

#define I3IPC_TYPE_WORKSPACE_EVENT (i3ipc_workspace_event_get_type())
#define I3IPC_TYPE_GENERIC_EVENT (i3ipc_generic_event_get_type())
#define I3IPC_TYPE_WINDOW_EVENT (i3ipc_window_event_get_type())
#define I3IPC_TYPE_BARCONFIG_UPDATE_EVENT (i3ipc_barconfig_update_event_get_type())
#define I3IPC_TYPE_BINDING_INFO (i3ipc_binding_info_get_type())
#define I3IPC_TYPE_BINDING_EVENT (i3ipc_binding_event_get_type())

typedef struct _i3ipcWorkspaceEvent i3ipcWorkspaceEvent;
typedef struct _i3ipcGenericEvent i3ipcGenericEvent;
typedef struct _i3ipcWindowEvent i3ipcWindowEvent;
typedef struct _i3ipcBarconfigUpdateEvent i3ipcBarconfigUpdateEvent;
typedef struct _i3ipcBindingInfo i3ipcBindingInfo;
typedef struct _i3ipcBindingEvent i3ipcBindingEvent;

/**
 * i3ipcEvent:
 * @I3IPC_EVENT_WORKSPACE:
 * @I3IPC_EVENT_OUTPUT:
 * @I3IPC_EVENT_MODE:
 * @I3IPC_EVENT_WINDOW:
 * @I3IPC_EVENT_BARCONFIG_UPDATE:
 * @I3IPC_EVENT_BINDING:
 *
 * Event enumeration for #i3ipcConnection
 *
 * This enumeration can be extended at later date
 */
typedef enum { /*< underscore_name=i3ipc_event >*/
               I3IPC_EVENT_WORKSPACE = (1 << 0),
               I3IPC_EVENT_OUTPUT = (1 << 1),
               I3IPC_EVENT_MODE = (1 << 2),
               I3IPC_EVENT_WINDOW = (1 << 3),
               I3IPC_EVENT_BARCONFIG_UPDATE = (1 << 4),
               I3IPC_EVENT_BINDING = (1 << 5),
} i3ipcEvent;

/**
 * i3ipcWorkspaceEvent:
 * @change: indicates the type of change ("focus", "init", "empty", "urgent")
 * @current: when the change is "focus", the currently focused workspace
 * @old: when the change is "focus", the previously focused workspace
 *
 * The #i3ipcWorkspaceEvent contains data about a workspace event.
 */
struct _i3ipcWorkspaceEvent {
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
struct _i3ipcGenericEvent {
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
struct _i3ipcWindowEvent {
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
struct _i3ipcBarconfigUpdateEvent {
    gchar *id;
    gchar *hidden_state;
    gchar *mode;
};

i3ipcBarconfigUpdateEvent *i3ipc_barconfig_update_event_copy(i3ipcBarconfigUpdateEvent *event);
void i3ipc_barconfig_update_event_free(i3ipcBarconfigUpdateEvent *event);
GType i3ipc_barconfig_update_event_get_type(void);

/**
 * i3ipcBindingInfo:
 * @command: The i3 command that is configured to run for this binding.
 * @mods: (element-type utf8): The modifier keys that were configured with this binding.
 * @input_code: If the binding was configured with bindcode, this will be the
 * key code that was given for the binding. If the binding is a mouse binding,
 * it will be the number of the mouse button that was pressed. Otherwise it
 * will be 0.
 * @symbol: If this is a keyboard binding that was configured with bindsym,
 * this field will contain the given symbol.
 * @input_type: This will be "keyboard" or "mouse" depending on whether or not
 * this was a keyboard or a mouse binding.
 */
struct _i3ipcBindingInfo {
    gchar *command;
    GSList *mods;
    gint input_code;
    gchar *symbol;
    gchar *input_type;
};

i3ipcBindingInfo *i3ipc_binding_info_copy(i3ipcBindingInfo *info);
void i3ipc_binding_info_free(i3ipcBindingInfo *info);
GType i3ipc_binding_info_get_type(void);

/**
 * i3ipcBindingEvent:
 * @binding: A #i3ipcBindingInfo that contains info about this binding
 * @change: The type of binding event that was triggered (right now, only "run").
 */
struct _i3ipcBindingEvent {
    i3ipcBindingInfo *binding;
    gchar *change;
};

i3ipcBindingEvent *i3ipc_binding_event_copy(i3ipcBindingEvent *event);
void i3ipc_binding_event_free(i3ipcBindingEvent *event);
GType i3ipc_binding_event_get_type(void);

#endif /* __I3IPC_EVENT_TYPES_H__ */
