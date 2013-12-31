#include <pebble.h>
#include "Watch-Data.h"

static int percent_countdown_sun = -1;
static int percent_countdown_moon = -1;

typedef enum {HORIZON, CIVIL, NAUTICAL, ASTRONOMICAL} DAY_COUNT_DOWN;

typedef struct config {
    DAY_COUNT_DOWN day_countdown;
    bool notify_on_solar_noon;
    bool notify_on_solar_midnight;
    bool notify_on_sunrise;
    bool notify_on_sunset;
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
    int  of_total_mins;
} REMAINING;

static int min_of_day() {
    time_t* clock = malloc(sizeof(time_t));
    time(clock); //update clock to current time
    struct tm* tm = localtime(clock);
    
    free(clock);
 
    return 60*tm->tm_hour + tm->tm_min;
}

static void is_day(SEARCH_RESULT *based_on, REMAINING* in) {
    int yesterday_day_end = 0;
    int today_day_begin = 0;
    int today_day_end   = 0;
    int tomorrow_day_begin = 0;
    
    switch(config->day_countdown) {
        case HORIZON:
            //yesterday_day_end = based_on->yesterday->sun_set;
            today_day_begin = based_on->today->sun_rise;
            today_day_end   = based_on->today->sun_set;
            tomorrow_day_begin = based_on->tomorrow->sun_rise;
            break;
        case CIVIL:
            //yesterday_day_end = based_on->yesterday->civil_twilight_end;
            today_day_begin = based_on->today->civil_twilight_begin;
            today_day_end   = based_on->today->civil_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->civil_twilight_begin;
            break;
        case NAUTICAL:
            //yesterday_day_end = based_on->yesterday->nautical_twilight_end;
            today_day_begin = based_on->today->nautical_twilight_begin;
            today_day_end   = based_on->today->nautical_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->nautical_twilight_begin;
            break;
        case ASTRONOMICAL:
            //yesterday_day_end = based_on->yesterday->astronomical_twilight_end;
            today_day_begin = based_on->today->astronomical_twilight_begin;
            today_day_end   = based_on->today->astronomical_twilight_end;
            tomorrow_day_begin = based_on->tomorrow->astronomical_twilight_begin;
    }

    int minute_of_day = min_of_day();
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "is_day basedon day_begin: %d and day_end: %d", today_day_begin, today_day_end);
    //APP_LOG(APP_LOG_LEVEL_INFO, "is_day basedon current_min_of_day: %d ", minute_of_day);

    if(minute_of_day >= today_day_begin && minute_of_day <  today_day_end) {
        in->is_object_up = true;
        in->mins = today_day_end - minute_of_day;
        in->of_total_mins = today_day_end - today_day_begin;
    } else if (minute_of_day < today_day_begin) {
        in->is_object_up = false;
        in->mins = today_day_begin - minute_of_day;
        in->of_total_mins = 24*60 - yesterday_day_end + today_day_begin;
    } else {
        in->is_object_up = false;
        in->mins = 24*60 - minute_of_day + tomorrow_day_begin;
        in->of_total_mins = 24*60 - today_day_end + tomorrow_day_begin;
    }
    
}

static void moon_remaining(SEARCH_RESULT *based_on, REMAINING *in) {
    int minute_of_day = min_of_day();
    
    int last_yesterday_event = based_on->yesterday->moon_rise;
    if(based_on->yesterday->moon_set > last_yesterday_event) {
        last_yesterday_event = based_on->yesterday->moon_set;
    }
    
    int last_today_event = based_on->today->moon_rise;
    if(based_on->today->moon_set > last_today_event) {
        last_today_event = based_on->today->moon_set;
    }

    if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_set) {
        //upcoming moon set and countdown = set - time
        in->is_object_up = true;
        in->mins = based_on->today->moon_set - minute_of_day;
        in->of_total_mins = 24*60 - based_on->today->moon_set + last_yesterday_event;
        
    } else if (based_on->today->moon_set < based_on->today->moon_rise && minute_of_day < based_on->today->moon_rise) {
        //upcoming moon rise and countdown = rise - time
        in->is_object_up = false;
        in->mins = based_on->today->moon_rise - minute_of_day;
        in->of_total_mins = based_on->today->moon_rise - based_on->today->moon_set;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_rise) {
        //upcoming moon rise and countdown = rise - time
        in->is_object_up = false;
        in->mins = based_on->today->moon_rise - minute_of_day;
        in->of_total_mins = 24*60 - based_on->today->moon_rise + last_yesterday_event;
        
    } else if (based_on->today->moon_rise < based_on->today->moon_set && minute_of_day < based_on->today->moon_set) {
        //upcoming moon set and countdown = set - time
        in->is_object_up = true;
        in->mins = based_on->today->moon_set - minute_of_day;
        in->of_total_mins = based_on->today->moon_set - based_on->today->moon_rise;
        
    } else if (based_on->tomorrow->moon_set < based_on->tomorrow->moon_rise) {
        //upcoming moon set and countdown = 24hr - time + next set
        in->is_object_up = true;
        in->mins = 24*60 - minute_of_day + based_on->tomorrow->moon_set;
        in->of_total_mins = 24*60 - last_today_event + based_on->tomorrow->moon_set;
        
    } else {
        //upcoming moon rise and countdown = 24hr - time + next rise
        in->is_object_up = false;
        in->mins = 24*60 - minute_of_day + based_on->tomorrow->moon_rise;
        in->of_total_mins = 24*60 - last_today_event + based_on->tomorrow->moon_rise;
    }

}

typedef struct event {
    bool is_object_rising;
    bool is_today;
    int  minute_of_day;
} EVENT;

static void next_two_moon_events(SEARCH_RESULT *based_on, EVENT (*in)[1]) {
    EVENT *first_event = in[0];
    EVENT *second_event = in[1];
    
    int minute_of_day = min_of_day();
    
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
static TextLayer *notification_layer = 0;


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

static SEARCH_RESULT* create_search_result(void) {
    DATA *yesterday_data = malloc(sizeof(DATA));
    DATA *today_data = malloc(sizeof(DATA));
    DATA *tomorrow_data = malloc(sizeof(DATA));
    SEARCH_RESULT *result = malloc(sizeof(SEARCH_RESULT));
    
    result->yesterday = yesterday_data;
    result->today = today_data;
    result->tomorrow = tomorrow_data;
    
    return result;
}

static void destroy_search_result(SEARCH_RESULT *result) {
    free(result->yesterday);
    free(result->today);
    free(result->tomorrow);
    free(result);
}

static void setup_day_countdown_bufs(void) {
    SEARCH_RESULT *result = create_search_result();
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    

    if(ret_result == 0) {
        percent_countdown_sun = -1;
        snprintf(line_1_buf, BUFFER_SIZE, "--NO DATA--");
        snprintf(line_2_buf, BUFFER_SIZE, " ");
        setup_color_scheme(true);
    } else {
        REMAINING *remaining = malloc(sizeof(REMAINING));
        is_day(result, remaining);
        
        percent_countdown_sun = 100 - (remaining->mins * 100 / remaining->of_total_mins);
        
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
    
    destroy_search_result(result);
}

static void setup_moon_countdown_bufs(void) {
    SEARCH_RESULT *result = create_search_result();
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    

    if(ret_result == 0) {
        percent_countdown_moon = -1;
        snprintf(line_3_buf, BUFFER_SIZE, "--NO DATA--");
        snprintf(line_4_buf, BUFFER_SIZE, " ");
    } else {
        REMAINING *remaining = malloc(sizeof(REMAINING));
        moon_remaining(result, remaining);
        
        percent_countdown_moon = 100 - (remaining->mins * 100 / remaining->of_total_mins);
        //APP_LOG(APP_LOG_LEVEL_INFO, "setup_moon_countdown mins %d: total_mins: %d percent %d", remaining->mins, remaining->of_total_mins, percent_countdown_moon);
        
        if(remaining->is_object_up) {
            snprintf(line_3_buf, BUFFER_SIZE, "Moon Set");
        } else {
            snprintf(line_3_buf, BUFFER_SIZE, "Moon Rise");
        }
        
        countdown_mins_to_char(remaining->mins, line_4_buf, BUFFER_SIZE);
        
        free(remaining);
    }
    
    destroy_search_result(result);
}


static void min_of_day_to_char(int minute_of_day, char* buf, int buffer_size) {
    memset(buf, 0, buffer_size);
    
    int hour = minute_of_day / 60;
    int min =  minute_of_day % 60;
    
    char* am_pm = "a";
    if(hour > 12) {
        hour -= 12;
        am_pm = "p";
    } else if (hour == 0) {
        hour = 12;
    }
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "min_of_day_to_char called with min_of_day: %d am_pm: NA", minute_of_day);
    
    snprintf(buf, buffer_size, "%d:%02d%s", hour, min, am_pm);
}


static void setup_no_data_tiny_bufs(void) {
    snprintf(line_5_buf, BUFFER_SIZE, "Please use Sky Watch");
    snprintf(line_6_buf, BUFFER_SIZE, "iPhone app to push data.");
}

static void setup_choosen_twilight_tiny_bufs(void) {
    SEARCH_RESULT *result = create_search_result();
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
    
    destroy_search_result(result);
}

/** Set this up to get called instead of setup_moon_tiny_bufs **/
static void setup_sunrise_sunset_tiny_bufs(void) {
    SEARCH_RESULT *result = create_search_result();
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
    
    destroy_search_result(result);
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
    SEARCH_RESULT *result = create_search_result();
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
    
    destroy_search_result(result);
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

static void draw_bar_graph(Layer *layer, GContext* ctx, int percent_complete) {
    
    if(current_background_color == GColorBlack) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_fill_color(ctx, GColorBlack);
    }
    
    graphics_draw_rect(ctx, (GRect) { .origin = { 3, 0 }, .size = { 5, 42 } });
    
    graphics_draw_line(ctx, (GPoint) { 2,  0 }, (GPoint) {  2,  0 } ); //100% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8,  0 }, (GPoint) {  8,  0 } ); //100% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 0, 11 }, (GPoint) {  2, 11 } ); //75% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, 11 }, (GPoint) { 10, 11 } ); //75% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 0, 21 }, (GPoint) {  2, 21 } ); //50% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, 21 }, (GPoint) { 10, 21 } ); //50% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 0, 32 }, (GPoint) {  2, 32 } ); //25% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, 32 }, (GPoint) { 10, 32 } ); //25% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 2, 41 }, (GPoint) {  2, 41 } ); //0% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, 41 }, (GPoint) {  8, 41 } ); //0% mark right of bar
    
    int height = 42 * percent_complete / 100;
    
    graphics_fill_rect(ctx, (GRect) { .origin = { 3, 42 - height}, .size = { 5, height } }, 0, 0);
}

static void draw_battery_graph(Layer *layer, GContext* ctx) {
    
    BatteryChargeState battery_state = battery_state_service_peek();
    
    if(current_background_color == GColorBlack) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_fill_color(ctx, GColorBlack);
    }
    
    int y_offset = 9;

    //draw plus
    graphics_draw_line(ctx, (GPoint) { 4, 2 }, (GPoint) { 6, 2 } );
    graphics_draw_line(ctx, (GPoint) { 5, 0 }, (GPoint) { 5, 4 } );
    
    graphics_draw_rect(ctx, (GRect) { .origin = { 4, y_offset-2 }, .size = { 3,  2 } }); //draw top of battery button
    graphics_draw_rect(ctx, (GRect) { .origin = { 3, y_offset+0 }, .size = { 5, 20 } }); //draw battery outline
    
    graphics_draw_line(ctx, (GPoint) { 1, y_offset +5 }, (GPoint) {  2, y_offset +5 } ); //75% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, y_offset +5 }, (GPoint) {  9, y_offset +5 } ); //75% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 1, y_offset+10 }, (GPoint) {  2, y_offset+10 } ); //50% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, y_offset+10 }, (GPoint) {  9, y_offset+10 } ); //50% mark right of bar
    
    graphics_draw_line(ctx, (GPoint) { 1, y_offset+15 }, (GPoint) {  2, y_offset+15 } ); //25% mark left of bar
    graphics_draw_line(ctx, (GPoint) { 8, y_offset+15 }, (GPoint) {  9, y_offset+15 } ); //25% mark right of bar
    
    int height = 21 * battery_state.charge_percent / 100;
    graphics_fill_rect(ctx, (GRect) { .origin = { 3, y_offset + 21 - height}, .size = { 5, height } }, 0, 0);
    
    //graphics_draw_line(ctx, (GPoint) { 4, y_offset + 21 + 4 }, (GPoint) { 6, y_offset + 21 + 4 } ); //draw minus
    
}

static void draw_bar_graph_for_sun(Layer *layer, GContext *ctx) {
    
    if(percent_countdown_sun >= 0) {
        draw_bar_graph(layer, ctx, percent_countdown_sun);
    }
    
}


static void draw_bar_graph_for_moon(Layer *layer, GContext *ctx) {
    
    if(percent_countdown_moon >= 0) {
        draw_bar_graph(layer, ctx, percent_countdown_moon);
    }
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


static BitmapLayer *moon_image_layer = (void*) 0;
static GBitmap *moon_image = (void*) 0;

//WARNING: something should be done to make this more accurate. The Age of Moon is not that accurate.
//it can skip days!!
static void setup_moon_image_layer() {
    SEARCH_RESULT *result = create_search_result();
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    
    if(ret_result != 0) {
        REMAINING *remaining = malloc(sizeof(REMAINING));
        
        if(moon_image != 0 ) {
            gbitmap_destroy(moon_image);
            moon_image = 0;
        }
        
        is_day(result, remaining);
        
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "set image to waning_half_white moon age: %d", result->today->moon_age);
        
        switch (result->today->moon_age) {
            case 1:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_NEW_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_NEW_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 1");
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_CRESCENT_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_CRESCENT_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 2");
                break;
            case 6:
            case 7:
            case 8:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_FIRST_QUARTER_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_FIRST_QUARTER_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 3");
                break;
            case 9:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_HALF_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_HALF_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 4");
                break;
            case 10:
            case 11:
            case 12:
            case 13:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_GIBBOUS_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WAXING_GIBBOUS_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 5");
                break;
            case 14:
            case 15:
            case 16:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_FULL_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_FULL_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 6");
                break;
            case 17:
            case 18:
            case 19:
            case 20:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_GIBBOUS_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_GIBBOUS_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 7");
                break;
            case 21:
            case 22:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_HALF_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_HALF_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 8");
                break;
            case 23:
            case 24:
            case 25:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_LAST_QUARTER_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_LAST_QUARTER_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 9");
                break;
            case 26:
            case 27:
            case 28:
            case 29:
                if(remaining->is_object_up) {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_CRESCENT_WHITE);
                } else {
                    moon_image = gbitmap_create_with_resource(RESOURCE_ID_MOON_WANING_CRESCENT_BLACK);
                }
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "case 10");
                break;
        }
        
        
        bitmap_layer_set_bitmap(moon_image_layer, moon_image);
        free(remaining);
    }
    
    destroy_search_result(result);
    
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
    
  //setup bar graph for light/dark countdown
  Layer *light_dark_bar_layer = layer_create((GRect) { .origin = { 2, 5 }, .size = { 12, 48-5} });
  layer_set_update_proc(light_dark_bar_layer, draw_bar_graph_for_sun);
  layer_add_child(window_layer, light_dark_bar_layer);
    
  //setup bar graph for moon rise/set countdown
  Layer *moon_rise_set_layer = layer_create((GRect) { .origin = { 2, 5+24+24+3 }, .size = { 12, 48-5} });
  layer_set_update_proc(moon_rise_set_layer, draw_bar_graph_for_moon);
  layer_add_child(window_layer, moon_rise_set_layer);
    
    
    //setup moon phase image layer
    moon_image_layer = bitmap_layer_create((GRect) { .origin = { 144-21, 5+24+24+13}, .size = {20, 20} });
    bitmap_layer_set_alignment(moon_image_layer, GAlignCenter);
    layer_add_child(window_layer, bitmap_layer_get_layer(moon_image_layer));
    

  //set 5th line of text
  text_layer5 = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30}, .size = { bounds.size.w, 20} });
  text_layer_set_font(text_layer5, fonts_get_system_font("RESOURCE_ID_GOTHIC_14"));
  text_layer_set_background_color(text_layer5, GColorBlack);
  text_layer_set_text_color(text_layer5, GColorWhite);
  memset(line_5_buf, 0, BUFFER_SIZE);
  text_layer_set_text(text_layer5, line_5_buf);
  text_layer_set_text_alignment(text_layer5, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer5));
    
  //setup creation of line visible on black background
  Layer *line_layer2 = layer_create((GRect) { .origin = { 0, 0+24+24+24+30}, .size = {bounds.size.w, 2} });
  layer_set_update_proc(line_layer2, draw_line_callback2);
  layer_add_child(window_layer, line_layer2);

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
    

    //setup battery graph
    Layer *battery_layer = layer_create((GRect) { .origin = { bounds.size.w - 15, 10 }, .size = { 15, 60 } });
    layer_set_update_proc(battery_layer, draw_battery_graph);
    layer_add_child(window_layer, battery_layer);
    
    
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
  text_layer_destroy(notification_layer);
  bitmap_layer_destroy(moon_image_layer);
    
  if(moon_image != 0) {
    gbitmap_destroy(moon_image);
    moon_image = (void*) 0;
  }
     
}

typedef struct notification {
    TIME *time;
    char *message;
    int vibration_count;
    GColor current_background_color;
} NOTIFICATION;

NOTIFICATION *next_notification;

/**
 message is copied
 **/
static NOTIFICATION* create_notification(int buffer_size) {
    NOTIFICATION *ret = malloc(sizeof(NOTIFICATION));
    memset(ret, 0, sizeof(NOTIFICATION));
    
    ret->time = malloc(sizeof(TIME));
    memset(ret->time, 0, sizeof(TIME));
    
    ret->message = malloc(sizeof(char)*buffer_size);
    memset(ret->message, 0, sizeof(char)*buffer_size);
    
    ret->current_background_color = GColorBlack;
    
    return ret;
}

static void destroy_notification(NOTIFICATION *notification) {
    if(notification == 0) {
        return;
    }
    
    free(notification->time);
    free(notification->message);
    free(notification);
}

static void wink_for_notification(NOTIFICATION *notification) {
    int minute_of_day = min_of_day();
    int next_notification_min_of_day = notification->time->hour*60 + notification->time->min;
    
    if(next_notification_min_of_day == minute_of_day) {
        
        if(notification_layer == 0) {
            //setup notification text -- covers up rotating lines of text
            Layer *window_layer = window_get_root_layer(window);
            GRect bounds = layer_get_bounds(window_layer);
            notification_layer = text_layer_create((GRect) { .origin = { 0, 0+24+24+24+30 }, .size = { bounds.size.w, 36 } });
            text_layer_set_font(notification_layer, fonts_get_system_font("RESOURCE_ID_GOTHIC_24_BOLD"));
            text_layer_set_background_color(notification_layer, GColorBlack);
            text_layer_set_text_color(notification_layer, GColorWhite);
            text_layer_set_text(notification_layer, next_notification->message);
            text_layer_set_text_alignment(notification_layer, GTextAlignmentCenter);
            layer_add_child(window_layer, text_layer_get_layer(notification_layer));
        }
    
        if(notification->current_background_color == GColorWhite) {
            notification->current_background_color = GColorBlack;
            
            text_layer_set_background_color(notification_layer, GColorBlack);
            text_layer_set_text_color(notification_layer, GColorWhite);
            
        } else {
            notification->current_background_color = GColorWhite;
            
            text_layer_set_background_color(notification_layer, GColorWhite);
            text_layer_set_text_color(notification_layer, GColorBlack);
        }
    
    } else if (minute_of_day > next_notification_min_of_day && notification_layer!=0) {
        text_layer_destroy(notification_layer);
        notification_layer = 0;
    }
    
}


static void vibrate_for_notification(NOTIFICATION *notification) {
    int minute_of_day = min_of_day();
    int next_notification_min_of_day = notification->time->hour*60 + notification->time->min;
    
    if(minute_of_day == next_notification_min_of_day) {

        if(notification->vibration_count % 3 == 0 && notification->vibration_count < 6) {
            static const uint32_t const segments[] = {
                50, 50, 100,
                50, 50, 100,
                50, 50, 100,
                50, 50, 100,
            
                50, 50, 500,
            
                25, 25, 25,
                25, 25, 25,
                25, 25, 25,
                25, 25, 25,
                25, 25, 25,
                25, 25, 25,
                25, 25, 25,
            };
            VibePattern vibration_pattern = {
                .durations = segments,
                .num_segments = ARRAY_LENGTH(segments),
            };
        
            vibes_enqueue_custom_pattern(vibration_pattern);
        
        }
    
        notification->vibration_count++;
        
    }
    
}

static bool test_and_update_notification(NOTIFICATION *working_notification, int event_minute_of_day, const char* message) {
    int minute_of_day = min_of_day();
    int working_min_of_day = working_notification->time->hour*60 + working_notification->time->min;
    
    if(event_minute_of_day > minute_of_day && event_minute_of_day < working_min_of_day) {
        working_notification->time->hour = event_minute_of_day/60;
        working_notification->time->min  = event_minute_of_day%60;
        
        memset(working_notification->message, 0, BUFFER_SIZE);
        snprintf(working_notification->message, BUFFER_SIZE, message);
        
        return true;
    }
    
    return false;
    
}

static void setup_and_handle_notifications() {
    SEARCH_RESULT *result = create_search_result();
    SEARCH_RESULT *ret_result = locate_data_for_current_date(result);
    
    if(ret_result == 0) {
        return;
    }
    
    NOTIFICATION* working_notification = create_notification(BUFFER_SIZE);
    working_notification->time->hour = 23;
    working_notification->time->min  = 59;
    bool found_notification = false;
    
    if(config->notify_on_solar_noon) {
        found_notification = found_notification || test_and_update_notification(working_notification, result->today->solar_noon, "Solar Noon");
    }
    
    if(config->notify_on_solar_midnight) {
        found_notification = found_notification || test_and_update_notification(working_notification, result->today->solar_midnight, "Solar Midnight");
    }
    
    if(config->notify_on_sunrise) {
        found_notification = found_notification || test_and_update_notification(working_notification, result->today->sun_rise, "Sunrise");
    }
    
    if(config->notify_on_sunset) {
        found_notification = found_notification || test_and_update_notification(working_notification, result->today->sun_set, "Sunset");
    }
    
    int test_mins = (8+12)*60 + 30;
    found_notification = found_notification || test_and_update_notification(working_notification, test_mins, "Test Notification");
    
    if(found_notification) {
        if(next_notification != 0) {
            destroy_notification(next_notification);
        }
        
        next_notification = working_notification;
    }
    
    if(next_notification!=0) {
        vibrate_for_notification(next_notification);
        wink_for_notification(next_notification);
    }
    
    
    destroy_search_result(result);

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
    
  setup_moon_image_layer();
    
  //setup_and_handle_notifications();
    
  //if the time changed, refresh the window
  if(!strcmp(previous_time_buf, time_buf) || !strcmp(previous_date_buf, date_buf)) { 
    layer_mark_dirty(window_get_root_layer(window));
  }
  free(previous_time_buf);
  free(previous_date_buf);
    
  app_timer_register(1000, setup_time_and_date_callback, (void*) 0);
}

static void init(void) {
  next_notification = 0;
    
  config = malloc(sizeof(CONFIG));
  config->day_countdown = ASTRONOMICAL;
  config->notify_on_solar_noon = true;
  config->notify_on_solar_midnight = true;
  config->notify_on_sunrise = true;
  config->notify_on_sunset = true;
    
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

  if(next_notification != 0) {
    destroy_notification(next_notification);
    next_notification = 0;
  }

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


