/* vim:ts=2:sw=2:expandtab
 *
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

/**
 * i3ipc_rect_copy:
 * @rect: an #i3ipcRect struct
 *
 * Creates a dynamically allocated i3ipc rect as a copy of @rect.
 *
 * Returns:(transfer full): a newly-allocated copy of @rect.
 */
i3ipcRect *i3ipc_rect_copy(i3ipcRect *rect) {
  i3ipcRect *retval;

  g_return_val_if_fail(rect != NULL, NULL);

  retval = g_slice_new0(i3ipcRect);
  *retval = *rect;

  return retval;
}

/**
 * i3ipc_rect_free:
 * @rect: (allow-none): an #i3ipcRect
 *
 * Frees @rect. If @rect is %NULL, it simply returns.
 */
void i3ipc_rect_free(i3ipcRect *rect) {
  if (!rect)
    return;

  g_slice_free(i3ipcRect, rect);
}

G_DEFINE_BOXED_TYPE(i3ipcRect, i3ipc_rect,
    i3ipc_rect_copy, i3ipc_rect_free);

struct _i3ipcConPrivate {
  gint id;
  gchar *name;
  gchar *border;
  gint current_border_width;
  gchar *layout;
  gchar *orientation;
  gfloat percent;
  gint window;
  gboolean urgent;
  gboolean focused;

  i3ipcRect *rect;
  GList *nodes;
  i3ipcCon *parent;
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

  PROP_RECT,
  PROP_PARENT,
  PROP_NODES,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void i3ipc_con_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  //i3ipcCon *self = I3IPC_CON(object);

  switch (property_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void i3ipc_con_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  i3ipcCon *self = I3IPC_CON(object);

  switch (property_id)
  {
    case PROP_ID:
      g_value_set_int(value, self->priv->id);
      break;

    case PROP_NAME:
      g_value_set_string(value, self->priv->name);
      break;

    case PROP_BORDER:
      g_value_set_string(value, self->priv->border);
      break;

    case PROP_CURRENT_BORDER_WIDTH:
      g_value_set_int(value, self->priv->current_border_width);
      break;

    case PROP_LAYOUT:
      g_value_set_string(value, self->priv->layout);
      break;

    case PROP_ORIENTATION:
      g_value_set_string(value, self->priv->orientation);
      break;

    case PROP_PERCENT:
      g_value_set_double(value, self->priv->percent);
      break;

    case PROP_WINDOW:
      g_value_set_int(value, self->priv->window);
      break;

    case PROP_URGENT:
      g_value_set_boolean(value, self->priv->urgent);
      break;

    case PROP_FOCUSED:
      g_value_set_boolean(value, self->priv->focused);
      break;

    case PROP_RECT:
      g_value_set_boxed(value, self->priv->rect);
      break;

    case PROP_PARENT:
      g_value_set_object(value, self->priv->parent);
      break;

    case PROP_NODES:
      g_value_set_pointer(value, self->priv->nodes);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_con_dispose(GObject *gobject) {
  i3ipcCon *self = I3IPC_CON(gobject);

  self->priv->parent = NULL;

  G_OBJECT_CLASS(i3ipc_con_parent_class)->dispose(gobject);
}

static void i3ipc_con_list_free_func(gpointer data) {
  if (data != NULL && I3IPC_IS_CON(data))
    g_clear_object(&data);
}

static void i3ipc_con_finalize(GObject *gobject) {
  i3ipcCon *self = I3IPC_CON(gobject);

  g_free(self->priv->layout);
  g_free(self->priv->orientation);
  g_free(self->priv->name);
  g_free(self->priv->border);
  i3ipc_rect_free(self->priv->rect);

  if (self->priv->nodes)
    g_list_free_full(self->priv->nodes, i3ipc_con_list_free_func);
}

static void i3ipc_con_class_init(i3ipcConClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->set_property = i3ipc_con_set_property;
  gobject_class->get_property = i3ipc_con_get_property;
  gobject_class->dispose = i3ipc_con_dispose;
  gobject_class->finalize = i3ipc_con_finalize;

  /**
   * i3ipcCon:id:
   */
  obj_properties[PROP_ID] =
    g_param_spec_int("id",
        "Con id",
        "The internal ID (actually a C pointer value) of this container. Do not make any assumptions about it. You can use it to (re-)identify and address containers when talking to i3.",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_NAME] =
    g_param_spec_string("name",
        "Con name",
        "The internal name of this container. For all containers which are part of the tree structure down to the workspace contents, this is set to a nice human-readable name of the container. For all other containers, the content is not defined (yet).",
        "", /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_BORDER] =
    g_param_spec_string("border",
        "Can be either \"normal\", \"none\" or \"1pixel\", dependending on the container’s border style.",
        "Set/get the con's border style",
        "", /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_CURRENT_BORDER_WIDTH] =
    g_param_spec_int("current-border-width",
        "Con current border width",
        "Number of pixels of the border width.",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_LAYOUT] =
    g_param_spec_string("layout",
        "Con layout",
        "Can be either \"splith\", \"splitv\", \"stacked\", \"tabbed\", \"dockarea\" or \"output\". Other values might be possible in the future, should we add new layouts.",
        "", /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_ORIENTATION] =
    g_param_spec_string("orientation",
        "Con orientation",
        "Can be either \"none\" (for non-split containers), \"horizontal\" or \"vertical\". THIS FIELD IS OBSOLETE. It is still present, but your code should not use it. Instead, rely on the layout field.",
        "", /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_PERCENT] =
    g_param_spec_float("percent",
        "Con percent",
        "The percentage which this container takes in its parent. A value of null means that the percent property does not make sense for this container, for example for the root container.",
        0, /* to -> */ G_MAXFLOAT,
        0, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_WINDOW] =
    g_param_spec_int("window",
        "Con window",
        "The X11 window ID of the actual client window inside this container. This field is set to null for split containers or otherwise empty containers. This ID corresponds to what xwininfo(1) and other X11-related tools display (usually in hex).",
        0, /* to -> */ G_MAXINT,
        0, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_URGENT] =
    g_param_spec_boolean("urgent",
        "Con urgent value",
        "Whether this container (window or workspace) has the urgency hint set.",
        FALSE, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_FOCUSED] =
    g_param_spec_boolean("focused",
        "Con focused",
        "Whether this container is currently focused.",
        FALSE, /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_PARENT] =
    g_param_spec_object("parent",
        "Con parent",
        "The con's parent",
        I3IPC_TYPE_CON,
        G_PARAM_READABLE);

  obj_properties[PROP_RECT] =
    g_param_spec_boxed("rect",
        "Con rect",
        "The con's rect",
        I3IPC_TYPE_RECT,
        G_PARAM_READABLE);

  /**
   * i3ipcCon:nodes:(type GList(i3ipcCon)):
   *
   * This property is a list of the con's nodes.
   */
  obj_properties[PROP_NODES] =
     g_param_spec_pointer("nodes",
         "Con nodes",
         "The con's nodes",
         G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);
}

static void i3ipc_con_init(i3ipcCon *self) {
  self->priv = i3ipc_con_get_instance_private(self);
  self->priv->rect = g_slice_new0(i3ipcRect);
  self->priv->nodes = NULL;
}

static void i3ipc_con_initialize_nodes(JsonArray *array, guint index_, JsonNode *element_node, gpointer user_data) {
  i3ipcCon *parent = I3IPC_CON(user_data);
  JsonObject *data = json_node_get_object(element_node);

  i3ipcCon *con = i3ipc_con_new(parent, data);

  parent->priv->nodes = g_list_append(parent->priv->nodes, con);
}

/**
 * i3ipc_con_new:
 * @parent:(allow-none): the parent of the con
 * @data: the json data of the con
 *
 * Allocates a new #i3ipcCon
 *
 * Returns:(transfer full): a new #i3ipcCon
 */
i3ipcCon *i3ipc_con_new(i3ipcCon *parent, JsonObject *data) {
  i3ipcCon *con;
  con = g_object_new(I3IPC_TYPE_CON, NULL);

  if (!json_object_get_null_member(data, "percent"))
    con->priv->percent = json_object_get_double_member(data, "percent");

  if (!json_object_get_null_member(data, "window"))
    con->priv->window = json_object_get_int_member(data, "window");

  con->priv->name = g_strdup(json_object_get_string_member(data, "name"));
  con->priv->focused = json_object_get_boolean_member(data, "focused");
  con->priv->urgent = json_object_get_boolean_member(data, "urgent");
  con->priv->layout = g_strdup(json_object_get_string_member(data, "layout"));
  con->priv->orientation = g_strdup(json_object_get_string_member(data, "orientation"));
  con->priv->current_border_width = json_object_get_int_member(data, "current_border_width");
  con->priv->border = g_strdup(json_object_get_string_member(data, "border"));
  con->priv->id = json_object_get_int_member(data, "id");

  con->priv->parent = parent;

  JsonObject *rect_data = json_object_get_object_member(data, "rect");

  con->priv->rect->x = json_object_get_int_member(rect_data, "x");
  con->priv->rect->y = json_object_get_int_member(rect_data, "y");
  con->priv->rect->width = json_object_get_int_member(rect_data, "width");
  con->priv->rect->height = json_object_get_int_member(rect_data, "height");

  JsonArray *nodes_array = json_object_get_array_member(data, "nodes");

  json_array_foreach_element(nodes_array, i3ipc_con_initialize_nodes, con);

  return con;
}

/**
 * i3ipc_con_get_nodes:
 * @self: an #i3ipcCon
 *
 * Returns:(transfer none) (element-type i3ipcCon): A list of child nodes.
 */
GList *i3ipc_con_get_nodes(i3ipcCon *self) {
  return self->priv->nodes;
}

/**
 * i3ipc_con_root:
 * @self: an #i3ipcCon
 *
 * Returns:(transfer none): The root node of the tree.
 */
i3ipcCon *i3ipc_con_root(i3ipcCon *self) {
  i3ipcCon *retval = self;
  i3ipcCon *parent = self->priv->parent;

  while (parent != NULL) {
    parent = retval->priv->parent;

    if (parent != NULL)
      retval = parent;
  }

  return retval;
}
