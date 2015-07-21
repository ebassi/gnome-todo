/* gtd-arrow-frame.c
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

#include "gtd-arrow-frame.h"
#include "gtd-task-row.h"

typedef struct
{
  GtdTaskRow          *row;

  GtkGesture          *pan_gesture;

  GdkWindow           *handle_window;
  gdouble              offset;
  gint                 moving : 1;
} GtdArrowFramePrivate;

struct _GtdArrowFrame
{
  GtkFrame              parent;

  /*<private>*/
  GtdArrowFramePrivate *priv;
};

#define ARROW_HEIGHT 28
#define ARROW_WIDTH  8
#define HANDLE_GAP   (ARROW_WIDTH + 5)

G_DEFINE_TYPE_WITH_PRIVATE (GtdArrowFrame, gtd_arrow_frame, GTK_TYPE_FRAME)

enum {
  PROP_0,
  PROP_TASK_ROW,
  LAST_PROP
};

GtkWidget*
gtd_arrow_frame_new (void)
{
  return g_object_new (GTD_TYPE_ARROW_FRAME, NULL);
}

static void
on_drag_begin_cb (GtdArrowFrame *frame,
                  gdouble        x,
                  gdouble        y,
                  GtkGesturePan *gesture)
{
  GtdArrowFramePrivate *priv = frame->priv;
  GdkEventSequence *sequence;
  const GdkEvent *event;

  sequence = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  event = gtk_gesture_get_last_event (GTK_GESTURE (gesture), sequence);

  priv->moving = FALSE;

  if (event->any.window == priv->handle_window)
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
  else
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
}

static void
on_pan_cb (GtkWidget       *widget,
           GtkPanDirection  direction,
           gdouble          offset,
           GtkGesturePan   *pan)
{
  GtdArrowFramePrivate *priv;
  GtkTextDirection dir;
  gdouble offset_x;

  priv = GTD_ARROW_FRAME (widget)->priv;
  dir = gtk_widget_get_direction (widget);

  priv->moving = TRUE;

  gtk_gesture_drag_get_offset (GTK_GESTURE_DRAG (pan),
                               &offset_x,
                               NULL);

  if (dir == GTK_TEXT_DIR_RTL)
    priv->offset = MAX (0, offset_x);
  else
    priv->offset = MAX (0, priv->offset + -1 * offset_x);


  gtk_widget_queue_resize (widget);
}

static void
on_drag_end_cb (GtdArrowFrame *frame,
                gdouble        x,
                gdouble        y,
                GtkGesturePan *pan)
{
  GtdArrowFramePrivate *priv = frame->priv;

  if (!priv->moving)
    gtk_gesture_set_state (GTK_GESTURE (pan), GTK_EVENT_SEQUENCE_DENIED);
}

static void
gtd_arrow_frame_finalize (GObject *object)
{
  GtdArrowFrame *self = (GtdArrowFrame *)object;
  GtdArrowFramePrivate *priv = gtd_arrow_frame_get_instance_private (self);

  g_clear_object (&priv->pan_gesture);

  G_OBJECT_CLASS (gtd_arrow_frame_parent_class)->finalize (object);
}

static void
gtd_arrow_frame_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtdArrowFrame *self = GTD_ARROW_FRAME (object);

  switch (prop_id)
    {
    case PROP_TASK_ROW:
      g_value_set_object (value, self->priv->row);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_arrow_frame_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtdArrowFrame *self = GTD_ARROW_FRAME (object);

  switch (prop_id)
    {
    case PROP_TASK_ROW:
      gtd_arrow_frame_set_row (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_arrow_frame__row_destroyed (GtdArrowFrame *frame)
{
  gtd_arrow_frame_set_row (frame, NULL);
}

static gint
gtd_arrow_frame__get_row_y (GtdArrowFrame *frame)
{
  GtkWidget *widget;
  GtkWidget *row;
  gint row_height;
  gint dest_y;

  widget = GTK_WIDGET (frame);

  if (!frame->priv->row)
    return gtk_widget_get_allocated_height (widget) / 2;

  row = GTK_WIDGET (frame->priv->row);
  row_height = gtk_widget_get_allocated_height (row);

  gtk_widget_translate_coordinates (row,
                                    widget,
                                    0,
                                    row_height / 2,
                                    NULL,
                                    &dest_y);

  return CLAMP (dest_y,
                0,
                gtk_widget_get_allocated_height (widget));
}

static void
gtd_arrow_frame__draw_arrow (GtdArrowFrame    *frame,
                             cairo_t          *cr,
                             GtkTextDirection  dir)
{
  GtkWidget *widget = GTK_WIDGET (frame);
  GtkStyleContext *context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder border;
  GdkRGBA border_color;
  gint border_width;
  gint start_x;
  gint start_y;
  gint tip_x;
  gint tip_y;
  gint end_x;
  gint end_y;

  context = gtk_widget_get_style_context (widget);
  state = gtk_widget_get_state_flags (widget);

  gtk_style_context_get_border (context,
                                state,
                                &border);

  gtk_style_context_get (context,
                         state,
                         GTK_STYLE_PROPERTY_BORDER_COLOR, &border_color,
                         NULL);

  /* widget size */
  gtk_widget_get_allocation (widget, &alloc);

  if (dir == GTK_TEXT_DIR_LTR)
    {
      start_x = ARROW_WIDTH + border.left;
      tip_x = 0;
      end_x = ARROW_WIDTH + border.left;
      border_width = border.left;
    }
  else
    {
      start_x = alloc.width - ARROW_WIDTH - border.right;
      tip_x = alloc.width;
      end_x = alloc.width - ARROW_WIDTH - border.right;
      border_width = border.right;
    }

  tip_y = gtd_arrow_frame__get_row_y (frame);
  start_y = tip_y - (ARROW_HEIGHT / 2);
  end_y = tip_y + (ARROW_HEIGHT / 2);

  /* draw arrow */
  cairo_save (cr);

  cairo_set_line_width (cr, 1.0);
  cairo_move_to (cr, start_x, start_y);
  cairo_line_to (cr, tip_x,   tip_y);
  cairo_line_to (cr, end_x,   end_y);

  /*
   * Don't allow that gtk_render_background renders
   * anything out of (tip_x, start_y) (end_x, end_y).
   */
  cairo_clip (cr);

  /* render the arrow background */
  gtk_render_background (context,
                         cr,
                         0,
                         0,
                         alloc.width,
                         alloc.height);

  /* draw the border */
  if (border_width > 0)
    {
G_GNUC_BEGIN_IGNORE_DEPRECATIONS

      gtk_style_context_get_border_color (context,
                                          state,
                                          &border_color);

G_GNUC_END_IGNORE_DEPRECATIONS

      gdk_cairo_set_source_rgba (cr, &border_color);

      cairo_set_line_width (cr, 1);
      cairo_move_to (cr, start_x, start_y);
      cairo_line_to (cr, tip_x,   tip_y);
      cairo_line_to (cr, end_x,   end_y);

      cairo_set_line_width (cr, border_width + 1);
      cairo_stroke (cr);
    }

  cairo_restore (cr);
}

static GtkGesture*
gtd_arrow_frame__create_pan_gesture (GtdArrowFrame  *frame)
{
  GtkGesture *gesture;

  gesture = gtk_gesture_pan_new (GTK_WIDGET (frame), GTK_ORIENTATION_HORIZONTAL);

  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (gesture), FALSE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture), GTK_PHASE_CAPTURE);

  g_signal_connect_swapped (gesture,
                            "drag-begin",
                            G_CALLBACK (on_drag_begin_cb),
                            frame);
  g_signal_connect_swapped (gesture,
                            "pan",
                            G_CALLBACK (on_pan_cb),
                            frame);
  g_signal_connect_swapped (gesture,
                            "drag-end",
                            G_CALLBACK (on_drag_end_cb),
                            frame);

  return gesture;
}

static void
gtd_arrow_frame__draw_background (GtdArrowFrame    *frame,
                                  cairo_t          *cr,
                                  GtkTextDirection  dir)
{
  GtkWidget *widget = GTK_WIDGET (frame);
  GtkStyleContext *context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder margin;
  gint start_x;
  gint start_y;
  gint start_gap;
  gint end_x;
  gint end_y;
  gint end_gap;

  context = gtk_widget_get_style_context (widget);
  state = gtk_widget_get_state_flags (widget);

  /* widget size */
  gtk_widget_get_allocation (widget, &alloc);

  /* margin */
  gtk_style_context_get_margin (context,
                                state,
                                &margin);

  if (dir == GTK_TEXT_DIR_LTR)
    {
      start_x = ARROW_WIDTH + margin.left;
      end_x = alloc.width - margin.right;
    }
  else
    {
      start_x = margin.left;
      end_x = alloc.width - margin.right - ARROW_WIDTH;
    }


  start_y = margin.top;
  end_y = alloc.height - margin.bottom;

  start_gap = ((end_y - start_y - ARROW_HEIGHT) / 2);
  end_gap = ((end_y - start_y + ARROW_HEIGHT) / 2);

  gtk_render_background (context,
                         cr,
                         start_x,
                         start_y,
                         end_x,
                         end_y);

  gtk_render_frame_gap (context,
                        cr,
                        start_x,
                        start_y,
                        end_x,
                        end_y,
                        dir == GTK_TEXT_DIR_LTR ? GTK_POS_LEFT : GTK_POS_RIGHT,
                        start_gap,
                        end_gap);

}

static gboolean
gtd_arrow_frame_draw (GtkWidget *widget,
                      cairo_t   *cr)
{
  GtdArrowFrame *frame = GTD_ARROW_FRAME (widget);
  GtkTextDirection direction;
  GtkWidget *child;

  direction = gtk_widget_get_direction (widget);

  gtd_arrow_frame__draw_background (frame,
                                    cr,
                                    direction);

  gtd_arrow_frame__draw_arrow (frame,
                               cr,
                               direction);

  child = gtk_bin_get_child (GTK_BIN (widget));

  if (child)
    {
      gtk_container_propagate_draw (GTK_CONTAINER (widget),
                                    child,
                                    cr);
    }

  return TRUE;
}

static void
gtd_arrow_frame_compute_child_allocation (GtkFrame      *frame,
                                          GtkAllocation *allocation)
{
  GTK_FRAME_CLASS (gtd_arrow_frame_parent_class)->compute_child_allocation (frame, allocation);

  allocation->width -= ARROW_WIDTH;

  if (gtk_widget_get_direction (GTK_WIDGET (frame)) != GTK_TEXT_DIR_RTL)
    {
      allocation->x += ARROW_WIDTH;
    }
}

static void
gtd_arrow_frame_get_preferred_width (GtkWidget *widget,
                                     gint      *minimum_width,
                                     gint      *natural_width)
{
  GtdArrowFramePrivate *priv;

  priv = GTD_ARROW_FRAME (widget)->priv;

  GTK_WIDGET_CLASS (gtd_arrow_frame_parent_class)->get_preferred_width (widget,
                                                                        minimum_width,
                                                                        natural_width);

  *minimum_width += ARROW_WIDTH;
  *natural_width += ARROW_WIDTH;

  *natural_width = MAX (*minimum_width, *natural_width + priv->offset);
}

static void
gtd_arrow_frame_realize (GtkWidget *widget)
{
  GtdArrowFramePrivate *priv;
  GtkTextDirection dir;
  GtkAllocation allocation;
  GdkWindowAttr attributes = { 0 };
  GdkDisplay *display;
  GdkWindow *parent_window;
  gint attributes_mask;

  priv = GTD_ARROW_FRAME (widget)->priv;
  dir = gtk_widget_get_direction (widget);
  display = gtk_widget_get_display (widget);
  parent_window = gtk_widget_get_parent_window (widget);

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_set_window (widget, parent_window);
  g_object_ref (parent_window);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.x = dir == GTK_TEXT_DIR_LTR ? allocation.x : allocation.height - HANDLE_GAP;
  attributes.y = allocation.y;
  attributes.width = HANDLE_GAP;
  attributes.height = allocation.height;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.cursor = gdk_cursor_new_for_display (display, GDK_SB_H_DOUBLE_ARROW);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_POINTER_MOTION_MASK);

  attributes_mask = GDK_WA_CURSOR | GDK_WA_X | GDK_WA_Y;

  priv->handle_window = gdk_window_new (parent_window,
                                        &attributes,
                                        attributes_mask);

  gtk_widget_register_window (widget, priv->handle_window);

  g_clear_object (&attributes.cursor);
}

static void
gtd_arrow_frame_unrealize (GtkWidget *widget)
{
  GtdArrowFramePrivate *priv;

  priv = GTD_ARROW_FRAME (widget)->priv;

  if (priv->handle_window)
    {
      gdk_window_hide (priv->handle_window);
      gtk_widget_unregister_window (widget, priv->handle_window);
      g_clear_pointer (&priv->handle_window, gdk_window_destroy);
    }

  GTK_WIDGET_CLASS (gtd_arrow_frame_parent_class)->unrealize (widget);
}

static void
gtd_arrow_frame_map (GtkWidget *widget)
{
  GtdArrowFramePrivate *priv;

  priv = GTD_ARROW_FRAME (widget)->priv;

  if (priv->handle_window)
    gdk_window_show (priv->handle_window);

  GTK_WIDGET_CLASS (gtd_arrow_frame_parent_class)->map (widget);
}

static void
gtd_arrow_frame_unmap (GtkWidget *widget)
{
  GtdArrowFramePrivate *priv;

  priv = GTD_ARROW_FRAME (widget)->priv;

  if (priv->handle_window)
    gdk_window_hide (priv->handle_window);

  GTK_WIDGET_CLASS (gtd_arrow_frame_parent_class)->unmap (widget);
}

static void
gtd_arrow_frame_size_allocate (GtkWidget     *widget,
                               GtkAllocation *allocation)
{
  GtdArrowFramePrivate *priv;
  GtkTextDirection dir;

  priv = GTD_ARROW_FRAME (widget)->priv;
  dir = gtk_widget_get_direction (widget);

  GTK_WIDGET_CLASS (gtd_arrow_frame_parent_class)->size_allocate (widget, allocation);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize (priv->handle_window,
                              dir == GTK_TEXT_DIR_RTL ? allocation->width - HANDLE_GAP : allocation->x,
                              allocation->y,
                              HANDLE_GAP,
                              allocation->height);

      gdk_window_raise (priv->handle_window);
    }
}

static void
gtd_arrow_frame_class_init (GtdArrowFrameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkFrameClass *frame_class = GTK_FRAME_CLASS (klass);

  object_class->finalize = gtd_arrow_frame_finalize;
  object_class->get_property = gtd_arrow_frame_get_property;
  object_class->set_property = gtd_arrow_frame_set_property;

  widget_class->draw = gtd_arrow_frame_draw;
  widget_class->get_preferred_width = gtd_arrow_frame_get_preferred_width;
  widget_class->map = gtd_arrow_frame_map;
  widget_class->unmap = gtd_arrow_frame_unmap;
  widget_class->realize = gtd_arrow_frame_realize;
  widget_class->unrealize = gtd_arrow_frame_unrealize;
  widget_class->size_allocate = gtd_arrow_frame_size_allocate;

  frame_class->compute_child_allocation = gtd_arrow_frame_compute_child_allocation;
}

static void
gtd_arrow_frame_init (GtdArrowFrame *self)
{
  self->priv = gtd_arrow_frame_get_instance_private (self);

  self->priv->pan_gesture = gtd_arrow_frame__create_pan_gesture (self);
}

void
gtd_arrow_frame_set_row (GtdArrowFrame *frame,
                         GtdTaskRow    *row)
{
  g_return_if_fail (GTD_IS_ARROW_FRAME (frame));

  if (frame->priv->row)
    {
      g_signal_handlers_disconnect_by_func (frame->priv->row, gtd_arrow_frame__row_destroyed, frame);
    }

  frame->priv->row = row;

  if (row)
    {
      g_signal_connect_swapped (row,
                                "destroy",
                                G_CALLBACK (gtd_arrow_frame__row_destroyed),
                                frame);

      gtk_widget_queue_draw (GTK_WIDGET (frame));
    }
}
