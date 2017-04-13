/*
	Copyright (C) 2015-2016 EOSYSTEM CO., LTD. (www.eosystem.com)

  	msgproc.c
  		msgproc interface for message processing, etc.
  		This file is part of amod-mainapp.

  	Written by
  		Seung-hwan, Park (seunghwan.park@me.com)
 */

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "core/msgproc.h"
#include "core/kvmf.h"
#include "core/logger.h"
#include "core/taqdata_manager.h"
#include "etc/util.h"
#include "modules/uart.h"

/* constant macro defines - kvmf processor path */
#define PATH_KVMF_PROCESSOR     "/root/amod/kvmf-processor"

/* constant macro defines - packet start/end flag */
#define PACKET_SOM              0x7E
#define PACKET_EOM              0xE7
#define MIN_PACKET_LENGTH       (unsigned short) 6

/* constant macro defines - tx status */
#define TXSTAT_IDLE             0
#define TXSTAT_WAIT_ACK         1
#define TXSTAT_RECV_ACK         2
#define TXSTAT_TIMEOUT          3

/* constant macro defines - response time out */
#define MSGPROC_TIMEOUT         3.0
#define MAX_RETRY_XMIT          3

/* constant macro defines - packet assembly flag */
#define ASSEMBLE_PACKET_OK                  0
#define ASSEMBLE_PACKET_LENGTH_MISMATCH     -1
#define ASSEMBLE_PACKET_INPROC              1

/* constant macro defines - message type */
#define MESSAGE_TYPE_ORIGINAL               0
#define MESSAGE_TYPE_ACK                    1
#define MESSAGE_TYPE_NACK_CANTPRO2          2
#define MESSAGE_TYPE_NACK_CANTPRO15         15
#define MESSAGE_TYPE_NACK_CANTPRO19         19
#define MESSAGE_TYPE_NACK_CANTPRO25         25

/* function macro defines */
#define	SWAP_INT16(x)           (unsigned short) ((((x) >> 8) & 0x00ff) | (((x) << 8) & 0xff00))
#define	SWAP_INT32(x)	        ((((x) >>24) & 0x000000ff) | (((x) >> 8) & 0x0000ff00) | (((x) << 8) & 0x00ff0000) | (((x) <<24) & 0xff000000))
#define FEETS(x)                (int) lround(x * 3.28084)
#define LATITUDE(x)             (unsigned int) (lround(x / 5.364418359e-06))
#define LONGITUDE(x)            (unsigned int) (lround(x / 5.364418190e-06))
#define NEGATIVE_LATITUDE(x)    (unsigned int) (33554432 + lround(x / 5.364418359e-06))
#define NEGATIVE_LONGITUDE(x)   (unsigned int) (67108864 + lround(x / 5.364418190e-06))
#define NEGATIVE_ALTITUDE(x)    (unsigned int) (131072 + x)
#define NEGATIVE_SHIFT(x)       (unsigned short) (8192 + x)

/* structure declaration -  msgproc attribute */
struct msgproc_attribute
{
    /* external interface */
    struct msgproc_interface extif;

    /* internal interface */
    bool exitThread[3];

    int autoXmit;
    int testProgress;
    int testResult;
    int txStatus;
    int result;
    int socket[2];              // 0 : tcp/ip, 1 : udp
    unsigned int urn[2];

    pthread_t tid[3];
    pthread_cond_t cond;
    pthread_mutex_t mtx;

    struct sockaddr_in addr[4];
    struct uart_interface *uart;

    struct dlpif
    {
        bool inProc;
        bool heartBeat;
        bool kvmfProc;
        pid_t pid;
    } dlpif;
};


/* global variables */
static int g_xmit_result = 0;
static bool g_kvmfproc_signal = false;
static struct timespec g_txtime = {.tv_sec = 0, .tv_nsec = 0};

/* external global variables */
extern char **environ;


/* functions for msgproc unit test */
#define _MSGPROC_UNIT_TEST_
#ifdef _MSGPROC_UNIT_TEST_

static void
msgproc_print_kvmfhead(void *hdr)
{
    KVMF_HEADER *ptr = (KVMF_HEADER *)hdr;

    LOGMSG(1, ("================== dump receivced dlpheader data ====================\n"));
    LOGMSG(1, ("dlpHeader.controlCode = %d\n", SWAP_INT16(ptr->dlpHeader.controlCode)));
    LOGMSG(1, ("dlpHeader.sender = %d\n", SWAP_INT32(ptr->dlpHeader.sender)));
    LOGMSG(1, ("dlpHeader.receiver = %d\n", SWAP_INT32(ptr->dlpHeader.receiver)));
    LOGMSG(1, ("dlpHeader.dataSize = %d\n", SWAP_INT16(ptr->dlpHeader.dataSize)));
    LOGMSG(1, ("originatorUnitClassify = %d\n", ptr->originatorUnitClassify));
    LOGMSG(1, ("originatorURN = %d\n", SWAP_INT32(ptr->originatorURN)));
    LOGMSG(1, ("originatorUnitName = %s\n", ptr->originatorUnitName));
    LOGMSG(1, ("priority = %d\n", ptr->priority));
    LOGMSG(1, ("delayDTR = %d\n", ptr->delayDTR));
    LOGMSG(1, ("reliabilityDTR = %d\n", ptr->reliabilityDTR));
    LOGMSG(1, ("gpi_1 = %d\n", ptr->gpi_1));

    if (ptr->gpi_1)
    {
        TLOGMSG(1, ("realAddresseeGrpRepeatNum = %d\n", SWAP_INT32(ptr->realAddresseeGrpRepeatNum)));

        for (int i = 0; i < SWAP_INT32(ptr->realAddresseeGrpRepeatNum); i++)
        {
            LOGMSG(1, ("addresseeGrp[%d].addresseeUnitClassify = %d\n", i, ptr->addresseeGrp[i].addresseeUnitClassify));
            LOGMSG(1, ("addresseeGrp[%d].receipentClassify = %d\n", i, ptr->addresseeGrp[i].receipentClassify));
            LOGMSG(1, ("addresseeGrp[%d].addresseeInfoURN = %d\n", i, SWAP_INT32(ptr->addresseeGrp[i].addresseeInfoURN)));
            LOGMSG(1, ("addresseeGrp[%d].addresseeInfoUnitName = %s\n", i, ptr->addresseeGrp[i].addresseeInfoUnitName));
        }
    }

    LOGMSG(1, ("gpi_2 = %d\n", ptr->gpi_2));

    if (ptr->gpi_2)
    {
        LOGMSG(1, ("realMsgHandleGrpRepeatNum = %d\n", SWAP_INT32(ptr->realMsgHandleGrpRepeatNum)));

        for (int i = 0; i < SWAP_INT32(ptr->realMsgHandleGrpRepeatNum); i++)
        {
            LOGMSG(1, ("msgHandleGrp[%d].realMsgLength = %d\n", i, SWAP_INT32(ptr->msgHandleGrp[i].realMsgLength)));
            LOGMSG(1, ("msgHandleGrp[%d].UMF = %d\n", i, ptr->msgHandleGrp[i].UMF));
            LOGMSG(1, ("msgHandleGrp[%d].gpi_3 = %d\n", i, ptr->msgHandleGrp[i].gpi_3));

            if (ptr->msgHandleGrp[i].gpi_3)
            {
                LOGMSG(1, ("msgHandleGrp[%d].vmfMsgIdentGrp.msgGroupFad = %d\n", i, ptr->msgHandleGrp[i].vmfMsgIdentGrp.msgGroupFad));
                LOGMSG(1, ("msgHandleGrp[%d].vmfMsgIdentGrp.msgNumber = %d\n", i, ptr->msgHandleGrp[i].vmfMsgIdentGrp.msgNumber));
                LOGMSG(1, ("msgHandleGrp[%d].vmfMsgIdentGrp.fpi_1 = %d\n", i, ptr->msgHandleGrp[i].vmfMsgIdentGrp.fpi_1));

                if (ptr->msgHandleGrp[i].vmfMsgIdentGrp.fpi_1)
                    LOGMSG(1, ("msgHandleGrp[%d].vmfMsgIdentGrp.msgSubType = %d\n", i, ptr->msgHandleGrp[i].vmfMsgIdentGrp.msgSubType));
            }

            LOGMSG(1, ("msgHandleGrp[%d].fpi_2 = %d\n", i, ptr->msgHandleGrp[i].fpi_2));

            if (ptr->msgHandleGrp[i].fpi_2)
                LOGMSG(1, ("msgHandleGrp[%d].filename = %s\n", i, ptr->msgHandleGrp[i].filename));

            LOGMSG(1, ("msgHandleGrp[%d].operationIndicator = %d\n", i, ptr->msgHandleGrp[i].operationIndicator));
            LOGMSG(1, ("msgHandleGrp[%d].retransmitIndicator = %d\n", i, ptr->msgHandleGrp[i].retransmitIndicator));
            LOGMSG(1, ("msgHandleGrp[%d].msgPrecedenceCode = %d\n", i, ptr->msgHandleGrp[i].msgPrecedenceCode));
            LOGMSG(1, ("msgHandleGrp[%d].securityClassification = %d\n", i, ptr->msgHandleGrp[i].securityClassification));
            LOGMSG(1, ("msgHandleGrp[%d].gpi_4 = %d\n", i, ptr->msgHandleGrp[i].gpi_4));

            if (ptr->msgHandleGrp[i].gpi_4)
            {
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.year = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.year));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.month = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.month));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.day = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.day));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.hour = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.hour));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.minute = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.minute));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.second = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.second));
                LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.fpi_3 = %d\n", i, ptr->msgHandleGrp[i].originatorDtgGrp.fpi_3));

                if (ptr->msgHandleGrp[i].originatorDtgGrp.fpi_3)
                    LOGMSG(1, ("msgHandleGrp[%d].originatorDtgGrp.dtgExtension1 = %d\n", i, SWAP_INT16(ptr->msgHandleGrp[i].originatorDtgGrp.dtgExtension1)));
            }

            LOGMSG(1, ("msgHandleGrp[%d].gpi_5 = %d\n", i, ptr->msgHandleGrp[i].gpi_5));

            if (ptr->msgHandleGrp[i].gpi_5)
            {
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.year = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.year));
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.month = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.month));
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.day = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.day));
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.hour = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.hour));
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.minute = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.minute));
                LOGMSG(1, ("msgHandleGrp[%d].perishabilityDtgGrp.second = %d\n", i, ptr->msgHandleGrp[i].perishabilityDtgGrp.second));
            }

            LOGMSG(1, ("msgHandleGrp[%d].gpi_6 = %d\n", i, ptr->msgHandleGrp[i].gpi_6));

            if (ptr->msgHandleGrp[i].gpi_6)
            {
                LOGMSG(1, ("msgHandleGrp[%d].ackReqGrp.machineAckReqIndicator = %d\n", i, ptr->msgHandleGrp[i].ackReqGrp.machineAckReqIndicator));
                LOGMSG(1, ("msgHandleGrp[%d].ackReqGrp.operatorAckReqIndicator = %d\n", i, ptr->msgHandleGrp[i].ackReqGrp.operatorAckReqIndicator));
                LOGMSG(1, ("msgHandleGrp[%d].ackReqGrp.operatorRepReqIndicator = %d\n", i, ptr->msgHandleGrp[i].ackReqGrp.operatorRepReqIndicator));
            }

            LOGMSG(1, ("msgHandleGrp[%d].gpi_7 = %d\n", i, ptr->msgHandleGrp[i].gpi_7));

            if (ptr->msgHandleGrp[i].gpi_7)
            {
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.year = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.year));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.month = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.month));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.day = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.day));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.hour = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.hour));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.minute = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.minute));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.second = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.second));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.fpi_4 = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.fpi_4));

                if (ptr->msgHandleGrp[i].responseDataGrp.fpi_4)
                    LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.dtgExtension2 = %d\n", i, SWAP_INT16(ptr->msgHandleGrp[i].responseDataGrp.dtgExtension2)));

                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.rc = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.rc));
                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.fpi_5 = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.fpi_5));

                if (ptr->msgHandleGrp[i].responseDataGrp.fpi_5)
                    LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.cantcoReasonCode = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.cantcoReasonCode));

                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.fpi_6 = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.fpi_6));

                if (ptr->msgHandleGrp[i].responseDataGrp.fpi_6)
                    LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.cantproReasonCode = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.cantproReasonCode));

                LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.fpi_7 = %d\n", i, ptr->msgHandleGrp[i].responseDataGrp.fpi_7));

                if (ptr->msgHandleGrp[i].responseDataGrp.fpi_7)
                    LOGMSG(1, ("msgHandleGrp[%d].responseDataGrp.replyAmplification = %s\n", i, ptr->msgHandleGrp[i].responseDataGrp.replyAmplification));
            }

            LOGMSG(1, ("msgHandleGrp[%d].gpi_8 = %d\n", i, ptr->msgHandleGrp[i].gpi_8));

            if (ptr->msgHandleGrp[i].gpi_8)
            {
                LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.realrefMsgDataDetailGrpRepeatNum = %d\n",
                        i, SWAP_INT32(ptr->msgHandleGrp[i].refMsgDataGrp.realrefMsgDataDetailGrpRepeatNum)));

                for (int j = 0; j < SWAP_INT32(ptr->msgHandleGrp[i].refMsgDataGrp.realrefMsgDataDetailGrpRepeatNum); j++)
                {
                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].refUnitClassify = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].refUnitClassify));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].refMsgDataURN = %d\n",
                            i, j, SWAP_INT32(ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].refMsgDataURN)));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].refMsgDataUnitName = %s\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].refMsgDataUnitName));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].year = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].year));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].month = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].month));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].day = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].day));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].hour = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].hour));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].minute = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].minute));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].second = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].second));

                    LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].fpi_8 = %d\n",
                            i, j, ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].fpi_8));

                    if (ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].fpi_8)
                        LOGMSG(1, ("msgHandleGrp[%d].refMsgDataGrp.refMsgDataDetailGrp[%d].dtgExtension3 = %d\n",
                                i, j, SWAP_INT16(ptr->msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].dtgExtension3)));
                }
            }

            LOGMSG(1, ("msgHandleGrp[%d].gpi_9 = %d\n", i, ptr->msgHandleGrp[i].gpi_9));

            if (ptr->msgHandleGrp[i].gpi_9)
                LOGMSG(1, ("msgHandleGrp[%d].msgSecurityGrp.securityParamInfo = %d\n", i, ptr->msgHandleGrp[i].msgSecurityGrp.securityParamInfo));
        }
    }

    LOGMSG(1, ("====================================================================\n\n"));
}


static void
msgproc_print_k001(K00_1 *msg)
{
    LOGMSG(1, ("===================== dump received K00.1 data ======================\n"));
    LOGMSG(1, ("NUMBER_OF_SUB_NETWORKS = %d\n", msg->NUMBER_OF_SUB_NETWORKS));
    LOGMSG(1, ("NETWORK_MONITORING_MESSAGE_TYPE = %d\n", msg->NETWORK_MONITORING_MESSAGE_TYPE));
    LOGMSG(1, ("URN = %d\n", SWAP_INT32(msg->URN)));
    LOGMSG(1, ("REAL_GRI_1_GROUP_REPEAT_NUM = %d\n", SWAP_INT32(msg->REAL_GRI_1_GROUP_REPEAT_NUM)));

    for (int i = 0; i < SWAP_INT32(msg->REAL_GRI_1_GROUP_REPEAT_NUM); i++)
    {
        LOGMSG(1, ("GRI_1_GROUP[%d].URN_3 = %d\n", i, SWAP_INT32(msg->GRI_1_GROUP[i].URN_3)));
        LOGMSG(1, ("GRI_1_GROUP[%d].REAL_GRI_2_GROUP_REPEAT_NUM = %d\n", i, SWAP_INT32(msg->GRI_1_GROUP[i].REAL_GRI_2_GROUP_REPEAT_NUM)));

        for (int j = 0; j < SWAP_INT32(msg->GRI_1_GROUP[i].REAL_GRI_2_GROUP_REPEAT_NUM); j++)
        {
            LOGMSG(1, ("GRI_1_GROUP[%d].GRI_2_GROUP[%d].DATA_COLLECTION_DAY = %d\n", i, j, msg->GRI_1_GROUP[i].GRI_2_GROUP[j].DATA_COLLECTION_DAY));
            LOGMSG(1, ("GRI_1_GROUP[%d].GRI_2_GROUP[%d].DATA_COLLECTION_HOUR = %d\n", i, j, msg->GRI_1_GROUP[i].GRI_2_GROUP[j].DATA_COLLECTION_HOUR));
            LOGMSG(1, ("GRI_1_GROUP[%d].GRI_2_GROUP[%d].DATA_COLLECTION_MINUTE = %d\n", i, j, msg->GRI_1_GROUP[i].GRI_2_GROUP[j].DATA_COLLECTION_MINUTE));
            LOGMSG(1, ("GRI_1_GROUP[%d].GRI_2_GROUP[%d].DATA_COLLECTION_SECOND = %d\n", i, j, msg->GRI_1_GROUP[i].GRI_2_GROUP[j].DATA_COLLECTION_SECOND));
        }

        LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1));

        if (msg->GRI_1_GROUP[i].GPI_1)
        {
            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.DATA_MEASUREMENT_INDICATOR = %d\n",
                    i, msg->GRI_1_GROUP[i].GPI_1_GROUP.DATA_MEASUREMENT_INDICATOR));

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.REAL_GRI_3_GROUP_REPEAT_NUM = %d\n",
                    i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.REAL_GRI_3_GROUP_REPEAT_NUM)));

            for (int j = 0; j < SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.REAL_GRI_3_GROUP_REPEAT_NUM); j++)
            {
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GRI_3_GROUP[%d].PROTOCOL_TYPE = %d\n",
                        i, j, msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].PROTOCOL_TYPE));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GRI_3_GROUP[%d].REAL_GRI_4_GROUP_REPEAT_NUM = %d\n",
                        i, j, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].REAL_GRI_4_GROUP_REPEAT_NUM)));

                for (int k = 0; k < SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].REAL_GRI_4_GROUP_REPEAT_NUM); k++)
                {
                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GRI_3_GROUP[%d].GRI_4_GROUP[%d].TRAFFIC_TYPE = %d\n",
                            i, j, k, msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k].TRAFFIC_TYPE));

                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GRI_3_GROUP[%d].GRI_4_GROUP[%d].TRAFFIC_TYPE = %d\n",
                            i, j, k, msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k].CURRENT_NETWORK_LOAD));

                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GRI_3_GROUP[%d].GRI_4_GROUP[%d].TRAFFIC_TYPE = %d\n",
                            i, j, k, msg->GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k].AVERAGE_NETWORK_LOAD));
                }
            }

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.FPI_1 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.FPI_1));

            if (msg->GRI_1_GROUP[i].GPI_1_GROUP.FPI_1)
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.URN_13 = %d\n", i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.URN_13)));

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.NETWORK_STATUS = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.NETWORK_STATUS));
            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2));

            if (msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2)
            {
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_SA_DATA_DOWN = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_SA_DATA_DOWN)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_C2_DATA_DOWN = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_C2_DATA_DOWN)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_LOCAL_CSMA_SA_UP = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_LOCAL_CSMA_SA_UP)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_LOCAL_CSMA_C2_UP = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.QUANTITY_OF_LOCAL_CSMA_C2_UP)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.NUMBER_OF_SERVER_CLIENTS = %d\n",
                        i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.NUMBER_OF_SERVER_CLIENTS));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.REAL_GRI_5_GROUP_REPEAT_NUM = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.REAL_GRI_5_GROUP_REPEAT_NUM)));

                for (int j = 0; j < SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.REAL_GRI_5_GROUP_REPEAT_NUM); j++)
                {
                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[%d].URN_20 = %d\n",
                            i, j, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[j].URN_20)));

                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[%d].CLIENT_CONFIGURATION = %d\n",
                            i, j, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[j].CLIENT_CONFIGURATION)));
                }
            }

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_3 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3));

            if (msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3)
            {
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_3_GROUP.QUANTITY_OF_WIDE_AREA_CSMA_C2_UP = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3_GROUP.QUANTITY_OF_WIDE_AREA_CSMA_C2_UP)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_3_GROUP.FPI_2 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3_GROUP.FPI_2));

                if (msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3_GROUP.FPI_2)
                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_3_GROUP.QUANTITY_OF_WIDE_AREA_CSMA_SA_UP = %d\n",
                            i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_3_GROUP.QUANTITY_OF_WIDE_AREA_CSMA_SA_UP)));
            }

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_4 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_4));

            if (msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_4)
            {
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_4_GROUP.QUANTITY_OF_SVC_C2_DATA_UP = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_4_GROUP.QUANTITY_OF_SVC_C2_DATA_UP)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_4_GROUP.NUMBER_OF_SVCS_UP = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_4_GROUP.NUMBER_OF_SVCS_UP)));
            }

            LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_5 = %d\n", i, msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5));

            if (msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5)
            {
                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_5_GROUP.NUMBER_OF_MULTICAST_GROUPS = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.NUMBER_OF_MULTICAST_GROUPS)));

                LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_5_GROUP.REAL_FRI_1_REPEAT_NUM = %d\n",
                        i, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.REAL_FRI_1_REPEAT_NUM)));

                for (int j = 0; j < SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.REAL_FRI_1_REPEAT_NUM); j++)
                    LOGMSG(1, ("GRI_1_GROUP[%d].GPI_1_GROUP.GPI_5_GROUP.FRI_1[%d].URN_27 = %d\n",
                            i, j, SWAP_INT32(msg->GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[j].URN_27)));
            }
        }
        else
            continue;
    }

    LOGMSG(1, ("====================================================================\n\n"));
}


static void
msgproc_print_kvmf_data(char *packet, int nrx, struct sockaddr_in *addr)
{
    int nwr = 0;
    int idx = 0;
    int nrow = 0;
    char buf[512] = {0};

    TLOGMSG(0, ("received %d bytes from %s:%d\n", nrx, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port)));
    TLOGMSG(1, ("received %d bytes from kvmfproc\n", nrx));
    LOGMSG(1, ("\nbit-coded kvmf message data\n"));

    memset(buf, 0x00, sizeof(buf));

    for (int i = 0; i < nrx; i++)
    {
        if (i % 10 == 0)
        {
            nwr = snprintf(&buf[idx], sizeof(buf) - idx, "\n %02d : ", nrow++);
            idx = idx + nwr;
        }

        nwr = snprintf(&buf[idx], sizeof(buf) - idx, "0x%02X ", *(packet + i));
        idx = idx + nwr;
    }

    LOGMSG(1, ("%s\n\n", buf));
}


static void
msgproc_print_packet(char *packet, int len)
{
    int nwr = 0;
    int idx = 0;
    int nrow = 0;
    char buf[512] = {0};

    memset(buf, 0x00, sizeof(buf));

    for (int i = 0; i < len; i++)
    {
        if (i % 10 == 0)
        {
            nwr = snprintf(&buf[idx], sizeof(buf) - idx, "\n %02d : ", nrow++);
            idx = idx + nwr;
        }

        nwr = snprintf(&buf[idx], sizeof(buf) - idx, "0x%02X ", *(packet + i));
        idx = idx + nwr;
    }

    LOGMSG(1, ("%s\n\n", buf));
}


static void
msgproc_dump_dlpmsg(char *msg, int msglen)
{
    int nwr = 0;
    int idx = 0;
    int nrow = 0;
    char buf[4096] = {0};

    memset(buf, 0x00, sizeof(buf));

    for (int i = 0; i < msglen; i++)
    {
        if (i % 10 == 0)
        {
            nwr = snprintf(&buf[idx], sizeof(buf) - idx, "\n %02d : ", nrow++);
            idx = idx + nwr;
        }

        nwr = snprintf(&buf[idx], sizeof(buf), "0x%02X ", *(msg + i));
        idx = idx + nwr;
    }

    LOGMSG(1, ("=================== dump received dlpmsg data ======================="));
    LOGMSG(1, ("%s\n\n", buf));
    LOGMSG(1, ("=====================================================================\n\n"));
}

#endif /* _MSGPROC_UNIT_TEST_ */


static void
handle_kvmfproc_signal(int sig)
{
    if (sig == SIGUSR1)
        g_kvmfproc_signal = true;
}


static size_t
msgproc_get_msgsize(unsigned char msggrp, unsigned char msgnum)
{
    size_t size = 0;

    switch(msggrp)
    {
        case MSGGRP_K00:
            if (msgnum == MSGNUM_NETWORK_MONITOR)
                size = sizeof(K00_1);
            else
                TLOGMSG(1, (DBGINFOFMT "invalid message number\n", DBGINFO));

            break;

        case MSGGRP_K02:
            switch(msgnum)
            {
                case MSGNUM_TARGET_DATA:
                    size = sizeof(K02_9);
                    break;

                case MSGNUM_SUBSEQUENT_ADJUST:
                    size = sizeof(K02_22);
                    break;

                case MSGNUM_OBREADY_REPORT:
                    size = sizeof(K02_37);
                    break;

                default:
                    TLOGMSG(1, (DBGINFOFMT "invalid message number\n", DBGINFO));
                    break;
            }

            break;

        case MSGGRP_K05:
            if (msgnum == MSGNUM_POSITION_REPORT)
                size = sizeof(K05_1);
            else
                TLOGMSG(1, (DBGINFOFMT "invalid message number\n", DBGINFO));
            break;

        default:
            TLOGMSG(1, (DBGINFOFMT "invalid message group number\n", DBGINFO));
            break;
    }

    return size;
}


static unsigned int
msgproc_pack_header(char *input, char *output)
{
    KVMF_HEADER header;
    unsigned short packsize; // size_pack_header
    char pack_header[sizeof(KVMF_HEADER)]; //packed_header
    int  pack_header_size = 0;
    int  repeatnum1 = 0;
    int  repeatnum2 = 0;
    int  repeatnum3 = 0;

    memcpy(&header, input, sizeof(KVMF_HEADER));
    memcpy((char *)(pack_header + pack_header_size), (char *)&header.dlpHeader.controlCode,
           (char *)&header.addresseeGrp[0].addresseeUnitClassify - (char *)&header.dlpHeader.controlCode);

    pack_header_size += (int)((char *)&header.addresseeGrp[0].addresseeUnitClassify - (char *)&header.dlpHeader.controlCode);
    repeatnum1 = SWAP_INT32(header.realAddresseeGrpRepeatNum);

    for (int i = 0; i < repeatnum1; i++)
    {
        memcpy((char *)(pack_header + pack_header_size), &header.addresseeGrp[i].addresseeUnitClassify, MEMBERSIZE(struct _KVMF_HEADER_, addresseeGrp[i]));
        pack_header_size += MEMBERSIZE(struct _KVMF_HEADER_, addresseeGrp[i]);
    }

    memcpy((char *)(pack_header + pack_header_size), &header.gpi_2, (char *)&header.msgHandleGrp[0].realMsgLength - (char *)&header.gpi_2);
    pack_header_size += (int)((char *) &header.msgHandleGrp[0].realMsgLength - (char *) &header.gpi_2);
    repeatnum2 = SWAP_INT32(header.realMsgHandleGrpRepeatNum);

    for (int j = 0; j < repeatnum2; j++)
    {
        memcpy((char *)(pack_header + pack_header_size), &header.msgHandleGrp[j].realMsgLength,
               (char *)&header.msgHandleGrp[j].refMsgDataGrp.refMsgDataDetailGrp[0].refUnitClassify -(char *) &header.msgHandleGrp[j].realMsgLength);

        pack_header_size += (int)((char *)&header.msgHandleGrp[j].refMsgDataGrp.refMsgDataDetailGrp[0].refUnitClassify - (char *)&header.msgHandleGrp[j].realMsgLength);
        repeatnum3 = SWAP_INT32(header.msgHandleGrp[j].refMsgDataGrp.realrefMsgDataDetailGrpRepeatNum);

        for (int k = 0; k < repeatnum3; k++)
        {
            memcpy((char *)(pack_header + pack_header_size), &header.msgHandleGrp[j].refMsgDataGrp.refMsgDataDetailGrp[k].refUnitClassify,
                   MEMBERSIZE(struct _KVMF_HEADER_, msgHandleGrp[j].refMsgDataGrp.refMsgDataDetailGrp[k]));

            pack_header_size += MEMBERSIZE(struct _KVMF_HEADER_, msgHandleGrp[j].refMsgDataGrp.refMsgDataDetailGrp[k]);
        }

        memcpy((char *)(pack_header + pack_header_size), &header.msgHandleGrp[j].gpi_9,
               (char *)&header.msgHandleGrp[j].msgSecurityGrp.securityParamInfo - (char *)&header.msgHandleGrp[j].gpi_9);

        pack_header_size += (int)((char *)&header.msgHandleGrp[j].msgSecurityGrp.securityParamInfo - (int)(char *)&header.msgHandleGrp[j].gpi_9);

        memcpy((char *)(pack_header + pack_header_size), &header.msgHandleGrp[j].msgSecurityGrp.securityParamInfo,
               sizeof(header.msgHandleGrp[j].msgSecurityGrp.securityParamInfo));

        pack_header_size += sizeof(header.msgHandleGrp[j].msgSecurityGrp.securityParamInfo);
    }

    memcpy(output, (char *)pack_header, pack_header_size);
    packsize = SWAP_INT16((unsigned short)pack_header_size);
    memcpy((output + 10), &packsize, sizeof(unsigned short));

    //TLOGMSG(1, ("pack = %d\n", SWAP_INT16(packsize));

    return (unsigned int) pack_header_size;
}


static unsigned int
msgproc_unpack_header(char *input, char *output)
{
    int repeatnum1 = 0;
    int repeatnum2 = 0;
    int repeatnum3 = 0;
    unsigned int packsize = 0;
    KVMF_HEADER hdr = {0};

    memset(&hdr, 0x00, sizeof(KVMF_HEADER));
    memcpy(&hdr.dlpHeader.controlCode, input + packsize, (char *)&hdr.addresseeGrp[0].addresseeUnitClassify - (char *)&hdr.dlpHeader.controlCode);
    packsize += (int)((char *)&hdr.addresseeGrp[0].addresseeUnitClassify - (char *)&hdr.dlpHeader.controlCode);
    repeatnum1 = SWAP_INT32(hdr.realAddresseeGrpRepeatNum);

    for (int i = 0; i < repeatnum1; i++)
    {
        memcpy(&(hdr.addresseeGrp[i].addresseeUnitClassify), input + packsize, MEMBERSIZE(KVMF_HEADER, addresseeGrp[i]));
        packsize = packsize + MEMBERSIZE(KVMF_HEADER, addresseeGrp[i]);
    }

    memcpy(&hdr.gpi_2, input + packsize, (char *)&hdr.msgHandleGrp[0].realMsgLength - (char *)&hdr.gpi_2);
    packsize += (int)((char *)&hdr.msgHandleGrp[0].realMsgLength - (char *)&hdr.gpi_2);
    repeatnum2 = SWAP_INT32(hdr.realMsgHandleGrpRepeatNum);

    for (int i = 0; i < repeatnum2; i++)
    {
        memcpy(&hdr.msgHandleGrp[i].realMsgLength, input + packsize,
               (char *)&hdr.msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[0].refUnitClassify - (char *)&hdr.msgHandleGrp[i].realMsgLength);

        packsize += (int)((char *)&hdr.msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[0].refUnitClassify - (char *)&hdr.msgHandleGrp[i].realMsgLength);
        repeatnum3 = SWAP_INT32(hdr.msgHandleGrp[i].refMsgDataGrp.realrefMsgDataDetailGrpRepeatNum);

        for (int j = 0; j < repeatnum3; j++)
        {
            memcpy (&hdr.msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j].refUnitClassify, input + packsize,
                    MEMBERSIZE(KVMF_HEADER, msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j]));

            packsize += MEMBERSIZE(KVMF_HEADER, msgHandleGrp[i].refMsgDataGrp.refMsgDataDetailGrp[j]);
        }

        memcpy (&hdr.msgHandleGrp[i].gpi_9, (input + packsize),
                (char *)&hdr.msgHandleGrp[i].msgSecurityGrp.securityParamInfo - (char*)&hdr.msgHandleGrp[i].gpi_9);

        packsize += (int)((char *)&hdr.msgHandleGrp[i].msgSecurityGrp.securityParamInfo - (char *)&hdr.msgHandleGrp[i].gpi_9);
        memcpy(&hdr.msgHandleGrp[i].msgSecurityGrp.securityParamInfo, input + packsize,
               MEMBERSIZE(KVMF_HEADER, msgHandleGrp[i].msgSecurityGrp.securityParamInfo));

        packsize += MEMBERSIZE(KVMF_HEADER, msgHandleGrp[i].msgSecurityGrp.securityParamInfo);
    }

    memcpy(output, &hdr, sizeof(KVMF_HEADER));

    return packsize;
}


static unsigned int
msgproc_unpack_k001(char *input, char *output)
{
    int repeatnum1 = 0;
    int repeatnum2 = 0;
    int repeatnum3 = 0;
    int repeatnum4 = 0;
    int repeatnum5 = 0;
    int repeatnum6 = 0;
    unsigned int packsize = 0;

    K00_1 k001 = {0};

    memset(&k001, 0x00, sizeof(K00_1));
    memcpy(&k001.NUMBER_OF_SUB_NETWORKS, input + packsize, (char *)&k001.GRI_1_GROUP[0].URN_3 - (char *)&k001.NUMBER_OF_SUB_NETWORKS);
    packsize += (int)((char *)&k001.GRI_1_GROUP[0].URN_3 - (char *)&k001.NUMBER_OF_SUB_NETWORKS);
    repeatnum1 = SWAP_INT32(k001.REAL_GRI_1_GROUP_REPEAT_NUM);

    for (int i = 0; i < repeatnum1; i++)
    {
        memcpy(&k001.GRI_1_GROUP[i].URN_3, input + packsize,
               (char *)&k001.GRI_1_GROUP[i].GRI_2_GROUP[0].DATA_COLLECTION_DAY - (char *)&k001.GRI_1_GROUP[i].URN_3);

        packsize += (int)((char *)&k001.GRI_1_GROUP[i].GRI_2_GROUP[0].DATA_COLLECTION_DAY - (char *)&k001.GRI_1_GROUP[i].URN_3);
        repeatnum2 = SWAP_INT32(k001.GRI_1_GROUP[i].REAL_GRI_2_GROUP_REPEAT_NUM);

        for (int j = 0; j < repeatnum2; j++)
        {
            memcpy(&k001.GRI_1_GROUP[i].GRI_2_GROUP[j], input + packsize, MEMBERSIZE(K00_1, GRI_1_GROUP[i].GRI_2_GROUP[j]));
            packsize += MEMBERSIZE(K00_1, GRI_1_GROUP[i].GRI_2_GROUP[j]);
        }

        memcpy(&k001.GRI_1_GROUP[i].GPI_1, input + packsize,
               (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[0].PROTOCOL_TYPE - (char *)&k001.GRI_1_GROUP[i].GPI_1);

        packsize += (int)((char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[0].PROTOCOL_TYPE - (char *)&k001.GRI_1_GROUP[i].GPI_1);
        repeatnum3 = SWAP_INT32(k001.GRI_1_GROUP[i].GPI_1_GROUP.REAL_GRI_3_GROUP_REPEAT_NUM);

        for (int j = 0; j < repeatnum3; j++)
        {
            memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].PROTOCOL_TYPE, input + packsize,
                   (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[0].TRAFFIC_TYPE -
                   (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].PROTOCOL_TYPE);

            packsize += (int)((char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[0].TRAFFIC_TYPE -
                              (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].PROTOCOL_TYPE);

            repeatnum4 = SWAP_INT32(k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].REAL_GRI_4_GROUP_REPEAT_NUM);

            for (int k = 0; k < repeatnum4; k++)
            {
                memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k], input + packsize,
                       MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k]));

                packsize = MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GRI_3_GROUP[j].GRI_4_GROUP[k]);
            }
        }

        memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.FPI_1, input + packsize,
               (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[0].URN_20 - (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.FPI_1);

        packsize += (int)((char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[0].URN_20 - (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.FPI_1);
        repeatnum5 = SWAP_INT32(k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.REAL_GRI_5_GROUP_REPEAT_NUM);

        for (int j = 0; j < repeatnum5; j++)
        {
            memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[j], input + packsize,
                   MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[j]));

            packsize += MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GPI_2_GROUP.GRI_5_GROUP[j]);
        }

        memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_3, input + packsize,
               (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[0].URN_27 -  (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_3);

        packsize += (int)((char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[0].URN_27 -  (char *)&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_3);
        repeatnum6 = SWAP_INT32(k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.REAL_FRI_1_REPEAT_NUM);

        for (int j = 0; j < repeatnum6; j++)
        {
            memcpy(&k001.GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[j], input + packsize,
                   MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[j]));

            packsize += MEMBERSIZE(K00_1, GRI_1_GROUP[i].GPI_1_GROUP.GPI_5_GROUP.FRI_1[j]);
        }
    }

    memcpy(output, &k001, sizeof(K00_1));

    return packsize;
}


static void *
msgproc_create_header(struct msgproc_interface *msgproc, int msgtype, unsigned char msggrp, unsigned char msgnum)
{
    void *header = NULL;
    struct tm *tm = NULL;
    struct timespec ts = {0};
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += get_time_offset();
        tm = localtime(&ts.tv_sec);

        if (!tm)
            return NULL;

        header = malloc(sizeof(KVMF_HEADER));

        if (header)
        {
            memset(header, 0x00, sizeof(KVMF_HEADER));
            ((KVMF_HEADER *)header)->dlpHeader.controlCode = SWAP_INT16(DLP_SEND_RECV_KVMF_MSG);
            ((KVMF_HEADER *)header)->dlpHeader.sender = SWAP_INT32(LINK_K_DLP);
            ((KVMF_HEADER *)header)->dlpHeader.receiver = SWAP_INT32(MSG_PROCESSOR);
            ((KVMF_HEADER *)header)->dlpHeader.dataSize = SWAP_INT16(sizeof(KVMF_HEADER));
            ((KVMF_HEADER *)header)->originatorUnitClassify = 0;
            ((KVMF_HEADER *)header)->originatorURN = SWAP_INT32(this->urn[0]);
            ((KVMF_HEADER *)header)->priority = 3;
            ((KVMF_HEADER *)header)->delayDTR = 0;
            ((KVMF_HEADER *)header)->reliabilityDTR = 0;
            ((KVMF_HEADER *)header)->gpi_1 = 1;
            ((KVMF_HEADER *)header)->realAddresseeGrpRepeatNum = SWAP_INT32(1);
            ((KVMF_HEADER *)header)->addresseeGrp[0].addresseeUnitClassify = 0;
            ((KVMF_HEADER *)header)->addresseeGrp[0].addresseeInfoURN = SWAP_INT32(this->urn[1]);
            ((KVMF_HEADER *)header)->gpi_2 = 1;
            ((KVMF_HEADER *)header)->realMsgHandleGrpRepeatNum = SWAP_INT32(1);

            if (msgtype == MESSAGE_TYPE_ORIGINAL)
                ((KVMF_HEADER *)header)->msgHandleGrp[0].realMsgLength = SWAP_INT32(msgproc_get_msgsize(msggrp, msgnum));
            else
                ((KVMF_HEADER *)header)->msgHandleGrp[0].realMsgLength = SWAP_INT32(0);

            ((KVMF_HEADER *)header)->msgHandleGrp[0].UMF = 15;
            ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_3 = 1;
            ((KVMF_HEADER *) header)->msgHandleGrp[0].vmfMsgIdentGrp.msgGroupFad = msggrp;
            ((KVMF_HEADER *) header)->msgHandleGrp[0].vmfMsgIdentGrp.msgNumber = msgnum;
            ((KVMF_HEADER *) header)->msgHandleGrp[0].vmfMsgIdentGrp.fpi_1 = 0;
            ((KVMF_HEADER *)header)->msgHandleGrp[0].fpi_2 = 0;
            ((KVMF_HEADER *)header)->msgHandleGrp[0].operationIndicator = 0;
            ((KVMF_HEADER *)header)->msgHandleGrp[0].retransmitIndicator = 0;

            if ((msggrp == MSGGRP_K02) && (msgnum == MSGNUM_OBREADY_REPORT))
                ((KVMF_HEADER *)header)->msgHandleGrp[0].msgPrecedenceCode = 1;
            else if ((msggrp == MSGGRP_K02) && (msgnum == MSGNUM_SUBSEQUENT_ADJUST))
                ((KVMF_HEADER *)header)->msgHandleGrp[0].msgPrecedenceCode = 1;
            else
                ((KVMF_HEADER *)header)->msgHandleGrp[0].msgPrecedenceCode = 0;

            if ((msggrp == MSGGRP_K02) && (msgnum == MSGNUM_TARGET_DATA))
                ((KVMF_HEADER *)header)->msgHandleGrp[0].securityClassification = 1;
            else if ((msggrp == MSGGRP_K02) && (msgnum == MSGNUM_OBREADY_REPORT))
                ((KVMF_HEADER *)header)->msgHandleGrp[0].securityClassification = 1;
            else
                ((KVMF_HEADER *)header)->msgHandleGrp[0].securityClassification = 0;

            if (msgtype == MESSAGE_TYPE_ORIGINAL)
            {
                ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_4 = 1;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.year = (unsigned char) (tm->tm_year - 100);
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.month = (unsigned char) (tm->tm_mon + 1);
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.day = (unsigned char) tm->tm_mday;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.hour = (unsigned char) tm->tm_hour;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.minute = (unsigned char) tm->tm_min;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.second = (unsigned char) tm->tm_sec;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].originatorDtgGrp.fpi_3 = 0;
            }
            else
                ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_4 = 0;

            ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_5 = 0;

            if (msgtype == MESSAGE_TYPE_ORIGINAL)
            {
                ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_6 = 1;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].ackReqGrp.machineAckReqIndicator = 1;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].ackReqGrp.operatorAckReqIndicator = 0;
                ((KVMF_HEADER *) header)->msgHandleGrp[0].ackReqGrp.operatorRepReqIndicator = 0;
            }
            else
                ((KVMF_HEADER *) header)->msgHandleGrp[0].gpi_6 = 0;

            if (msgtype == MESSAGE_TYPE_ORIGINAL)
                ((KVMF_HEADER *)header)->msgHandleGrp[0].gpi_7 = 0;
            else
            {
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.year = (unsigned char) (tm->tm_year - 100);
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.month = (unsigned char) (tm->tm_mon + 1);
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.day = (unsigned char) tm->tm_mday;
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.hour = (unsigned char) tm->tm_hour;
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.minute = (unsigned char) tm->tm_min;
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.second = (unsigned char) tm->tm_sec;
                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.fpi_4 = 0;

                if (msgtype == MESSAGE_TYPE_ACK)
                    ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.rc = 1;
                else
                    ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.rc = 2;

                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.fpi_5 = 0;

                if (msgtype == MESSAGE_TYPE_ACK)
                    ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.fpi_6 = 0;
                else
                    ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.cantproReasonCode = (char)msgtype;

                ((KVMF_HEADER *)header)->msgHandleGrp[0].responseDataGrp.fpi_7 = 0;
            }

            ((KVMF_HEADER *)header)->msgHandleGrp[0].gpi_8 = 0;
            ((KVMF_HEADER *)header)->msgHandleGrp[0].gpi_9 = 0;
        }
        else
            TLOGMSG(1, (DBGINFOFMT "failed to create message header\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));

    return header;
}


static void
msgproc_fill_k001_field(void *data, unsigned int originator, unsigned int addressee)
{
    struct tm *tm = NULL;
    struct timespec ts = {0};
    K00_1 *msg = (K00_1 *) data;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += get_time_offset();
    tm = localtime(&ts.tv_sec);

    if (tm)
    {
        msg->NUMBER_OF_SUB_NETWORKS = 1;
        msg->NETWORK_MONITORING_MESSAGE_TYPE = 0;
        msg->URN = SWAP_INT32(originator);
        msg->REAL_GRI_1_GROUP_REPEAT_NUM = SWAP_INT32(1);
        msg->GRI_1_GROUP[0].URN_3 = SWAP_INT32(addressee);
        msg->GRI_1_GROUP[0].REAL_GRI_2_GROUP_REPEAT_NUM = SWAP_INT32(1);
        msg->GRI_1_GROUP[0].GRI_2_GROUP[0].DATA_COLLECTION_DAY = (unsigned char) tm->tm_mday;
        msg->GRI_1_GROUP[0].GRI_2_GROUP[0].DATA_COLLECTION_HOUR = (unsigned char) tm->tm_hour;
        msg->GRI_1_GROUP[0].GRI_2_GROUP[0].DATA_COLLECTION_MINUTE = (unsigned char) tm->tm_min;
        msg->GRI_1_GROUP[0].GRI_2_GROUP[0].DATA_COLLECTION_SECOND = (unsigned char) tm->tm_sec;
        msg->GRI_1_GROUP[0].GPI_1 = 0;
    }
}


static void
msgproc_fill_k029_field(void *data, taqdata_t *taqdata)
{
    int feet = 0;
    K02_9 *msg = (K02_9 *) data;

    msg->TARGET_REPORT_TYPE = 0;
    msg->ACTION_DESIGNATOR  = 0;
    msg->FPI_1 = 0;
    msg->GPI_1 = 1;
    msg->GPI_1_GROUP.REAL_GRI_1_GROUP_REPEAT_NUM = SWAP_INT32(1);
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2 = 1;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_3 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4 = 1;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.REAL_GRI_2_GROUP_REPEAT_NUM = SWAP_INT32(1);
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].FPI_4 = 0;

    if (taqdata->target.latitude > 0.0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_LATITUDE = SWAP_INT32(LATITUDE(taqdata->target.latitude));
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_LATITUDE = SWAP_INT32(NEGATIVE_LATITUDE(taqdata->target.latitude));

    if (taqdata->target.longitude > 0.0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_LONGITUDE = SWAP_INT32(LONGITUDE(taqdata->target.longitude));
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_LONGITUDE = SWAP_INT32(NEGATIVE_LONGITUDE(taqdata->target.longitude));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].FPI_5 = 1;
    feet = FEETS(taqdata->target.altitude);

    if (feet >= 0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_ELEVATION__MSL = SWAP_INT32((unsigned int) feet);
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].TARGET_ELEVATION__MSL = SWAP_INT32(NEGATIVE_ALTITUDE(feet));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].FPI_6 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5 = 1;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.TARGET_LOCATION_STATUS_INDICATOR = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.FPI_7 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.FPI_8 = 1;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.TARGET_ACQUISITION_SOURCE_TYPE = 19;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.FPI_9 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.FPI_10 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_5_GROUP.FPI_11 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_6 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2_GROUP.GPI_4_GROUP.GRI_2_GROUP[0].GPI_8 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_9 = 1;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_9_GROUP.MOVING_TARGET_AZIMUTH = SWAP_INT16((unsigned short)lround(DEG2MIL(taqdata->observer.fwdaz)));
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_9_GROUP.MOVING_TARGET_SPEED = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_10 = 0;

    if (taqdata->target.shape == TARGET_SHAPE_CIRCLE)
    {
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11 = 1;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_18 = 0;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.GPI_12 = 0;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_19 = 1;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.RADIUS = SWAP_INT16(taqdata->target.radius);
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_20 = 0;
    }
    else if (taqdata->target.shape == TARGET_SHAPE_SQUARE)
    {
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11 = 1;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_18 = 0;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.GPI_12 = 1;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.GPI_12_GROUP.LENGTH = SWAP_INT16(taqdata->target.length);
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.GPI_12_GROUP.WIDTH  = SWAP_INT16(taqdata->target.width);
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.GPI_12_GROUP.ATTITUDE = SWAP_INT16(taqdata->target.attitude);
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_19 = 0;
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11_GROUP.FPI_20 = 0;
    }
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_11 = 0;

    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_13 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_14 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_15 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_16 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_38 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_39 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_17 = 0;
}


static void
msgproc_fill_k0222_field(void *data, taqdata_t *taqdata)
{
    K02_22 *msg = (K02_22 *) data;

    msg->SUBSEQUENT_ADJUSTMENT_TYPE = 0;
    msg->TARGET_NUMBER_ASC[0] = 'A';
    msg->TARGET_NUMBER_ASC[1] = 'A';
    msg->TARGET_NUMBER_DEC = SWAP_INT16((unsigned short) 9999);
    msg->GUN_TARGET_LINE_INDICATOR = 0;
    msg->FPI_1 = 1;
    msg->REAL_FRI_1_REPEAT_NUM = SWAP_INT32(1);
    msg->FRI_1[0].OBSERVER_TARGET_AZIMUTH = SWAP_INT16((unsigned short)lround(DEG2MIL(taqdata->shift.fwdaz)));
    msg->FPI_2 = 0;
    msg->FPI_3 = 0;
    msg->FPI_4 = 0;
    msg->FPI_5 = 0;
    msg->GPI_1 = 1;
    msg->GPI_1_GROUP.REAL_GRI_1_GROUP_REPEAT_NUM = SWAP_INT32(1);
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_6 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_7 = 1;

    if (taqdata->shift.lateral >= 0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].LATERAL_SHIFT = SWAP_INT16((unsigned short)taqdata->shift.lateral);
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].LATERAL_SHIFT = SWAP_INT16(NEGATIVE_SHIFT(taqdata->shift.lateral));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_8 = 1;

    if (taqdata->shift.range >= 0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].RANGE_SHIFT = SWAP_INT16((unsigned short)taqdata->shift.range);
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].RANGE_SHIFT = SWAP_INT16(NEGATIVE_SHIFT(taqdata->shift.range));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_9 = 0;
    msg->GPI_2 = 0;
    msg->GPI_3 = 0;
    msg->GPI_4 = 0;
    msg->GPI_5 = 0;
    msg->GPI_6 = 0;
    msg->GPI_7 = 0;
    msg->GPI_10 = 0;
    msg->GPI_11 = 0;
    msg->GPI_13 = 0;
    msg->GPI_14 = 0;
    msg->GPI_15 = 0;
}


static void
msgproc_fill_k0237_field(void *data, taqdata_t *taqdata, unsigned int urn)
{
    int feet = 0;
    K02_37 *msg = (K02_37 *) data;

    msg->URN = SWAP_INT32(urn);
    msg->GPI_1 = 1;
    msg->GPI_1_GROUP.REAL_GRI_1_GROUP_REPEAT_NUM = SWAP_INT32(1);

    if (taqdata->observer.latitude > 0.0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_LATITUDE = SWAP_INT32(LATITUDE((taqdata->observer.latitude)));
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_LATITUDE = SWAP_INT32(NEGATIVE_LATITUDE((taqdata->observer.latitude)));

    if (taqdata->observer.longitude > 0.0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_LONGITUDE = SWAP_INT32(LONGITUDE((taqdata->observer.longitude)));
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_LONGITUDE = SWAP_INT32(NEGATIVE_LONGITUDE((taqdata->observer.longitude)));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_1 = 1;
    feet = FEETS(taqdata->observer.altitude);

    if (feet >= 0)
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_ELEVATION__MSL = SWAP_INT32((unsigned int) feet);
    else
        msg->GPI_1_GROUP.GRI_1_GROUP[0].OBSERVER_LOCATION_ELEVATION__MSL = SWAP_INT32(NEGATIVE_ALTITUDE(feet));

    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_2 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_3 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_4 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_5 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_6 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_2 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].GPI_4 = 0;
    msg->GPI_1_GROUP.GRI_1_GROUP[0].FPI_10 = 0;
}


static void
msgproc_fill_k051_field(void *data, taqdata_t *taqdata, unsigned int urn)
{
    int feet = 0;
    K05_1 *msg = (K05_1 *) data;

    msg->REAL_GRI_1_GROUP_REPEAT_NUM = SWAP_INT32(1);
    msg->GRI_1_GROUP[0].URN = SWAP_INT32(urn);

    if (taqdata->observer.latitude > 0.0)
        msg->GRI_1_GROUP[0].UNIT_LATITUDE = SWAP_INT32(LATITUDE((taqdata->observer.latitude)));
    else
        msg->GRI_1_GROUP[0].UNIT_LATITUDE = SWAP_INT32(NEGATIVE_LATITUDE((taqdata->observer.latitude)));

    if (taqdata->observer.longitude > 0.0)
        msg->GRI_1_GROUP[0].UNIT_LONGITUDE = SWAP_INT32(LONGITUDE((taqdata->observer.longitude)));
    else
        msg->GRI_1_GROUP[0].UNIT_LONGITUDE = SWAP_INT32(NEGATIVE_LONGITUDE((taqdata->observer.longitude)));

    msg->GRI_1_GROUP[0].LOCATION_DERIVATION = 1;
    msg->GRI_1_GROUP[0].FPI_1 = 0;
    msg->GRI_1_GROUP[0].EXERCISE_INDICATOR = 0;
    msg->GRI_1_GROUP[0].GPI_1 = 0;
    msg->GRI_1_GROUP[0].FPI_2 = 1;
    feet = FEETS(taqdata->observer.altitude);

    if (feet >= 0)
        msg->GRI_1_GROUP[0].ELEVATION__FEET = SWAP_INT32((unsigned int) feet);
    else
        msg->GRI_1_GROUP[0].ELEVATION__FEET = SWAP_INT32(NEGATIVE_ALTITUDE(feet));

    msg->GRI_1_GROUP[0].FPI_3 = 0;
    msg->GRI_1_GROUP[0].GPI_2 = 0;
    msg->GRI_1_GROUP[0].GPI_3 = 0;
    msg->GRI_1_GROUP[0].ORIGINATOR_ENVIRONMENT = 2;
    msg->GRI_1_GROUP[0].GPI_4 = 1;
    msg->GRI_1_GROUP[0].GPI_4_GROUP.FPI_7 = 0;
    msg->GRI_1_GROUP[0].GPI_4_GROUP.FPI_8 = 0;
    msg->GRI_1_GROUP[0].GPI_4_GROUP.FPI_9 = 0;
    msg->GRI_1_GROUP[0].GPI_4_GROUP.FPI_10 = 1;
    msg->GRI_1_GROUP[0].GPI_4_GROUP.LAND_SPECIFIC_TYPE = SWAP_INT16((unsigned short)8);
}


static void *
msgproc_create_body(struct msgproc_interface *msgproc, unsigned char msggrp, unsigned char msgnum, taqdata_t *data)
{
    size_t size = 0;
    void *body = NULL;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        size = msgproc_get_msgsize(msggrp, msgnum);

        if (size != 0)
        {
            body = malloc(size);

            if (body)
            {
                memset(body, 0x00, size);

                if (msggrp == MSGGRP_K00)
                {
                    if (msgnum == MSGNUM_NETWORK_MONITOR)
                        msgproc_fill_k001_field(body, this->urn[0], this->urn[1]);
                    else
                    {
                        free(body);
                        body = NULL;
                        TLOGMSG(1, (DBGINFOFMT "invalid message number for K00 message group\n", DBGINFO));
                    }
                }
                else if (msggrp == MSGGRP_K02)
                {
                    if (msgnum == MSGNUM_TARGET_DATA)
                        msgproc_fill_k029_field(body, data);
                    else if (msgnum == MSGNUM_SUBSEQUENT_ADJUST)
                        msgproc_fill_k0222_field(body, data);
                    else if (msgnum == MSGNUM_OBREADY_REPORT)
                        msgproc_fill_k0237_field(body, data, this->urn[1]);
                    else
                    {
                        free(body);
                        body = NULL;
                        TLOGMSG(1, (DBGINFOFMT "invalid message number for K02 message group\n", DBGINFO));
                    }
                }
                else if (msggrp == MSGGRP_K05)
                {
                    if (msgnum == MSGNUM_POSITION_REPORT)
                        msgproc_fill_k051_field(body, data, this->urn[1]);
                    else
                    {
                        free(body);
                        body = NULL;
                        TLOGMSG(1, (DBGINFOFMT "invalid message number for K05 message group\n", DBGINFO));
                    }
                }
                else
                {
                    free(body);
                    body = NULL;
                    TLOGMSG(1, (DBGINFOFMT "invalid message group\n", DBGINFO));
                }
            }
            else
                TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid message size\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));

    return body;
}


static void *
msgproc_create_kvmfmsg(struct msgproc_interface *msgproc, unsigned char msggrp, unsigned char msgnum, taqdata_t *taqdata, void *msg, size_t *msglen)
{
    char buf[1024] = {0};
    void *header = NULL;
    void *body = NULL;
    size_t bodysize = 0;
    size_t packsize = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        bodysize = msgproc_get_msgsize(msggrp, msgnum);

        if ( bodysize != 0)
        {
            header = msgproc_create_header(msgproc, MESSAGE_TYPE_ORIGINAL, msggrp, msgnum);
            body = msgproc_create_body(msgproc, msggrp, msgnum, taqdata);

            if (header == NULL)
            {
                free(body);
                TLOGMSG(1, (DBGINFOFMT "failed to create message header\n", DBGINFO));
            }
            else if (body == NULL)
            {
                free(header);
                TLOGMSG(1, (DBGINFOFMT "failed to create message body\n", DBGINFO));
            }
            else
            {
                memset(buf, 0x00, sizeof(buf));
                packsize = (size_t) msgproc_pack_header(header, buf);
                memcpy(msg, buf, packsize);
                memcpy(msg + packsize, body, bodysize);
                free(header);
                free(body);
                *msglen = bodysize + packsize;
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid message\n", DBGINFO));
    }
    else
    {
        msg = NULL;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return msg;
}


static void
msgproc_wait_ack(struct msgproc_interface *msgproc)
{
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this->txStatus == TXSTAT_IDLE)
    {
        this->txStatus = TXSTAT_WAIT_ACK;
        pthread_mutex_lock(&this->mtx);

        while (this->txStatus == TXSTAT_WAIT_ACK)
            pthread_cond_wait(&this->cond, &this->mtx);

        pthread_mutex_unlock(&this->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "mcu is waiting ack\n", DBGINFO));
}


static void
msgproc_recv_ack(struct msgproc_interface *msgproc)
{
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this->txStatus == TXSTAT_WAIT_ACK)
    {
        pthread_mutex_lock(&this->mtx);
        this->txStatus = TXSTAT_RECV_ACK;
        pthread_mutex_unlock(&this->mtx);
        pthread_cond_signal(&this->cond);
    }
    else
        TLOGMSG(0, (DBGINFOFMT "msgproc is not waiting for ack\n", DBGINFO));

}


static void
msgproc_timeout(struct msgproc_interface *msgproc)
{
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this->txStatus == TXSTAT_WAIT_ACK)
    {
        pthread_mutex_lock(&this->mtx);
        this->txStatus = TXSTAT_TIMEOUT;
        pthread_mutex_unlock(&this->mtx);
        pthread_cond_signal(&this->cond);
    }
    else;
        TLOGMSG(0, (DBGINFOFMT "msgproc is not waiting for ack\n", DBGINFO));
}


static int
msgproc_transmit_kvmfmsg(struct msgproc_interface *msgproc, unsigned char msggrp, unsigned char msgnum, void *data)
{
    int ret = -1;
    int retry = 0;
    char msg[40960] = {0};
    size_t msglen = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (this->dlpif.inProc)
        {
            if ((msgproc_create_kvmfmsg(msgproc, msggrp, msgnum, (taqdata_t *)data, (void *)msg, &msglen)) != NULL)
            {
                for (int i = 0; i < MAX_RETRY_XMIT; i++)
                {
                    if (write(this->socket[0], msg, msglen) != -1)
                    {
                        TLOGMSG(1, ("transmit K0%d.%d message to kvmfproc\n", msggrp, msgnum));
                        clock_gettime(CLOCK_REALTIME, &g_txtime);
                        msgproc_wait_ack(msgproc);

                        if (this->txStatus == TXSTAT_RECV_ACK)
                        {
                            this->txStatus = TXSTAT_IDLE;
                            ret = g_xmit_result;
                            break;
                        }
                        else
                        {
                            if (i < MAX_RETRY_XMIT)
                                TLOGMSG(1, ("reponse timeout - retransmit message %d\n", i + 1));
                            else
                                TLOGMSG(1, ("reponse timeout\n"));

                            this->txStatus = TXSTAT_IDLE;
                            continue;
                        }
                    }
                    else
                        TLOGMSG(1, (DBGINFOFMT "write return failed\n", DBGINFO));
                }
            }
            else
                TLOGMSG(1, (DBGINFOFMT "failed to create message\n", DBGINFO));
        }
        else
            TLOGMSG(1, (DBGINFOFMT "dlpif is not established\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));

    return ret;
}


static int
msgproc_transmit_ack(struct msgproc_interface *msgproc, int msgtype)
{
    int ret = 0;
    char buf[1024] = {0};
    void *header = NULL;
    size_t packsize = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        header = msgproc_create_header(msgproc, msgtype, MSGGRP_K00, MSGNUM_NETWORK_MONITOR);

        if (header)
        {
            memset(buf, 0x00, sizeof(buf));
            packsize = (size_t) msgproc_pack_header(header, buf);

            if (write(this->socket[0], buf, packsize) != -1)
                TLOGMSG(1, (DBGINFOFMT "transmit ack to kvmfproc\n", DBGINFO));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "write return failed\n", DBGINFO));
            }

            free(header);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to make header\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_exec_kvmfproc(struct msgproc_interface *msgproc)
{
    int ret = 0;
    int count = 0;
    char *argv[] = {PATH_KVMF_PROCESSOR, NULL};
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (!this->dlpif.kvmfProc)
        {
            if (posix_spawn(&this->dlpif.pid, PATH_KVMF_PROCESSOR, NULL, NULL, argv, environ) == 0)
            {
                signal(SIGUSR1, handle_kvmfproc_signal);

                while (1)
                {
                    if (g_kvmfproc_signal)
                    {
                        this->dlpif.kvmfProc = true;
                        TLOGMSG(1, ("start kvmf processor (pid : %i)\n", this->dlpif.pid));
                        break;
                    }
                    else
                    {
                        if (count < 50)
                        {
                            MSLEEP(100);
                            count++;
                        }
                        else
                        {
                            ret = -1;
                            kill(this->dlpif.pid, SIGKILL);
                            TLOGMSG(1, (DBGINFOFMT "no response from kvmfproc\n", DBGINFO));
                            break;
                        }
                    }
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "posix_spwan return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "kvmf processor is already running\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_kill_kvmfproc(struct msgproc_interface *msgproc)
{
    int ret = 0;
    int status = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (this->dlpif.kvmfProc)
        {
            kill(this->dlpif.pid, SIGKILL);

            if (waitpid(this->dlpif.pid, &status, 0) != -1)
            {
                if (WIFEXITED(status))
                    TLOGMSG(1, ("kvmfproc exited, status = %d \n", WEXITSTATUS(status)));
                else if (WIFSIGNALED(status))
                    TLOGMSG(1, ("kvmfproc killed by signal %d (%s)\n", WSTOPSIG(status), strsignal(WSTOPSIG(status))));
                else
                    TLOGMSG(1, ("unknown kvmfproc status = %d\n", status));

                this->dlpif.kvmfProc = false;
                g_kvmfproc_signal = false;
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "waitpid return fail\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "kvmfproc already killed\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_init_socket(struct msgproc_interface *msgproc, int target)
{
    int ret = 0;
    int sock = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (this->dlpif.kvmfProc)
        {
            if (target == MSGPROC_TCP)
            {
                sock = socket(PF_INET, SOCK_STREAM, 0);

                if (sock != -1)
                {
                    if (connect(sock, (struct sockaddr *)&this->addr[KVMFPROC_TCP], sizeof(struct sockaddr)) == 0)
                    {
                        this->socket[0] = sock;
                        TLOGMSG(1, ("initailized tcp socket\n"));
                    }
                    else
                    {
                        ret = -1;
                        close(sock);
                        TLOGMSG(1, (DBGINFOFMT "failed to connect kvmfproc\n", DBGINFO));
                    }
                }
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "failed to create tcp socket, %s\n", DBGINFO, strerror(errno)));
                }
            }
            else if (target == MSGPROC_UDP)
            {
                sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

                if (sock != -1)
                {
                    if (bind(sock, (struct sockaddr *)&this->addr[MSGPROC_UDP], sizeof(struct sockaddr)) != 0)
                    {
                        ret = -1;
                        close(sock);
                        TLOGMSG(1,(DBGINFOFMT "failed to bind, %s\n", DBGINFO, strerror(errno)));
                    }
                    else
                    {
                        this->socket[1] = sock;
                        TLOGMSG(1, ("initailized udp socket\n"));
                    }
                }
                else
                {
                    ret = -1;
                    TLOGMSG(1, (DBGINFOFMT "failed to create udp socket\n", DBGINFO));
                }
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "invalid argument\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "kvmfproc is not running\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_set_urn(struct msgproc_interface *msgproc, unsigned int src, unsigned int dst)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        this->urn[0] = src;
        this->urn[1] = dst;
    }
    else
    {
        ret = 1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_get_urn(struct msgproc_interface *msgproc, unsigned int *src, unsigned int *dst)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        *src = this->urn[0];
        *dst = this->urn[1];
    }
    else
    {
        ret = 1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_set_ipaddr(struct msgproc_interface *msgproc, int target, char *ipaddr, char *port)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        switch (target)
        {
            case MSGPROC_TCP:
            case MSGPROC_UDP:
            case KVMFPROC_TCP:
            case KVMFPROC_UDP:
                this->addr[target].sin_family = AF_INET;
                this->addr[target].sin_addr.s_addr = inet_addr(ipaddr);
                this->addr[target].sin_port = htons(atoi(port));
                break;

            default:
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "invalid target to set ipaddr\n", DBGINFO));
                break;
        }
    }
    else
    {
        ret = 1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_check_message(char *msg)
{
    int ret = 0;
    int size = 0;
    K00_1 k001 = {0};
    KVMF_HEADER head = {0};

    memset(&head, 0x00, sizeof(KVMF_HEADER));
    size = msgproc_unpack_header(msg, (char *)&head);

#ifdef _MSGPROC_UNIT_TEST_
    msgproc_print_kvmfhead(&head);
#endif

    if (head.msgHandleGrp[0].responseDataGrp.rc == 2)
    {
        ret = head.msgHandleGrp[0].responseDataGrp.cantproReasonCode;
        TLOGMSG(1, ("received nack (cantpro %d)\n"));
    }
    else
    {
        ret = head.msgHandleGrp[0].responseDataGrp.rc;
        TLOGMSG(1, ("received ack\n"));
    }

/*
    if ((head.gpi_1) && (head.gpi_2) && (head.msgHandleGrp[0].gpi_3))
    {
        if ((head.msgHandleGrp[0].vmfMsgIdentGrp.msgGroupFad == 0) && (head.msgHandleGrp[0].vmfMsgIdentGrp.msgNumber == 1))
        {
            memset(&k001, 0x00, sizeof(K00_1));
            msgproc_unpack_k001(msg + size, (char *)&k001);

            if (SWAP_INT32(k001.GRI_1_GROUP[0].URN_3) == URN_SOURCE)
                ret = MESSAGE_TYPE_ACK;
            else
            {
                ret = MESSAGE_TYPE_NACK_CANTPRO19;
                TLOGMSG(1, (DBGINFOFMT "unknown addressee urn\n. discard received message\n", DBGINFO));
            }

#ifdef _MSGPROC_UNIT_TEST_
            msgproc_print_k001(&k001);
#endif
        }
        else
        {
            ret = MESSAGE_TYPE_NACK_CANTPRO15;
            TLOGMSG(1, (DBGINFOFMT "can't parse K%02d.%d message. discard received message\n", DBGINFO,
                    head.msgHandleGrp[0].vmfMsgIdentGrp.msgGroupFad, head.msgHandleGrp[0].vmfMsgIdentGrp.msgNumber));
        }
    }
    else
    {
        ret = MESSAGE_TYPE_NACK_CANTPRO2;
        TLOGMSG(1, (DBGINFOFMT "invalid header data. discard received message\n", DBGINFO));
    }
*/

    return ret;
}


static int
msgproc_parse_dlpmsg(struct msgproc_interface *msgproc, char *msg, int msglen)
{
    int ret = 0;
    DlpCommonHeader dlphdr = {0};
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    memcpy(&dlphdr, msg, sizeof(DlpCommonHeader));

    switch(SWAP_INT16(dlphdr.controlCode))
    {
        case DLP_INTERFACE_START_ACKNOWLEDGE_MSG:
            this->dlpif.inProc = true;
            TLOGMSG(1, ("start DLP INTERFACE\n"));
            break;

        case DLP_INTERFACE_START_NOT_ACKNOWLEDGE_MSG:
            this->dlpif.inProc = false;
            TLOGMSG(1, ("failed to start DLP INTERFACE\n"));
            break;

        case DLP_HEARTBEAT_MSG:
            if (this->txStatus == TXSTAT_WAIT_ACK)
            {
                clock_gettime(CLOCK_REALTIME, &ts);

                if (get_elapsed_time(&g_txtime, &ts) > MSGPROC_TIMEOUT)
                    msgproc_timeout(msgproc);
            }

            this->dlpif.heartBeat = true;
            TLOGMSG(0, ("received DLP_HEARTBEAT_MSG\n"));
            break;

        case DLP_SEND_RECV_KVMF_MSG:
#ifdef _MSGPROC_UNIT_TEST_
            //msgproc_dump_dlpmsg(msg, msglen);
#endif
            if (this->txStatus == TXSTAT_WAIT_ACK)
            {
                g_xmit_result = msgproc_check_message(msg);
                msgproc_recv_ack(msgproc);
            }
            else
                TLOGMSG(1, ("received kvmf message but amod is not in wait ack status. discard message.\n"));

            break;

        default:
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "unknown message type\n", DBGINFO));
            break;
    }

    return ret;
}


static void *
msgproc_recv_tcp(void *arg)
{
    int ret = 0;
    int nread = 0;
    char buf[4096] = {0};
    struct msgproc_interface *msgproc = (struct msgproc_interface *) arg;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;
    struct pollfd pfd = {.fd = this->socket[0], .events = POLLIN, .revents = 0};

    while (!this->exitThread[0])
    {
        ret = poll(&pfd, 1, 1000);

        if (ret > 0)
        {
            ioctl(this->socket[0], FIONREAD, &nread);

            if (nread > 0)
            {
                if (read(this->socket[0], buf, nread) == nread)
                    msgproc_parse_dlpmsg(msgproc, buf, nread);
                else
                    continue;
            }
            else
                continue;
        }
        else
            continue;
    }

    return NULL;
}


static char *
msgproc_make_packet(char *data, int data_size)
{
    char *packet = NULL;
    unsigned short crc = 0;
    unsigned short len = 0;
    unsigned short swap_len = 0;
    unsigned short swap_crc = 0;

    packet = (char *) malloc(data_size + MIN_PACKET_LENGTH);

    if (packet)
    {
        memset(packet, 0x00, data_size + MIN_PACKET_LENGTH);

        /* packet length calculation and fill length field */
        len = (unsigned short)data_size;
        swap_len = SWAP_INT16(len);
        memcpy(packet + 1, &swap_len, sizeof(unsigned short));

        /* copy data to packet */
        memcpy(packet + 3, data, data_size);

        /* crc calculation and fill crc field */
        crc = crc16_ccitt(packet + 1, data_size + 2);
        swap_crc = SWAP_INT16(crc);
        memcpy(packet + data_size + 3, &swap_crc, sizeof(unsigned short));

        /* attach SOM and EOM */
        *(packet) = PACKET_SOM;
        *(packet + data_size + 5) = PACKET_EOM;

        /* print raw data of packet */
        LOGMSG(1, ("\ngenerated packet to trasmit\n"));
        msgproc_print_packet(packet, data_size + MIN_PACKET_LENGTH);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return packet;
}


static void *
msgproc_recv_udp(void *arg)
{
    int nrx = 0;
    char *packet = NULL;
    char buf[2048] = {0};
    socklen_t addrlen = sizeof (struct sockaddr_in);
    struct sockaddr_in addr = {0};
    struct msgproc_interface *msgproc = (struct msgproc_interface *) arg;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    while(!this->exitThread[1])
    {
        nrx = recvfrom(this->socket[1], buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);

        if (nrx > 0)
        {
            msgproc_print_kvmf_data(buf, nrx, &addr);

#ifdef _KVTT_TEST_
            this->uart->write(this->uart, buf, nrx);
            TLOGMSG(1, ("transmitted %d bytes to KVTT\n", nrx));
#else
            packet = msgproc_make_packet(buf, nrx);

            if (packet)
            {
                this->uart->write(this->uart, packet, nrx + MIN_PACKET_LENGTH);
                free(packet);
                TLOGMSG(1, ("transmitted %d bytes to DM\n", nrx + MIN_PACKET_LENGTH));
            }
            else
                TLOGMSG(1, (DBGINFOFMT "failed to make packet\n", DBGINFO));

#endif  /* _KVTT_TEST_ */

        }
        else
        {
            TLOGMSG(1, ("nrx = %d, %s\n", nrx, strerror(errno)));
            continue;
        }
    }

    return NULL;
}


static int
msgproc_assemble_packet(struct uart_interface *uart, int nread, char *packet, int *offset)
{
    int ret = ASSEMBLE_PACKET_INPROC;
    int idx = *offset;
    char data = 0;
    unsigned short length = 0;
    unsigned short swap_len = 0;

    for (int i = 0; i < nread; i++)
    {
        if (uart->read(uart, &data, 1) == 1)
        {
            if (data == PACKET_SOM)
            {
                if (*packet != PACKET_SOM)
                    TLOGMSG(1, ("found packet start (0x7E)\n"));

                *(packet + idx++) = data;
            }
            else if (data == PACKET_EOM)
            {
                if (*packet == PACKET_SOM)
                {
                    /* packet length check */
                    if (idx > MIN_PACKET_LENGTH)
                    {
                        /**/
                        memcpy(&swap_len, (void *) (packet + 1), sizeof(unsigned short));
                        length = SWAP_INT16(swap_len) + MIN_PACKET_LENGTH;

                        if ((idx + 1) == length)
                        {
                            *(packet + idx) = data;
                            ret = ASSEMBLE_PACKET_OK;
                            TLOGMSG(1, ("found packet end (0xE7)\n"));
                            TLOGMSG(1, ("received %dbytes from DM\n", length));
                            break;
                        }
                        else if ((idx + 1) > length)
                        {
                            TLOGMSG(1, (DBGINFOFMT "packet length mismatch\n", DBGINFO));
                            ret = ASSEMBLE_PACKET_LENGTH_MISMATCH;
                            break;
                        }
                        else
                            *(packet + idx++) = data;
                    }
                    else
                        *(packet + idx++) = data;
                }
                else
                    continue;
            }
            else
            {
                if (*packet == PACKET_SOM)
                    *(packet + idx++) = data;
                else
                    continue;
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to read\n", DBGINFO));
            break;
        }
    }

    *offset = idx;

    return ret;
}


static void *
msgproc_recv_uart(void *arg)
{
    int assem = 0;
    int offset = 0;
    int nread = 0;
    char data = 0;
    char pkt[128] = {0};
    char str[128] = {0};
    unsigned short crc = 0;
    socklen_t socklen = (socklen_t) sizeof(struct sockaddr_in);
    struct msgproc_interface *msgproc = (struct msgproc_interface *) arg;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;
    struct pollfd pfd = {.fd = this->uart->getFileDesc(this->uart), .events = POLLIN, .revents = 0};
    struct timespec t1 = {.tv_sec = 0, .tv_nsec = 0};
    struct timespec t2 = {.tv_sec = 0, .tv_nsec = 0};

    tcflush(this->uart->getFileDesc(this->uart), TCIOFLUSH);
    memset(pkt, 0x00, sizeof(pkt));
    memset(str, 0x00, sizeof(str));

    while(!this->exitThread[2])
    {
        if (poll(&pfd, 1, 1000) > 0)
        {
            resume_assembling_packet:
            nread = this->uart->getNumRead(this->uart);

            if (nread > 0)
            {
                assem = msgproc_assemble_packet(this->uart, nread, pkt, &offset);

                switch(assem)
                {
                    case ASSEMBLE_PACKET_OK:
                        TLOGMSG(1, ("received packet data from DM\n"));
                        msgproc_print_packet(pkt, offset + 1);

                        // check crc
                        memcpy(&crc, pkt + offset - 2, sizeof(unsigned short));

                        if (SWAP_INT16(crc) == crc16_ccitt(pkt + 1, offset - 3))
                            sendto(this->socket[1], pkt + 3, offset - 5, 0, (struct sockaddr *)&this->addr[KVMFPROC_UDP], socklen);
                        else
                            TLOGMSG(1, (DBGINFOFMT "crc mismatch (0x%04X), discard recevied packet\n", DBGINFO, SWAP_INT16(crc)));

                        offset = 0;
                        memset(pkt, 0x00, sizeof(pkt));
                        break;

                    case ASSEMBLE_PACKET_LENGTH_MISMATCH:
                        offset = 0;
                        memset(pkt, 0x00, sizeof(pkt));
                        break;

                    default:
                        clock_gettime(CLOCK_REALTIME, &t1);
                        TLOGMSG(0, ("assembling packet is in process\n"));
                        break;
                }

                if (this->uart->getNumRead(this->uart) != 0)
                    goto resume_assembling_packet;
                else
                    continue;
            }
            else
            {
                TLOGMSG(1, (DBGINFOFMT "nothing to read from uart\n", DBGINFO));
                continue;
            }
        }
        else
        {
            if (assem == ASSEMBLE_PACKET_INPROC)
            {
                clock_gettime(CLOCK_REALTIME, &t2);

                if (get_elapsed_time(&t1, &t2) > 3.0)
                {
                    offset = 0;
                    assem = ASSEMBLE_PACKET_OK;
                    memset(pkt, 0x00, sizeof(pkt));
                    TLOGMSG(1, (DBGINFOFMT "timeout occured while receiving packet, discard received data\n", DBGINFO));
                }
                else
                    continue;
            }
            else
                continue;
        }
    }

    return NULL;
}


static int
msgproc_init_interface(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        this->exitThread[0] = false;
        this->exitThread[1] = false;
        this->exitThread[2] = false;

        if(msgproc_init_socket(msgproc, MSGPROC_TCP) != 0)
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create failed to init tcp socket\n", DBGINFO));
            goto func_exit;
        }

        if (pthread_create(&this->tid[0], NULL, msgproc_recv_tcp, (void *)msgproc) != 0)
        {
            ret = -1;
            close(this->socket[0]);
            TLOGMSG(1, (DBGINFOFMT "failed to create msgproc_recv_tcp thread\n", DBGINFO));
            goto func_exit;
        }

        if(msgproc_init_socket(msgproc, MSGPROC_UDP) != 0)
        {
            ret = -1;
            this->exitThread[0] = true;
            pthread_join(this->tid[0], NULL);
            close(this->socket[0]);
            TLOGMSG(1, (DBGINFOFMT "failed to create msgproc_recv_udp thread\n", DBGINFO));
            goto func_exit;
        }

        if (pthread_create(&this->tid[1], NULL, msgproc_recv_udp, (void *)msgproc) != 0)
        {
            ret = -1;
            this->exitThread[0] = true;
            pthread_join(this->tid[0], NULL);
            close(this->socket[0]);
            close(this->socket[1]);
            TLOGMSG(1, (DBGINFOFMT "failed to create msgproc_recv_udp thread\n", DBGINFO));
            goto func_exit;
        }

        if (pthread_create(&this->tid[2], NULL, msgproc_recv_uart, (void *)msgproc) != 0)
        {
            ret = -1;

            for (int i = 0; i < 2; i++)
            {
                this->exitThread[i] = true;
                pthread_join(this->tid[i], NULL);
                close(this->socket[i]);
            }

            TLOGMSG(1, (DBGINFOFMT "failed to create msgproc_recv_uart thread\n", DBGINFO));
            goto func_exit;
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    func_exit:
    return ret;
}



static int
msgproc_transmit_dlpmsg(struct msgproc_interface *msgproc, int ctrlcode)
{
    int ret = 0;
    void *dlpmsg = NULL;
    size_t msglen = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (this->dlpif.kvmfProc)
        {
            switch(ctrlcode)
            {
            case DLP_INTERFACE_START_MSG:
                dlpmsg = malloc(sizeof(DlpInterfaceStartMsg));

                if (dlpmsg)
                {
                    memset(dlpmsg, 0x00, sizeof(DlpInterfaceStartMsg));
                    ((DlpInterfaceStartMsg *)dlpmsg)->dlpHeader.controlCode = SWAP_INT16(DLP_INTERFACE_START_MSG);
                    ((DlpInterfaceStartMsg *)dlpmsg)->dlpHeader.sender      = SWAP_INT32(LINK_K_DLP);
                    ((DlpInterfaceStartMsg *)dlpmsg)->dlpHeader.receiver    = SWAP_INT32(MSG_PROCESSOR);
                    ((DlpInterfaceStartMsg *)dlpmsg)->dlpHeader.dataSize    = SWAP_INT16(sizeof(DlpInterfaceStartMsg));
                    msglen = sizeof(DlpInterfaceStartMsg);
                }
                else
                    TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

                break;

            case DLP_START_SETTING_MSG:
                dlpmsg = malloc(sizeof(DlpStartSetMsg));

                if (dlpmsg)
                {
                    memset(dlpmsg, 0x00, sizeof(DlpStartSetMsg));
                    ((DlpStartSetMsg *)dlpmsg)->dlpHeader.controlCode = SWAP_INT16(DLP_START_SETTING_MSG);
                    ((DlpStartSetMsg *)dlpmsg)->dlpHeader.sender      = SWAP_INT32(LINK_K_DLP);
                    ((DlpStartSetMsg *)dlpmsg)->dlpHeader.receiver    = SWAP_INT32(MSG_PROCESSOR);
                    ((DlpStartSetMsg *)dlpmsg)->dlpHeader.dataSize    = SWAP_INT16((sizeof(DlpStartSetMsg)));
                    ((DlpStartSetMsg *)dlpmsg)->myNodeURN        = SWAP_INT32(this->urn[0]);
                    ((DlpStartSetMsg *)dlpmsg)->appProtVersion   = 13;
                    ((DlpStartSetMsg *)dlpmsg)->dataCompressType = 0;
                    ((DlpStartSetMsg *)dlpmsg)->msgStdVersion    = 2;
                    ((DlpStartSetMsg *)dlpmsg)->useMsgSecurity   = 0;
                    ((DlpStartSetMsg *)dlpmsg)->srByteSize       = SWAP_INT16(496);
                    ((DlpStartSetMsg *)dlpmsg)->msgACKSecTimeout = SWAP_INT16(3000);
                    ((DlpStartSetMsg *)dlpmsg)->retransmitUse    = 0;
                    msglen = sizeof(DlpStartSetMsg);
                }
                else
                    TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

                break;

            default:
                TLOGMSG(1, (DBGINFOFMT "invalid control code for dlp interface\n", DBGINFO));
                break;
            }

            if (dlpmsg)
            {
                write(this->socket[0], dlpmsg, msglen);
                free(dlpmsg);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to create dlpmsg\n", DBGINFO));
            }

        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "kvmfproc is not running\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}



static int
msgproc_start_interface(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        if (!this->dlpif.inProc)
        {
            for (int i = 0; i < 3; i++)
            {
                if (msgproc_transmit_dlpmsg(msgproc, DLP_INTERFACE_START_MSG) == 0)
                {
                    MSLEEP(1000);

                    if (this->dlpif.inProc)
                    {
                        msgproc_transmit_dlpmsg(msgproc, DLP_START_SETTING_MSG);
                        break;
                    }
                    else
                        continue;
                }
                else
                {
                    TLOGMSG(1, (DBGINFOFMT "failed to transmit dlpmsg\n", DBGINFO));
                    break;
                }
            }

            if (this->dlpif.inProc)
                TLOGMSG(1, ("dlpif is established\n"));
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to dlpif\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "dlpif is already established\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_stop_interface(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        for (int i = 0; i < 3 ; i++)
        {
            this->exitThread[i] = true;
            pthread_join(this->tid[0], NULL);
        }

        for(int i = 0; i < 2; i++)
            close(this->socket[i]);

        this->dlpif.inProc = false;
        this->dlpif.heartBeat = false;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_reset_test(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        this->testProgress = MSGPROC_TEST_PROGRESS_STANDBY;
        this->testResult = MSGPROC_TEST_RESULT_FAIL;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_test_interface(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        this->testProgress = MSGPROC_TEST_PROGRESS_INPROC;

        if (msgproc->xmitMessage(msgproc, MSGGRP_K00, MSGNUM_NETWORK_MONITOR, NULL) == MSGPROC_RECV_ACK)
            this->testResult = MSGPROC_TEST_RESULT_OK;
        else
            this->testResult = MSGPROC_TEST_RESULT_FAIL;

        this->testProgress = MSGPROC_TEST_PROGRESS_DONE;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_get_test_progress(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
        ret = this->testProgress;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_get_test_result(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
        ret = this->testResult;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_enable_auto_xmit(struct msgproc_interface *msgproc, int flag)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
        this->autoXmit = flag;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


static int
msgproc_get_auto_xmit(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
        ret = this->autoXmit;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}


struct msgproc_interface *
msgproc_create(void)
{
    struct msgproc_interface *msgproc = NULL;
    struct msgproc_attribute *this = malloc(sizeof(struct msgproc_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct msgproc_attribute));
        this->uart = uart_create();

        if (this->uart)
        {
            this->uart->open(this->uart, UART_TTYMXC3, B115200);
            pthread_mutex_init(&this->mtx, NULL);
            pthread_cond_init(&this->cond, NULL);

            msgproc = &(this->extif);
            msgproc->initInterface  = msgproc_init_interface;
            msgproc->startInterface = msgproc_start_interface;
            msgproc->stopInterface  = msgproc_stop_interface;
            msgproc->execKvmfProc   = msgproc_exec_kvmfproc;
            msgproc->killKvmfProc   = msgproc_kill_kvmfproc;
            msgproc->xmitMessage    = msgproc_transmit_kvmfmsg;
            msgproc->setAddr        = msgproc_set_ipaddr;
            msgproc->setUrn         = msgproc_set_urn;
            msgproc->enableAutoXmit = msgproc_enable_auto_xmit;
            msgproc->getAutoXmit    = msgproc_get_auto_xmit;
            msgproc->resetTest      = msgproc_reset_test;
            msgproc->testInterface  = msgproc_test_interface;
            msgproc->getTestResult  = msgproc_get_test_result;
            msgproc->getTestProgress = msgproc_get_test_progress;
            TLOGMSG(1, ("create msgproc interface\n"));
        }
        else
        {
            free(this);
            TLOGMSG(1, ("failed to create msgproc interface\n"));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return msgproc;
}


int
msgproc_destroy(struct msgproc_interface *msgproc)
{
    int ret = 0;
    struct msgproc_attribute *this = (struct msgproc_attribute *) msgproc;

    if (this)
    {
        msgproc_stop_interface(msgproc);

        if (this->dlpif.kvmfProc)
            msgproc_kill_kvmfproc(msgproc);

        uart_destroy(this->uart);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        free(this);
        TLOGMSG(1, (DBGINFOFMT "destroy msgproc interface\n", DBGINFO));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null msgproc interface\n", DBGINFO));
    }

    return ret;
}
