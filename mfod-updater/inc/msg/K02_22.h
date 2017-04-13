#ifndef _K02_22_H_
#define _K02_22_H_

/*
    Message No : K02.22
    Message Title : SUBSEQUENT ADJUST
    Message Purpose : TO ADJUST THE FALL OF THE SHOT AGAINST AN AREA TARGET,
                      A DESTRUCTION MISSION, OR FIRE A REGISTRATION FIRE MISSION.
*/

#pragma pack(push,1)
typedef struct K02_22
{
    unsigned char SUBSEQUENT_ADJUSTMENT_TYPE; // Bit_Size : 4 // Valid_Range : 0~12
    // TARGET_NUMBER EXCEPTION HANDLING ===============>
    unsigned char TARGET_NUMBER_ASC[2]; // Bit_Size : 14 // Valid_Character : 65~90
    unsigned short TARGET_NUMBER_DEC; // Bit_Size : 14 // Valid_Range : 0~9999
    // <================================================
    unsigned char GUN_TARGET_LINE_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned char FPI_1; // Bit_Size : 1 // Valid_Range : 0~1

    int REAL_FRI_1_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

    struct FRI1_K02_22
    {
        unsigned short OBSERVER_TARGET_AZIMUTH; // Bit_Size : 13 // Valid_Range : 0~6399
    } FRI_1[2];
    unsigned char FPI_2; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned int URN; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
    unsigned char FPI_3; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned char OBSERVATION_OF_ROUNDS; // Bit_Size : 3 // Valid_Range : 0~5
    unsigned char FPI_4; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned char NUMBER_OF_OBSERVED_FIRE_ROUNDS; // Bit_Size : 4 // Valid_Range : 1~8
    unsigned char FPI_5; // Bit_Size : 1 // Valid_Range : 0~1
    unsigned short DISTANCE_TO_FRIENDLIES; // Bit_Size : 14 // Valid_Range : 0~16383

    unsigned char GPI_1; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_1_K02_22
    {

        int REAL_GRI_1_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

        struct GRI_GROUP_1_K02_22
        {
            unsigned char FPI_6; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char SAME_DATA; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char FPI_7; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short LATERAL_SHIFT; // Bit_Size : 13 // Valid_Range : 0~4095,4097~8191
            unsigned char FPI_8; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short RANGE_SHIFT; // Bit_Size : 13 // Valid_Range : 0~4095,4097~8191
            unsigned char FPI_9; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short VERTICAL_SHIFT; // Bit_Size : 15 // Valid_Range : 0~9999,22769~32767
        } GRI_1_GROUP[2];

    } GPI_1_GROUP;

    unsigned char GPI_2; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_2_K02_22
    {
        unsigned char OBSERVER_STATUS_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FPI_10; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char RADAR_OBSERVATION_REPORT; // Bit_Size : 3 // Valid_Range : 1~6
        unsigned char FPI_11; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short MARGIN_OF_ERROR_DISTANCE; // Bit_Size : 10 // Valid_Range : 1~1023
    } GPI_2_GROUP;

    unsigned char GPI_3; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_3_K02_22
    {
        unsigned int IMPACT_LATITUDE; // Bit_Size : 25 // Valid_Range : 0~33554431
        unsigned int IMPACT_LONGITUDE; // Bit_Size : 26 // Valid_Range : 0~67108863
        unsigned char FPI_12; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int IMPACT_ELEVATION__MSL; // Bit_Size : 17 // Valid_Range : 0~65535,129752~131071
        unsigned char FPI_13; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int IMPACT_ELEVATION__HAE; // Bit_Size : 22 // Valid_Range : 0~1280000,2097152,4130304~4194303
    } GPI_3_GROUP;

    unsigned char GPI_4; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_4_K02_22
    {
        unsigned int AIMPOINT_LATITUDE; // Bit_Size : 25 // Valid_Range : 0~33554431
        unsigned int AIMPOINT_LONGITUDE; // Bit_Size : 26 // Valid_Range : 0~67108863
        unsigned char FPI_14; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int AIMPOINT_ELEVATION__MSL; // Bit_Size : 17 // Valid_Range : 0~65535,129752~131071
        unsigned char FPI_15; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int AIMPOINT_ELEVATION__HAE; // Bit_Size : 22 // Valid_Range : 0~1280000,2097152,4130304~4194303
    } GPI_4_GROUP;

    unsigned char GPI_5; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_5_K02_22
    {
        unsigned char FPI_16; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short REFERENCE_DIRECTION; // Bit_Size : 13 // Valid_Range : 0~6399
        unsigned char FPI_17; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short DIRECTION_ERROR; // Bit_Size : 11 // Valid_Range : 0~1023,1025~2047
        unsigned char FPI_18; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short REFERENCE_VERTICAL_ANGLE; // Bit_Size : 12 // Valid_Range : 0~1600,2496~4095
        unsigned char FPI_19; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short VERTICAL_ANGLE_ERROR; // Bit_Size : 11 // Valid_Range : 0~1023,1025~2047
    } GPI_5_GROUP;

    unsigned char GPI_6; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_6_K02_22
    {
        unsigned char TARGET_GENERIC_TYPE; // Bit_Size : 5 // Valid_Range : 0~22
        unsigned char TARGET_SUBTYPE; // Bit_Size : 8 // Valid_Range : 0~137
        unsigned char FPI_20; // Bit_Size : 1 // Valid_Range : 0~1

        int REAL_FRI_2_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

        struct FRI2_K02_22
        {
            unsigned char TARGET_SUBTYPE_ELEMENT; // Bit_Size : 7 // Valid_Range : 0~82
        } FRI_2[3];
    } GPI_6_GROUP;

    unsigned char GPI_7; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_7_K02_22
    {
        unsigned char TIME_REPEAT_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FPI_21; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char REGISTRATION_COMMAND; // Bit_Size : 3 // Valid_Range : 0~5

        unsigned char GPI_8; // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_8_K02_22
        {
            unsigned char METHOD_OF_CONTROL; // Bit_Size : 5 // Valid_Range : 0~21
            unsigned char METHOD_OF_FIRE; // Bit_Size : 4 // Valid_Range : 0~11
            unsigned char FPI_22; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char RE_FIRE_FOR_EFFECT_ROUNDS_INDICATOR; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char FPI_23; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short TIME_BETWEEN_ROUNDS; // Bit_Size : 10 // Valid_Range : 1~1023
            unsigned char FPI_24; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char DISTRIBUTION_OF_FIRE; // Bit_Size : 4 // Valid_Range : 0~12
        } GPI_8_GROUP;
        unsigned char FPI_25; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char KOREAN_METHOD_OF_ENGAGEMENT; // Bit_Size : 4 // Valid_Range : 0~5
        unsigned char FPI_26; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char TRAJECTORY_TYPE; // Bit_Size : 2 // Valid_Range : 0~3
        unsigned char FPI_27; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char IMPACT_DISTRIBUTION; // Bit_Size : 3 // Valid_Range : 0~5
        unsigned char FPI_28; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char DISTRIBUTION_OF_FIRE_40; // Bit_Size : 4 // Valid_Range : 0~12
        unsigned char FPI_29; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char ILLUMINATION_TYPE_DESIGNATOR; // Bit_Size : 3 // Valid_Range : 0~6

        unsigned char GPI_9; // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_9_K02_22
        {
            unsigned char FPI_30; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char NUMBER_OF_NAVAL_SURFACE_WEAPONS; // Bit_Size : 7 // Valid_Range : 0~99
            unsigned char FPI_31; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char NUMBER_OF_SALVOS; // Bit_Size : 7 // Valid_Range : 0~99
        } GPI_9_GROUP;
    } GPI_7_GROUP;

    unsigned char GPI_10; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_10_K02_22
    {
        unsigned char FPI_32; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int URN_44; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
        unsigned char FPI_33; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short ADJUSTING_PROJECTILE; // Bit_Size : 9 // Valid_Range : 0~269
        unsigned char FPI_34; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char ADJUSTING_FUZE; // Bit_Size : 7 // Valid_Range : 0~119
        unsigned char FPI_35; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FUZE_MODE; // Bit_Size : 3 // Valid_Range : 0~4
        unsigned char FPI_36; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char REGISTRATION_POINT_MARKING; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FPI_37; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char RECORD_ROUNDS; // Bit_Size : 7 // Valid_Range : 0~127
        unsigned char FPI_38; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char REGISTRATION_FUZE; // Bit_Size : 7 // Valid_Range : 0~75
        unsigned char FPI_39; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FUZE_MODE_51; // Bit_Size : 3 // Valid_Range : 0~4
        unsigned char FPI_40; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char FUZE_COUNTRY_OF_ORIGIN; // Bit_Size : 4 // Valid_Range : 0~13
    } GPI_10_GROUP;

    unsigned char GPI_11; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_11_K02_22
    {

        int REAL_GRI_2_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

        struct GRI_GROUP_2_K02_22
        {
            unsigned char FPI_41; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short NUMBER_OF_FIRE_FOR_EFFECT_ROUNDS; // Bit_Size : 10 // Valid_Range : 1~1023
            unsigned char FPI_42; // Bit_Size : 1 // Valid_Range : 0~1

            int REAL_FRI_3_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~12 // Max_Repeat : 12

            struct FRI3_K02_22
            {
                unsigned int URN_54; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
            } FRI_3[12];

            unsigned char GPI_12; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_12_K02_22
            {

                int REAL_GRI_3_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

                struct GRI_GROUP_3_K02_22
                {
                    unsigned char FPI_43; // Bit_Size : 1 // Valid_Range : 0~1
                    unsigned short FIRE_FOR_EFFECT_PROJECTILE; // Bit_Size : 9 // Valid_Range : 0~307
                    unsigned char FPI_44; // Bit_Size : 1 // Valid_Range : 0~1
                    unsigned char FIRE_FOR_EFFECT_FUZE; // Bit_Size : 7 // Valid_Range : 0~119
                    unsigned char FPI_45; // Bit_Size : 1 // Valid_Range : 0~1
                    unsigned char FUZE_MODE_57; // Bit_Size : 3 // Valid_Range : 0~4
                    unsigned char FPI_46; // Bit_Size : 1 // Valid_Range : 0~1
                    unsigned char FUZE_COUNTRY_OF_ORIGIN_58; // Bit_Size : 4 // Valid_Range : 0~13
                } GRI_3_GROUP[2];

            } GPI_12_GROUP;
        } GRI_2_GROUP[2];

    } GPI_11_GROUP;

    unsigned char GPI_13; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_13_K02_22
    {
        unsigned char DAY_ON_TARGET; // Bit_Size : 5 // Valid_Range : 1~31
        unsigned char HOUR_ON_TARGET; // Bit_Size : 5 // Valid_Range : 0~23
        unsigned char MINUTE_ON_TARGET; // Bit_Size : 6 // Valid_Range : 0~59
        unsigned char FPI_47; // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char SECOND_ON_TARGET; // Bit_Size : 6 // Valid_Range : 0~59,63
    } GPI_13_GROUP;

    unsigned char GPI_14; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_14_K02_22
    {
        unsigned int URN_63; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
        unsigned int ENTITY_ID_SERIAL_NUMBER; // Bit_Size : 32 // Valid_Range : 0~4294967295
        unsigned char DAY_OF_MONTH; // Bit_Size : 5 // Valid_Range : 1~31
        unsigned char HOUR; // Bit_Size : 5 // Valid_Range : 0~23,31
        unsigned char MINUTE; // Bit_Size : 6 // Valid_Range : 0~59,63
        unsigned char SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
    } GPI_14_GROUP;

    unsigned char GPI_15; // Bit_Size : 1 // Valid_Range : 0~1
    struct GPI_GROUP_15_K02_22
    {
        unsigned short MOVING_TARGET_AZIMUTH; // Bit_Size : 13 // Valid_Range : 0~6399
        unsigned char MOVING_TARGET_SPEED; // Bit_Size : 7 // Valid_Range : 0~127
    } GPI_15_GROUP;

} K02_22;
#pragma pack(pop)


#endif /* _K02_22_H_ */
