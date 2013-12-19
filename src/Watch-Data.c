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
DATA* locate_data_for_current_date(DATA* in) {
  time_t* clock = malloc(sizeof(time_t));
  time(clock); //update clock to current time
  struct tm* tm = localtime(clock);

  DATA *retrieved_data = malloc(sizeof(DATA));

  DATA *found = 0;
  for(int i=0; i<MAX_DATA_SLOTS; i++) {
    uint32_t storage_key = get_data_storage_key(i);

    if(persist_exists(storage_key)) {
      memset(retrieved_data, 0, sizeof(DATA));
      persist_read_data(storage_key, retrieved_data, sizeof(DATA)); 

      if(retrieved_data->year == tm->tm_year+1900 &&
         retrieved_data->day_of_year == tm->tm_yday+1) {

        APP_LOG(APP_LOG_LEVEL_INFO, "found matching stored data with day of year: %d", retrieved_data->day_of_year);
        found = retrieved_data;
        break;
      }

    }

  }

  if(found) {
    memcpy(in, found, sizeof(DATA));
  }

  free(retrieved_data);
  free(clock);

  if(found) {
    return in;
  } else {
    return 0;
  }

}
