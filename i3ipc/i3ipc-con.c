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

#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "i3ipc-con.h"

struct _i3ipcConPrivate {
  gint reserved;
};

G_DEFINE_TYPE_WITH_PRIVATE (i3ipcCon, i3ipc_con, G_TYPE_OBJECT);

enum {
  PROP_0,

  PROP_ID,
  PROP_NAME,
  PROP_BORDER,
  PROP_CURRENT_BORDER_WIDTH,
  PROP_LAYOUT,
  PROP_ORIENTATION,
  PROP_PERCENT,
  PROP_WINDOW,
  PROP_URGENT,
  PROP_FOCUSED,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void i3ipc_con_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  i3ipcCon *self = I3IPC_CON(object);

  switch (property_id)
  {
    case PROP_ID:
      self->id = g_value_get_int(value);
      break;

    case PROP_NAME:
      g_free(self->name);
      self->name = g_value_dup_string(value);
      break;

    case PROP_BORDER:
      g_free(self->border);
      self->border = g_value_dup_string(value);
      break;

    case PROP_CURRENT_BORDER_WIDTH:
      self->current_border_width = g_value_get_int(value);
      break;

    case PROP_LAYOUT:
      g_free(self->layout);
      self->layout = g_value_dup_string(value);
      break;

    case PROP_ORIENTATION:
      g_free(self->orientation);
      self->orientation = g_value_dup_string(value);
      break;

    case PROP_PERCENT:
      self->percent = g_value_get_float(value);
      break;

    case PROP_WINDOW:
      self->window = g_value_get_int(value);
      break;

    case PROP_URGENT:
      self->urgent = g_value_get_boolean(value);
      break;

    case PROP_FOCUSED:
      self->focused = g_value_get_boolean(value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_con_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  i3ipcCon *self = I3IPC_CON(object);

  switch (property_id)
  {
    case PROP_ID:
      g_value_set_int(value, self->id);
      break;

    case PROP_NAME:
      g_value_set_string(value, self->name);
      break;

    case PROP_BORDER:
      g_value_set_string(value, self->border);
      break;

    case PROP_CURRENT_BORDER_WIDTH:
      g_value_set_int(value, self->current_border_width);
      break;

    case PROP_LAYOUT:
      g_value_set_string(value, self->layout);
      break;

    case PROP_ORIENTATION:
      g_value_set_string(value, self->orientation);
      break;

    case PROP_PERCENT:
      g_value_set_float(value, self->percent);
      break;

    case PROP_WINDOW:
      g_value_set_int(value, self->window);
      break;

    case PROP_URGENT:
      g_value_set_boolean(value, self->urgent);
      break;

    case PROP_FOCUSED:
      g_value_set_boolean(value, self->focused);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_con_class_init(i3ipcConClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->set_property = i3ipc_con_set_property;
  gobject_class->get_property = i3ipc_con_get_property;

  obj_properties[PROP_ID] =
    g_param_spec_int("con-id",
        "Con id",
        "The internal ID (actually a C pointer value) of this container. Do not make any assumptions about it. You can use it to (re-)identify and address containers when talking to i3.",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_NAME] =
    g_param_spec_string("con-name",
        "Con name",
        "The internal name of this container. For all containers which are part of the tree structure down to the workspace contents, this is set to a nice human-readable name of the container. For all other containers, the content is not defined (yet).",
        "", /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_BORDER] =
    g_param_spec_string("con-border",
        "Can be either \"normal\", \"none\" or \"1pixel\", dependending on the container’s border style.",
        "Set/get the con's border style",
        "", /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_CURRENT_BORDER_WIDTH] =
    g_param_spec_int("con-current-border-width",
        "Con current border width",
        "Number of pixels of the border width.",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_LAYOUT] =
    g_param_spec_string("con-layout",
        "Con layout",
        "Can be either \"splith\", \"splitv\", \"stacked\", \"tabbed\", \"dockarea\" or \"output\". Other values might be possible in the future, should we add new layouts.",
        "", /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_ORIENTATION] =
    g_param_spec_string("con-orientation",
        "Con orientation",
        "Can be either \"none\" (for non-split containers), \"horizontal\" or \"vertical\". THIS FIELD IS OBSOLETE. It is still present, but your code should not use it. Instead, rely on the layout field.",
        "", /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_PERCENT] =
    g_param_spec_float("con-percent",
        "Con percent",
        "The percentage which this container takes in its parent. A value of null means that the percent property does not make sense for this container, for example for the root container.",
        0, /* to -> */ G_MAXFLOAT,
        0, /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_WINDOW] =
    g_param_spec_int("con-window",
        "Con window",
        "The X11 window ID of the actual client window inside this container. This field is set to null for split containers or otherwise empty containers. This ID corresponds to what xwininfo(1) and other X11-related tools display (usually in hex).",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_URGENT] =
    g_param_spec_boolean("con-urgent",
        "Con urgent value",
        "Whether this container (window or workspace) has the urgency hint set.",
        FALSE, /* default */
        G_PARAM_READWRITE);

  obj_properties[PROP_FOCUSED] =
    g_param_spec_boolean("con-focused",
        "Con focused",
        "Whether this container is currently focused.",
        FALSE, /* default */
        G_PARAM_READWRITE);

  g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);
}

static void i3ipc_con_init(i3ipcCon *self) {
  self->priv = i3ipc_con_get_instance_private(self);
}

/**
 * i3ipc_con_new:
 *
 * Allocates a new #i3ipcCon
 *
 * Returns: a new #i3ipcCon
 *
 */
i3ipcCon *i3ipc_con_new() {
  i3ipcCon *con;
  con = g_object_new(I3IPC_TYPE_CON, NULL);
  return con;
}
