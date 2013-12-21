#define MAX_DATA_SLOTS  28

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


typedef struct search_result {
  DATA* today;
  DATA* tomorrow;
} SEARCH_RESULT;


/**
 calls APP_LOG to log all of the information contained in the 
 data_buf
 */
void dump_to_log(DATA *data_buf);

/**
 Max expected slot number is 128 which takes up 8 bits.
 Day key is 1101 or 13 in decimal shifted 16 bits to the left.

 For slot 0, the returned number in decimal is 851969.
 **/
uint32_t get_data_storage_key(uint8_t slot);

/**
 @return 0 when not found 
 */
SEARCH_RESULT* locate_data_for_current_date(SEARCH_RESULT* in);
