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

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "i3ipc-connection.h"
#include "i3ipc-con-private.h"

/**
 * i3ipc_rect_copy:
 * @rect: an #i3ipcRect struct
 *
 * Creates a dynamically allocated i3ipc rect as a copy of @rect.
 *
 * Returns: (transfer full): a newly-allocated copy of @rect.
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
  gchar *type;
  gchar *window_class;

  i3ipcConnection *conn;
  i3ipcRect *rect;
  GList *nodes;
  GList *floating_nodes;
  i3ipcCon *parent;
};

G_DEFINE_TYPE(i3ipcCon, i3ipc_con, G_TYPE_OBJECT);

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
  PROP_TYPE,
  PROP_WINDOW_CLASS,

  PROP_RECT,
  PROP_PARENT,
  PROP_NODES,
  PROP_FLOATING_NODES,

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

    case PROP_TYPE:
      g_value_set_string(value, self->priv->type);
      break;

    case PROP_WINDOW_CLASS:
      g_value_set_string(value, self->priv->window_class);
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

    case PROP_FLOATING_NODES:
      g_value_set_pointer(value, self->priv->floating_nodes);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void i3ipc_con_dispose(GObject *gobject) {
  i3ipcCon *self = I3IPC_CON(gobject);

  self->priv->parent = NULL;

  self->priv->rect = (i3ipc_rect_free(self->priv->rect), NULL);

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
  g_free(self->priv->type);
  g_free(self->priv->window_class);

  g_object_unref(self->priv->conn);

  if (self->priv->parent)
    g_object_unref(self->priv->parent);

  if (self->priv->nodes)
    g_list_free_full(self->priv->nodes, i3ipc_con_list_free_func);

  if (self->priv->floating_nodes)
    g_list_free_full(self->priv->floating_nodes, i3ipc_con_list_free_func);

  G_OBJECT_CLASS(i3ipc_con_parent_class)->finalize(gobject);
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
  obj_properties[PROP_TYPE] =
    g_param_spec_string("type",
        "Con type",
        "What type of container this is",
        "", /* default */
        G_PARAM_READABLE);

  obj_properties[PROP_WINDOW_CLASS] =
    g_param_spec_string("window_class",
        "Con window class",
        "The class of the window according to WM_CLASS",
        "", /* default */
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
   * i3ipcCon:nodes: (type GList(i3ipcCon)):
   *
   * This property is a list of the con's nodes.
   */
  obj_properties[PROP_NODES] =
     g_param_spec_pointer("nodes",
         "Con nodes",
         "The con's nodes",
         G_PARAM_READABLE);
  /**
   * i3ipcCon:floating-nodes: (type GList(i3ipcCon)):
   *
   * This property is a list of the con's floating nodes.
   */
  obj_properties[PROP_FLOATING_NODES] =
     g_param_spec_pointer("floating-nodes",
         "Con floating nodes",
         "The con's floating nodes",
         G_PARAM_READABLE);

  g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);

  g_type_class_add_private(klass, sizeof(i3ipcConPrivate));
}

static void i3ipc_con_init(i3ipcCon *self) {
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, I3IPC_TYPE_CON, i3ipcConPrivate);
  self->priv->rect = g_slice_new0(i3ipcRect);
  self->priv->nodes = NULL;
  self->priv->floating_nodes = NULL;
}

static void i3ipc_con_initialize_nodes(JsonArray *array, guint index_, JsonNode *element_node, gpointer user_data) {
  i3ipcCon *parent = I3IPC_CON(user_data);
  JsonObject *data = json_node_get_object(element_node);

  i3ipcCon *con = i3ipc_con_new(parent, data, parent->priv->conn);

  parent->priv->nodes = g_list_append(parent->priv->nodes, con);
}

static void i3ipc_con_initialize_floating_nodes(JsonArray *array, guint index_, JsonNode *element_node, gpointer user_data) {
  i3ipcCon *parent = I3IPC_CON(user_data);
  JsonObject *data = json_node_get_object(element_node);

  i3ipcCon *con = i3ipc_con_new(parent, data, parent->priv->conn);

  parent->priv->floating_nodes = g_list_append(parent->priv->floating_nodes, con);
}

i3ipcCon *i3ipc_con_new(i3ipcCon *parent, JsonObject *data, i3ipcConnection *conn) {
  i3ipcCon *con;
  con = g_object_new(I3IPC_TYPE_CON, NULL);

  g_object_ref(conn);
  con->priv->conn = conn;

  if (!json_object_get_null_member(data, "percent"))
    con->priv->percent = json_object_get_double_member(data, "percent");

  if (!json_object_get_null_member(data, "window"))
    con->priv->window = json_object_get_int_member(data, "window");

  if (json_object_has_member(data, "window_properties")) {
    JsonObject *window_properties = json_object_get_object_member(data, "window_properties");

    if (json_object_has_member(window_properties, "class"))
      con->priv->window_class = g_strdup(json_object_get_string_member(window_properties, "class"));
  }

  con->priv->name = g_strdup(json_object_get_string_member(data, "name"));
  con->priv->focused = json_object_get_boolean_member(data, "focused");
  con->priv->urgent = json_object_get_boolean_member(data, "urgent");
  con->priv->layout = g_strdup(json_object_get_string_member(data, "layout"));
  con->priv->orientation = g_strdup(json_object_get_string_member(data, "orientation"));
  con->priv->current_border_width = json_object_get_int_member(data, "current_border_width");
  con->priv->border = g_strdup(json_object_get_string_member(data, "border"));
  con->priv->id = json_object_get_int_member(data, "id");
  con->priv->type = g_strdup(json_object_get_string_member(data, "type"));

  if (parent) {
    g_object_ref(parent);
    con->priv->parent = parent;
  }

  JsonObject *rect_data = json_object_get_object_member(data, "rect");

  con->priv->rect->x = json_object_get_int_member(rect_data, "x");
  con->priv->rect->y = json_object_get_int_member(rect_data, "y");
  con->priv->rect->width = json_object_get_int_member(rect_data, "width");
  con->priv->rect->height = json_object_get_int_member(rect_data, "height");

  JsonArray *nodes_array = json_object_get_array_member(data, "nodes");
  json_array_foreach_element(nodes_array, i3ipc_con_initialize_nodes, con);

  JsonArray *floating_nodes_array = json_object_get_array_member(data, "floating_nodes");
  json_array_foreach_element(floating_nodes_array, i3ipc_con_initialize_floating_nodes, con);

  return con;
}

/**
 * i3ipc_con_get_nodes:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer none) (element-type i3ipcCon): A list of child nodes.
 */
const GList *i3ipc_con_get_nodes(i3ipcCon *self) {
  return self->priv->nodes;
}

/**
 * i3ipc_con_get_floating_nodes:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer none) (element-type i3ipcCon): A list of child floating nodes.
 */
const GList *i3ipc_con_get_floating_nodes(i3ipcCon *self) {
  return self->priv->floating_nodes;
}

/**
 * i3ipc_con_root:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer none): The root node of the tree.
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

static void i3ipc_con_collect_descendents_func(gpointer data, gpointer user_data) {
  i3ipcCon *con = I3IPC_CON(data);
  GList *descendents = (GList *)user_data;

  descendents = g_list_append(descendents, con);

  if (descendents != NULL) {
    g_list_foreach(con->priv->nodes, i3ipc_con_collect_descendents_func, descendents);
    g_list_foreach(con->priv->floating_nodes, i3ipc_con_collect_descendents_func, descendents);
  }
}

/**
 * i3ipc_con_descendents:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer container) (element-type i3ipcCon): a list of descendent nodes
 */
GList *i3ipc_con_descendents(i3ipcCon *self) {
  GList *retval;

  if (!self->priv->nodes)
    return NULL;

  /* the list has to have a first element for some reason. */
  retval = g_list_alloc();

  g_list_foreach(self->priv->nodes, i3ipc_con_collect_descendents_func, retval);
  g_list_foreach(self->priv->floating_nodes, i3ipc_con_collect_descendents_func, retval);

  /* XXX: I hope this doesn't leak */
  retval = g_list_remove_link(retval, g_list_first(retval));

  return retval;
}

/**
 * i3ipc_con_leaves:
 * @self: an #i3ipcCon
 *
 * Finds the leaf descendent nodes of a given container excluding dock clients.
 *
 * Returns: (transfer container) (element-type i3ipcCon): a list of leaf descendent nodes
 */
GList *i3ipc_con_leaves(i3ipcCon *self) {
  GList *descendents;
  GList *retval = NULL;

  descendents = i3ipc_con_descendents(self);
  guint len = g_list_length(descendents);

  for (gint i = 0; i < len; i += 1) {
    i3ipcCon *con = I3IPC_CON(g_list_nth_data(descendents, i));

    if (g_list_length(con->priv->nodes) == 0
        && g_strcmp0(con->priv->type, "con") == 0
        && g_strcmp0(con->priv->parent->priv->type, "dockarea") != 0)
      retval = g_list_append(retval, con);
  }

  g_list_free(descendents);

  return retval;
}

/**
 * i3ipc_con_get_name:
 * @self: an #i3ipcCon
 *
 * Convenience function to get the commonly-needed "name" property of the Con.
 *
 * Returns: (transfer none): The "name" property of the Con
 */
const gchar *i3ipc_con_get_name(i3ipcCon *self) {
  return self->priv->name;
}

/**
 * i3ipc_con_command:
 * @self: an #i3ipcCon
 * @command: the command to execute on the con
 * @err (allow-none): the location of a GError or NULL
 *
 * Convenience function to execute a command in the context of the container
 * (it will be selected by criteria)
 *
 */
void i3ipc_con_command(i3ipcCon *self, const gchar* command, GError **err) {
  gchar *reply;
  gchar *context_command;
  GError *tmp_error = NULL;

  g_return_if_fail(err == NULL || *err == NULL);

  context_command = g_strdup_printf("[con_id=\"%d\"] %s", self->priv->id, command);

  reply = i3ipc_connection_message(self->priv->conn, I3IPC_MESSAGE_TYPE_COMMAND, context_command, &tmp_error);

  if (tmp_error != NULL)
    g_propagate_error(err, tmp_error);

  g_free(reply);
  g_free(context_command);
}

/**
 * i3ipc_con_command_children:
 * @self: an #i3ipcCon
 * @command: the command to execute on the con's nodes
 * @err: (allow-none): the location of a GError or NULL
 *
 * Convenience function to execute a command in the context of the container's
 * children (the immediate descendents will be selected by criteria)
 *
 */
void i3ipc_con_command_children(i3ipcCon *self, const gchar* command, GError **err) {
  guint len;
  gchar *reply;
  GString *payload;
  GError *tmp_error = NULL;

  len = g_list_length(self->priv->nodes);

  if (len == 0)
    return;

  payload = g_string_new("");

  for (gint i = 0; i < len; i += 1)
    g_string_append_printf(payload, "[con_id=\"%d\"] %s; ", I3IPC_CON(g_list_nth_data(self->priv->nodes, i))->priv->id, command);

  reply = i3ipc_connection_message(self->priv->conn, I3IPC_MESSAGE_TYPE_COMMAND, payload->str, &tmp_error);

  if (tmp_error != NULL)
    g_propagate_error(err, tmp_error);

  g_free(reply);
  g_string_free(payload, TRUE);
}

static void i3ipc_con_collect_workspaces_func(gpointer data, gpointer user_data) {
  i3ipcCon *con = I3IPC_CON(data);
  GList *workspaces = (GList *)user_data;

  if (g_strcmp0(con->priv->type, "workspace") == 0 && !g_str_has_prefix(con->priv->name, "__"))
    workspaces = g_list_append(workspaces, con);
  else if (workspaces != NULL)
    g_list_foreach(con->priv->nodes, i3ipc_con_collect_workspaces_func, workspaces);
}

/**
 * i3ipc_con_workspaces:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer container) (element-type i3ipcCon): a list of workspaces in the tree
 */
GList *i3ipc_con_workspaces(i3ipcCon *self) {
  GList *retval;
  i3ipcCon *root;

  root = i3ipc_con_root(self);

  /* this could happen for incomplete trees */
  if (!root->priv->nodes)
    return NULL;

  /* the list has to have a first element for some reason. */
  retval = g_list_alloc();

  g_list_foreach(root->priv->nodes, i3ipc_con_collect_workspaces_func, retval);

  /* XXX: I hope this doesn't leak */
  retval = g_list_remove_link(retval, g_list_first(retval));

  return retval;
}

static gint i3ipc_con_focused_cmp_func(gconstpointer a, gconstpointer b) {
  const i3ipcCon *con = a;

  return con->priv->focused ? 0 : 1;
}

/**
 * i3ipc_con_find_focused:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer none): The focused Con, or NULL if not found in this Con.
 *
 */
i3ipcCon *i3ipc_con_find_focused(i3ipcCon *self) {
  GList *descendents;
  GList *cmp_result;
  i3ipcCon *retval = NULL;

  descendents = i3ipc_con_descendents(self);

  if (descendents == NULL)
    return NULL;

  cmp_result = g_list_find_custom(descendents, NULL, i3ipc_con_focused_cmp_func);

  if (cmp_result != NULL)
    retval = I3IPC_CON(cmp_result->data);

  g_list_free(descendents);

  return retval;
}

/**
 * i3ipc_con_find_by_id:
 * @self: an #i3ipcCon
 * @con_id: the id of the con to find
 *
 * Returns: (transfer none): The con with the given con_id among this con's descendents
 */
i3ipcCon *i3ipc_con_find_by_id(i3ipcCon *self, const gint con_id) {
  GList *descendents;
  i3ipcCon *retval = NULL;

  descendents = i3ipc_con_descendents(self);
  guint len = g_list_length(descendents);

  for (gint i = 0; i < len; i += 1) {
    i3ipcCon *con = I3IPC_CON(g_list_nth_data(descendents, i));

    if (con->priv->id == con_id) {
      retval = con;
      break;
    }
  }

  g_list_free(descendents);

  return retval;
}

/**
 * i3ipc_con_find_by_window:
 * @self: an #i3ipcCon
 * @window_id: the window id of the con to find
 *
 * Returns: (transfer none): The con with the given window id among this con's descendents
 */
i3ipcCon *i3ipc_con_find_by_window(i3ipcCon *self, const gint window_id) {
  GList *descendents;
  i3ipcCon *retval = NULL;

  descendents = i3ipc_con_descendents(self);
  guint len = g_list_length(descendents);

  for (gint i = 0; i < len; i += 1) {
    i3ipcCon *con = I3IPC_CON(g_list_nth_data(descendents, i));

    if (con->priv->window == window_id) {
      retval = con;
      break;
    }
  }

  g_list_free(descendents);

  return retval;
}

/**
 * i3ipc_con_find_named:
 * @self: an #i3ipcCon
 * @pattern: a perl-compatible regular expression pattern
 * @err: (allow-none): return location for a GError or NULL
 *
 * Returns: (transfer container) (element-type i3ipcCon): A list of descendent Cons which have a name that matches the pattern
 */
GList *i3ipc_con_find_named(i3ipcCon *self, const gchar *pattern, GError **err) {
  GList *descendents;
  GRegex *regex;
  GList *retval = NULL;
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  regex = g_regex_new(pattern, 0, 0, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  descendents = i3ipc_con_descendents(self);
  guint len = g_list_length(descendents);

  for (gint i = 0; i < len; i += 1) {
    i3ipcCon *con = I3IPC_CON(g_list_nth_data(descendents, i));

    if (g_regex_match(regex, con->priv->name, 0, NULL))
      retval = g_list_append(retval, con);
  }

  g_list_free(descendents);
  g_regex_unref(regex);

  return retval;
}

/**
 * i3ipc_con_find_classed:
 * @self: an #i3ipcCon
 * @pattern: a perl-compatible regular expression pattern
 * @err: (allow-none): return location for a GError or NULL
 *
 * Returns: (transfer container) (element-type i3ipcCon): A list of descendent Cons which have a WM_CLASS class property that matches the pattern
 */
GList *i3ipc_con_find_classed(i3ipcCon *self, const gchar *pattern, GError **err) {
  GList *descendents;
  GRegex *regex;
  GList *retval = NULL;
  GError *tmp_error = NULL;

  g_return_val_if_fail(err == NULL || *err == NULL, NULL);

  regex = g_regex_new(pattern, 0, 0, &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(err, tmp_error);
    return NULL;
  }

  descendents = i3ipc_con_descendents(self);
  guint len = g_list_length(descendents);

  for (gint i = 0; i < len; i += 1) {
    i3ipcCon *con = I3IPC_CON(g_list_nth_data(descendents, i));

    if (con->priv->window_class && g_regex_match(regex, con->priv->window_class, 0, NULL))
      retval = g_list_append(retval, con);
  }

  g_list_free(descendents);
  g_regex_unref(regex);

  return retval;
}

/**
 * i3ipc_con_workspace:
 * @self: an #i3ipcCon
 *
 * Returns: (transfer none): The closest workspace con
 */
i3ipcCon *i3ipc_con_workspace(i3ipcCon *self) {
  i3ipcCon *retval = self->priv->parent;

  while (retval != NULL) {
    if (g_strcmp0(retval->priv->type, "workspace") == 0)
      break;

    retval = retval->priv->parent;
  }

  return retval;
}
