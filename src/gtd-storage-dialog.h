/* gtd-storage-dialog.h
 *
 * Copyright (C) 2015 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTD_STORAGE_DIALOG_H
#define GTD_STORAGE_DIALOG_H

#include "gtd-types.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTD_TYPE_STORAGE_DIALOG (gtd_storage_dialog_get_type())

G_DECLARE_FINAL_TYPE (GtdStorageDialog, gtd_storage_dialog, GTD, STORAGE_DIALOG, GtkDialog)

GtkWidget*         gtd_storage_dialog_new                        (void);

GtdManager*        gtd_storage_dialog_get_manager                (GtdStorageDialog   *dialog);

void               gtd_storage_dialog_set_manager                (GtdStorageDialog   *dialog,
                                                                  GtdManager         *manager);

G_END_DECLS

#endif /* GTD_STORAGE_DIALOG_H */
