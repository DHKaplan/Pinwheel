#include "pebble.h"

Window *window;

Layer         *LineLayer;

Layer         *RoundBatteryLayer;

static GPath *triangle_overlay_path = NULL;

static Layer *triangle_overlay_layer;

static Layer *s_hands_layer;

GFont        fontRobotoCondensed19;

static char day_text[] =  "  ";

static GPath *minute_arrow_path, *hour_arrow_path;

static int ix;

static int ctr = 0;

static int BTConnected = 1;

static int  batterychargepct;
static int  batterycharging = 0;

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
                                   ,GColorRajahARGB8     };

void handle_bluetooth(bool connected) {
    if (connected) {
         BTConnected = 1;     // Connected

    } else {
         BTConnected = 0;      // Not Connected  
    }
    layer_mark_dirty(s_hands_layer);
}

void handle_battery(BatteryChargeState charge_state) {
  batterychargepct = charge_state.charge_percent;

  if (charge_state.is_charging) {
    batterycharging = 1;
  } else {
    batterycharging = 0;
  }

  #ifdef PBL_PLATFORM_BASALT
     layer_mark_dirty(LineLayer);
  #else
     layer_mark_dirty(RoundBatteryLayer);
  #endif
}

//Basalt Flat Battery line
void line_layer_update_callback(Layer *LineLayer, GContext* batctx) {

     graphics_fill_rect(batctx, layer_get_bounds(LineLayer), 3, GCornersAll);

     if (batterycharging == 1) {
          graphics_context_set_fill_color(batctx, GColorBlue);
          graphics_fill_rect(batctx, GRect(2, 1, 100, 4), 3, GCornersAll);

     } else if (batterychargepct > 20) {
          graphics_context_set_fill_color(batctx, GColorGreen);
          graphics_fill_rect(batctx, GRect(2, 1, batterychargepct, 4), 3, GCornersAll);

     } else {
          graphics_context_set_fill_color(batctx, GColorRed);
          graphics_fill_rect(batctx, GRect(2, 1, batterychargepct, 4), 3, GCornersAll);
     }
}

// Chalk Circle Battery Line
void RoundBatteryLayer_update_callback(Layer *RoundBatteryLayer, GContext* Roundctx) {
        graphics_context_set_antialiased(Roundctx, true);
        graphics_context_set_fill_color(Roundctx, GColorBlack);
        graphics_fill_radial(Roundctx, GRect(0, 0, 180, 180), GOvalScaleModeFillCircle, 10 /*thickness*/, 0, TRIG_MAX_ANGLE);
        
        
  if (batterycharging == 1) {
          graphics_context_set_fill_color(Roundctx, GColorBlue);
          graphics_fill_radial(Roundctx, GRect(0, 0, 180, 180), GOvalScaleModeFillCircle, 8 /*thickness*/, 0, TRIG_MAX_ANGLE);

     } else if (batterychargepct > 20) {
          graphics_context_set_fill_color(Roundctx, GColorGreen);
          graphics_fill_radial(Roundctx, GRect(0, 0, 180, 180), GOvalScaleModeFillCircle, 8 /*thickness*/, 0, batterychargepct * 0.01  * TRIG_MAX_ANGLE);

     } else {
          graphics_context_set_fill_color(Roundctx, GColorRed);
          graphics_fill_radial(Roundctx, GRect(0, 0, 180, 180), GOvalScaleModeFillCircle, 8 /*thickness*/, 0, batterychargepct  * 0.01 * TRIG_MAX_ANGLE);
     }    
}
static void triangle_display_layer_update_callback(Layer *layer, GContext *ctx) {
     for(ix = 0; ix < 12; ix = ix + 1 ) {
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

    if (BTConnected == 1) {
       graphics_context_set_text_color(hands_ctx, GColorDukeBlue);
       graphics_context_set_fill_color(hands_ctx, GColorYellow);
    } else {
       graphics_context_set_text_color(hands_ctx, GColorWhite);
       graphics_context_set_fill_color(hands_ctx, GColorRed);
    }  

  graphics_fill_circle(hands_ctx, GPoint(hands_bounds.size.w / 2, hands_bounds.size.h / 2), 13);
  #ifdef PBL_PLATFORM_BASALT
      graphics_draw_text(hands_ctx, day_text, fontRobotoCondensed19, GRect(61, 72, 24, 24), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  #else   //  PEBBLE_PLATFORM_CHALK
      graphics_draw_text(hands_ctx, day_text, fontRobotoCondensed19, GRect(78, 78, 24, 24), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  #endif

}


void handle_appfocus(bool in_focus){
    if (in_focus) {
        handle_bluetooth(bluetooth_connection_service_peek());
        layer_mark_dirty(s_hands_layer);
    }
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed) { 
    
  strftime(day_text, sizeof(day_text), "%d", tick_time);
  
  ctr++;
    if (ctr > 11) {
       ctr = 0;
    }
  
    layer_mark_dirty(s_hands_layer);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();

  layer_destroy(triangle_overlay_layer);
  layer_destroy(s_hands_layer);
  
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_focus_service_unsubscribe();
  
  fonts_unload_custom_font(fontRobotoCondensed19);
  
  window_destroy(window);
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  
  fontRobotoCondensed19  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_19));
  
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
  
  // Battery Line Basalt
   #ifdef PBL_PLATFORM_BASALT
      GRect line_frame = GRect(22, 160, 104, 6);
      LineLayer = layer_create(line_frame);
      layer_set_update_proc(LineLayer, line_layer_update_callback);
      layer_add_child(window_layer, LineLayer);
   #else //Chalk
      GRect line_round_frame = GRect(1, 1, 180, 180);
      RoundBatteryLayer = layer_create(line_round_frame);
      layer_set_update_proc(RoundBatteryLayer, RoundBatteryLayer_update_callback);
      layer_add_child(window_layer,RoundBatteryLayer);
   #endif
     
  //Service subscribes:
  battery_state_service_subscribe(&handle_battery);
  
  handle_battery(battery_state_service_peek());
  
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  handle_bluetooth(bluetooth_connection_service_peek());
     
  app_focus_service_subscribe(&handle_appfocus);

}

int main(void) {
   handle_init();

   app_event_loop();

   handle_deinit();
}
