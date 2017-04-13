#ifndef _K02_9_H_
#define _K02_9_H_


/*
    Message No : K02.9
    Message Title : TARGET DATA
    Message Purpose : TO REPORT TARGET INFORMATION, ENHANCED COUNTERFIRE PROCESSING INFORMATION,
                      OR TO TRANSMIT TARGET INFORMATION TO A FIRE SUPPORT AGENCY IN RESPONSE TO A ONE TIME QUERY
                      OR A STANDING REQUEST FOR INFORMATION (SRI)
*/

 #pragma pack(push,1)
typedef struct K02_9
{
    unsigned char TARGET_REPORT_TYPE; // Bit_Size : 3 // Valid_Range : 0~7
    unsigned char ACTION_DESIGNATOR; // Bit_Size : 2 // Valid_Range : 0~3
    unsigned char FPI_1; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned short NUMBER_OF_TARGETS_MEETING_CRITERIA; // Bit_Size : 11 // Valid_Range : 0~2047

    unsigned char GPI_1; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_1_K02_9
    {

        int REAL_GRI_1_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~20 // Max_Repeat : 20

        struct GRI_GROUP_1_K02_9
        {
            unsigned char FPI_2; // Bit_Size : 1 // Valid_Range : 0~1
            // TARGET_NUMBER EXCEPTION HANDLING ===============>
            unsigned char TARGET_NUMBER_ASC[2]; // Bit_Size : 14 // Valid_Character : 65~90
            unsigned short TARGET_NUMBER_DEC; // Bit_Size : 14 // Valid_Range : 0~9999
            // <================================================
            unsigned char FPI_3; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char TARGET_REPORT_DESIGNATOR; // Bit_Size : 4 // Valid_Range : 0~10

            unsigned char GPI_2; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_2_K02_9
            {

                unsigned char GPI_3; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_3_K02_9
                {
                    unsigned int PREDICTED_IMPACT_LATITUDE; // Bit_Size : 25 // Valid_Range : 0~33554431
                    unsigned int PREDICTED_IMPACT_LONGITUDE; // Bit_Size : 26 // Valid_Range : 0~67108863
                } GPI_3_GROUP;

                unsigned char GPI_4; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_4_K02_9
                {

                    int REAL_GRI_2_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~15 // Max_Repeat : 15

                    struct GRI_GROUP_2
                    {
                        unsigned char FPI_4; // Bit_Size : 1 // Valid_Range : 0~1
                        unsigned char TARGET_NUMBER_EXTENSION; // Bit_Size : 4 // Valid_Range : 1~15
                        unsigned int TARGET_LATITUDE; // Bit_Size : 25 // Valid_Range : 0~33554431
                        unsigned int TARGET_LONGITUDE; // Bit_Size : 26 // Valid_Range : 0~67108863
                        unsigned char FPI_5; // Bit_Size : 1 // Valid_Range : 0~1
                        unsigned int TARGET_ELEVATION__MSL; // Bit_Size : 17 // Valid_Range : 0~65535,129752~131071
                        unsigned char FPI_6; // Bit_Size : 1 // Valid_Range : 0~1
                        unsigned int TARGET_ELEVATION__HAE; // Bit_Size : 22 // Valid_Range : 0~1280000,2097152,4130304~4194303

                        unsigned char GPI_5; // Bit_Size : 1 // Valid_Range : 0~1
                        struct GPI_GROUP_5_K02_9
                        {
                            unsigned char TARGET_LOCATION_STATUS_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char FPI_7; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char TARGET_AIR_DEFENSES; // Bit_Size : 3 // Valid_Range : 0~4
                            unsigned char FPI_8; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char TARGET_ACQUISITION_SOURCE_TYPE; // Bit_Size : 5 // Valid_Range : 0~29
                            unsigned char FPI_9; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char RELIABILITY_EVALUATION; // Bit_Size : 3 // Valid_Range : 0~5
                            unsigned char FPI_10; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned short TARGET_LOCATION_ACCURACY; // Bit_Size : 10 // Valid_Range : 0~1023
                            unsigned char FPI_11; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char ACCURACY_EVALUATION; // Bit_Size : 3 // Valid_Range : 0~5
                        } GPI_5_GROUP;

                        unsigned char GPI_6; // Bit_Size : 1 // Valid_Range : 0~1
                        struct GPI_GROUP_6_K02_9
                        {
                            unsigned char TARGET_GENERIC_TYPE; // Bit_Size : 5 // Valid_Range : 0~22

                            int REAL_FRI_1_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

                            struct FRI1_K02_9
                            {
                                unsigned char TARGET_SUBTYPE; // Bit_Size : 8 // Valid_Range : 0~137
                            } FRI_1[2];
                            unsigned char FPI_12; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char TARGET_VALUE_TYPE; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char FPI_13; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char ACTIVITY; // Bit_Size : 6 // Valid_Range : 0~53
                            unsigned char FPI_14; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char DEGREE_OF_PROTECTION; // Bit_Size : 4 // Valid_Range : 0~10
                            unsigned char FPI_15; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char TARGET_FORMATION; // Bit_Size : 2 // Valid_Range : 0~3
                            unsigned char FPI_16; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned short NUMBER_OF_TARGET_ELEMENTS; // Bit_Size : 14 // Valid_Range : 0~16383
                            unsigned char FPI_17; // Bit_Size : 1 // Valid_Range : 0~1
                            unsigned char TARGET_SHAPE; // Bit_Size : 3 // Valid_Range : 0~4

                            unsigned char GPI_7; // Bit_Size : 1 // Valid_Range : 0~1
                            struct GPI_GROUP_7_K02_9
                            {

                                int REAL_GRI_3_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

                                struct GRI_GROUP_3_K02_9
                                {
                                    unsigned char TARGET_SUBTYPE_ELEMENT; // Bit_Size : 7 // Valid_Range : 0~82
                                    unsigned short NUMBER_OF_TARGET_SUBTYPE_ELEMENTS; // Bit_Size : 10 // Valid_Range : 1~1023
                                } GRI_3_GROUP[3];

                            } GPI_7_GROUP;
                        } GPI_6_GROUP;

                        unsigned char GPI_8; // Bit_Size : 1 // Valid_Range : 0~1
                        struct GPI_GROUP_8_K02_9
                        {
                            unsigned char MOVING_TARGET_SIGHTING_HOUR; // Bit_Size : 5 // Valid_Range : 0~23
                            unsigned char MOVING_TARGET_SIGHTING_MINUTE; // Bit_Size : 6 // Valid_Range : 0~59
                            unsigned char MOVING_TARGET_SIGHTING_SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
                        } GPI_8_GROUP;
                    } GRI_2_GROUP[15];

                } GPI_4_GROUP;
            } GPI_2_GROUP;

            unsigned char GPI_9; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_9_K02_9
            {
                unsigned short MOVING_TARGET_AZIMUTH; // Bit_Size : 13 // Valid_Range : 0~6399
                unsigned char MOVING_TARGET_SPEED; // Bit_Size : 7 // Valid_Range : 0~127
            } GPI_9_GROUP;

            unsigned char GPI_10; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_10_K02_9
            {
                unsigned short DIRECTION_TO_ENEMY_WEAPON; // Bit_Size : 13 // Valid_Range : 0~6399
                unsigned short NUMBER_OF_ENEMY_ROUNDS; // Bit_Size : 10 // Valid_Range : 1~1023
                unsigned char WEAPON_CALIBER; // Bit_Size : 6 // Valid_Range : 0~42
            } GPI_10_GROUP;

            unsigned char GPI_11; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_11_K02_9
            {
                unsigned char FPI_18; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char SIZE; // Bit_Size : 5 // Valid_Range : 0~29,31

                unsigned char GPI_12; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_12_K02_9
                {
                    unsigned short LENGTH; // Bit_Size : 14 // Valid_Range : 1~16383
                    unsigned short WIDTH; // Bit_Size : 14 // Valid_Range : 1~16383
                    unsigned short ATTITUDE; // Bit_Size : 13 // Valid_Range : 0~6399
                } GPI_12_GROUP;
                unsigned char FPI_19; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short RADIUS; // Bit_Size : 14 // Valid_Range : 1~16383
                unsigned char FPI_20; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char RADAR_ZONE_NAME[30]; // Bit_Size : 210 // Valid_Character : 32,48~57,65~90,127
            } GPI_11_GROUP;

            unsigned char GPI_13; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_13_K02_9
            {
                unsigned char FPI_21; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short ATTACK_DIRECTION; // Bit_Size : 13 // Valid_Range : 0~6399
                unsigned char FPI_22; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short FALSE_TARGET_DENSITY; // Bit_Size : 14 // Valid_Range : 0~16383
                unsigned char FPI_23; // Bit_Size : 1 // Valid_Range : 0~1

                int REAL_FRI_2_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

                struct FRI2_K02_9
                {
                    unsigned char WEATHER_CONDITIONS; // Bit_Size : 5 // Valid_Range : 0~26
                } FRI_2[3];
                unsigned char FPI_24; // Bit_Size : 1 // Valid_Range : 0~1

                int REAL_FRI_3_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

                struct FRI3_K02_9
                {
                    unsigned char TERRAIN_DESCRIPTION; // Bit_Size : 4 // Valid_Range : 0~15
                } FRI_3[3];
                unsigned char FPI_25; // Bit_Size : 1 // Valid_Range : 0~1

                int REAL_FRI_4_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

                struct FRI4_K02_9
                {
                    unsigned char COUNTERMEASURES; // Bit_Size : 4 // Valid_Range : 0~13
                } FRI_4[3];
                unsigned char FPI_26; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char TARGET_AREA_MET_IDENTIFICATION[2]; // Bit_Size : 14 // Valid_Character : 32,48~57,65~90
                unsigned char FPI_27; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char SUBMUNITIONS_SHEAF_TYPE; // Bit_Size : 5 // Valid_Range : 1~26
                unsigned char FPI_28; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char FIRE_MISSION_PRIORITY; // Bit_Size : 3 // Valid_Range : 0~4
            } GPI_13_GROUP;

            unsigned char GPI_14; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_14_K02_9
            {
                unsigned char MISSION_FIRED_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char FPI_29; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char ENEMY_PERSONNEL_CLOTHING_CODE; // Bit_Size : 2 // Valid_Range : 0~3
                unsigned char FPI_30; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char ENEMY_TRAINING_IN_NBC; // Bit_Size : 2 // Valid_Range : 1~3
                unsigned char FPI_31; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char VEGETATION_TYPE; // Bit_Size : 3 // Valid_Range : 0~5
                unsigned char FPI_32; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char TARGET_PERMANENCE; // Bit_Size : 3 // Valid_Range : 0~4
                unsigned char FPI_33; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char FIRE_SUPPORT_EFFECT_ACHIEVED; // Bit_Size : 4 // Valid_Range : 0~14
                unsigned char FPI_34; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short NUMBER_OF_ENEMY_CASUALTIES; // Bit_Size : 14 // Valid_Range : 0~16383
            } GPI_14_GROUP;

            unsigned char GPI_15; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_15_K02_9
            {
                unsigned char DO_NOT_ADJUST_LOCATION_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char DO_NOT_COMBINE_TARGET_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char RECORD_AS_TARGET_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char ASSIGN_KNOWN_POINT_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char TARGET_INTELLIGENCE_FILE_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char LAST_TARGET_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char FIRE_REQUEST_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
            } GPI_15_GROUP;

            unsigned char GPI_16; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_16_K02_9
            {
                unsigned char FPI_35; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_YEAR; // Bit_Size : 7 // Valid_Range : 0~99
                unsigned char FPI_36; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_MONTH; // Bit_Size : 4 // Valid_Range : 1~12
                unsigned char EFFECTIVE_DAY; // Bit_Size : 5 // Valid_Range : 1~31
                unsigned char EFFECTIVE_HOUR; // Bit_Size : 5 // Valid_Range : 0~23
                unsigned char EFFECTIVE_MINUTE; // Bit_Size : 6 // Valid_Range : 0~59
                unsigned char FPI_37; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned char EFFECTIVE_SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
            } GPI_16_GROUP;
            unsigned char FPI_38; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char TARGET_ACQUISITION_WAY_TEXT[20]; // Bit_Size : 140 // Valid_Character : 32~127
            unsigned char FPI_39; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char COMMENTS[200]; // Bit_Size : 1400 // Valid_Character : 9~10,13,32~127

            unsigned char GPI_17; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_17_K02_9
            {
                unsigned char FPI_40; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short ELAPSED_TIME_INTERVAL; // Bit_Size : 10 // Valid_Range : 1~1023
                unsigned char FPI_41; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned short NUMBER_OF_ENEMY_ROUNDS_73; // Bit_Size : 10 // Valid_Range : 1~1023

                unsigned char GPI_18; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_18_K02_9
                {
                    unsigned char TIME_FIRED_HOUR; // Bit_Size : 5 // Valid_Range : 0~23
                    unsigned char TIME_FIRED_MINUTE; // Bit_Size : 6 // Valid_Range : 0~59
                    unsigned char FPI_42; // Bit_Size : 1 // Valid_Range : 0~1
                    unsigned char TIME_FIRED_SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
                } GPI_18_GROUP;

                unsigned char GPI_19; // Bit_Size : 1 // Valid_Range : 0~1
                struct GPI_GROUP_19_K02_9
                {
                    unsigned short ESTIMATED_WEAPON_IMPACT_RANGE; // Bit_Size : 10 // Valid_Range : 1~1023
                    unsigned short ESTIMATED_WEAPON_IMPACT_AZIMUTH; // Bit_Size : 13 // Valid_Range : 0~6399
                    unsigned short ESTIMATED_WEAPON_QUADRANT_ELEVATION; // Bit_Size : 15 // Valid_Range : 0~15560,28769~32767
                    unsigned short HEIGHT_OF_TRAJECTORY_ESTIMATE; // Bit_Size : 14 // Valid_Range : 1~16383
                    unsigned short ESTIMATED_EQUIVALENT_INITIAL_VELOCITY; // Bit_Size : 14 // Valid_Range : 1~16383
                    unsigned char RADAR_CROSS_SECTION; // Bit_Size : 6 // Valid_Range : 0~9,13~63
                } GPI_19_GROUP;
            } GPI_17_GROUP;
        } GRI_1_GROUP[20];

    } GPI_1_GROUP;
} K02_9;

#pragma pack(pop)

#endif /* _K02_9_H_ */
