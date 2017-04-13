#ifndef _DLP_HEADER_H_
#define _DLP_HEADER_H_

// ICD MESSAGE CONTROL CODE DEFINITION WITH AGENT
#define DLP_INTERFACE_START_MSG						0x0001		// DLP Interface Start : ?곕룞 ?쒖옉 ?붿껌
#define DLP_INTERFACE_START_ACKNOWLEDGE_MSG			0x0002		// DLP Interface Start Acknowledge : ?곕룞 ?묐떟, ?뱀씤
#define DLP_INTERFACE_START_NOT_ACKNOWLEDGE_MSG		0x0003		// DLP Interface Start Not Acknowledge : ?곕룞 ?묐떟, 誘몄듅??
#define	DLP_START_SETTING_MSG						0x0004		// DLP Start Setting : ?곕룞 ?ㅼ젙
#define DLP_SECURITY_CONF_MSG						0x0005		// DLP Security Conf : 蹂댁븞 ?몃? ?ㅼ젙
#define DLP_HEARTBEAT_MSG							0x0006		// DLP Heartbeat : ?곕룞 ?곹깭 蹂닿퀬
#define DLP_DEBUG_SETTING_MSG						0x0007		// DLP Debug Setting : ?붾쾭洹??듭뀡 ?ㅼ젙
#define DLP_HEX_LOGGING_SETTING_MSG					0x0008		// DLP Hex Message Logging Setting : HEX 媛?異쒕젰 ?ㅼ젙
#define DLP_SEND_RECV_KVMF_MSG						0x0021		// ?쒖? ?명꽣?섏씠??洹쒓꺽???뺤쓽???꾨Ц ?ㅻ뜑
#define DLP_SEND_RECV_MONITOR_MSG					0x00AA		// DLP 吏꾨떒 ?꾨줈洹몃옩 ?≪닔??硫붿떆吏

// ICD SENDER OR RECEIVER NUMBER DEFINITION WITH AGENT
#define MSG_PROCESSOR								1			// 硫붿떆吏 泥섎━湲?
#define LINK_K_DLP									2			// Link-K DLP
#define ATCIS										3			// ATCIS
#define FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE1		4			// 誘몃옒 泥닿퀎 ?먮뒗 ?대?紐⑤뱢 1
#define FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE2		5			// 誘몃옒 泥닿퀎 ?먮뒗 ?대?紐⑤뱢 2
#define FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE3		6			// 誘몃옒 泥닿퀎 ?먮뒗 ?대?紐⑤뱢 3

// MESSAGE PROCESSOR INTERNAL MODULE IDENTIFICATION FOR USING PROCESSING
#define AGENT_ID									100
#define APPPROT_ID									200
#define TERMINAL_ID									300
#define MONITOR_ID									400

#pragma pack(push, 1)

typedef struct _DlpCommonHeader_ 		// 紐⑤뱺 硫붿떆吏?????怨듯넻 ?ㅻ뜑
{
    unsigned short controlCode;			// ?≪닔??硫붿떆吏 ?앸퀎踰덊샇 : DLP_INTERFACE_START_MSG(0x0001), DLP_INTERFACE_START_ACKNOWLEDGE_MSG(0x0002), DLP_INTERFACE_START_NOT_ACKNOWLEDGE_MSG(0x0003), DLP_START_SETTING_MSG(0x0004), DLP_SECURITY_CONF_MSG(0x0005), DLP_HEARTBEAT_MSG(0x0006), DLP_DEBUG_SETTING_MSG(0x0007), DLP_HEX_LOGGING_MSG(0x0008), DLP_SEND_RECV_KVMF_MSG(0x0021)
    unsigned int sender;				// ?≪떊??踰덊샇 : MSG_PROCESSOR(1), LINK_K_DLP(2), ATCIS(3), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE1(4), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE2(5), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE3(6), ...
    unsigned int receiver;				// ?섏떊??踰덊샇 : MSG_PROCESSOR(1), LINK_K_DLP(2), ATCIS(3), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE1(4), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE2(5), FUTURE_SYSTEM_OR_DLP_INTERNAL_MODULE3(6), ...
    unsigned short dataSize;			// 援ъ“泥대줈 ?뺤쓽??硫붿떆吏 ?ㅻ뜑瑜??ы븿???꾩껜 ?곗씠???ш린 (Byte). Only sizeof (Sub message Header Struct) (Ex:Agent_Message_Header.msgHdr.dataSize == sizeof (Agent_Message_Header), Dlp_Start_Setting.msgHdr.dataSize == sizeof (Dlp_Start_Setting))
} DlpCommonHeader;

// DLP_INTERFACE_START_MSG STRUCT
typedef struct _DlpInterfaceStartMsg_
{
    DlpCommonHeader dlpHeader;
} DlpInterfaceStartMsg;

// DLP_INTERFACE_START_ACKNOWLEDGE_MSG STRUCT
typedef struct _DlpInterfaceStartAckMsg_
{
    DlpCommonHeader dlpHeader;
} DlpInterfaceStartAckMsg;

// DLP_INTERFACE_START_NOT_ACKNOWLEDGE_MSG STRUCT
typedef struct _DlpInterfaceStartNackMsg_
{
    DlpCommonHeader dlpHeader;
    unsigned char notAckReason;		// 0 : Interface(TCP/UDP/SERIAL/...) Initialize Fail, 1 : On Initializing DLP
} DlpInterfaceStartNackMsg;

// DLP_START_SETTING_MSG STRUCT
typedef struct _DlpStartSetMsg_
{
    DlpCommonHeader dlpHeader;

    unsigned int myNodeURN;				// My Node URN
    unsigned char appProtVersion;		// 47001 Version Number
    unsigned char dataCompressType;		// Compress Type
    unsigned char msgStdVersion;		// Message Standard Version
    unsigned char useMsgSecurity;		// ?뷀샇???ъ슜 ?щ?
    unsigned short srByteSize;			// ?묒슜?꾨줈?좎퐳 S/R ?ш린[Bytes]
    unsigned short msgACKSecTimeout;	// ?묒슜?꾨줈?좎퐳 Ack ?묐떟?쒓컙 [msec]
    unsigned char retransmitUse;		// ?묒슜?꾨줈?좎퐳 ?ъ쟾??0 : 誘몄궗?? 1: ?ъ슜
} DlpStartSetMsg;

// DLP_SECURITY_CONF_MSG STRUCT
typedef struct _DlpSecurityConfMsg
{
    DlpCommonHeader dlpHeader;

    unsigned char gpi_1;							// For keyMaterialGrp Struct

    struct _keyMaterialGrp
    {
        unsigned char keyMaterialIDLen;				// 0 ~ 7 : 1~8 Byte??keyMaterialID 湲몄씠
        unsigned char keyMaterialID[8];				//
    } keyMaterialGrp;

    unsigned char gpi_2;							// For cryptoInitGrp Struct

    struct _cryptoInitGrp
    {
        unsigned char cryptoInitLen;				// 0 ~ 15 : 1~16 Block??cryptoInit 湲몄씠
        unsigned char cryptoInit[128];
    } cryptoInitGrp;

    unsigned char gpi_3;							// For keyTokenGrp Struct

    struct _keyTokenGrp
    {
        unsigned char keyTokenLen;					// 0 ~ 255 : 1~256 Block??keyToken 湲몄씠
        unsigned int realKeyTokenDataGrpRepeatNum;	// Real keyTokenDataGrp repeat count (1~17) (Default Minimum Repeat Count >= 1)

        struct _keyTokenDataGrp
        {
            unsigned char keyToken[2048];
        } keyTokenDataGrp[17];

    } keyTokenGrp;

    unsigned char gpi_4;							// For authenAGrp Struct

    struct _authenAGrp
    {
        unsigned char authenDataALen;				// 0 ~ 127 : 1~128 Block??authenDataA 湲몄씠
        unsigned char authenDataA[1024];
    } authenAGrp;

    unsigned char gpi_5;							// For authenBGrp Struct

    struct _authenBGrp
    {
        unsigned char authenDataBLen;				// 0 ~ 127 : 1~128 Block??authenDataB 湲몄씠
        unsigned char authenDataB[1024];
    } authenBGrp;

    unsigned char signedACKReqIndicator;			// 0 : signed ACK 遺덊븘?? 1 : signed ACK ?꾩슂
    unsigned char gpi_6;							// For msgSecurityPaddingGrp Struct

    struct _msgSecurityPaddingGrp
    {
        unsigned char msgSecurityPaddingLen;		// 0 ~ 255 : 0~255 Byt??msgSecurityPadding 湲몄씠
        unsigned char fpi2;							// For msgSecurityPadding
        unsigned char msgSecurityPadding[255];
    } msgSecurityPaddingGrp;
} DlpSecurityConfMsg;

// DLP_HEARTBEAT_MSG STRUCT (Period 1 Second)
typedef struct _DlpHeartbeatMsg
{
    DlpCommonHeader dlpHeader;

    unsigned char reserved1;
    unsigned char reserved2;

} DlpHeartbeatMsg;


// DLP_DEBUG_SETTING_MSG STRUCT
typedef struct _DlpDebugSetMsg
{
    DlpCommonHeader dlpHeader;
    unsigned char debugPrintOption;			// File Debugging Option (0 : Not Using, 1 : Creating Base DLP Processing Information, 2 : Creating Detail DLP Processing Information)
} DlpDebugSetMsg;

// DLP_HEX_LOGGING_SETTING_MSG STRUCT
typedef struct _DlpHexLogSetMsg
{
    DlpCommonHeader dlpHeader;
    unsigned char agentRxLogging;			// 0 : Stop, 1 : Start
    unsigned char agentTxLogging;			// 0 : Stop, 1 : Start
    unsigned char terminalRxLogging;		// 0 : Stop, 1 : Start
    unsigned char terminalTxLogging;		// 0 : Stop, 1 : Start
} DlpHexLogSetMsg;

#pragma pack(pop)

#endif /*_DLP_HEADER_H_ */
