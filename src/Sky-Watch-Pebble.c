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

/**
 The time related events, such as the value for SUN_RISE_KEY, are encoded as
 the minute of the day 0 to 1,4440. e.g. 0 = 12:00am and 14439 = 11:59pm
 
 The watch and iPhone share the same timezone at the time of synchronization.
 */
#define CHUNK_KEY                         500 // 8bit int
#define DAY_SLOT_KEY                     1000 // 8bit int

/** Chunk 1 **/
#define DAY_OF_YEAR_KEY                  1001 //16bit int
#define YEAR_KEY                         1002 //16bit int
#define SUN_RISE_KEY                     1003 //16bit int
#define SUN_RISE_AZIMUTH_KEY             1004 //16bit int
#define SUN_SET_KEY                      1005 //16bit int
#define SUN_SET_AZIMUTH_KEY              1006 //16bit int
#define SOLAR_NOON_KEY                   1007 //16bit int
#define SOLAR_MIDNIGHT_KEY               1008 //16bit int

/** Chunk 2 **/
#define GOLDEN_HOUR_BEGIN_KEY            1009 //16bit int
#define GOLDEN_HOUR_END_KEY              1010 //16bit int
#define CIVIL_TWILIGHT_BEGIN_KEY         1011 //16bit int
#define CIVIL_TWILIGHT_END_KEY           1012 //16bit int
#define NAUTICAL_TWILIGHT_BEGIN_KEY      1013 //16bit int
#define NAUTICAL_TWILIGHT_END_KEY        1014 //16bit int

/** Chunk 3 **/
#define ASTRONOMICAL_TWILIGHT_BEGIN_KEY  1015 //16bit int
#define ASTRONOMICAL_TWILIGHT_END_KEY    1016 //16bit int
#define MOON_RISE_KEY                    1017 //16bit int
#define MOON_RISE_AZIMUTH_KEY            1018 //16bit int
#define MOON_SET_KEY                     1019 //16bit int
#define MOON_SET_AZIMUTH_KEY             1020 //16bit int
#define MOON_AGE_KEY                     1021 // 8bit int
#define MOON_PERCENT_ILLUMINATION_KEY    1022 // 8bit int

#define CHUNK_VALUE_1   0
#define CHUNK_VALUE_2   1
#define CHUNK_VALUE_3   2

typedef struct day_of_data {
    uint8_t  chunk;
    uint8_t  day_slot;
    
    uint16_t day_of_year;
    uint16_t year;
    uint16_t sun_rise;
    uint16_t sun_rise_azimuth;
    uint16_t sun_set;
    uint16_t sun_set_azimuth;
    uint16_t solar_noon;
    uint16_t solar_midnight;
    
    uint16_t golden_hour_begin;
    uint16_t golden_hour_end;
    uint16_t civil_twilight_begin;
    uint16_t civil_twilight_end;
    uint16_t nautical_twilight_begin;
    uint16_t nautical_twilight_end;
    uint16_t astronomical_twilight_begin;
    uint16_t astronomical_twilight_end;
    
    uint16_t moon_rise;
    uint16_t moon_rise_azimuth;
    uint16_t moon_set;
    uint16_t moon_set_azimuth;
    uint8_t  moon_age;
    uint8_t  moon_percent_illumination;
} DATA;

typedef struct time {
    uint8_t hour;
    uint8_t min;
} TIME;


void intToTime(int t, TIME *time) {
    time->hour = t/60;
    time->min  = t % 60;
}

char* intToChar(int t, char *c, int size_of_buf) {
    memset(c, 0, size_of_buf);
    TIME *time = malloc(sizeof(TIME));
    intToTime(t, time);
    snprintf(c, size_of_buf, "%d:%02d", time->hour, time->min);
    free(time);
    
    return c;
}


void dump_to_log(DATA *data_buf) {
    int c_size = 10;
    char *c = malloc(sizeof(char)*c_size);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "\n\n **************** ");
    APP_LOG(APP_LOG_LEVEL_INFO, "data slot: %d", data_buf->day_slot);
    APP_LOG(APP_LOG_LEVEL_INFO, "day of year: %d", data_buf->day_of_year);
    APP_LOG(APP_LOG_LEVEL_INFO, "year: %d", data_buf->year);
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise: %s", intToChar(data_buf->sun_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise azimuth: %d", data_buf->sun_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set: %s", intToChar(data_buf->sun_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set azimuth: %d", data_buf->sun_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "solar noon: %s", intToChar(data_buf->solar_noon, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "solar midnight: %s", intToChar(data_buf->solar_midnight, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour begin: %s", intToChar(data_buf->golden_hour_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour end: %s", intToChar(data_buf->golden_hour_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight begin: %s", intToChar(data_buf->civil_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight end: %s", intToChar(data_buf->civil_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight begin: %s", intToChar(data_buf->nautical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight end: %s", intToChar(data_buf->nautical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight begin: %s", intToChar(data_buf->astronomical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight end: %s", intToChar(data_buf->astronomical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise: %s", intToChar(data_buf->moon_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise azimuth: %d", data_buf->moon_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set: %s", intToChar(data_buf->moon_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set azimuth: %d", data_buf->moon_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon age: %d", data_buf->moon_age);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon percent illumination: %d", data_buf->moon_percent_illumination);
    
    free(c);
}

/**
 Max expected slot number is 128 which takes up 8 bits.
 Day key is 13 in decimal shifted 16 bits to the left.
 **/
uint32_t get_data_storage_key(uint8_t slot) {
    
    uint32_t k = 13 << 16;
    k = k | slot;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "slot %d to storage_key %d", slot, (int) k);
    return k;
    
}

static DATA* data_buf;

// incoming message received
void in_received_handler(DictionaryIterator *received, void *context) {
    //int size = (int)received->end - (int)received->dictionary;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler... Size of incoming dictionary: %d", size);
    
    Tuple *tuple = dict_read_first(received);

    while (tuple) {
        switch (tuple->key) {
            case CHUNK_KEY:
                data_buf->chunk = tuple->value[0].int8;
                break;
            case DAY_SLOT_KEY:
                data_buf->day_slot = tuple->value[0].int8;
                break;
            case DAY_OF_YEAR_KEY:
                data_buf->day_of_year = tuple->value[0].int16;
                break;
            case YEAR_KEY:
                data_buf->year = tuple->value[0].int16;
                break;
            case SUN_RISE_KEY:
                data_buf->sun_rise = tuple->value[0].int16;
                break;
            case SUN_RISE_AZIMUTH_KEY:
                data_buf->sun_rise_azimuth = tuple->value[0].int16;
                break;
            case SUN_SET_KEY:
                data_buf->sun_set = tuple->value[0].int16;
                break;
            case SUN_SET_AZIMUTH_KEY:
                data_buf->sun_set_azimuth = tuple->value[0].int16;
                break;
            case SOLAR_NOON_KEY:
                data_buf->solar_noon = tuple->value[0].int16;
                break;
            case SOLAR_MIDNIGHT_KEY:
                data_buf->solar_midnight = tuple->value[0].int16;
                break;
            case GOLDEN_HOUR_BEGIN_KEY:
                data_buf->golden_hour_begin = tuple->value[0].int16;
                break;
            case GOLDEN_HOUR_END_KEY:
                data_buf->golden_hour_end = tuple->value[0].int16;
                break;
            case CIVIL_TWILIGHT_BEGIN_KEY:
                data_buf->civil_twilight_begin = tuple->value[0].int16;
                break;
            case CIVIL_TWILIGHT_END_KEY:
                data_buf->civil_twilight_end = tuple->value[0].int16;
                break;
            case NAUTICAL_TWILIGHT_BEGIN_KEY:
                data_buf->nautical_twilight_begin = tuple->value[0].int16;
                break;
            case NAUTICAL_TWILIGHT_END_KEY:
                data_buf->nautical_twilight_end = tuple->value[0].int16;
                break;
            case ASTRONOMICAL_TWILIGHT_BEGIN_KEY:
                data_buf->astronomical_twilight_begin = tuple->value[0].int16;
                break;
            case ASTRONOMICAL_TWILIGHT_END_KEY:
                data_buf->astronomical_twilight_end = tuple->value[0].int16;
                break;
            case MOON_RISE_KEY:
                data_buf->moon_rise = tuple->value[0].int16;
                break;
            case MOON_RISE_AZIMUTH_KEY:
                data_buf->moon_rise_azimuth = tuple->value[0].int16;
                break;
            case MOON_SET_KEY:
                data_buf->moon_set = tuple->value[0].int16;
                break;
            case MOON_SET_AZIMUTH_KEY:
                data_buf->moon_set_azimuth = tuple->value[0].int16;
                break;
            case MOON_AGE_KEY:
                data_buf->moon_age = tuple->value[0].int8;
                break;
            case MOON_PERCENT_ILLUMINATION_KEY:
                data_buf->moon_percent_illumination = tuple->value[0].int8;
                break;
        }
        
        tuple = dict_read_next(received);
    }
    
    if(data_buf->chunk == CHUNK_VALUE_3) {
//        dump_to_log(data_buf);

        //store the day's worth of data into storage
        uint32_t key = get_data_storage_key(data_buf->day_slot);
        persist_write_data (key, data_buf, sizeof(DATA));

        memset(data_buf, 0, sizeof(DATA));
    }

    free(tuple);

}



void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "App message was dropped!");
  // incoming message dropped
}

// ****** the callback functions...

void initCommunication(void) {
  data_buf = malloc(sizeof(DATA));
  memset(data_buf, 0, sizeof(DATA));
    
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);

  const uint32_t inbound_size = 124;
  const uint32_t outbound_size = 124;
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


