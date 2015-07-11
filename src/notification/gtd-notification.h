/* gtd-notification.h
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

#ifndef GTD_NOTIFICATION_H
#define GTD_NOTIFICATION_H

#include "gtd-object.h"
#include "gtd-types.h"

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GTD_TYPE_NOTIFICATION (gtd_notification_get_type())

G_DECLARE_FINAL_TYPE (GtdNotification, gtd_notification, GTD, NOTIFICATION, GtdObject)

typedef void (*GtdNotificationActionFunc) (GtdNotification *notification,
                                           gpointer         user_data);


GtdNotification*     gtd_notification_new                        (const gchar        *text,
                                                                  gdouble             timeout);

void                 gtd_notification_execute_primary_action     (GtdNotification    *notification);

void                 gtd_notification_execute_secondary_action   (GtdNotification    *notification);

void                 gtd_notification_start                      (GtdNotification    *notification);

void                 gtd_notification_stop                       (GtdNotification    *notification);

void                 gtd_notification_set_primary_action         (GtdNotification    *notification,
                                                                  GtdNotificationActionFunc func,
                                                                  gpointer            user_data);

void                 gtd_notification_set_secondary_action       (GtdNotification    *notification,
                                                                  const gchar        *name,
                                                                  GtdNotificationActionFunc func,
                                                                  gpointer            user_data);

const gchar*         gtd_notification_get_text                   (GtdNotification    *notification);

void                 gtd_notification_set_text                   (GtdNotification    *notification,
                                                                  const gchar        *text);

gdouble              gtd_notification_get_timeout                (GtdNotification    *notification);

void                 gtd_notification_set_timeout                (GtdNotification    *notification,
                                                                  gdouble             timeout);

G_END_DECLS

#endif /* GTD_NOTIFICATION_H */
