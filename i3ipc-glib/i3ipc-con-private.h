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

#ifndef __I3IPC_CON_PRIVATE_H__
#define __I3IPC_CON_PRIVATE_H__

#include <json-glib/json-glib.h>
#include "i3ipc-con.h"
#include "i3ipc-connection.h"

i3ipcCon *i3ipc_con_new(i3ipcCon *parent, JsonObject *data, i3ipcConnection *conn);

#endif /* __I3IPC_CON_PRIVATE_H__ */
