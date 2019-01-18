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

#include <glib-object.h>

#include "i3ipc-con.h"
#include "i3ipc-enum-types.h"
#include "i3ipc-event-types.h"

/**
 * i3ipc_workspace_event_copy:
 * @event: a #i3ipcWorkspaceEvent
 *
 * Creates a dynamically allocated i3ipc workspace event data container as a copy
 * of @event.
 *
 * Returns: (transfer full): a newly-allocated copy of @event
 */
i3ipcWorkspaceEvent *i3ipc_workspace_event_copy(i3ipcWorkspaceEvent *event) {
    i3ipcWorkspaceEvent *retval;

    g_return_val_if_fail(event != NULL, NULL);

    retval = g_slice_new0(i3ipcWorkspaceEvent);
    *retval = *event;

    retval->change = g_strdup(event->change);

    if (event->current && I3IPC_IS_CON(event->current))
        g_object_ref(event->current);

    if (event->old && I3IPC_IS_CON(event->old))
        g_object_ref(event->old);

    return retval;
}

/**
 * i3ipc_workspace_event_free:
 * @event: (allow-none): a #i3ipcWorkspaceEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_workspace_event_free(i3ipcWorkspaceEvent *event) {
    if (!event)
        return;

    g_free(event->change);

    if (event->current && I3IPC_IS_CON(event->current))
        g_clear_object(&event->current);

    if (event->old && I3IPC_IS_CON(event->old))
        g_clear_object(&event->old);

    g_slice_free(i3ipcWorkspaceEvent, event);
}

G_DEFINE_BOXED_TYPE(i3ipcWorkspaceEvent, i3ipc_workspace_event, i3ipc_workspace_event_copy,
                    i3ipc_workspace_event_free);

/**
 * i3ipc_generic_event_copy:
 * @event: a #i3ipcGenericEvent
 *
 * Creates a dynamically allocated i3ipc generic event data container as a copy
 * of @event.
 *
 * Returns: (transfer full): a newly-allocated copy of @event
 */
i3ipcGenericEvent *i3ipc_generic_event_copy(i3ipcGenericEvent *event) {
    i3ipcGenericEvent *retval;

    g_return_val_if_fail(event != NULL, NULL);

    retval = g_slice_new0(i3ipcGenericEvent);
    *retval = *event;

    retval->change = g_strdup(event->change);

    return retval;
}

/**
 * i3ipc_generic_event_free:
 * @event: (allow-none): a #i3ipcGenericEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_generic_event_free(i3ipcGenericEvent *event) {
    if (!event)
        return;

    g_free(event->change);

    g_slice_free(i3ipcGenericEvent, event);
}

G_DEFINE_BOXED_TYPE(i3ipcGenericEvent, i3ipc_generic_event, i3ipc_generic_event_copy,
                    i3ipc_generic_event_free);

/**
 * i3ipc_window_event_copy:
 * @event: a #i3ipcWindowEvent
 *
 * Creates a dynamically allocated i3ipc window event data container as a copy
 * of @event.
 *
 * Returns: (transfer full): a newly-allocated copy of @event
 */
i3ipcWindowEvent *i3ipc_window_event_copy(i3ipcWindowEvent *event) {
    i3ipcWindowEvent *retval;

    g_return_val_if_fail(event != NULL, NULL);

    retval = g_slice_new0(i3ipcWindowEvent);
    *retval = *event;

    retval->change = g_strdup(event->change);
    g_object_ref(event->container);

    return retval;
}

/**
 * i3ipc_window_event_free:
 * @event: (allow-none): a #i3ipcWindowEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_window_event_free(i3ipcWindowEvent *event) {
    if (!event)
        return;

    g_free(event->change);

    g_clear_object(&event->container);

    g_slice_free(i3ipcWindowEvent, event);
}

G_DEFINE_BOXED_TYPE(i3ipcWindowEvent, i3ipc_window_event, i3ipc_window_event_copy,
                    i3ipc_window_event_free);

/**
 * i3ipc_barconfig_update_event_copy:
 * @event: a #i3ipcBarconfigUpdateEvent
 *
 * Creates a dynamically allocated i3ipc barconfig update event data container
 * as a copy of @event.
 * Returns: (transfer full): a newly-allocated copy of @event
 */
i3ipcBarconfigUpdateEvent *i3ipc_barconfig_update_event_copy(i3ipcBarconfigUpdateEvent *event) {
    i3ipcBarconfigUpdateEvent *retval;

    g_return_val_if_fail(event != NULL, NULL);

    retval = g_slice_new0(i3ipcBarconfigUpdateEvent);
    *retval = *event;

    retval->hidden_state = g_strdup(event->hidden_state);
    retval->id = g_strdup(event->id);
    retval->mode = g_strdup(event->mode);

    return retval;
}

/**
 * i3ipc_barconfig_update_free:
 * @event: (allow-none): a #i3ipcBarconfigUpdateEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_barconfig_update_event_free(i3ipcBarconfigUpdateEvent *event) {
    if (!event)
        return;

    g_free(event->id);
    g_free(event->hidden_state);
    g_free(event->mode);

    g_slice_free(i3ipcBarconfigUpdateEvent, event);
}

G_DEFINE_BOXED_TYPE(i3ipcBarconfigUpdateEvent, i3ipc_barconfig_update_event,
                    i3ipc_barconfig_update_event_copy, i3ipc_barconfig_update_event_free);

/**
 * i3ipc_binding_info_copy:
 * @info: a #i3ipcBindingInfo
 *
 * Creates a dynamically allocated i3ipc binding info container as a copy of
 * @info.
 * Returns: (transfer full): a newly-allocated copy of @info
 */
i3ipcBindingInfo *i3ipc_binding_info_copy(i3ipcBindingInfo *info) {
    i3ipcBindingInfo *retval;

    g_return_val_if_fail(info != NULL, NULL);

    retval = g_slice_new0(i3ipcBindingInfo);
    *retval = *info;

    retval->command = g_strdup(info->command);
    retval->symbol = g_strdup(info->symbol);
    retval->input_type = g_strdup(info->input_type);
    retval->mods = g_slist_copy_deep(info->mods, (GCopyFunc)g_strdup, NULL);

    return retval;
}

/**
 * i3ipc_binding_info_free:
 * @info: (allow-none): a #i3ipcBindingInfo
 *
 * Frees @info. If @info is %NULL, it simply returns.
 */
void i3ipc_binding_info_free(i3ipcBindingInfo *info) {
    if (!info)
        return;

    g_free(info->command);
    g_free(info->input_type);
    g_free(info->symbol);
    g_slist_free_full(info->mods, g_free);

    g_slice_free(i3ipcBindingInfo, info);
}

G_DEFINE_BOXED_TYPE(i3ipcBindingInfo, i3ipc_binding_info, i3ipc_binding_info_copy,
                    i3ipc_binding_info_free);

/**
 * i3ipc_binding_event_copy:
 * @event: a #i3ipcBindingEvent
 *
 * Creates a dynamically allocated i3ipc binding event data container as a copy
 * of @event.
 * Returns: (transfer full): a newly-allocated copy of @event
 */
i3ipcBindingEvent *i3ipc_binding_event_copy(i3ipcBindingEvent *event) {
    i3ipcBindingEvent *retval;

    g_return_val_if_fail(event != NULL, NULL);

    retval = g_slice_new0(i3ipcBindingEvent);
    *retval = *event;

    retval->binding = i3ipc_binding_info_copy(event->binding);
    retval->change = g_strdup(event->change);

    return retval;
}

/**
 * i3ipc_binding_event_free:
 * @event: (allow-none): a #i3ipcBindingEvent
 *
 * Frees @event. If @event is %NULL, it simply returns.
 */
void i3ipc_binding_event_free(i3ipcBindingEvent *event) {
    if (!event)
        return;

    g_free(event->change);
    i3ipc_binding_info_free(event->binding);

    g_slice_free(i3ipcBindingEvent, event);
}

G_DEFINE_BOXED_TYPE(i3ipcBindingEvent, i3ipc_binding_event, i3ipc_binding_event_copy,
                    i3ipc_binding_event_free);
