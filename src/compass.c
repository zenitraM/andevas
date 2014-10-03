/*
 * This application shows how to use the Compass API to build a simple watchface
 * that shows where magnetic north is.
 *
 * The compass background image source is:
 *    <http://opengameart.org/content/north-and-southalpha-chanel>
 *
 */

#include "pebble.h"

// Group all UI elements in a global struct
static struct CompassUI {
  Window *window;
  BitmapLayer *bitmap_layer;
  GBitmap *background;
  Layer *path_layer;
  TextLayer *text_layer;
  TextLayer *text_layer_calib_state;
  TextLayer *text_layer_name;
  GPath *needle_north, *needle_south;
} s_ui;

// Store the target we're going to
static struct TargetDest {
  uint32_t heading;
  uint32_t distance;
  uint32_t accuracy;
} target_dest;

static AppSync sync;
static uint8_t sync_buffer[128];
// Vector paths for the compass needles
static const GPathInfo NEEDLE_NORTH_POINTS = { 3,
  (GPoint []) { { -8, 0 }, { 8, 0 }, { 0, -36 } }
};
static const GPathInfo NEEDLE_SOUTH_POINTS = { 3,
  (GPoint []) { { 8, 0 }, { 0, 36 }, { -8, 0 } }
};

// This is the function called by the Compass Service every time the compass heading
// changes by more than the filter (2 degrees in this example).
void compass_heading_handler(CompassHeadingData heading_data){
  // calculate difference
  // :q
  // rotate needle accordingly
  gpath_rotate_to(s_ui.needle_north, heading_data.magnetic_heading + ((target_dest.heading / (1000*360.0)) * TRIG_MAX_ANGLE));
  gpath_rotate_to(s_ui.needle_south, heading_data.magnetic_heading + ((target_dest.heading / (1000*360.0)) * TRIG_MAX_ANGLE));

  // display heading in degrees and radians, and distance and precision
  static char heading_buf[64];
  snprintf(heading_buf, sizeof(heading_buf),
      " %ldm\n±%ldm",
      target_dest.distance,
      target_dest.accuracy
      );
  text_layer_set_text(s_ui.text_layer, heading_buf);


  static char valid_buf[18];
  switch (heading_data.compass_status) {
    case CompassStatusDataInvalid:
      snprintf(valid_buf, sizeof(valid_buf), "%s", "Calibrating");
      break;
    case CompassStatusCalibrating:
      snprintf(valid_buf, sizeof(valid_buf), "%s", "Fine calibrating");
      break;
    case CompassStatusCalibrated:
      snprintf(valid_buf, sizeof(valid_buf), "%s", "Calibrated");
  }
  text_layer_set_text(s_ui.text_layer_calib_state, valid_buf);

  // trigger layer for refresh
  layer_mark_dirty(s_ui.path_layer);
}

// This is the draw callback for the path_layer function. This function will draw
// both compass needles.
static void path_layer_update_callback(Layer *path, GContext *ctx) {
  gpath_draw_filled(ctx, s_ui.needle_north);                      // north filled
  gpath_draw_outline(ctx, s_ui.needle_south);                     // south outlined
  GRect bounds = layer_get_frame(path);                           // grabbing frame of current layer
  GPoint path_center = GPoint(bounds.size.w/2, bounds.size.h/2);  // creating centerpoint
  graphics_fill_circle(ctx, path_center, 3);                      // use it to make a black, centered circle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, path_center, 2);                      // then put a white circle on top
}

// Initializes the window and all the UI elements
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Place text layers onto screen: one for the heading and one for calibration status
  s_ui.text_layer = text_layer_create( (GRect){
      .origin = {.x = 12, .y = bounds.size.h*3/4},
      .size = {.w = bounds.size.w/2, .h = bounds.size.h/5}
      });
  text_layer_set_text(s_ui.text_layer, "No Data");

  s_ui.text_layer_calib_state = text_layer_create( (GRect){
      .origin = {.x = 0, .y = 0},
      .size = {.w = bounds.size.w, .h = bounds.size.h/7}
      });
  text_layer_set_text_alignment(s_ui.text_layer_calib_state, GTextAlignmentRight);
  text_layer_set_background_color(s_ui.text_layer_calib_state, GColorClear);

  s_ui.text_layer_name = text_layer_create( (GRect){
      .origin = {.x = 0, .y = bounds.size.h-15},
      .size = {.w = bounds.size.w, .h = bounds.size.h/7}
      });

  text_layer_set_text_alignment(s_ui.text_layer_name, GTextAlignmentCenter);
  text_layer_set_background_color(s_ui.text_layer_name, GColorClear);

  layer_add_child(window_layer, text_layer_get_layer(s_ui.text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_ui.text_layer_calib_state));
  layer_add_child(window_layer, text_layer_get_layer(s_ui.text_layer_name));
  // Create the bitmap for the background and put it on the screen
  s_ui.bitmap_layer = bitmap_layer_create(bounds);
  s_ui.background = gbitmap_create_with_resource(RESOURCE_ID_COMPASS_BACKGROUND);
  bitmap_layer_set_bitmap(s_ui.bitmap_layer, s_ui.background);
  // Make needle background 'transparent' with GCompOpAnd
  bitmap_layer_set_compositing_mode(s_ui.bitmap_layer, GCompOpAnd);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_ui.bitmap_layer));

  // Create the layer in which we will draw the compass needles
  s_ui.path_layer = layer_create(bounds);
  //  Define the draw callback to use for this layer
  layer_set_update_proc(s_ui.path_layer, path_layer_update_callback);
  layer_add_child(window_layer, s_ui.path_layer);

  // Initialize and define the two paths used to draw the needle to north and to south
  // see Examples/watchapps/feature_gpath for more info
  s_ui.needle_north = gpath_create(&NEEDLE_NORTH_POINTS);
  s_ui.needle_south = gpath_create(&NEEDLE_SOUTH_POINTS);

  // Move the needles to the center of the screen.
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  gpath_move_to(s_ui.needle_north, center);
  gpath_move_to(s_ui.needle_south, center);

}

// Free all memory initialized in window_load()
static void window_unload(Window *window) {
  text_layer_destroy(s_ui.text_layer);
  text_layer_destroy(s_ui.text_layer_calib_state);
  gpath_destroy(s_ui.needle_north);
  gpath_destroy(s_ui.needle_south);
  layer_destroy(s_ui.path_layer);
  gbitmap_destroy(s_ui.background);
  bitmap_layer_destroy(s_ui.bitmap_layer);
}
void sync_tuple_changed_callback(const uint32_t key,
    const Tuple* new_tuple, const Tuple* old_tuple, void* context)
{
  switch (new_tuple->key) {
    case 0: // Distance
      target_dest.distance = new_tuple->value->uint32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "distance: %ld", target_dest.distance);
      break;
    case 1:
      target_dest.heading = new_tuple->value->uint32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "heading: %ld", target_dest.heading);
      break;
    case 2:
      target_dest.accuracy = new_tuple->value->uint32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "accuracy: %ld", target_dest.accuracy);
      break;
    case 3:
      text_layer_set_text(s_ui.text_layer_name, new_tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "name: %s", new_tuple->value->cstring);
      break;
  }
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}
static void init(void) {
  const int inbound_size = 128;
  const int outbound_size = 128;
  app_message_open(inbound_size, outbound_size);

  Tuplet initial_values[] = {
    TupletInteger(0, (uint32_t) 0),
    TupletInteger(1, (uint32_t) 0),
    TupletInteger(2, (uint32_t) 0),
    TupletCString(3, "Loading..")
  };
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  // initialize compass and set a filter to 2 degrees
  compass_service_set_heading_filter((TRIG_MAX_ANGLE/360));
  compass_service_subscribe(&compass_heading_handler);

  // initialize base window
  s_ui.window = window_create();
  window_set_window_handlers(s_ui.window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });
  window_set_fullscreen(s_ui.window, true);
  window_stack_push(s_ui.window, true);
}

static void deinit(void) {
  app_sync_deinit(&sync);
  compass_service_unsubscribe();
  window_destroy(s_ui.window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
