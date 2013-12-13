#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer2;
static TextLayer *text_layer3;
static int counter = 0; //notice how this is reset when watch face reloads

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

static void setup_time_buf(void) {
  time_t* clock = malloc(sizeof(time_t));
  time(clock); //fetch clock to current time
  struct tm* tm = localtime(clock);
  char* tmp = malloc(BUFFER_SIZE * sizeof(char));
  memset(tmp, 0, BUFFER_SIZE);
  strftime(tmp, BUFFER_SIZE, "%I:%M", tm);
  memset(line_3_buf, 0, BUFFER_SIZE);
  if(tmp[0] == '0') {
    //remove leading zero
    strncpy(line_3_buf, (char*) &tmp[1], BUFFER_SIZE-1);
  } else {
    strcpy(line_3_buf, tmp);
  }
  free(tmp);
  free(clock);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //set first line of text
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  
  memset(line_1_buf, 0, BUFFER_SIZE);
  snprintf(line_1_buf, BUFFER_SIZE, "Count: %d: %d", counter, get_global_counter());
  counter++;

  text_layer_set_text(text_layer, line_1_buf);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  //set second line of text
  text_layer2 = text_layer_create((GRect) { .origin = { 0, 72+20}, .size = { bounds.size.w, 20} });

  memset(line_2_buf, 0, BUFFER_SIZE);
  snprintf(line_2_buf, BUFFER_SIZE, "Second line...");

  text_layer_set_text(text_layer2, line_2_buf);
  text_layer_set_text_alignment(text_layer2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer2));

  //set third line of text
  text_layer3 = text_layer_create((GRect) { .origin = { 0, 72+20+20}, .size = { bounds.size.w, 46} });

  setup_time_buf();

  text_layer_set_text(text_layer3, line_3_buf);
  text_layer_set_font(text_layer3, fonts_get_system_font("RESOURCE_ID_BITHAM_42_BOLD"));
  text_layer_set_text_alignment(text_layer3, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer3));
  
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void setup_time_callback(void* data) {
  char* tmp = malloc(BUFFER_SIZE * sizeof(char));
  memset(tmp, 0, BUFFER_SIZE);
  strcpy(tmp, line_3_buf);  

  setup_time_buf();

  //if the time changed, refresh the window
  if(! strcmp(tmp, line_3_buf)) { 
    layer_mark_dirty(window_get_root_layer(window));
  }
  free(tmp);
  app_timer_register(1000, setup_time_callback, (void*) 0);
}

static void init(void) {
  line_1_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_2_buf = malloc(BUFFER_SIZE * sizeof(char));
  line_3_buf = malloc(BUFFER_SIZE * sizeof(char));

  app_timer_register(1000, setup_time_callback, (void*) 0);

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
}

// ****** the callback functions...


void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
}

//const uint32_t SOME_DATA_KEY = 0xb00bf00b;
//const uint32_t SOME_STRING_KEY = 0xabbababe;

#define SOME_DATA_KEY  0xb00bf00b
#define SOME_STRING_KEY  0xabbababe

void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
//int size = (int)received->end - (int)received->dictionary;

Tuple *tuple = dict_read_first(received);
int i=0;
int LEN = 25;
char* str = malloc(LEN*sizeof(char));

while (tuple) {
  switch (tuple->key) {
    case SOME_DATA_KEY:
      i++;
      //snprintf(str, LEN, "received: %d", tuple->value[0].int8);
      //text_layer_set_text(text_layer, str);
      break;
    case SOME_STRING_KEY:
      i++;
      APP_LOG(APP_LOG_LEVEL_INFO, "Successfully read! %s", tuple->value[0].cstring);
      snprintf(str, LEN, "received: %s", tuple->value[0].cstring);
      text_layer_set_text(text_layer, str);
      break;
  }
  tuple = dict_read_next(received);
}

//snprintf(str, LEN, "read num: %d", i);
//text_layer_set_text(text_layer, str);
free(str);


}


void in_dropped_handler(AppMessageResult reason, void *context) {
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
}

int main(void) {
  init();
  initCommunication();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}


