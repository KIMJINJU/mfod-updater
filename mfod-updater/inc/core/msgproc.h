/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    msgproc.h
        external function/variables/defines for msgproc interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _MSGPROC_H_
#define _MSGPROC_H_


/* constant macro defines */
#define MSGPROC_TCP                 0
#define MSGPROC_UDP                 1
#define KVMFPROC_TCP                2
#define KVMFPROC_UDP                3
#define IPADDR_MSGPROC_TCP          "127.0.0.1"
#define IPADDR_MSGPROC_UDP          "127.0.0.4"
#define IPADDR_KVMFPROC_TCP         "127.0.0.2"
#define IPADDR_KVMFPROC_UDP         "127.0.0.3"
#define PORT_KVMFPROC_TCP           "40000"
#define PORT_KVMFPROC_UDP           "1581"
#define PORT_KVMFPROC_SR            "1624"
#define PORT_MSGPROC_TCP            "40001"
#define PORT_MSGPROC_UDP            "1582"
#define PORT_MSGPROC_SR             "1625"
#define URN_SOURCE                  0x808001
#define URN_TARGET                  0X808002
#define MSGGRP_K00                  0
#define MSGGRP_K02                  2
#define MSGGRP_K05                  5
#define MSGNUM_POSITION_REPORT      1
#define MSGNUM_NETWORK_MONITOR      1
#define MSGNUM_TARGET_DATA          9
#define MSGNUM_SUBSEQUENT_ADJUST    22
#define MSGNUM_OBREADY_REPORT       37

#define MSGPROC_DM_NOT_REPONSE          -1
#define MSGPROC_RECV_ACK                1
#define MSGPROC_RECV_NACK_CANTPRO2      2
#define MSGPROC_RECV_NACK_CANTPRO15     15
#define MSGPROC_RECV_NACK_CANTPRO19     19
#define MSGPROC_RECV_NACK_CANTPRO25     25

#define MSGPROC_DISABLE_AUTO_XMIT       0
#define MSGPROC_ENABLE_AUTO_XMIT        1

#define MSGPROC_TEST_PROGRESS_STANDBY   0
#define MSGPROC_TEST_PROGRESS_INPROC    1
#define MSGPROC_TEST_PROGRESS_DONE      2

#define MSGPROC_TEST_RESULT_OK          0
#define MSGPROC_TEST_RESULT_FAIL        1

#define MSGPROC_AUTO_XMIT_ON            1
#define MSGPROC_AUTO_XMIT_OFF           0

/* structure declaration : msgproc interface */
struct msgproc_interface
{
    int (*enableAutoXmit) (struct msgproc_interface *, int flag);
    int (*getAutoXmit)    (struct msgproc_interface *);
    int (*initInterface)  (struct msgproc_interface *);
    int (*startInterface) (struct msgproc_interface *);
    int (*stopInterface)  (struct msgproc_interface *);
    int (*execKvmfProc)   (struct msgproc_interface *);
    int (*killKvmfProc)   (struct msgproc_interface *);
    int (*xmitMessage)    (struct msgproc_interface *, unsigned char, unsigned char, void *);
    int (*setUrn)         (struct msgproc_interface *, unsigned int, unsigned int);
    int (*setAddr)        (struct msgproc_interface *, int, char *, char *);
    int (*resetTest)      (struct msgproc_interface *);
    int (*testInterface)  (struct msgproc_interface *);
    int (*getTestProgress)(struct msgproc_interface *);
    int (*getTestResult)  (struct msgproc_interface *);

#ifdef _ENABLE_MSGPROC_INTERNAL_INTERFACE_

#endif
};


/* external functions for create/destroy interface */
extern struct msgproc_interface *msgproc_create(void);
extern int msgproc_destroy(struct msgproc_interface *msgproc);

#endif /* _MSGPROC_H_ */
