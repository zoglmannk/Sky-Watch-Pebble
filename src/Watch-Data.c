#include <pebble.h>
#include "Watch-Data.h"


/**
 converts the minute of day as an integer and stores the result
 in the TIME struct.
 */
void convert_int_to_time(int t, TIME *time) {
    time->hour = t/60;
    time->min  = t % 60;
}


/**
 converts the minute of day as an integer, converts it to a
 string, and stores the result in parameter c.
 */
char* convert_int_to_str(int t, char *c, int size_of_buf) {
    memset(c, 0, size_of_buf);
    TIME *time = malloc(sizeof(TIME));
    convert_int_to_time(t, time);
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
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise: %s", convert_int_to_str(data_buf->sun_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise azimuth: %d", data_buf->sun_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set: %s", convert_int_to_str(data_buf->sun_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set azimuth: %d", data_buf->sun_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "solar noon: %s", convert_int_to_str(data_buf->solar_noon, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "solar midnight: %s", convert_int_to_str(data_buf->solar_midnight, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour begin: %s", convert_int_to_str(data_buf->golden_hour_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour end: %s", convert_int_to_str(data_buf->golden_hour_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight begin: %s", convert_int_to_str(data_buf->civil_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight end: %s", convert_int_to_str(data_buf->civil_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight begin: %s", convert_int_to_str(data_buf->nautical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight end: %s", convert_int_to_str(data_buf->nautical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight begin: %s", convert_int_to_str(data_buf->astronomical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight end: %s", convert_int_to_str(data_buf->astronomical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise: %s", convert_int_to_str(data_buf->moon_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise azimuth: %d", data_buf->moon_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set: %s", convert_int_to_str(data_buf->moon_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set azimuth: %d", data_buf->moon_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon age: %d", data_buf->moon_age);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon percent illumination: %d", data_buf->moon_percent_illumination);
    
    free(c);
}


uint32_t get_data_storage_key(uint8_t slot) {
    
    uint32_t k = 13 << 16;
    k = k | slot;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "slot %d to storage_key %d", slot, (int) k);
    return k;
    
}


//for now assumes that the local time and stored calculations are in the
//same timezone.
SEARCH_RESULT* locate_data_for_current_date(SEARCH_RESULT* in) {
  time_t* clock = malloc(sizeof(time_t));
  time(clock); //update clock to current time
  struct tm* tm = localtime(clock);

  DATA *retrieved_yesterday = malloc(sizeof(DATA));
  DATA *retrieved_today = malloc(sizeof(DATA));
  DATA *retrieved_tomorrow = malloc(sizeof(DATA));

  DATA *found_yesterday = 0;
  DATA *found_today = 0;
  DATA *found_tomorrow = 0;
  for(int i=0; i<MAX_DATA_SLOTS-1; i++) {
    uint32_t storage_key_yesterday= get_data_storage_key(i);
    uint32_t storage_key_today    = get_data_storage_key(i+1);
    uint32_t storage_key_tomorrow = get_data_storage_key(i+2);

    if(persist_exists(storage_key_yesterday) && 
       persist_exists(storage_key_today) && 
       persist_exists(storage_key_tomorrow)) {

      memset(retrieved_today, 0, sizeof(DATA));
      persist_read_data(storage_key_today, retrieved_today, sizeof(DATA)); 

      if(retrieved_today->year == tm->tm_year+1900 &&
         retrieved_today->day_of_year == tm->tm_yday+1) {

        memset(retrieved_yesterday,0, sizeof(DATA));
        persist_read_data(storage_key_yesterday, retrieved_yesterday, sizeof(DATA));

        memset(retrieved_tomorrow, 0, sizeof(DATA));
        persist_read_data(storage_key_tomorrow, retrieved_tomorrow, sizeof(DATA)); 

        found_yesterday = retrieved_yesterday;
        found_today = retrieved_today;
        found_tomorrow = retrieved_tomorrow;
        break;
      }

    }

  }

  if(found_today) {
    memcpy(in->yesterday, found_yesterday, sizeof(DATA));
    memcpy(in->today, found_today, sizeof(DATA));
    memcpy(in->tomorrow, found_tomorrow, sizeof(DATA));
  }

  free(retrieved_yesterday);
  free(retrieved_today);
  free(retrieved_tomorrow);
  free(clock);

  if(found_today) {
    return in;
  } else {
    return 0;
  }

}
