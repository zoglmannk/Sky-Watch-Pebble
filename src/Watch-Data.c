#include <pebble.h>
#include "Watch-Data.h"


void convertIntToTime(int t, TIME *time) {
    time->hour = t/60;
    time->min  = t % 60;
}

char* convertIntToStr(int t, char *c, int size_of_buf) {
    memset(c, 0, size_of_buf);
    TIME *time = malloc(sizeof(TIME));
    convertIntToTime(t, time);
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
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise: %s", convertIntToStr(data_buf->sun_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun rise azimuth: %d", data_buf->sun_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set: %s", convertIntToStr(data_buf->sun_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "sun set azimuth: %d", data_buf->sun_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "solar noon: %s", convertIntToStr(data_buf->solar_noon, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "solar midnight: %s", convertIntToStr(data_buf->solar_midnight, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour begin: %s", convertIntToStr(data_buf->golden_hour_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "golden hour end: %s", convertIntToStr(data_buf->golden_hour_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight begin: %s", convertIntToStr(data_buf->civil_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "civil twilight end: %s", convertIntToStr(data_buf->civil_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight begin: %s", convertIntToStr(data_buf->nautical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "nautical twilight end: %s", convertIntToStr(data_buf->nautical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight begin: %s", convertIntToStr(data_buf->astronomical_twilight_begin, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "astronomical twilight end: %s", convertIntToStr(data_buf->astronomical_twilight_end, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise: %s", convertIntToStr(data_buf->moon_rise, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon rise azimuth: %d", data_buf->moon_rise_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set: %s", convertIntToStr(data_buf->moon_set, c, c_size));
    APP_LOG(APP_LOG_LEVEL_INFO, "moon set azimuth: %d", data_buf->moon_set_azimuth);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon age: %d", data_buf->moon_age);
    APP_LOG(APP_LOG_LEVEL_INFO, "moon percent illumination: %d", data_buf->moon_percent_illumination);
    
    free(c);
}


/**
 Max expected slot number is 128 which takes up 8 bits.
 Day key is 1101 or 13 in decimal shifted 16 bits to the left.
 
 For slot 0, the returned number in decimal is 851969.
 **/
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

  DATA *retrieved_today = malloc(sizeof(DATA));
  DATA *retrieved_tomorrow = malloc(sizeof(DATA));

  DATA *found_today = 0;
  DATA *found_tomorrow = 0;
  for(int i=0; i<MAX_DATA_SLOTS-1; i++) {
    uint32_t storage_key_today    = get_data_storage_key(i);
    uint32_t storage_key_tomorrow = get_data_storage_key(i+1);

    if(persist_exists(storage_key_today) && persist_exists(storage_key_tomorrow)) {
      memset(retrieved_today, 0, sizeof(DATA));
      persist_read_data(storage_key_today, retrieved_today, sizeof(DATA)); 

      if(retrieved_today->year == tm->tm_year+1900 &&
         retrieved_today->day_of_year == tm->tm_yday+1) {

        APP_LOG(APP_LOG_LEVEL_INFO, "found matching stored data with day of year: %d", retrieved_today->day_of_year);

        memset(retrieved_tomorrow, 0, sizeof(DATA));
        persist_read_data(storage_key_tomorrow, retrieved_tomorrow, sizeof(DATA)); 

        found_today = retrieved_today;
        found_tomorrow = retrieved_tomorrow;
        break;
      }

    }

  }

  if(found_today) {
    memcpy(in->today, found_today, sizeof(DATA));
    memcpy(in->tomorrow, found_tomorrow, sizeof(DATA));
  }

  free(retrieved_today);
  free(retrieved_tomorrow);
  free(clock);

  if(found_today) {
    return in;
  } else {
    return 0;
  }

}
