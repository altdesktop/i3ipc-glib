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

#ifndef __I3IPC_CON_H__
#define __I3IPC_CON_H__

#include <glib-object.h>
#include <json-glib/json-glib.h>

/**
 * SECTION: i3ipc-con
 * @short_description: A representation of an i3 window container.
 *
 * #i3ipcCon is a class to model a node in the i3 window container tree.
 */

#define I3IPC_TYPE_CON             (i3ipc_con_get_type ())
#define I3IPC_CON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), I3IPC_TYPE_CON, i3ipcCon))
#define I3IPC_IS_CON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), I3IPC_TYPE_CON))
#define I3IPC_CON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), I3IPC_TYPE_CON, i3ipcConClass))
#define I3IPC_IS_CON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), I3IPC_TYPE_CON))
#define I3IPC_CON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), I3IPC_TYPE_CON, i3ipcConClass))

#define I3IPC_TYPE_RECT            (i3ipc_rect_get_type ())

typedef struct _i3ipcRect          i3ipcRect;

typedef struct _i3ipcCon           i3ipcCon;
typedef struct _i3ipcConClass      i3ipcConClass;
typedef struct _i3ipcConPrivate    i3ipcConPrivate;

/**
 * i3ipcRect:
 * @x:
 * @y:
 * @width:
 * @height:
 *
 * An #i3ipcRect descrites the extents of a container.
 */
struct _i3ipcRect
{
  gint x;
  gint y;
  gint width;
  gint height;
};

i3ipcRect *i3ipc_rect_copy(i3ipcRect *rect);
void i3ipc_rect_free(i3ipcRect *rect);
GType i3ipc_rect_get_type(void);

struct _i3ipcCon
{
  GObject parent_instance;

  /* instance members */
  i3ipcConPrivate *priv;
};

struct _i3ipcConClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by I3IPC_TYPE_CON */
GType i3ipc_con_get_type(void);

i3ipcCon *i3ipc_con_new(i3ipcCon *parent, JsonObject *data);

/* Method definitions */

GList *i3ipc_con_get_nodes(i3ipcCon *self);

i3ipcCon *i3ipc_con_root(i3ipcCon *self);

GList *i3ipc_con_descendents(i3ipcCon *self);

gchar *i3ipc_con_get_name(i3ipcCon *self);

#endif