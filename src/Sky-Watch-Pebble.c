#include <pebble.h>
#include "Watch-Data.h"


typedef enum {HORIZON, CIVIL, NAUTICAL, ASTRONOMICAL} DAY_COUNT_DOWN;

typedef struct config {
    DAY_COUNT_DOWN day_countdown;
} CONFIG;

static CONFIG *config;

static char* get_day_countdown_header() {
    switch(config->day_countdown) {
        case HORIZON:
            return "Daylight";
        case CIVIL:
            return "Civil Light";
        case NAUTICAL:
            return "Nautical Light";
        case ASTRONOMICAL:
            return "Astro Light";
    }
    
    return 0;
}

static char* get_night_countdown_header() {
    switch(config->day_countdown) {
        case HORIZON:
            return "Dark";
        case CIVIL:
            return "Civil Dark";
        case NAUTICAL:
            return "Nautical Dark";
        case ASTRONOMICAL:
            return "Astro Dark";
    }
    
    return 0;
}

typedef struct remaining {
    bool is_object_up;
    int  mins;
} REMAINING;

static void is_day(SEARCH_RESULT *based_on, REMAINING* in) {
    time_t* clock = malloc(sizeof(time_t));
    time(clock); //update clock to current time
    struct tm* tm = localtime(clock);

    int today_day_begin = 0;
    int today_day_end   = 0;
    int tomorrow_day_begin = 0;
    
    switch(config->day_countdown) {
        case HORIZON:
            today_day_begin = based_on->today->sun_rise;
            today_day_end   = based_on->today->sun_set;
            tomorrow_day_begin = based_on->tomorrow->sun_rise;
            break;
        case CIVIL:
            today_day_begin = based_on->today->civil_twilight_begin;
            today_day_end   = based_on->today->civil_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->civil_twilight_begin;
            break;
        case NAUTICAL:
            today_day_begin = based_on->today->nautical_twilight_begin;
            today_day_end   = based_on->today->nautical_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->nautical_twilight_begin;
            break;
        case ASTRONOMICAL:
            today_day_begin = based_on->today->astronomical_twilight_begin;
            today_day_end   = based_on->today->astronomical_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->astronomical_twilight_begin;
    }

    int minute_of_day = 60*tm->tm_hour + tm->tm_min;
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "is_day basedon day_begin: %d and day_end: %d", today_day_begin, today_day_end);
    //APP_LOG(APP_LOG_LEVEL_INFO, "is_day basedon current_min_of_day: %d ", minute_of_day);

    if(minute_of_day >= today_day_begin && minute_of_day <  today_day_end) {
        in->is_object_up = true;
        in->mins = today_day_end - minute_of_day;
    } else if (minute_of_day < today_day_begin) {
        in->is_object_up = false;
        in->mins = tomorrow_day_begin - minute_of_day;
    } else {
        in->is_object_up = false;
        in->mins = 24*60 - minute_of_day + tomorrow_day_begin;
    }
    
    free(clock);
}

static void moon_remaining(SEARCH_RESULT *based_on, REMAINING *in) {
    time_t* clock = malloc(sizeof(time_t));
    time(clock); //update clock to current time
    struct tm* tm = localtime(clock);
    
    int minute_of_day = 60*tm->tm_hour + tm->tm_min;

    if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_set) {
        //upcoming moon set and countdown = set - time
        in->is_object_up = true;
        in->mins = based_on->today->moon_set - minute_of_day;
        
    } else if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_rise) {
        //upcoming moon rise and countdown = rise - time
        in->is_object_up = false;
        in->mins = based_on->today->moon_rise - minute_of_day;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_rise) {
        //upcoming moon rise and countdown = rise - time
        in->is_object_up = false;
        in->mins = based_on->today->moon_rise - minute_of_day;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_set) {
        //upcoming moon set and countdown = set - time
        in->is_object_up = true;
        in->mins = based_on->today->moon_set - minute_of_day;
        
    } else if (based_on->tomorrow->moon_set < based_on->tomorrow->moon_rise) {
        //upcoming moon set and countdown = 24hr - time + next set
        in->is_object_up = true;
        in->mins = 24*60 - minute_of_day + based_on->tomorrow->moon_set;
        
    } else {
        //upcoming moon rise and countdown = 24hr - time + next rise
        in->is_object_up = false;
        in->mins = 24*60 - minute_of_day + based_on->tomorrow->moon_rise;
        
    }

    free(clock);
}

typedef struct event {
    bool is_object_rising;
    bool is_today;
    int  minute_of_day;
} EVENT;

static void next_two_moon_events(SEARCH_RESULT *based_on, EVENT (*in)[1]) {
    EVENT *first_event = in[0];
    EVENT *second_event = in[1];
    
    time_t* clock = malloc(sizeof(time_t));
    time(clock); //update clock to current time
    struct tm* tm = localtime(clock);
    
    int minute_of_day = 60*tm->tm_hour + tm->tm_min;
    
    if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_set) {
        //upcoming today moon set, then today moon rise
        first_event->is_object_rising = false;
        first_event->is_today = true;
        first_event->minute_of_day = based_on->today->moon_set;
        
        second_event->is_object_rising = true;
        second_event->is_today = true;
        second_event->minute_of_day = based_on->today->moon_rise;
        
    } else if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_rise) {
        //upcoming today moon rise, then tomorrow moon set
        first_event->is_object_rising = true;
        first_event->is_today = true;
        first_event->minute_of_day = based_on->today->moon_rise;
        
        second_event->is_object_rising = false;
        second_event->is_today = false;
        second_event->minute_of_day = based_on->tomorrow->moon_set;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_rise) {
        //upcoming today moon rise, then today moon set
        first_event->is_object_rising = true;
        first_event->is_today = true;
        first_event->minute_of_day = based_on->today->moon_rise;
        
        second_event->is_object_rising = false;
        second_event->is_today = true;
        second_event->minute_of_day = based_on->today->moon_set;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_set) {
        //upcoming today moon set, then tomorrow moon rise
        first_event->is_object_rising = false;
        first_event->is_today = true;
        first_event->minute_of_day = based_on->today->moon_set;
        
        second_event->is_object_rising = true;
        second_event->is_today = false;
        second_event->minute_of_day = based_on->tomorrow->moon_rise;
        
    } else if (based_on->tomorrow->moon_set < based_on->tomorrow->moon_rise) {
        //upcoming tomorrow moon set, then tomorrow moon rise
        first_event->is_object_rising = false;
        first_event->is_today = false;
        first_event->minute_of_day = based_on->tomorrow->moon_set;
        
        second_event->is_object_rising = true;
        second_event->is_today = false;
        second_event->minute_of_day = based_on->tomorrow->moon_rise;
        
    } else {
        //upcoming tomorrow moon rise, then tomorrow moon set
        first_event->is_object_rising = true;
        first_event->is_today = false;
        first_event->minute_of_day = based_on->tomorrow->moon_rise;
        
        second_event->is_object_rising = false;
        second_event->is_today = false;
        second_event->minute_of_day = based_on->tomorrow->moon_set;
        
    }
    
    free(clock);
}

static void countdown_mins_to_char(int mins, char* buf, int buffer_size) {
    if(mins < 60) {
        snprintf(buf, buffer_size, "-%dmin", mins);
    } else {
        snprintf(buf, buffer_size, "-%dhr %dmin", (mins/60), (mins%60));
    }
}


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
static GColor current_background_color;

static void setup_color_scheme(bool black_background) {
    if(black_background) {
        current_background_color = GColorBlack;
        window_set_background_color(window, GColorBlack);
        
        text_layer_set_background_color(text_layer1, GColorBlack);
        text_layer_set_text_color(text_layer1, GColorWhite);
        
        text_layer_set_font(text_layer2, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
        text_layer_set_background_color(text_layer2, GColorBlack);
        text_layer_set_text_color(text_layer2, GColorWhite);
        
        text_layer_set_background_color(text_layer3, GColorBlack);
        text_layer_set_text_color(text_layer3, GColorWhite);
        
        text_layer_set_font(text_layer4, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
        text_layer_set_background_color(text_layer4, GColorBlack);
        text_layer_set_text_color(text_layer4, GColorWhite);
        
        text_layer_set_background_color(text_layer5, GColorBlack);
        text_layer_set_text_color(text_layer5, GColorWhite);
        
        text_layer_set_background_color(text_layer6, GColorBlack);
        text_layer_set_text_color(text_layer6, GColorWhite);
        
        text_layer_set_background_color(time_layer, GColorBlack);
        text_layer_set_text_color(time_layer, GColorWhite);
        
        text_layer_set_font(date_layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
        text_layer_set_background_color(date_layer, GColorBlack);
        text_layer_set_text_color(date_layer, GColorWhite);
        
    } else {
        current_background_color = GColorWhite;
        window_set_background_color(window, GColorWhite);
        
        text_layer_set_background_color(text_layer1, GColorWhite);
        text_layer_set_text_color(text_layer1, GColorBlack);
        
        text_layer_set_font(text_layer2, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
        text_layer_set_background_color(text_layer2, GColorWhite);
        text_layer_set_text_color(text_layer2, GColorBlack);
        
        text_layer_set_background_color(text_layer3, GColorWhite);
        text_layer_set_text_color(text_layer3, GColorBlack);

        text_layer_set_font(text_layer4, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
        text_layer_set_background_color(text_layer4, GColorWhite);
        text_layer_set_text_color(text_layer4, GColorBlack);
        
        text_layer_set_background_color(text_layer5, GColorBlack);
        text_layer_set_text_color(text_layer5, GColorWhite);
        
        text_layer_set_background_color(text_layer6, GColorBlack);
        text_layer_set_text_color(text_layer6, GColorWhite);
        
        text_layer_set_background_color(time_layer, GColorWhite);
        text_layer_set_text_color(time_layer, GColorBlack);
        
        text_layer_set_font(date_layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
        text_layer_set_background_color(date_layer, GColorWhite);
        text_layer_set_text_color(date_layer, GColorBlack);
    }
}

static void setup_day_countdown_bufs(void) {
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    

    if(ret_result == 0) {
        snprintf(line_1_buf, BUFFER_SIZE, "--NO DATA--");
        snprintf(line_2_buf, BUFFER_SIZE, " ");
        setup_color_scheme(true);
    } else {
        REMAINING *remaining = malloc(sizeof(REMAINING));
        is_day(result, remaining);
        
        if(remaining->is_object_up) {
            snprintf(line_1_buf, BUFFER_SIZE, get_night_countdown_header());
            setup_color_scheme(false);
        } else {
            snprintf(line_1_buf, BUFFER_SIZE, get_day_countdown_header());
            setup_color_scheme(true);
        }
        
        countdown_mins_to_char(remaining->mins, line_2_buf, BUFFER_SIZE);
        
        free(remaining);
    }
    
    free(today_data);
    free(tomorrow_data);
    free(result);
}

static void setup_moon_countdown_bufs(void) {
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    

    if(ret_result == 0) {
        snprintf(line_3_buf, BUFFER_SIZE, "--NO DATA--");
        snprintf(line_4_buf, BUFFER_SIZE, " ");
    } else {
        REMAINING *remaining = malloc(sizeof(REMAINING));
        moon_remaining(result, remaining);
        
        if(remaining->is_object_up) {
            snprintf(line_3_buf, BUFFER_SIZE, "Moon Set");
        } else {
            snprintf(line_3_buf, BUFFER_SIZE, "Moon Rise");
        }
        
        countdown_mins_to_char(remaining->mins, line_4_buf, BUFFER_SIZE);
        
        free(remaining);
    }
    
    free(today_data);
    free(tomorrow_data);
    free(result);
}


static void min_of_day_to_char(int minute_of_day, char* buf, int buffer_size) {
    memset(buf, 0, buffer_size);
    
    int hour = minute_of_day / 60;
    int min =  minute_of_day % 60;
    
    char* am_pm = "a";
    if(hour > 12) {
        hour -= 12;
        am_pm = "p";
    }
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "min_of_day_to_char called with min_of_day: %d am_pm: NA", minute_of_day);
    
    snprintf(buf, buffer_size, "%d:%02d%s", hour, min, am_pm);
}


static void setup_no_data_tiny_bufs(void) {
    snprintf(line_5_buf, BUFFER_SIZE, "Please use Sky Watch");
    snprintf(line_6_buf, BUFFER_SIZE, "iPhone app to push data.");
}

static void setup_choosen_twilight_tiny_bufs(void) {
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    
    int twilight_begin = 0;
    int twilight_end   = 0;
    char *kind_of_twilight = "";
    
    
    if(ret_result == 0) {
        setup_no_data_tiny_bufs();
    } else {
        switch(config->day_countdown) {
            case CIVIL:
                kind_of_twilight = "Civil";
                twilight_begin = result->today->civil_twilight_begin;
                twilight_end   = result->today->civil_twilight_end;
                break;
            case NAUTICAL:
                kind_of_twilight = "Nautical";
                twilight_begin = result->today->nautical_twilight_begin;
                twilight_end   = result->today->nautical_twilight_end;
                break;
            case ASTRONOMICAL:
                kind_of_twilight = "Astronomical";
                twilight_begin = result->today->astronomical_twilight_begin;
                twilight_end   = result->today->astronomical_twilight_end;
                break;
            default:
                return; //don't setup display
        }
        
        snprintf(line_5_buf, BUFFER_SIZE, "%s Twilight", kind_of_twilight);
        
        char *begin = malloc(sizeof(char)*BUFFER_SIZE);
        char *end = malloc(sizeof(char)*BUFFER_SIZE);
        min_of_day_to_char(twilight_begin, begin, BUFFER_SIZE);
        min_of_day_to_char(twilight_end, end, BUFFER_SIZE);
        snprintf(line_6_buf, BUFFER_SIZE, "Begin %s - End %s", begin, end);
        
        free(end);
        free(begin);
    }
    
    free(today_data);
    free(tomorrow_data);
    free(result);
}

/** Set this up to get called instead of setup_moon_tiny_bufs **/
static void setup_sunrise_sunset_tiny_bufs(void) {
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    
    if(ret_result == 0) {
        setup_no_data_tiny_bufs();
    } else {
        char *time = malloc(sizeof(char)*BUFFER_SIZE);
        
        min_of_day_to_char(result->today->sun_rise, time, BUFFER_SIZE);
        snprintf(line_5_buf, BUFFER_SIZE, "Sunrise %s", time);
        
        min_of_day_to_char(result->today->sun_set, time, BUFFER_SIZE);
        snprintf(line_6_buf, BUFFER_SIZE, "Sunset %s", time);
        
        free(time);
    }
    
    free(today_data);
    free(tomorrow_data);
    free(result);
}

static void moon_event_to_char(EVENT *event, char *buf, int buffer_size) {
    char *rise_set =       malloc(sizeof(char)*BUFFER_SIZE);
    char *time =           malloc(sizeof(char)*BUFFER_SIZE);
    char *today_tomorrow = malloc(sizeof(char)*BUFFER_SIZE);
    
    memset(rise_set, 0, BUFFER_SIZE);
    memset(time, 0, BUFFER_SIZE);
    memset(today_tomorrow, 0, BUFFER_SIZE);
    
    memcpy(rise_set,(event->is_object_rising ? "rise" : "set"),BUFFER_SIZE);
    min_of_day_to_char(event->minute_of_day, time, BUFFER_SIZE);
    memcpy(today_tomorrow,(event->is_today ? "Today" : "Tom."),BUFFER_SIZE);
    
    snprintf(buf, buffer_size, "Moon %s %s %s",
             rise_set,
             time,
             today_tomorrow );
    
    free(today_tomorrow);
    free(rise_set);
    free(time);
}

static void setup_moon_tiny_bufs(void) {
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    
    
    if(ret_result == 0) {
        setup_no_data_tiny_bufs();
    } else {
        EVENT (*events)[1] = malloc(sizeof(EVENT)*2);
        memset(events, 0, sizeof(EVENT)*2);
        next_two_moon_events(result, events);
        
        moon_event_to_char(events[0], line_5_buf, BUFFER_SIZE);
        moon_event_to_char(events[1], line_6_buf, BUFFER_SIZE);

        free(events);
    }
    
    free(today_data);
    free(tomorrow_data);
    free(result);
}

typedef enum {TWILIGHT, SUNRISE_SUNSET, MOON_RISE_SET} TINY_DISPLAY;

static TINY_DISPLAY next_tiny_display;
static int tiny_display_count = 0;
static bool force_display_tiny_bufs = true;

static void setup_tiny_bufs() {
    
    tiny_display_count++;
    if(tiny_display_count >= 1000) {
        tiny_display_count = 0;
    }
    
    //only change the display every 8 seconds
    if(tiny_display_count%8 != 0 && !force_display_tiny_bufs) {
        return;
    }
    
    force_display_tiny_bufs = false;
    
    switch(next_tiny_display) {
        case TWILIGHT:
            setup_choosen_twilight_tiny_bufs();
            next_tiny_display = SUNRISE_SUNSET;
            break;
        case SUNRISE_SUNSET:
            setup_sunrise_sunset_tiny_bufs();
            next_tiny_display = MOON_RISE_SET;
            break;
        case MOON_RISE_SET:
            setup_moon_tiny_bufs();
            next_tiny_display = TWILIGHT;
            break;
        default:
            next_tiny_display = SUNRISE_SUNSET;
    }
    
}

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

  //work around font rendering issue when starting with 4 with particular font
    if(time_buf[0] == '4') {
      memset(tmp, 0, BUFFER_SIZE);
      snprintf(tmp, BUFFER_SIZE, " %s", time_buf);
      memset(time_buf, 0, BUFFER_SIZE);
      strncpy(time_buf, tmp, BUFFER_SIZE);
    }
  free(tmp);
  free(clock);
}

//draws the horizontal line across the display
static void draw_line_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, (GPoint) { 15, 0 }, (GPoint) { 144-15,0 });
}


//draws the horizontal line across the display
static void draw_line_callback2(Layer *layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, (GPoint) { 0, 0 }, (GPoint) { 144,0 });
}

static void window_load(Window *window) {
  current_background_color = GColorBlack;
  window_set_background_color(window, GColorBlack); //flip background
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //set second line of text
  text_layer2 = text_layer_create((GRect) { .origin = { 0, 0+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer2, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  text_layer_set_background_color(text_layer2, GColorBlack);
  text_layer_set_text_color(text_layer2, GColorWhite);
  memset(line_2_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer2, line_2_buf);
  text_layer_set_text_alignment(text_layer2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer2));
    
  //set first line of text
  text_layer1 = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 29 } });
  text_layer_set_font(text_layer1, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
  text_layer_set_background_color(text_layer1, GColorBlack);
  text_layer_set_text_color(text_layer1, GColorWhite);
  text_layer_set_text(text_layer1, line_1_buf);
  memset(line_1_buf, 0, BUFFER_SIZE);
  text_layer_set_text_alignment(text_layer1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer1));
    
  //set third line of text
  text_layer3 = text_layer_create((GRect) { .origin = { 0, 0+24+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer3, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
  text_layer_set_text_color(text_layer3, GColorWhite);
  text_layer_set_background_color(text_layer3, GColorBlack);
  memset(line_3_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer3, line_3_buf);
  text_layer_set_text_alignment(text_layer3, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer3));
    
  //setup creation of line visible on white background
  Layer *line_layer1 = layer_create((GRect) { .origin = { 0, 0+24+28}, .size = {bounds.size.w, 2} });
  layer_set_update_proc(line_layer1, draw_line_callback);
  layer_add_child(window_layer, line_layer1);

  //set fourth line of text
  text_layer4 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24}, .size = { bounds.size.w, 24} });
  text_layer_set_font(text_layer4, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  text_layer_set_background_color(text_layer4, GColorBlack);
  text_layer_set_text_color(text_layer4, GColorWhite);
  memset(line_4_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer4, line_4_buf);
  text_layer_set_text_alignment(text_layer4, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer4));

  //setup creation of line visible on black background
  Layer *line_layer2 = layer_create((GRect) { .origin = { 0, 0+24+24+24+28}, .size = {bounds.size.w, 2} });
  layer_set_update_proc(line_layer2, draw_line_callback2);
  layer_add_child(window_layer, line_layer2);

  //set 5th line of text
  text_layer5 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30}, .size = { bounds.size.w, 20} });
  text_layer_set_font(text_layer5, fonts_get_system_font("RESOURCE_ID_GOTHIC_14"));
  text_layer_set_background_color(text_layer5, GColorBlack);
  text_layer_set_text_color(text_layer5, GColorWhite);
  memset(line_5_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer5, line_5_buf);
  text_layer_set_text_alignment(text_layer5, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer5));

  //set 7th line of text
  time_layer = text_layer_create((GRect) { .origin = { 5, 0+24+24+24+24+38}, .size = { 97, 46} });
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font("RESOURCE_ID_BITHAM_30_BLACK"));
  setup_time_buf();
  //text_layer_set_text(time_layer, "4:44"); //problem starting with 4
  text_layer_set_text(time_layer, time_buf);
  text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  //set 8th line of text
  date_layer = text_layer_create((GRect) { .origin = { 90, 0+24+24+24+24+38+5}, .size = { 144-90-2, 24} });
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_font(date_layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24"));
  memset(date_buf, 0, BUFFER_SIZE);
  text_layer_set_text(date_layer, date_buf);
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  //set 6th line of text -- this has to come after line 7 and 8
  text_layer6 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30+16}, .size = { bounds.size.w, 20} });
  text_layer_set_font(text_layer6, fonts_get_system_font("RESOURCE_ID_GOTHIC_14"));
  text_layer_set_background_color(text_layer6, GColorBlack);
  text_layer_set_text_color(text_layer6, GColorWhite);
  memset(line_6_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer6, line_6_buf);
  text_layer_set_text_alignment(text_layer6, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer6));
    
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
    
  setup_day_countdown_bufs();
  setup_moon_countdown_bufs();
    
  setup_tiny_bufs();
    
  //if the time changed, refresh the window
  if(!strcmp(previous_time_buf, time_buf) || !strcmp(previous_date_buf, date_buf)) { 
    layer_mark_dirty(window_get_root_layer(window));
  }
  free(previous_time_buf);
  free(previous_date_buf);
    
  app_timer_register(1000, setup_time_and_date_callback, (void*) 0);
}

static void init(void) {
  config = malloc(sizeof(CONFIG));
  config->day_countdown = ASTRONOMICAL;
    
  force_display_tiny_bufs = true;
  next_tiny_display = TWILIGHT;
    
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
        //dump_to_log(data_buf);

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


