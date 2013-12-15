#include <pebble.h>

static Window *window;
static TextLayer *text_layer1;
static TextLayer *text_layer2;
static TextLayer *text_layer3;
static TextLayer *text_layer4;
static TextLayer *text_layer5;
static TextLayer *text_layer6;
static TextLayer *time_layer;
static TextLayer *date_layer;

#define GLOBAL_COUNTER_STORAGE_KEY 1000


static int get_global_counter(void) {

  bool entry_exists = persist_read_bool(GLOBAL_COUNTER_STORAGE_KEY);
  int32_t global_counter = 0;
  if(entry_exists) {
    global_counter = persist_read_int(GLOBAL_COUNTER_STORAGE_KEY);
  } 

  global_counter++;

  persist_write_int(GLOBAL_COUNTER_STORAGE_KEY, global_counter);

  return global_counter;

}


#define BUFFER_SIZE 25
static char* line_1_buf;
static char* line_2_buf;
static char* line_3_buf;
static char* line_4_buf;
static char* line_5_buf;
static char* line_6_buf;
static char* time_buf;
static char* date_buf;

static void setup_date_buf(void) {
  time_t* clock = malloc(sizeof(time_t));
  time(clock); //update clock to current time
  struct tm* tm = localtime(clock);
  memset(date_buf, 0, BUFFER_SIZE);
  strftime(date_buf, BUFFER_SIZE, "%b %d", tm);

  free(clock);
}

static void setup_time_buf(void) {
  time_t* clock = malloc(sizeof(time_t));
  time(clock); //update clock to current time
  struct tm* tm = localtime(clock);
  char* tmp = malloc(BUFFER_SIZE * sizeof(char));
  memset(tmp, 0, BUFFER_SIZE);
  //see https://sourceware.org/newlib/libc.html#Timefns for format
  strftime(tmp, BUFFER_SIZE, "%I:%M", tm);
  memset(time_buf, 0, BUFFER_SIZE);
  if(tmp[0] == '0') {
    //remove leading zero
    strncpy(time_buf, (char*) &tmp[1], BUFFER_SIZE-1);
  } else {
    strcpy(time_buf, tmp);
  }
  free(tmp);
  free(clock);
}


//draws the horizontal line across the display
static void draw_line_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, (GPoint) { 0, 0 }, (GPoint) { 144,0 });
}

static void window_load(Window *window) {
  window_set_background_color(window, GColorBlack); //flip background
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //set first line of text
  text_layer1 = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 24 } });
  text_layer_set_font(text_layer1, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
  text_layer_set_background_color(text_layer1, GColorBlack);
  text_layer_set_text_color(text_layer1, GColorWhite);
  memset(line_1_buf, 0, BUFFER_SIZE);
  snprintf(line_1_buf, BUFFER_SIZE, "Astro Dark");
  text_layer_set_text(text_layer1, line_1_buf);
  text_layer_set_text_alignment(text_layer1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer1));

  //set second line of text
  text_layer2 = text_layer_create((GRect) { .origin = { 0, 0+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer2, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  text_layer_set_background_color(text_layer2, GColorBlack);
  text_layer_set_text_color(text_layer2, GColorWhite);
  memset(line_2_buf, 0, BUFFER_SIZE);
  snprintf(line_2_buf, BUFFER_SIZE, "-8hr 41min");
  text_layer_set_text(text_layer2, line_2_buf);
  text_layer_set_text_alignment(text_layer2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer2));

  //set third line of text
  text_layer3 = text_layer_create((GRect) { .origin = { 0, 0+24+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer3, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
  text_layer_set_text_color(text_layer3, GColorWhite);
  text_layer_set_background_color(text_layer3, GColorBlack);
  memset(line_3_buf, 0, BUFFER_SIZE);
  snprintf(line_3_buf, BUFFER_SIZE, "Moon Set");
  text_layer_set_text(text_layer3, line_3_buf);
  text_layer_set_text_alignment(text_layer3, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer3));

  //set fourth line of text
  text_layer4 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer4, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  text_layer_set_background_color(text_layer4, GColorBlack);
  text_layer_set_text_color(text_layer4, GColorWhite);
  memset(line_4_buf, 0, BUFFER_SIZE);
  snprintf(line_4_buf, BUFFER_SIZE, "-7hr 41min");
  text_layer_set_text(text_layer4, line_4_buf);
  text_layer_set_text_alignment(text_layer4, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer4));

  //setup creation of lines
  Layer *line_layer = layer_create((GRect) { .origin = { 0, 0+24+24+24+28}, .size = {bounds.size.w, 2} });
  layer_set_update_proc(line_layer, draw_line_callback);
  layer_add_child(window_layer, line_layer);

  //set 5th line of text
  text_layer5 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30}, .size = { bounds.size.w, 20} });
  text_layer_set_font(text_layer5, fonts_get_system_font("RESOURCE_ID_GOTHIC_14"));
  text_layer_set_background_color(text_layer5, GColorBlack);
  text_layer_set_text_color(text_layer5, GColorWhite);
  memset(line_5_buf, 0, BUFFER_SIZE);
  snprintf(line_5_buf, BUFFER_SIZE, "Moon Set: 4:02am Today");
  text_layer_set_text(text_layer5, line_5_buf);
  text_layer_set_text_alignment(text_layer5, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer5));

  //set 6th line of text
  text_layer6 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30+16}, .size = { bounds.size.w, 20} });
  text_layer_set_font(text_layer6, fonts_get_system_font("RESOURCE_ID_GOTHIC_14"));
  text_layer_set_background_color(text_layer6, GColorBlack);
  text_layer_set_text_color(text_layer6, GColorWhite);
  memset(line_6_buf, 0, BUFFER_SIZE);
  snprintf(line_6_buf, BUFFER_SIZE, "Moon Rise: 3:25p Tom.");
  text_layer_set_text(text_layer6, line_6_buf);
  text_layer_set_text_alignment(text_layer6, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer6));

  //set 7th line of text
  time_layer = text_layer_create((GRect) { .origin = { 5, 0+24+24+24+24+38}, .size = { 97, 46} });
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorWhite);

  setup_time_buf();

  //text_layer_set_text(time_layer, "12:44"); //widest?
  text_layer_set_text(time_layer, time_buf);
  text_layer_set_font(time_layer, fonts_get_system_font("RESOURCE_ID_BITHAM_30_BLACK"));
  text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  //set 8th line of text
  date_layer = text_layer_create((GRect) { .origin = { 90, 0+24+24+24+24+38+5}, .size = { 144-90-5, 24} });
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_font(date_layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  memset(date_buf, 0, BUFFER_SIZE);
  snprintf(date_buf, BUFFER_SIZE, "Mar 24");
  //snprintf(date_buf, BUFFER_SIZE, "Dec 24");
  text_layer_set_text(date_layer, date_buf);
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer1);
  text_layer_destroy(text_layer2);
  text_layer_destroy(text_layer3);
  text_layer_destroy(text_layer4);
  text_layer_destroy(text_layer5);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
}

static void setup_time_and_date_callback(void* data) {
  char* previous_time_buf = malloc(BUFFER_SIZE * sizeof(char));
  memset(previous_time_buf, 0, BUFFER_SIZE);
  strcpy(previous_time_buf, time_buf);  

  char* previous_date_buf = malloc(BUFFER_SIZE * sizeof(char));
  memset(previous_date_buf, 0, BUFFER_SIZE);
  strcpy(previous_date_buf, date_buf);

  setup_time_buf();
  setup_date_buf();

  //if the time changed, refresh the window
  if(!strcmp(previous_time_buf, time_buf) || !strcmp(previous_date_buf, date_buf)) { 
    layer_mark_dirty(window_get_root_layer(window));
  }
  free(previous_time_buf);
  free(previous_date_buf);

  app_timer_register(1000, setup_time_and_date_callback, (void*) 0);
}

static void init(void) {
  line_1_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_2_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_3_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_4_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_5_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_6_buf = malloc(BUFFER_SIZE * sizeof(char));
  time_buf = malloc(BUFFER_SIZE * sizeof(char));
  date_buf = malloc(BUFFER_SIZE * sizeof(char));

  app_timer_register(1000, setup_time_and_date_callback, (void*) 0);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  free(line_1_buf);
  free(line_2_buf);
  free(line_3_buf);
  free(line_4_buf);
  free(line_5_buf);
  free(line_6_buf);
  free(time_buf);
  free(date_buf);
}

// ****** the callback functions...


void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
}

const uint32_t SOME_DATA_KEY = 0xb00bf00b;
const uint32_t SOME_STRING_KEY = 0xabbababe;

#define SOME_DATA_KEY  0xb00bf00b
#define SOME_STRING_KEY  0xabbababe

void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  APP_LOG(APP_LOG_LEVEL_INFO, "in_received_handler called..");
  int size = (int)received->end - (int)received->dictionary;

  Tuple *tuple = dict_read_first(received);
  int i=0;
  int LEN = 25;
  char* str = malloc(LEN*sizeof(char));

  while (tuple) {
    switch (tuple->key) {
      case SOME_DATA_KEY:
        i++;
        APP_LOG(APP_LOG_LEVEL_INFO, "Successfully read! %d", tuple->value[0].int8);
        //snprintf(str, LEN, "received: %d", tuple->value[0].int8);
        //text_layer_set_text(text_layer, str);
        break;
      case SOME_STRING_KEY:
        i++;
        APP_LOG(APP_LOG_LEVEL_INFO, "Successfully read! %s", tuple->value[0].cstring);
        snprintf(str, LEN, "received: %s", tuple->value[0].cstring);
        //text_layer_set_text(text_layer, str);
        break;
    }
    tuple = dict_read_next(received);
  }

  //snprintf(str, LEN, "read num: %d", i);
  //text_layer_set_text(text_layer1, str);
  //layer_mark_dirty(window_get_root_layer(window));
  free(str);

}


void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "App message was dropped!");
  // incoming message dropped
}

// ****** the callback functions...

void initCommunication(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  APP_LOG(APP_LOG_LEVEL_INFO, "message handlers registered!");
}

int main(void) {
  init();
  initCommunication();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}


