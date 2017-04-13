#ifndef _K05_1_H_
#define _K05_1_H_

/*
    Message No : K05.1
    Message Title : POSITION REPORT
    Message Purpose : TO PROVIDE FRIENDLY UNIT LOCATION DATA.
*/

#pragma pack(push,1)
typedef struct K05_1
{
    int REAL_GRI_1_GROUP_REPEAT_NUM;        // Bit_Size : 0 // Valid_Range : 1~64 // Max_Repeat : 64

    struct GRI_GROUP_1_K05_1
    {
        unsigned int URN;                   // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
        unsigned int UNIT_LATITUDE;         // Bit_Size : 25 // Valid_Range : 0~33554431
        unsigned int UNIT_LONGITUDE;        // Bit_Size : 26 // Valid_Range : 0~67108863
        unsigned char LOCATION_DERIVATION;  // Bit_Size : 4 // Valid_Range : 0~13
        unsigned char FPI_1;                // Bit_Size : 1 // Valid_Range : 0~1
        unsigned char LOCATION_QUALITY;     // Bit_Size : 4 // Valid_Range : 0~11
        unsigned char EXERCISE_INDICATOR;   // Bit_Size : 1 // Valid_Range : 0~1

        unsigned char GPI_1;                // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_1_K05_1
        {
            unsigned short COURSE;          // Bit_Size : 9 // Valid_Range : 0~359,511
            unsigned short UNIT_SPEED__KPH; // Bit_Size : 11 // Valid_Range : 0~2047
        } GPI_1_GROUP;

        unsigned char FPI_2;                // Bit_Size : 1 // Valid_Range : 0~1
        unsigned int ELEVATION__FEET;       // Bit_Size : 17 // Valid_Range : 0~65535,129752~131071
        unsigned char FPI_3;                // Bit_Size : 1 // Valid_Range : 0~1
        unsigned short ALTITUDE__25_FT;     // Bit_Size : 13 // Valid_Range : 0~8191

        unsigned char GPI_2;                // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_2_K05_1
        {
            unsigned char FPI_4;            // Bit_Size : 1 // Valid_Range : 0~1
            unsigned char MODE_I_CODE;      // Bit_Size : 5 // Valid_Range : 0~3,8~11,16~19,24~27,32~35,40~43,48~51,56~59
            unsigned char FPI_5;            // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short MODE_II_CODE;    // Bit_Size : 12 // Valid_Range : 0~4095
            unsigned char FPI_6;            // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short MODE_III_CODE;   // Bit_Size : 12 // Valid_Range : 0~4095
        } GPI_2_GROUP;

        unsigned char GPI_3;                // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_3_K05_1
        {
            unsigned char YEAR;             // Bit_Size : 7 // Valid_Range : 0~99
            unsigned char MONTH;            // Bit_Size : 4 // Valid_Range : 1~12
            unsigned char DAY_OF_MONTH;     // Bit_Size : 5 // Valid_Range : 1~31
            unsigned char HOUR;             // Bit_Size : 5 // Valid_Range : 0~23,31
            unsigned char MINUTE;           // Bit_Size : 6 // Valid_Range : 0~59,63
            unsigned char SECOND;           // Bit_Size : 6 // Valid_Range : 0~59,63
        } GPI_3_GROUP;
        unsigned char ORIGINATOR_ENVIRONMENT;   // Bit_Size : 2 // Valid_Range : 0~3

        unsigned char GPI_4;                    // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_4_K05_1
        {
            unsigned char FPI_7;                        // Bit_Size : 1     // Valid_Range : 0~1
            unsigned short AIR_SPECIFIC_TYPE;           // Bit_Size : 12    // Valid_Range : 0~99,101~156,254~343,511~682,684,762~854,927~946,1020~1214,1277~1451,1535~1581,1791~1922,2042~2147,4000~4004,4094~4095
            unsigned char FPI_8;                        // Bit_Size : 1     // Valid_Range : 0~1
            unsigned short SURFACE_SPECIFIC_TYPE;       // Bit_Size : 12    // Valid_Range : 0~9,22~27,36~41,43,53~54,65~66,77~81,83~94,119~120,131,133,144~153,155~163,165~169,171~180,182~205,207,209~213,277,279,296~303,305~314,316~327,329,331~367,369~379,382~399,431~448,479~496,518,529~532,541~542,553,564~566,576~577,588,599~602,604~606,616~617,628~636,647~649,658~673,688~708,710,731,742,752~760,773~778,789,800,811~834,867,878~885,897~913,936,947,958~960,969,980,991,1002~1022,1025,1030~1031,1044~1045,1055,1066,1077~1080,1089,1100,1111,1122,1133,1144,1155~1165,1181~1183,1194~1225,1264~1281,1303,1314~1326,1335,1346~1348,1359,1370~1373,1384,1395~1396,1406~1409,1420~1421,1423~1427,1439~1446,1448~1458,1473~1474,1481,1492,1503~1542,1584~1624,1626~1628,1682~1686,1697,1708~1771,1843~1849,1851~1853,1864,1875~1881,1892~1915,2048~2053,2064,2075,2086,2097,2108~2125,2127~2135,2137~2177,2257~2260,2271~2272,2283~2290,2302,2313,2324,2335~2342,2352,2363~2364,2375~2384,2396,2407,2418~2427,2441~2475,2522~2524,2526,2528~2534,2550~2556,2567~2568,2570~2571,2581~2583,2594~2598,2609,2620~2623,2633~2638,2649~2651,2662~2664,2666~2681,2706,2717~2723,2734~2739,2750~2786,2824~2825,2836,2846,2848~2850,2852~2875,2877~2885,2937~2953,2974~2976,2987~3013,3043~3044,3055~3056,3066~3070,3080~3083,3094,3105,3116~3128,3146~3147,3158~3160,3162~3190,3228~3247,3274,3285,3296~3301,3312~3316,3327,3338,3349,3360~3377,3402~3421,3448,3459~3467,3469~3484,3486~3513,3580,3591~3596,3605~3615,3631,3633~3634,3643~3651,3660,3666~3673,3675~3698,3738~3739,3749~3753,3763,3765~3777,3798~3835,3841~3844,3851~3854,3862~3868,3879~3882,3893~3917,3937~3964,3990~3992,4003~4011,4024~4028,4039~4041,4053,4070~4094
            unsigned char FPI_9;                        // Bit_Size : 1     // Valid_Range : 0~1
            unsigned short SUBSURFACE_SPECIFIC_TYPE;    // Bit_Size : 12 // Valid_Range : 0~4,6~18,20~22,361~365,441~450,452~462,721~752,1201~1204,1246~1254,1346~1347,1349~1372,1666~1701,1946~1957,2259~2263,2565,2869,3171,4070~4094
            unsigned char FPI_10;                       // Bit_Size : 1 // Valid_Range : 0~1
            unsigned short LAND_SPECIFIC_TYPE;          // Bit_Size : 12 // Valid_Range : 0~3290,3701~3710,4000~4002,4094
        } GPI_4_GROUP;
    } GRI_1_GROUP[64];
} K05_1;
#pragma pack(pop)



#endif /* _K05_1_H_ */