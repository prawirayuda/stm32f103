
#include "julian_date_util.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>


static unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

unsigned int date_time_to_epoch(date_time_t* date_time)
{
    unsigned int second = date_time->second;  // 0-59
    unsigned int minute = date_time->minute;  // 0-59
    unsigned int hour   = date_time->hour;    // 0-23
    unsigned int day    = date_time->day-1;   // 0-30
    unsigned int month  = date_time->month-1; // 0-11
    unsigned int year   = date_time->year;    // 0-99
    return (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
}


void epoch_to_date_time(date_time_t* date_time,unsigned int epoch)
{
    date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    unsigned int years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    unsigned int year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    unsigned int month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->day   = epoch-days[year][month]+1;
}

// Offset from GMT+00
void offset_datetime(date_time_t *date_time, int offset_gmt){
    unsigned int epoch = date_time_to_epoch(date_time);
    date_time_t tmp_date_time;
    epoch += offset_gmt * 60 * 60;
    epoch_to_date_time(&tmp_date_time, epoch);
    memcpy(date_time, &tmp_date_time, sizeof(date_time_t));
}

void offset_datetime_day(date_time_t *date_time, int day){
    unsigned int epoch = date_time_to_epoch(date_time);
    date_time_t tmp_date_time;
    epoch += day * 24 * 60 * 60;
    epoch_to_date_time(&tmp_date_time, epoch);
    memcpy(date_time, &tmp_date_time, sizeof(date_time_t));
}
