/* gtd-window.c
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

#include "gtd-application.h"
#include "gtd-task-list-view.h"
#include "gtd-manager.h"
#include "gtd-notification.h"
#include "gtd-notification-widget.h"
#include "gtd-storage-dialog.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-task-list-item.h"
#include "gtd-window.h"

#include <glib/gi18n.h>

typedef struct
{
  GtkButton                     *back_button;
  GtkColorButton                *color_button;
  GtkHeaderBar                  *headerbar;
  GtkFlowBox                    *lists_flowbox;
  GtkStack                      *main_stack;
  GtkWidget                     *new_list_popover;
  GtdNotificationWidget         *notification_widget;
  GtdTaskListView               *scheduled_list_view;
  GtkSearchBar                  *search_bar;
  GtkToggleButton               *search_button;
  GtkSearchEntry                *search_entry;
  GtkStack                      *stack;
  GtkStackSwitcher              *stack_switcher;
  GtdStorageDialog              *storage_dialog;
  GtdTaskListView               *today_list_view;
  GtdTaskListView               *list_view;

  /* mode */
  GtdWindowMode                  mode;

  /* loading notification */
  GtdNotification               *loading_notification;

  GtdManager                    *manager;
} GtdWindowPrivate;

struct _GtdWindow
{
  GtkApplicationWindow  application;

  /*< private >*/
  GtdWindowPrivate     *priv;
};

static void          gtd_window__change_storage_action           (GSimpleAction         *simple,
                                                                  GVariant              *parameter,
                                                                  gpointer               user_data);

G_DEFINE_TYPE_WITH_PRIVATE (GtdWindow, gtd_window, GTK_TYPE_APPLICATION_WINDOW)

static const GActionEntry gtd_window_entries[] = {
  { "change-storage", gtd_window__change_storage_action }
};

enum {
  PROP_0,
  PROP_MANAGER,
  LAST_PROP
};

static void
gtd_window__stack_visible_child_cb (GtdWindow *window)
{
  GtdWindowPrivate *priv;

  priv = window->priv;

  gtk_widget_set_visible (
          GTK_WIDGET (priv->search_button),
          g_strcmp0 (gtk_stack_get_visible_child_name (priv->main_stack), "overview") == 0 &&
          g_strcmp0 (gtk_stack_get_visible_child_name (priv->stack), "lists") == 0);

}

static void
gtd_window_update_list_counters (GtdTaskList *list,
                                 GtdTask     *task,
                                 GtdWindow   *window)
{
  GtdWindowPrivate *priv;
  GtkWidget *container_child;
  gboolean is_today;
  GList *tasks;
  GList *l;
  gchar *new_title;
  gint counter;

  g_return_if_fail (GTD_IS_WINDOW (window));
  g_return_if_fail (GTD_IS_TASK_LIST (list));

  priv = window->priv;

  /* Count the number of incomplete tasks */
  counter = 0;
  tasks = gtd_task_list_get_tasks (list);

  for (l = tasks; l != NULL; l = l->next)
    {
      if (!gtd_task_get_complete (l->data))
        counter++;
    }

  /* Update the list title */
  is_today = list == gtd_manager_get_today_list (priv->manager);

  container_child = is_today ? GTK_WIDGET (priv->today_list_view) : GTK_WIDGET (priv->scheduled_list_view);

  if (counter == 0)
    {
      new_title = g_strdup_printf ("%s", is_today ? _("Today") : _("Scheduled"));
    }
  else
    {
      new_title = g_strdup_printf ("%s (%d)",
                                   is_today ? _("Today") : _("Scheduled"),
                                   counter);
    }

  gtk_container_child_set (GTK_CONTAINER (priv->stack),
                           container_child,
                           "title", new_title,
                           NULL);

  g_list_free (tasks);
  g_free (new_title);
}


static void
gtd_window__change_storage_action (GSimpleAction *simple,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
  GtdWindowPrivate *priv;

  g_return_if_fail (GTD_IS_WINDOW (user_data));

  priv = GTD_WINDOW (user_data)->priv;

  gtk_dialog_run (GTK_DIALOG (priv->storage_dialog));
}

static void
gtd_window__list_color_set (GtkColorChooser *button,
                            gpointer         user_data)
{
  GtdWindowPrivate *priv = GTD_WINDOW (user_data)->priv;
  GtdTaskList *list;
  GdkRGBA new_color;

  g_return_if_fail (GTD_IS_WINDOW (user_data));
  g_return_if_fail (gtd_task_list_view_get_task_list (priv->list_view));

  list = gtd_task_list_view_get_task_list (priv->list_view);

  g_debug ("%s: %s: %s",
           G_STRFUNC,
           _("Setting new color for task list"),
           gtd_task_list_get_name (list));

  gtk_color_chooser_get_rgba (button, &new_color);
  gtd_task_list_set_color (list, &new_color);

  gtd_manager_save_task_list (priv->manager, list);
}

static gboolean
gtd_window__on_key_press_event (GtkWidget    *widget,
                                GdkEvent     *event,
                                GtkSearchBar *bar)
{
  GtdWindowPrivate *priv;

  g_return_val_if_fail (GTD_IS_WINDOW (widget), TRUE);

  priv = GTD_WINDOW (widget)->priv;

  if (g_strcmp0 (gtk_stack_get_visible_child_name (GTK_STACK (priv->main_stack)), "overview") == 0 &&
      g_strcmp0 (gtk_stack_get_visible_child_name (GTK_STACK (priv->stack)), "lists") == 0)
    {
      return gtk_search_bar_handle_event (bar, event);
    }

  return FALSE;
}

static gint
gtd_window__flowbox_sort_func (GtdTaskListItem *a,
                               GtdTaskListItem *b,
                               gpointer         user_data)
{
  GtdTaskList *l1;
  GtdTaskList *l2;
  gint retval = 0;

  l1 = gtd_task_list_item_get_list (a);
  l2 = gtd_task_list_item_get_list (b);

  retval = g_strcmp0 (gtd_task_list_get_origin (l1), gtd_task_list_get_origin (l2));

  if (retval != 0)
    return retval;

  return g_strcmp0 (gtd_task_list_get_name (l1), gtd_task_list_get_name (l2));
}

static gboolean
gtd_window__flowbox_filter_func (GtdTaskListItem *item,
                                 GtdWindow       *window)
{
  GtdWindowPrivate *priv;
  GtdTaskList *list;
  gboolean return_value;
  gchar *search_folded;
  gchar *list_name_folded;
  gchar *haystack;

  g_return_val_if_fail (GTD_IS_WINDOW (window), FALSE);

  priv = window->priv;
  list = gtd_task_list_item_get_list (item);
  haystack = NULL;
  search_folded = g_utf8_casefold (gtk_entry_get_text (GTK_ENTRY (priv->search_entry)), -1);
  list_name_folded = g_utf8_casefold (gtd_task_list_get_name (list), -1);

  if (!search_folded || search_folded[0] == '\0')
    {
      return_value = TRUE;
      goto out;
    }

  haystack = g_strstr_len (list_name_folded,
                           -1,
                           search_folded);

  return_value = (haystack != NULL);

out:
  g_free (search_folded);
  g_free (list_name_folded);

  return return_value;
}

static void
gtd_window__manager_ready_changed (GObject    *source,
                                   GParamSpec *spec,
                                   gpointer    user_data)
{
  GtdWindowPrivate *priv = GTD_WINDOW (user_data)->priv;

  g_return_if_fail (GTD_IS_WINDOW (user_data));

  if (gtd_object_get_ready (GTD_OBJECT (source)))
    {
      gtd_notification_widget_cancel (priv->notification_widget, priv->loading_notification);
    }
  else
    {
      gtd_notification_widget_notify (priv->notification_widget, priv->loading_notification);
    }
}

static void
gtd_window__back_button_clicked (GtkButton *button,
                                 gpointer   user_data)
{
  GtdWindowPrivate *priv = GTD_WINDOW (user_data)->priv;

  g_return_if_fail (GTD_IS_WINDOW (user_data));

  gtk_stack_set_visible_child_name (priv->main_stack, "overview");
  gtk_header_bar_set_custom_title (priv->headerbar, GTK_WIDGET (priv->stack_switcher));
  gtk_widget_hide (GTK_WIDGET (priv->back_button));
  gtk_widget_hide (GTK_WIDGET (priv->color_button));
}

static void
gtd_window__list_selected (GtkFlowBox      *flowbox,
                           GtdTaskListItem *item,
                           gpointer         user_data)
{
  GtdWindowPrivate *priv = GTD_WINDOW (user_data)->priv;
  GtdTaskList *list;
  GdkRGBA *list_color;

  g_return_if_fail (GTD_IS_WINDOW (user_data));
  g_return_if_fail (GTD_IS_TASK_LIST_ITEM (item));

  list = gtd_task_list_item_get_list (item);
  list_color = gtd_task_list_get_color (list);

  g_signal_handlers_block_by_func (priv->color_button,
                                   gtd_window__list_color_set,
                                   user_data);

  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->color_button), list_color);

  gtk_stack_set_visible_child_name (priv->main_stack, "tasks");
  gtk_header_bar_set_title (priv->headerbar, gtd_task_list_get_name (list));
  gtk_header_bar_set_subtitle (priv->headerbar, gtd_task_list_get_origin (list));
  gtk_header_bar_set_custom_title (priv->headerbar, NULL);
  gtk_search_bar_set_search_mode (priv->search_bar, FALSE);
  gtd_task_list_view_set_task_list (priv->list_view, list);
  gtd_task_list_view_set_show_completed (priv->list_view, FALSE);
  gtk_widget_show (GTK_WIDGET (priv->back_button));
  gtk_widget_show (GTK_WIDGET (priv->color_button));

  g_signal_handlers_unblock_by_func (priv->color_button,
                                     gtd_window__list_color_set,
                                     user_data);

  gdk_rgba_free (list_color);
}

static void
gtd_window__list_added (GtdManager  *manager,
                        GtdTaskList *list,
                        gpointer     user_data)
{
  GtdWindowPrivate *priv = GTD_WINDOW (user_data)->priv;
  GtkWidget *item;

  item = gtd_task_list_item_new (list);
  gtk_widget_show (item);

  gtk_flow_box_insert (priv->lists_flowbox,
                       item,
                       -1);
}

static void
gtd_window_constructed (GObject *object)
{
  GtdWindowPrivate *priv = GTD_WINDOW (object)->priv;

  G_OBJECT_CLASS (gtd_window_parent_class)->constructed (object);

  gtk_flow_box_set_sort_func (priv->lists_flowbox,
                              (GtkFlowBoxSortFunc) gtd_window__flowbox_sort_func,
                              NULL,
                              NULL);

  gtk_flow_box_set_filter_func (priv->lists_flowbox,
                                (GtkFlowBoxFilterFunc) gtd_window__flowbox_filter_func,
                                object,
                                NULL);

  g_object_bind_property (object,
                          "manager",
                          priv->new_list_popover,
                          "manager",
                          G_BINDING_DEFAULT);
  g_object_bind_property (object,
                          "manager",
                          priv->storage_dialog,
                          "manager",
                          G_BINDING_DEFAULT);
}

GtkWidget*
gtd_window_new (GtdApplication *application)
{
  return g_object_new (GTD_TYPE_WINDOW,
                       "application", application,
                       "manager",     gtd_application_get_manager (application),
                       NULL);
}

static void
gtd_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (gtd_window_parent_class)->finalize (object);
}

static void
gtd_window_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  GtdWindow *self = GTD_WINDOW (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      g_value_set_object (value, self->priv->manager);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_window_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  GtdWindow *self = GTD_WINDOW (object);
  GList *lists;
  GList *l;

  switch (prop_id)
    {
    case PROP_MANAGER:
      self->priv->manager = g_value_get_object (value);

      gtd_task_list_view_set_manager (self->priv->list_view, self->priv->manager);
      gtd_task_list_view_set_manager (self->priv->today_list_view, self->priv->manager);
      gtd_task_list_view_set_manager (self->priv->scheduled_list_view, self->priv->manager);

      g_signal_connect (self->priv->manager,
                        "notify::ready",
                        G_CALLBACK (gtd_window__manager_ready_changed),
                        self);
      g_signal_connect (self->priv->manager,
                        "list-added",
                        G_CALLBACK (gtd_window__list_added),
                        self);

      /* Add already loaded lists */
      lists = gtd_manager_get_task_lists (self->priv->manager);

      for (l = lists; l != NULL; l = l->next)
        {
          gtd_window__list_added (self->priv->manager,
                                  l->data,
                                  object);
        }

      g_list_free (lists);

      /* Setup 'Today' and 'Scheduled' lists */
      gtd_task_list_view_set_task_list (self->priv->today_list_view, gtd_manager_get_today_list (self->priv->manager));
      gtd_task_list_view_set_task_list (self->priv->scheduled_list_view, gtd_manager_get_scheduled_list (self->priv->manager));

      g_signal_connect (gtd_manager_get_today_list (self->priv->manager),
                        "task-added",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);
      g_signal_connect (gtd_manager_get_today_list (self->priv->manager),
                        "task-updated",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);
      g_signal_connect (gtd_manager_get_today_list (self->priv->manager),
                        "task-removed",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);
      g_signal_connect (gtd_manager_get_scheduled_list (self->priv->manager),
                        "task-added",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);
      g_signal_connect (gtd_manager_get_scheduled_list (self->priv->manager),
                        "task-updated",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);
      g_signal_connect (gtd_manager_get_scheduled_list (self->priv->manager),
                        "task-removed",
                        G_CALLBACK (gtd_window_update_list_counters),
                        self);

      g_object_notify (object, "manager");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_window_class_init (GtdWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_window_finalize;
  object_class->constructed = gtd_window_constructed;
  object_class->get_property = gtd_window_get_property;
  object_class->set_property = gtd_window_set_property;

  /**
   * GtdWindow::manager:
   *
   * A weak reference to the application's #GtdManager instance.
   */
  g_object_class_install_property (
        object_class,
        PROP_MANAGER,
        g_param_spec_object ("manager",
                             _("Manager of this window's application"),
                             _("The manager of the window's application"),
                             GTD_TYPE_MANAGER,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/window.ui");

  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, back_button);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, color_button);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, headerbar);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, lists_flowbox);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, list_view);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, main_stack);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, new_list_popover);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, notification_widget);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, scheduled_list_view);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, stack);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, search_bar);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, search_button);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, stack_switcher);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, storage_dialog);
  gtk_widget_class_bind_template_child_private (widget_class, GtdWindow, today_list_view);

  gtk_widget_class_bind_template_callback (widget_class, gtd_window__back_button_clicked);
  gtk_widget_class_bind_template_callback (widget_class, gtd_window__list_color_set);
  gtk_widget_class_bind_template_callback (widget_class, gtd_window__list_selected);
  gtk_widget_class_bind_template_callback (widget_class, gtd_window__on_key_press_event);
  gtk_widget_class_bind_template_callback (widget_class, gtd_window__stack_visible_child_cb);
}

static void
gtd_window_init (GtdWindow *self)
{
  self->priv = gtd_window_get_instance_private (self);

  self->priv->loading_notification = gtd_notification_new (_("Loading your task lists…"), 0);
  gtd_object_set_ready (GTD_OBJECT (self->priv->loading_notification), FALSE);

  /* add actions */
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   gtd_window_entries,
                                   G_N_ELEMENTS (gtd_window_entries),
                                   self);

  gtk_widget_init_template (GTK_WIDGET (self));
}

/**
 * gtd_window_get_manager:
 * @window: a #GtdWindow
 *
 * Retrieves a weak reference for the @window's #GtdManager.
 *
 * Returns: (transfer none): the #GtdManager of @window.
 */
GtdManager*
gtd_window_get_manager (GtdWindow *window)
{
  g_return_val_if_fail (GTD_IS_WINDOW (window), NULL);

  return window->priv->manager;
}

/**
 * gtd_window_notify:
 * @window: a #GtdWindow
 * @notification: a #GtdNotification
 *
 * Shows a notification on the top of the main window.
 *
 * Returns:
 */
void
gtd_window_notify (GtdWindow       *window,
                   GtdNotification *notification)
{
  GtdWindowPrivate *priv;

  g_return_if_fail (GTD_IS_WINDOW (window));

  priv = window->priv;

  gtd_notification_widget_notify (priv->notification_widget, notification);
}

/**
 * gtd_window_cancel_notification:
 * @window: a #GtdManager
 * @notification: a #GtdNotification
 *
 * Cancels @notification.
 *
 * Returns:
 */
void
gtd_window_cancel_notification (GtdWindow       *window,
                                GtdNotification *notification)
{
  GtdWindowPrivate *priv;

  g_return_if_fail (GTD_IS_WINDOW (window));

  priv = window->priv;

  gtd_notification_widget_cancel (priv->notification_widget, notification);
}
