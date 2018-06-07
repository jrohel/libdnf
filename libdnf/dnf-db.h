#include <glib/garray.h>
/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __DNF_DB_H
#define __DNF_DB_H


#include "dnf-context.h"
#include "dnf-types.h"
#include "hy-package.h"

#ifdef __cplusplus
extern "C" {
#endif
void dnf_db_ensure_origin_pkg(DnfDb * db, DnfPackage * pkg);
void dnf_db_ensure_origin_pkglist(DnfDb * db, GPtrArray * pkglist);
#ifdef __cplusplus
}
#endif

#endif /* __DNF_DB_H */
