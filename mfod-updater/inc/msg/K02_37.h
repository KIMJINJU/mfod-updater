#ifndef _K02_37_H_
#define _K02_37_H_

/*
    Message No : K02.37
    Message Title : OBSERVER READINESS REPORT
    Message Purpose : TO ALLOW A FORWARD OBSERVER, FIRE SUPPORT TEAM, OR RADAR TO REPORT
                      A LOCATION INDICATING THAT THE OBSERVER/SENSOR IS READY TO PERFORM A MISSION.
*/

#pragma pack(push,1)
typedef struct K02_37
{
    unsigned int URN; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
    unsigned char GPI_1; // Bit_Size : 1 // Valid_Range : 0~1

    struct GPI_GROUP_1_K02_37
    {
        int REAL_GRI_1_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~7 // Max_Repeat : 7

        struct GRI_GROUP_1_K02_37
        {
            unsigned int OBSERVER_LOCATION_LATITUDE; // Bit_Size : 25 // Valid_Range : 0~33554431
            unsigned int OBSERVER_LOCATION_LONGITUDE; // Bit_Size : 26 // Valid_Range : 0~67108863
            unsigned char FPI_1; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned int OBSERVER_LOCATION_ELEVATION__MSL; // Bit_Size : 17 // Valid_Range : 0~65535,129752~131071
            unsigned char FPI_2; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned int OBSERVER_LOCATION_ELEVATION__HAE; // Bit_Size : 22 // Valid_Range : 0~1280000,2097152,4130304~4194303
            unsigned char FPI_3; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short ARC_AZIMUTH_LEFT_LIMIT; // Bit_Size : 13 // Valid_Range : 0~6399
            unsigned char FPI_4; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short ARC_AZIMUTH_RIGHT_LIMIT; // Bit_Size : 13 // Valid_Range : 0~6399
            unsigned char FPI_5; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char VISIBILITY; // Bit_Size : 7 // Valid_Range : 0~127
            unsigned char FPI_6; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short ALTITUDE__25_FT; // Bit_Size : 13 // Valid_Range : 0~8191

            unsigned char GPI_2; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_2_K02_37
            {
                unsigned char OBSERVER_LASER_VISIBILITY; // Bit_Size : 2 // Valid_Range : 0~3
                unsigned short CLOUD_BASE_ALTITUDE__FEET; // Bit_Size : 9 // Valid_Range : 0~511

                unsigned char GPI_3; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_3_K02_37
                {
                    unsigned char DESIGNATOR_AND_SEEKER_PULSE_CODE__FIRST_DIGIT; // Bit_Size : 3 // Valid_Range : 0~4
                    unsigned char DESIGNATOR_AND_SEEKER_PULSE_CODE__SECOND_DIGIT; // Bit_Size : 3 // Valid_Range : 0~7
                    unsigned char DESIGNATOR_AND_SEEKER_PULSE_CODE__THIRD_DIGIT; // Bit_Size : 3 // Valid_Range : 0~7
                    unsigned char DESIGNATOR_AND_SEEKER_PULSE_CODE__FOURTH_DIGIT; // Bit_Size : 3 // Valid_Range : 0~7
                } GPI_3_GROUP;
            } GPI_2_GROUP;

            unsigned char GPI_4; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_4_K02_37
            {
                unsigned char FPI_7; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_YEAR; // Bit_Size : 7 // Valid_Range : 0~99
                unsigned char FPI_8; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_MONTH; // Bit_Size : 4 // Valid_Range : 1~12
                unsigned char EFFECTIVE_DAY; // Bit_Size : 5 // Valid_Range : 1~31
                unsigned char EFFECTIVE_HOUR; // Bit_Size : 5 // Valid_Range : 0~23
                unsigned char EFFECTIVE_MINUTE; // Bit_Size : 6 // Valid_Range : 0~59
                unsigned char FPI_9; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
            } GPI_4_GROUP;
            unsigned char FPI_10; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char COMMENTS[200]; // Bit_Size : 1400 // Valid_Character : 9~10,13,32~127
        } GRI_1_GROUP[7];
    } GPI_1_GROUP;
} K02_37;

#pragma pack(pop)

#endif /* _K02_37_H_ */
