#ifndef _GUI_DIALOG_CREATOR_H_
#define _GUI_DIALOG_CREATOR_H_

#include "gui/gui_window.h"

/* external functions for create dialog */
extern dialog_t *create_dialog_display_adjust(void *arg);
extern dialog_t *create_dialog_obmode(void *arg);
extern dialog_t *create_dialog_ireis(void *arg);
extern dialog_t *create_dialog_irclr(void *arg);
extern dialog_t *create_dialog_irglv(void *arg);
extern dialog_t *create_dialog_enter_magcompmode(void *arg);
extern dialog_t *create_dialog_exit_magcompmode(void *arg);
extern dialog_t *create_dialog_apply_magparm(void *arg);
extern dialog_t *create_dialog_retry_magcomp(void *arg);
extern dialog_t *create_dialog_success_magcomp(void *arg);
extern dialog_t *create_dialog_magcomp_errfom(void *arg);
extern dialog_t *create_dialog_magcomp_errcalc(void *arg);
extern dialog_t *create_dialog_magcomp_errdaq(void *arg);
extern dialog_t *create_dialog_irheq(void *arg);
extern dialog_t *create_dialog_xmit_obloc(void *arg);
extern dialog_t *create_dialog_xmit_taloc(void *arg);
extern dialog_t *create_dialog_xmit_taloc_at_list(void *arg);
extern dialog_t *create_dialog_select_xmitifno(void *arg);
extern dialog_t *create_dialog_select_xmitifno_at_list(void *arg);
extern dialog_t *create_dialog_input_gridvar(void *arg);
extern dialog_t *create_dialog_input_obloc(void *arg);
extern dialog_t *create_dialog_input_range(void *arg);
extern dialog_t *create_dialog_input_rgate(void *arg);
extern dialog_t *create_dialog_gnss_status(void *arg);
extern dialog_t *create_dialog_coordsys_selection(void *arg);
extern dialog_t *create_dialog_timeset(void *arg);
extern dialog_t *create_dialog_taqsel(void *arg);
extern dialog_t *create_dialog_evout(void *arg);
extern dialog_t *create_dialog_dmif_test(void *arg);
extern dialog_t *create_dialog_standby_mode(void *arg);
extern dialog_t *create_dialog_termtaq(void *arg);
extern dialog_t *create_dialog_user_obloc(void *arg);
extern dialog_t *create_dialog_auto_obloc(void *arg);
extern dialog_t *create_dialog_invalid_obloc(void *arg);
extern dialog_t *create_dialog_user_range(void *arg);
extern dialog_t *create_dialog_auto_range(void *arg);
extern dialog_t *create_dialog_invalid_range(void *arg);
extern dialog_t *create_dialog_user_gridvar(void *arg);
extern dialog_t *create_dialog_auto_gridvar(void *arg);
extern dialog_t *create_dialog_low_voltage(void *arg);
extern dialog_t *create_dialog_invalid_gridvar(void *arg);
extern dialog_t *create_dialog_ircamctrl_error(void *arg);
extern dialog_t *create_dialog_ircampwr_error(void *arg);
extern dialog_t *create_dialog_dispctrl_error(void *arg);
extern dialog_t *create_dialog_evoutctrl_error(void *arg);
extern dialog_t *create_dialog_shtrctrl_error(void *arg);
extern dialog_t *create_dialog_lrfctrl_error(void *arg);
extern dialog_t *create_dialog_angle_error(void *arg);
extern dialog_t *create_dialog_unknown_error(void *arg);
extern dialog_t *create_dialog_magcomp_initerr(void *arg);
extern dialog_t *create_dialog_error_zero_range(void *arg);
extern dialog_t *create_dialog_error_target_shift(void *arg);
extern dialog_t *create_dialog_error_target_size(void *arg);
extern dialog_t *create_dialog_error_geodcalc(void *arg);
extern dialog_t *create_dialog_error_dmc_offline(void *arg);
extern dialog_t *create_dialog_error_gnss_offline(void *arg);
extern dialog_t *create_dialog_error_taq(void *arg);
extern dialog_t *create_dialog_success_xmit(void *arg);
extern dialog_t *create_dialog_success_dmif_test(void *arg);
extern dialog_t *create_dialog_timeout(void *arg);
extern dialog_t *create_dialog_nack_cantpro2(void *arg);
extern dialog_t *create_dialog_nack_cantpro15(void *arg);
extern dialog_t *create_dialog_nack_cantpro19(void *arg);
extern dialog_t *create_dialog_nack_cantpro25(void *arg);
extern dialog_t *create_dialog_error_gridvar(void *arg);
extern dialog_t *create_dialog_exec_bit(void *arg);
extern dialog_t *create_dialog_bit_pass(void *arg);
extern dialog_t *create_dialog_bit_fail(void *arg);
extern dialog_t *create_dialog_auto_xmit(void *arg);
extern dialog_t *create_dialog_target_list(void *arg);
extern dialog_t *create_dialog_target_detail(void *arg);
extern dialog_t *create_dialog_delete_target(void *arg);
extern dialog_t *create_dialog_select_shutter(void *arg);
extern dialog_t *create_dialog_cover_irlens(void *arg);
extern dialog_t *create_dialog_system_info(void *arg);

/* factory mode dialog create */
extern dialog_t *create_dialog_short_distance_correct(void *arg);
extern dialog_t *create_dialog_middle_distance_correct(void *arg);
extern dialog_t *create_dialog_long_distance_correct(void *arg);
extern dialog_t *create_dialog_distance_correct_check(void *arg);
extern dialog_t *create_dialog_azimuth_correct(void *arg);
extern dialog_t *create_dialog_altitude_correct(void *arg);
extern dialog_t *create_dialog_angle_correct_check(void *arg);
extern dialog_t *create_dialog_sensor_correct(void *arg);
extern dialog_t *create_dialog_sensor_correct_check(void *arg);
extern dialog_t *create_dialog_ir_array(void *arg);
extern dialog_t *create_dialog_ir_array_check(void *arg);
extern dialog_t *create_dialog_ir_scale_input(void *arg);
extern dialog_t *create_dialog_firmware_update(void *arg);
extern dialog_t *create_dialog_correct_init(void *arg);
extern dialog_t *create_dialog_ir_move(void *arg);
extern dialog_t *create_dialog_fwupdate_nofile(void *arg);
extern dialog_t *create_dialog_fwupdate_success(void *arg);
extern dialog_t *create_dialog_fwupdate_fail(void *arg);
extern dialog_t *create_dialog_fwupdate_nosdcard(void *arg);
/* end */

#endif /* _GUI_DIALOG_CREATOR_H_ */
