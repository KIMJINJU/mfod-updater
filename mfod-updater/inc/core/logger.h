/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    logger.h
        external function/variables/defines for logger interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _LOGGER_H_
#define _LOGGER_H_

/* macro defines  : logging device */
#define LOGDEV_NONE     0x00
#define LOGDEV_CONSOLE 	0x01
#define LOGDEV_FILE		0x02


/* external functions */
extern int logger_init(int logdev);
extern int logger_deinit(void);
extern int logger_write_log(const char *fmt, ...);
extern int logger_write_tlog(const char *fmt, ...);


/* logmsg macro defines */
#define _ENABLE_LOGMSG_
#ifdef _ENABLE_LOGMSG_
#define TLOGMSG(cond, printf_exp) ((void)((cond)?(logger_write_tlog printf_exp), 1:0))
#define LOGMSG(cond, printf_exp) ((void)((cond)?(logger_write_log printf_exp), 1:0))
#else
#define TLOGMSG(cond, printf_exp)
#define LOGMSG(cond, printf_exp)
#endif


#define DBGINFOFMT          "%s, %s, %d : "
#define DBGINFO             __FILE__, __func__, __LINE__


#endif /* _LOGGER_H_ */
