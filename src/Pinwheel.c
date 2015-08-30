#include "pebble.h"

Window *window;

static GPath *triangle_overlay_path = NULL;

static Layer *triangle_overlay_layer;

static Layer *s_hands_layer;

static GPath *minute_arrow_path, *hour_arrow_path;

static int ix;

static int ctr = 0;

static int processtriangle = 0;

static unsigned int angle = 15;

static const GPathInfo TRIANGLE_OVERLAY_POINTS = {
 3,
	(GPoint[]) {
		{-31, -110},
		{0, 0},
		{31, -110}
	}
};

static const GPathInfo MINUTE_HAND_POINTS = {
  3,
  (GPoint []) {
    { -8, 20 },
    { 8, 20 },
    { 0, -80 }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  3, (GPoint []){
    {-6, 20},
    {6, 20},
    {0, -60}
  }
};

static uint8_t Color_Array[12] = {  GColorRedARGB8        
                                   ,GColorGreenARGB8
                                   ,GColorChromeYellowARGB8
                                   ,GColorCyanARGB8
                                   ,GColorLimerickARGB8
                                   ,GColorBrilliantRoseARGB8
                                   ,GColorLavenderIndigoARGB8
                                   ,GColorJaegerGreenARGB8
                                   ,GColorDarkCandyAppleRedARGB8
                                   ,GColorBabyBlueEyesARGB8
                                   ,GColorYellowARGB8
                                   ,GColorPictonBlueARGB8     };

static void triangle_display_layer_update_callback(Layer *layer, GContext *ctx) {
     for(ix = 0; ix < 12; ix = ix + 1 ) {
           APP_LOG(APP_LOG_LEVEL_WARNING, "Triangle, ctr=%d", ctr);
	      gpath_rotate_to(triangle_overlay_path, (TRIG_MAX_ANGLE / 360) * angle);
	      graphics_context_set_fill_color(ctx, (GColor)Color_Array[ctr]);
	      gpath_draw_filled(ctx, triangle_overlay_path);
        angle = angle + 30;
        if (angle > 360) {
           angle = 15;
        }
        ctr++;
        if (ctr > 11) {
           ctr = 0;
        }
     }    
    processtriangle = 1;
}

static void hands_update_proc(Layer *layer, GContext *hands_ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
 
  graphics_context_set_fill_color(hands_ctx, GColorDukeBlue);

  gpath_rotate_to(minute_arrow_path, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(hands_ctx, minute_arrow_path);
  gpath_draw_outline(hands_ctx, minute_arrow_path);

  gpath_rotate_to(hour_arrow_path, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(hands_ctx, hour_arrow_path);
  gpath_draw_outline(hands_ctx, hour_arrow_path);

  // dot in the middle
  GRect hands_bounds = layer_get_bounds(s_hands_layer);

  graphics_context_set_fill_color(hands_ctx, GColorYellow);
  graphics_fill_circle(hands_ctx, GPoint(hands_bounds.size.w / 2, hands_bounds.size.h / 2), 5);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed) { 
  layer_mark_dirty(s_hands_layer);
  
  if(tick_time->tm_min % 5 == 0 || processtriangle == 0) {
     ctr=0;
     processtriangle = 1;
    
     switch (tick_time->tm_min) {
         case 0:
            ctr = 0;
            break;
         case 5:
            ctr = 2;
            break;
         case 10:
            ctr = 4;
            break;
         case 15:
            ctr = 6;
            break;
          case 20:
            ctr = 8;
            break;
          case 25:
            ctr = 10;
            break;
          case 30:
            ctr = 2;
            break;
          case 35:
            ctr = 4;
            break;
          case 40:
            ctr = 6;
            break;
          case 45:
            ctr = 8;
            break;
          case 50:
            ctr = 10;
            break;
          case 55:
            ctr = 2;
            break;
    }    
    layer_mark_dirty(triangle_overlay_layer);
  }
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();

  layer_destroy(triangle_overlay_layer);
  layer_destroy(s_hands_layer);
  
  window_destroy(window);
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  triangle_overlay_layer = layer_create(bounds);
	layer_set_update_proc(triangle_overlay_layer, triangle_display_layer_update_callback);
	layer_add_child(window_layer, triangle_overlay_layer);
	triangle_overlay_path = gpath_create(&TRIANGLE_OVERLAY_POINTS);
	gpath_move_to(triangle_overlay_path, grect_center_point(&bounds));
  
  // init hand paths
  minute_arrow_path = gpath_create(&MINUTE_HAND_POINTS);
  gpath_move_to(minute_arrow_path, grect_center_point(&bounds));
  
  hour_arrow_path = gpath_create(&HOUR_HAND_POINTS);
  gpath_move_to(hour_arrow_path, grect_center_point(&bounds));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

  s_hands_layer = layer_create(bounds);

  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

int main(void) {
   handle_init();

   app_event_loop();

   handle_deinit();
}
