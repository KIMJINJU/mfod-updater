#ifndef _K00_1_H_
#define _K00_1_H_

/*
    Message No : K00.1
    Message Title : NETWORK MONITORING
    Message Purpose : TO COLLECT AND PROVIDE NETWORK STATUS INFORMATION.
*/

#pragma pack(push,1)
typedef struct K00_1
{
    unsigned char NUMBER_OF_SUB_NETWORKS;           // Bit_Size : 3  Valid_Range : 0~7
    unsigned char NETWORK_MONITORING_MESSAGE_TYPE;  // Bit_Size : 2  Valid_Range : 0~2
    unsigned int URN;                               // Bit_Size : 24 Valid_Range : 0~16777212,16777214~16777215

    int REAL_GRI_1_GROUP_REPEAT_NUM;                // Bit_Size : 0  Valid_Range : 1~16  Max_Repeat : 16

    struct GRI_GROUP_1_K00_1
    {
        unsigned int URN_3; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215

        int REAL_GRI_2_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~2 // Max_Repeat : 2

        struct GRI_GROUP_2_K00_1
        {
            unsigned char DATA_COLLECTION_DAY; // Bit_Size : 5 // Valid_Range : 1~31
            unsigned char DATA_COLLECTION_HOUR; // Bit_Size : 5 // Valid_Range : 0~23
            unsigned char DATA_COLLECTION_MINUTE; // Bit_Size : 6 // Valid_Range : 0~59
            unsigned char DATA_COLLECTION_SECOND; // Bit_Size : 6 // Valid_Range : 0~59,63
        } GRI_2_GROUP[2];

        unsigned char GPI_1; // Bit_Size : 1 // Valid_Range : 0~1
        struct GPI_GROUP_1_K00_1
        {
            unsigned char DATA_MEASUREMENT_INDICATOR; // Bit_Size : 2 // Valid_Range : 0~3

            int REAL_GRI_3_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~5 // Max_Repeat : 5

            struct GRI_GROUP_3_K00_1
            {
                unsigned char PROTOCOL_TYPE; // Bit_Size : 3 // Valid_Range : 0~3

                int REAL_GRI_4_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~3 // Max_Repeat : 3

                struct GRI_GROUP_4_K00_1
                {
                    unsigned char TRAFFIC_TYPE; // Bit_Size : 2 // Valid_Range : 0~2
                    unsigned char CURRENT_NETWORK_LOAD; // Bit_Size : 4 // Valid_Range : 0~15
                    unsigned char AVERAGE_NETWORK_LOAD; // Bit_Size : 4 // Valid_Range : 0~15
                } GRI_4_GROUP[3];

            } GRI_3_GROUP[5];

            unsigned char FPI_1; // Bit_Size : 1 // Valid_Range : 0~1
            unsigned int URN_13; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
            unsigned char NETWORK_STATUS; // Bit_Size : 2 // Valid_Range : 0~2

            unsigned char GPI_2; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_2_K00_1
            {
                unsigned int QUANTITY_OF_SA_DATA_DOWN; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned int QUANTITY_OF_C2_DATA_DOWN; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned int QUANTITY_OF_LOCAL_CSMA_SA_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned int QUANTITY_OF_LOCAL_CSMA_C2_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned char NUMBER_OF_SERVER_CLIENTS; // Bit_Size : 8 // Valid_Range : 0~255

                int REAL_GRI_5_GROUP_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~255 // Max_Repeat : 255

                struct GRI_GROUP_5_K00_1
                {
                    unsigned int URN_20; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
                    unsigned char CLIENT_CONFIGURATION; // Bit_Size : 3 // Valid_Range : 0~2
                } GRI_5_GROUP[255];

            } GPI_2_GROUP;

            unsigned char GPI_3; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_3_K00_1
            {
                unsigned int QUANTITY_OF_WIDE_AREA_CSMA_C2_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned char FPI_2; // Bit_Size : 1 // Valid_Range : 0~1
                unsigned int QUANTITY_OF_WIDE_AREA_CSMA_SA_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
            } GPI_3_GROUP;

            unsigned char GPI_4; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_4_K00_1
            {
                unsigned int QUANTITY_OF_SVC_C2_DATA_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
                unsigned int NUMBER_OF_SVCS_UP; // Bit_Size : 24 // Valid_Range : 0~16777215
            } GPI_4_GROUP;

            unsigned char GPI_5; // Bit_Size : 1 // Valid_Range : 0~1
            struct GPI_GROUP_5_K00_1
            {
                unsigned char NUMBER_OF_MULTICAST_GROUPS; // Bit_Size : 8 // Valid_Range : 0~255

                int REAL_FRI_1_REPEAT_NUM; // Bit_Size : 0 // Valid_Range : 1~255 // Max_Repeat : 255

                struct FRI1
                {
                    unsigned int URN_27; // Bit_Size : 24 // Valid_Range : 0~16777212,16777214~16777215
                } FRI_1[255];
            } GPI_5_GROUP;
        } GPI_1_GROUP;
    } GRI_1_GROUP[16];
} K00_1;
#pragma pack(pop)

#endif /* _K00_1_H_ */
