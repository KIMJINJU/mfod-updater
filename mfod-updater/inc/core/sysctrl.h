/*
    Copyright (C) 2016 EOmain CO., LTD. (www.eomain.com)

    sysctrl.h
        external function/variables/defines top-level main control module
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eomain.com)
*/


#ifndef _SYSCTRL_H_
#define _SYSCTRL_H_

/* macro defines - sysctrl retrun codes */
#define SYSCTRL_RETCODE_CHANGE_OBMODE_SUCCESS				32
#define SYSCTRL_RETCODE_SUCCESS_IRMOVE						31
#define SYSCTRL_RETCODE_TARGET_DELETED                      30
#define SYSCTRL_RETCODE_EXEC_TARGET_LIST_FUNC               29
#define SYSCTRL_RETCODE_EMPTY_TARGET_LIST                   28
#define SYSCTRL_RETCODE_NUC_READY                           27
#define SYSCTRL_RETCODE_IBIT_PASS                           26
#define SYSCTRL_RETCODE_XMIT_TARGET_LOCATION                25
#define SYSCTRL_RETCODE_SELECT_TARGET_INFO_TO_XMIT          24
#define SYSCTRL_RETCODE_RECV_ACK                            23
#define SYSCTRL_RETCODE_RECV_NACK_CANTPRO2                  22
#define SYSCTRL_RETCODE_RECV_NACK_CANTPRO15                 21
#define SYSCTRL_RETCODE_RECV_NACK_CANTPRO19                 20
#define SYSCTRL_RETCODE_RECV_NACK_CANTPRO25                 19
#define SYSCTRL_RETCODE_RESPONSE_TIMEOUT                    18
#define SYSCTRL_RETCODE_TERMINATE_TARGET_ACQUISITION        17
#define SYSCTRL_RETCODE_SET_TAQMODE_CIRCULAR_TARGET         16
#define SYSCTRL_RETCODE_SET_TAQMODE_SQAURE_TARGET           15
#define SYSCTRL_RETCODE_SET_TAQMODE_FOS_CORRECTION          14
#define SYSCTRL_RETCODE_SUCCESS_TALOC                       13
#define SYSCTRL_RETCODE_INIT_MAGCOMP                        12
#define SYSCTRL_RETCODE_STOP_TARGET_ACQUISITION             11
#define SYSCTRL_RETCODE_SUCCESS_CIRCULAR_TARGET_ACUSITION   10
#define SYSCTRL_RETCODE_SUCCESS_SQUARE_TARGET_ACQUISITION   9
#define SYSCTRL_RETCODE_SUCCESS_FOS_CORRECTION              8
#define SYSCTRL_RETCODE_SUCCESS_POINT_TARGET_ACQUISITION    7
#define SYSCTRL_RETCODE_USE_AUTO_GRIDVAR                    6
#define SYSCTRL_RETCODE_USE_USER_GRIDVAR                    5
#define SYSCTRL_RETCODE_USE_AUTO_RANGE                      4
#define SYSCTRL_RETCODE_USE_USER_RANGE                      3
#define SYSCTRL_RETCODE_USE_AUTO_OBLOC                      2
#define SYSCTRL_RETCODE_USE_USER_OBLOC                      1
#define SYSCTRL_RETCODE_OK                                  0
#define SYSCTRL_RETCODE_ERROR                               -1
#define SYSCTRL_RETCODE_FAIL_LRF_CTRL                       -11
#define SYSCTRL_RETCODE_FAIL_SHTR_CTRL                      -12
#define SYSCTRL_RETCODE_FAIL_IRCAM_POWER                    -15
#define SYSCTRL_RETCODE_FAIL_DISP_CTRL                      -16
#define SYSCTRL_RETCODE_FAIL_IRCAM_CTRL                     -17
#define SYSCTRL_RETCODE_FAIL_EVOUT_CTRL                     -18
#define SYSCTRL_RETCODE_INVALID_OBLOC                       -19
#define SYSCTRL_RETCODE_INVALID_RANGE                       -20
#define SYSCTRL_RETCODE_INVALID_GRIDVAR                     -21
#define SYSCTRL_RETCODE_ANGLE_LIMIT                         -22
#define SYSCTRL_RETCODE_FAIL_START_RANGE_MEASURE            -23
#define SYSCTRL_RETCODE_FAIL_CIRCULAR_TARGET_ACQUISITION    -24
#define SYSCTRL_RETCODE_FAIL_SQUARE_TARGET_ACQUISITION      -25
#define SYSCTRL_RETCODE_FAIL_FOS_CORRECTION                 -26
#define SYSCTRL_RETCODE_FAIL_POINT_TARGET_ACQUISITION       -27
#define SYSCTRL_RETCODE_FAIL_INIT_MAGCOMP                   -28
#define SYSCTRL_RETCODE_ERROR_GRIDVAR                       -29
#define SYSCTRL_RETCODE_IBIT_FAIL                           -30
#define SYSCTRL_RETCODE_FAIL_IRMOVE							-31
#define	SYSCTRL_RETCODE_FWUPDATE_NOFILE						-32
#define SYSCTRL_RETCODE_FWUPDATE_FAIL						-33
#define SYSCTRL_RETCODE_FWUPDATE_NOSDCARD					-34

/* external functions */
extern int sysctrl_power_off(void *arg);
extern int sysctrl_push_trigger(void *);
extern int sysctrl_release_trigger(void *);
extern int sysctrl_handle_left_keyin(void *);
extern int sysctrl_handle_right_keyin(void *);
extern int sysctrl_toggle_irpol(void *);
extern int sysctrl_update_nuc(void *);
extern int sysctrl_toggle_obmode(void *);
extern int sysctrl_increase_dispbr(void *);
extern int sysctrl_decrease_dispbr(void *);
extern int sysctrl_increase_irbr(void *);
extern int sysctrl_decrease_irbr(void *);
extern int sysctrl_increase_ircont(void *);
extern int sysctrl_decrease_ircont(void *);
extern int sysctrl_increase_iredge(void *);
extern int sysctrl_decrease_iredge(void *);
extern int sysctrl_enable_ireis(void *);
extern int sysctrl_disable_ireis(void *);
extern int sysctrl_set_irclr_mono(void *);
extern int sysctrl_set_irclr_sepia(void *);
extern int sysctrl_set_irclr_spectrum(void *);
extern int sysctrl_set_irclr_isotherm(void *);
extern int sysctrl_enable_gheq(void *);
extern int sysctrl_enable_lheq(void *);
extern int sysctrl_increase_irgain(void *);
extern int sysctrl_decrease_irgain(void *);
extern int sysctrl_increase_irlevel(void *);
extern int sysctrl_decrease_irlevel(void *);
extern int sysctrl_set_coordsys_geodetic(void *);
extern int sysctrl_set_coordsys_utm(void *);
extern int sysctrl_set_coordsys_mgrs(void *);
extern int sysctrl_set_systime(void *);
extern int sysctrl_change_lastday(void *arg);
extern int sysctrl_enable_evout(void *);
extern int sysctrl_disable_evout(void *);
extern int sysctrl_use_user_obloc(void *);
extern int sysctrl_use_auto_obloc(void *);
extern int sysctrl_use_user_range(void *);
extern int sysctrl_use_auto_range(void *);
extern int sysctrl_use_user_gridvar(void *);
extern int sysctrl_use_auto_gridvar(void *);
extern int sysctrl_stop_target_acquisition(void *arg);
extern int sysctrl_terminate_target_acquisition(void *arg);
extern int sysctrl_enter_magcompmode(void *arg);
extern int sysctrl_exit_magcompmode(void *arg);
extern int sysctrl_retry_magcomp(void *arg);
extern int sysctrl_apply_magparm(void *arg);
extern int sysctrl_acquire_magcompdata(void *arg);
extern int sysctrl_xmit_observer_location(void *arg);
extern int sysctrl_xmit_target_location(void *arg);
extern int sysctrl_xmit_target_shift(void *arg);
extern int sysctrl_xmit_target_information(void *arg);
extern int sysctrl_xmit_target_infomation_at_list(void *arg);
extern int sysctrl_xmit_target_location_at_list(void *arg);
extern int sysctrl_xmit_target_shift_at_list(void *arg);
extern int sysctrl_test_dmif(void *arg);
extern int sysctrl_set_taqmode_circular_target(void *arg);
extern int sysctrl_set_taqmode_square_target(void *arg);
extern int sysctrl_set_taqmode_fos_correction(void *arg);
extern int sysctrl_enter_standby_mode(void *arg);
extern int sysctrl_exit_standby_mode(void *arg);
extern int sysctrl_execute_ibit(void *arg);
extern int sysctrl_enable_auto_xmit(void *arg);
extern int sysctrl_disable_auto_xmit(void *arg);
extern int sysctrl_select_internal_shutter(void *arg);
extern int sysctrl_select_external_shutter(void *arg);
extern int sysctrl_check_target_list(void *arg);
extern int sysctrl_delete_target(void *arg);
extern int sysctrl_save_short_dist_correct(void *arg);
extern int sysctrl_save_middle_dist_correct(void *arg);
extern int sysctrl_save_long_dist_correct(void *arg);
extern int sysctrl_save_azimuth_correct(void *arg);
extern int sysctrl_save_altitude_correct(void *arg);
extern int sysctrl_save_sensor_correct(void *arg);
extern int sysctrl_init_correct(void *arg);
extern int sysctrl_firmware_update(void *arg);
extern int sysctrl_move_ir_left(void *arg);
extern int sysctrl_move_ir_right(void *arg);
extern int sysctrl_move_ir_up(void *arg);
extern int sysctrl_move_ir_down(void *arg);
extern int sysctrl_irarray_end(void *arg);

#endif /* _SYSCTRL_H_ */
