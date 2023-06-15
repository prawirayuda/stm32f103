typedef struct
{
    unsigned char second; // 0-59
    unsigned char minute; // 0-59
    unsigned char hour;   // 0-23
    unsigned char day;    // 1-31
    unsigned char month;  // 1-12
    unsigned char year;   // 0-99 (representing 2000-2099)
}date_time_t;

void epoch_to_date_time(date_time_t* date_time,unsigned int epoch);
unsigned int date_time_to_epoch(date_time_t* date_time);
void offset_datetime(date_time_t *date_time, int offset_gmt);
void offset_datetime_day(date_time_t *date_time, int day);
