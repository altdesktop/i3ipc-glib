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
#include "i3ipc-reply-types.h"

/**
 * i3ipc_command_reply_copy:
 * @reply: a #i3ipcCommandReply struct
 *
 * Creates a dynamically allocated i3ipc command reply as a copy of @reply.
 *
 * Returns: (transfer full): a newly-allocated copy of @reply
 */
i3ipcCommandReply *i3ipc_command_reply_copy(i3ipcCommandReply *reply) {
    i3ipcCommandReply *retval;

    g_return_val_if_fail(reply != NULL, NULL);

    retval = g_slice_new0(i3ipcCommandReply);
    *retval = *reply;

    retval->error = g_strdup(reply->error);

    return retval;
}

/**
 * i3ipc_command_reply_free:
 * @reply: (allow-none): a #i3ipcCommandReply
 *
 * Frees @reply. If @reply is %NULL, it simply returns.
 */
void i3ipc_command_reply_free(i3ipcCommandReply *reply) {
    if (!reply)
        return;

    g_free(reply->error);

    g_slice_free(i3ipcCommandReply, reply);
}

G_DEFINE_BOXED_TYPE(i3ipcCommandReply, i3ipc_command_reply, i3ipc_command_reply_copy,
                    i3ipc_command_reply_free);

/**
 * i3ipc_version_reply_copy:
 * @version: a #i3ipcVersionReply struct
 *
 * Creates a dynamically allocated i3ipc version reply as a copy of @version.
 *
 * Returns: (transfer full): a newly-allocated copy of @version
 */
i3ipcVersionReply *i3ipc_version_reply_copy(i3ipcVersionReply *version) {
    i3ipcVersionReply *retval;

    g_return_val_if_fail(version != NULL, NULL);

    retval = g_slice_new0(i3ipcVersionReply);
    *retval = *version;

    retval->human_readable = g_strdup(version->human_readable);

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

G_DEFINE_BOXED_TYPE(i3ipcVersionReply, i3ipc_version_reply, i3ipc_version_reply_copy,
                    i3ipc_version_reply_free);

static void bar_config_colors_copy_func(gpointer _key, gpointer _value, gpointer user_data) {
    gchar *key = _key;
    gchar *value = _value;
    GHashTable *copy = user_data;

    g_hash_table_insert(copy, g_strdup(key), g_strdup(value));
}

/**
 * i3ipc_bar_config_reply_copy:
 * @config: a #i3ipcBarConfigReply struct
 *
 * Creates a dynamically allocated i3ipc version reply as a copy of @config.
 *
 * Returns: (transfer full): a newly-allocated copy of @config
 */
i3ipcBarConfigReply *i3ipc_bar_config_reply_copy(i3ipcBarConfigReply *config) {
    i3ipcBarConfigReply *retval;

    g_return_val_if_fail(config != NULL, NULL);

    retval = g_slice_new0(i3ipcBarConfigReply);
    *retval = *config;

    retval->position = g_strdup(config->position);
    retval->font = g_strdup(config->font);
    retval->mode = g_strdup(config->mode);
    retval->id = g_strdup(config->id);
    retval->status_command = g_strdup(config->status_command);

    retval->colors = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_foreach(config->colors, bar_config_colors_copy_func, retval->colors);

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

    g_slice_free(i3ipcBarConfigReply, config);
}

G_DEFINE_BOXED_TYPE(i3ipcBarConfigReply, i3ipc_bar_config_reply, i3ipc_bar_config_reply_copy,
                    i3ipc_bar_config_reply_free);

/**
 * i3ipc_output_reply_copy:
 * @output: a #i3ipcOutputReply struct
 *
 * Creates a dynamically allocated i3ipc output reply as a copy of @output.
 *
 * Returns: (transfer full): a newly-allocated copy of @output
 */
i3ipcOutputReply *i3ipc_output_reply_copy(i3ipcOutputReply *output) {
    i3ipcOutputReply *retval;

    g_return_val_if_fail(output != NULL, NULL);

    retval = g_slice_new0(i3ipcOutputReply);
    *retval = *output;

    retval->name = g_strdup(output->name);
    retval->current_workspace = g_strdup(output->current_workspace);
    retval->rect = i3ipc_rect_copy(output->rect);

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

    g_slice_free(i3ipcOutputReply, output);
}

G_DEFINE_BOXED_TYPE(i3ipcOutputReply, i3ipc_output_reply, i3ipc_output_reply_copy,
                    i3ipc_output_reply_free);

/**
 * i3ipc_workspace_reply_copy:
 * @workspace: a #i3ipcWorkspaceReply
 *
 * Creates a dynamically allocated i3ipc workspace reply as a copy of
 * @workspace.
 *
 * Returns: (transfer full): a newly-allocated copy of @workspace
 */
i3ipcWorkspaceReply *i3ipc_workspace_reply_copy(i3ipcWorkspaceReply *workspace) {
    i3ipcWorkspaceReply *retval;

    g_return_val_if_fail(workspace != NULL, NULL);

    retval = g_slice_new0(i3ipcWorkspaceReply);
    *retval = *workspace;

    retval->name = g_strdup(workspace->name);
    retval->output = g_strdup(workspace->output);
    retval->rect = i3ipc_rect_copy(workspace->rect);

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

    g_slice_free(i3ipcWorkspaceReply, workspace);
}

G_DEFINE_BOXED_TYPE(i3ipcWorkspaceReply, i3ipc_workspace_reply, i3ipc_workspace_reply_copy,
                    i3ipc_workspace_reply_free);
