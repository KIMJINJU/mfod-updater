#ifndef _KVMF_HEADER_H_
#define _KVMF_HEADER_H_

#include "dlp_header.h"

#pragma pack(push,1)

typedef struct _KVMF_HEADER_
{
    DlpCommonHeader dlpHeader;

    unsigned char originatorUnitClassify;			// 0 : URN Valid, 1 : UnitName Valid
    unsigned int originatorURN;						// originator urn
    unsigned char originatorUnitName [64];			// originator unit name
    unsigned char priority;							// 1 : Urgent, 2 : Priority, 3 : Routine (Use priority of kvmf message when _USE_PRIORITY_AND_REQTRANSMITCONFIRM_OF_MESSAGE is set)
    unsigned char delayDTR;							//
    unsigned char reliabilityDTR;					//
    unsigned char gpi_1;
    unsigned int realAddresseeGrpRepeatNum;			// Real addresseeGrp repeat count (1~16) (Default Minimum Repeat Count >= 1)

    struct _addresseeGrp
    {
        unsigned char addresseeUnitClassify;		// 0 : URN Valid, 1 : UnitName Valid
        unsigned char receipentClassify;			// ?섏떊??李몄“???앸퀎?? 0 : addressee (?섏떊??, 1 : information addressee (李몄“??
        unsigned int addresseeInfoURN;				// addressee or information addressee urn
        unsigned char addresseeInfoUnitName [64];	// addressee or information addressee unit name
    } addresseeGrp [16];

    unsigned char gpi_2;
    unsigned int realMsgHandleGrpRepeatNum;				// Real msgHandleGrp repeat count (1~16) (Default Minimum Repeat Count >= 1)

    struct _msgHandleGrp
    {
        unsigned int realMsgLength;						// Real Byte Size : Real message body size(Variable Bytes Size) exclude header size
        unsigned char UMF;
        unsigned char gpi_3;							// For vmfMsgIdentGrp Struct

        struct _vmfMsgIdentGrp
        {
            unsigned char msgGroupFad;
            unsigned char msgNumber;
            unsigned char fpi_1;						// For msgSubType
            unsigned char msgSubType;

        } vmfMsgIdentGrp;

        unsigned char fpi_2;							// For filename
        unsigned char filename [64];
        unsigned char operationIndicator;				// 0 : Operation, 1 : Exercise, 2 : Simulation, 3 : Test
        unsigned char retransmitIndicator;
        unsigned char msgPrecedenceCode;				// 0 : Routine, 1 : Priority, 2 : Immediate, 3 : Flash, 4 : Flash Override, 5 : CRITIC/ECP, 6~7 : Reserved
        unsigned char securityClassification;			// 0 : Unclassified, 1 : Confidential, 2 : Secret, 3 : Top secret
        unsigned char gpi_4;							// For originatorDtgGrp Struct

        struct _originatorDtgGrp
        {
            unsigned char year;							// 0~94 (2000 through 2094), 95~99 (1995 through 1999), Error (100 through 255)
            unsigned char month;						// 1~12
            unsigned char day;							// 1~31
            unsigned char hour;							// 0~23
            unsigned char minute;						// 0~59
            unsigned char second;						// 0~59
            unsigned char fpi_3;						// For dtgExtension
            unsigned short dtgExtension1;				// This field shall be a 12-bit binary field containing a value that uniquely identifies each message. This field is mandatory if more than one message is sent with the same Originator DTG.
        } originatorDtgGrp;

        unsigned char gpi_5;															// For perishabilityDtgGrp Struct

        struct _perishabilityDtgGrp
        {
            unsigned char year;															// 0~94 (2000 through 2094), 95~99 (1995 through 1999), Error (100 through 255)
            unsigned char month;														// 1~12
            unsigned char day;															// 1~31
            unsigned char hour;															// 0~23
            unsigned char minute;														// 0~59
            unsigned char second;														// 0~59
        } perishabilityDtgGrp;

        unsigned char gpi_6;															// For ackReqGrp Struct

        struct _ackReqGrp
        {
            unsigned char machineAckReqIndicator;
            unsigned char operatorAckReqIndicator;
            unsigned char operatorRepReqIndicator;
        } ackReqGrp;

        unsigned char gpi_7;															// For responseDataGrp Struct

        struct _responseDataGrp
        {
            unsigned char year;															// 0~94 (2000 through 2094), 95~99 (1995 through 1999), Error (100 through 255)
            unsigned char month;														// 1~12
            unsigned char day;															// 1~31
            unsigned char hour;															// 0~23
            unsigned char minute;														// 0~59
            unsigned char second;														// 0~59
            unsigned char fpi_4;														// For dtgExtension
            unsigned short dtgExtension2;												// This field shall be a 12-bit binary field containing a value that uniquely identifies each message. This field is mandatory if more than one message is sent with the same Originator DTG.
            unsigned char rc;															// Receipt Compliance
            unsigned char fpi_5;														// For cantcoReasonCode
            unsigned char cantcoReasonCode;
            unsigned char fpi_6;														// For cantproReasonCode
            unsigned char cantproReasonCode;
            unsigned char fpi_7;														// For replyAmplification
            unsigned char replyAmplification [50];

        } responseDataGrp;

        unsigned char gpi_8;															// For refMsgDataGrp Struct

        struct _refMsgDataGrp
        {
            unsigned int realrefMsgDataDetailGrpRepeatNum;								// Real refMsgDataDetailGrp repeat count (1~4) (Default Minimum Repeat Count >= 1)

            struct _refMsgDataDetailGrp
            {
                unsigned char refUnitClassify;											// 0 : URN Valid, 1 : UnitName Valid
                unsigned int refMsgDataURN;												// reference message data urn
                unsigned char refMsgDataUnitName [64];									// reference message data unit name
                unsigned char year;														// 0~94 (2000 through 2094), 95~99 (1995 through 1999), Error (100 through 255)
                unsigned char month;													// 1~12
                unsigned char day;														// 1~31
                unsigned char hour;														// 0~23
                unsigned char minute;													// 0~59
                unsigned char second;													// 0~59
                unsigned char fpi_8;													// For dtgExtension
                unsigned short dtgExtension3;											// This field shall be a 12-bit binary field containing a value that uniquely identifies each message. This field is mandatory if more than one message is sent with the same Originator DTG.

            } refMsgDataDetailGrp [4];
        } refMsgDataGrp;

        unsigned char gpi_9;															// For _msgSecurityGrp Struct

        struct _msgSecurityGrp
        {
            unsigned char securityParamInfo;											// 0 : SHA-1 and DSA (47001D), 1 : ARIA, 2 : Other Security1, 3 : Other Security2, 4 : Other Security3
        } msgSecurityGrp;

    } msgHandleGrp [16];

} KVMF_HEADER;

#pragma pack(pop)

#endif /* _KVMF_HEADER_H_ */
