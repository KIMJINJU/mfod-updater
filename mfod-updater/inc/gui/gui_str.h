/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_str.h
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_STR_H_
#define _GUI_STR_H_

/* indicator text */
#define STR_IR                          "\uC5F4\uC0C1"
#define STR_DV                          "\uC8FC\uAC04"
#define STR_FT							"\uACF5\uC7A5\uBAA8\uB4DC"
#define STR_BHOT                        "\uD751\uC0C1"
#define STR_WHOT                        "\uBC31\uC0C1"
#define STR_NUC                         "\uBCF4\uC815"
#define STR_PCLR                        "\uC758\uC0C9"
#define STR_EZOOM                       "X2"
#define STR_EIS                         "EIS"
#define STR_MNORTH                      "\uC790\uBD81"
#define STR_GNORTH                      "\uB3C4\uBD81"
#define STR_TNORTH                      "\uC9C4\uBD81"
#define STR_PRIMARY                     "1\uCC28"
#define STR_SECONDARY                   "2\uCC28"
#define STR_EXTERNAL                    "\uC678\uBD80"
#define STR_NO_GNSS_SIGNAL              "GPS \uC704\uC131\uC2E0\uD638 \uC218\uC2E0 \uB300\uAE30"
#define STR_AZIMUTH                     "\uBC29\uC704\uAC01"
#define STR_ELEVATION                   "\uACE0\uB3C4\uAC01"
#define STR_BANK                        "\uACBD\uC0AC\uAC01"
#define STR_MIL                         "\uBC00"
#define STR_LRF_CHARGE                  "\uB808\uC774\uC800 \uCDA9\uC804 \uC911"
#define STR_LRF_READY                   "\uB808\uC774\uC800 \uBC1C\uC0AC \uC900\uBE44 \uC644\uB8CC"
#define STR_MANUAL_RANGING              "\uC218\uB3D9 \uAC70\uB9AC \uCE21\uC815"
#define STR_MANUAL_DAQ                  "\uC218\uB3D9"
#define STR_AUTO_DAQ                    "\uC790\uB3D9"
#define STR_TAQMODE_CIRCULAR_TARGET     "\uC6D0\uD615 \uD45C\uC801 \uD68D\uB4DD"
#define STR_TAQMODE_SQUARE_TARGET       "\uC0AC\uAC01 \uD45C\uC801 \uD68D\uB4DD"
#define STR_TAQMODE_FOS_CORRECTION      "\uB099\uD0C4 \uC218\uC815"
#define STR_GUIDE_POINT_TARGET          "\uD45C\uC801\uC758 \uC911\uC2EC\uC744 \uC2DC\uC900\uD558\uC5EC \uAC70\uB9AC \uCE21\uC815"
#define STR_GUIDE_CIRCULAR_TARGET       "\uD45C\uC801\uC758 \uAC00\uC7A5\uC790\uB9AC\uB97C \uC2DC\uC900\uD558\uC5EC \uAC70\uB9AC \uCE21\uC815"
#define STR_GUIDE_SQUARE_TARGET_LENGTH  "\uD45C\uC801\uC758 \uC7A5\uCD95/\uB2E8\uCD95 \uB05D\uC744 \uC2DC\uC900\uD558\uC5EC \uAC70\uB9AC \uCE21\uC815"
#define STR_GUIDE_SQUARE_TARGET_WIDTH   "\uD45C\uC801\uC758 \uC7A5\uCD95/\uB2E8\uCD95 \uB05D\uC744 \uC2DC\uC900\uD558\uC5EC \uAC70\uB9AC \uCE21\uC815"
#define STR_GUIDE_FOS_CORRECTION        "\uCC29\uD0C4\uC810\uC744 \uC2DC\uC900\uD558\uC5EC \uAC70\uB9AC \uCE21\uC815"

/* main menu text */
#define STR_IRCONF                  "\uC5F4\uC0C1"
#define STR_TARGET                  "\uD45C\uC801"
#define STR_DEVCONF                 "\uC124\uC815"

/* factory menu text*/
#define STR_DIST_CORRECT			"\uAC70\uB9AC \uBCF4\uC815"
#define STR_ANGLE_CORRECT			"\uAC01\uB3C4 \uBCF4\uC815"
#define	STR_SENSOR_CORRECT			"\uADFC\uC811\uC13C\uC11C \uBCF4\uC815"
#define	STR_IR_ARRAY				"\uC5F4\uC0C1 \uC815\uB82C"
#define STR_FW_UPDATE				"\uC5C5\uB370\uC774\uD2B8"

/* factory distance correct submenu text */
#define STR_DISTCRR_SHORT			"\uB2E8\uAC70\uB9AC \uBCF4\uC815"
#define STR_DISTCRR_MIDDLE			"\uC911\uAC70\uB9AC \uBCF4\uC815"
#define	STR_DISTCRR_LONG			"\uC7A5\uAC70\uB9AC \uBCF4\uC815"
#define STR_DISTCRR_CHECK			"\uAC70\uB9AC\uBCF4\uC815\uCE58 \uD655\uC778"

/* factory angle correct submenu text */
#define STR_ANGCRR_AZM				"\uBC29\uC704\uAC01 \uBCF4\uC815"
#define STR_ANGCRR_ALTT				"\uACE0\uB3C4\uAC01 \uBCF4\uC815"
#define STR_ANGCRR_CHECK			"\uAC01\uB3C4\uBCF4\uC815\uCE58 \uD655\uC778"

/*factory sensor correct submenu text */
#define STR_SENCRR_SENSOR			"\uADFC\uC811\uC13C\uC11C \uBCF4\uC815"
#define STR_SENCRR_CHECK			"\uADFC\uC811\uC13C\uC11C\uBCF4\uC815\uCE58 \uD655\uC778"

/* factory ir array submenu text */
#define STR_IRARR_ARRAY				"\uC5F4\uC0C1 \uC815\uB82C"
#define STR_IRARR_CHECK				"\uC5F4\uC0C1\uC815\uB82C\uCE58 \uD655\uC778"
#define STR_IRRARR_IRSCALEINPUT		"\uC5F4\uC0C1 \uC2A4\uCF00\uC77C\uB9C1 \uC124\uC815"

/* factory update submenu text */
#define STR_FWUPDATE_UPDATE			"F/W \uC5C5\uB370\uC774\uD2B8"
#define STR_FWUPDATE_INIT			"\uBCF4\uC815\uCE58 \uCD08\uAE30\uD654"
#define STR_CORRECT_INIT			"\uBCF4\uC815\uCE58 \uCD08\uAE30\uD654\uB97C \uC218\uD589\uD569\uB2C8\uB2E4"
#define STR_FWUPDATE_CONFIRM		"F/W \uC5C5\uB370\uC774\uD2B8\uB97C \uC2E4\uD589\uD569\uB2C8\uB2E4"


/* irconf submenu text */
#define STR_IRCONF_IREIS            "\uC601\uC0C1 \uC548\uC815\uD654"
#define STR_IRCONF_IRCLR            "\uC758\uC0AC \uC0C9\uCC44"
#define STR_IRCONF_IRGLV            "\uC774\uB4DD/\uB808\uBCA8 \uC870\uC815"
#define STR_IRCONF_IRHEQ            "\uC5F4\uC0C1 \uCC98\uB9AC \uC120\uD0DD"
#define STR_IRCONF_SHTRSEL          "\uC5F4\uC0C1 \uC154\uD130 \uC120\uD0DD"

/* target submenu text */
#define STR_TARGET_SHOW_LIST        "\uD45C\uC801 \uBAA9\uB85D \uC5F4\uB78C"
#define STR_TARGET_XMIT_OBLOC       "\uAD00\uCE21 \uC704\uCE58 \uC804\uC1A1"
#define STR_TARGET_INPUT_OBLOC      "\uAD00\uCE21 \uC704\uCE58 \uC124\uC815"
#define STR_TARGET_INPUT_RANGE      "\uD45C\uC801 \uAC70\uB9AC \uC124\uC815"
#define STR_TARGET_INPUT_RGATE      "\uCE21\uC815 \uAD6C\uAC04 \uC124\uC815"
#define STR_TARGET_INPUT_GRIDVAR    "\uB3C4\uC790\uAC01 \uC124\uC815"
#define STR_TARGET_COMP_DMC         "\uC804\uC790\uB098\uCE68\uBC18 \uBCF4\uC815"
#define STR_TARGET_GNSS_STATUS      "GPS \uC218\uC2E0\uC0C1\uD0DC"

/* devconf submenu text */
#define STR_DEVCONF_DMIF            "\uAD00\uCE21\uC81C\uC6D0 \uC785\uCD9C\uB825\uAE30"
#define STR_DEVCONF_DMIF_TEST       "\uC120\uB85C \uC810\uAC80 \uC218\uD589"
#define STR_DEVCONF_XMIT_DATA       "\uC81C\uC6D0 \uC804\uC1A1 \uC124\uC815"
#define STR_DEVCONF_SET_COORDSYS    "\uC88C\uD45C \uD615\uC2DD \uC124\uC815"
#define STR_DEVCONF_TIME_CONFIG     "\uC2DC\uC2A4\uD15C \uC2DC\uAC04 \uC124\uC815"

#define STR_DEVCONF_EXTERN_VOUT     "\uC678\uBD80 \uC601\uC0C1 \uCD9C\uB825"
#define STR_DEVCONF_IBIT            "\uC790\uCCB4 \uC810\uAC80 \uC218\uD589"
#define STR_DEVCONF_SLEEP           "\uB300\uAE30 \uBAA8\uB4DC \uC9C4\uC785"
#define STR_DEVCONF_SYSTEM_INFO     "\uC2DC\uC2A4\uD15C \uC815\uBCF4"

/* obmode dialog */
#define STR_OBMODE                  "\uAD00\uCE21\uBAA8\uB4DC"
#define STR_CHANGE_OBMODE_IR        "\uC5F4\uC0C1\uAD00\uCE21\uBAA8\uB4DC\uB85C \uBCC0\uACBD\uD569\uB2C8\uB2E4"
#define STR_CHANGE_OBMODE_DV        "\uC8FC\uAC04\uAD00\uCE21\uBAA8\uB4DC\uB85C \uBCC0\uACBD\uD569\uB2C8\uB2E4"
#define STR_IRARRAY_END				"\uC5F4\uC0C1 \uC815\uB82C\uC744 \uC885\uB8CC\uD569\uB2C8\uB2E4"
#define STR_IRARRAY_START			"\uC5F4\uC0C1 \uC815\uB82C\uC744 \uC218\uD589\uD569\uB2C8\uB2E4"


/* adjust dialog */
#define STR_DISPBR                  "\uC804\uC2DC\uAE30 \uBC1D\uAE30"
#define STR_IRBR                    "\uC5F4\uC601\uC0C1 \uBC1D\uAE30"
#define STR_IRCONT                  "\uC5F4\uC601\uC0C1 \uB300\uC870\uBE44"
#define STR_IREDGE                  "\uC724\uACFD\uC120 \uAC15\uC870"

/* ireis dialog */
#define STR_IREIS                   "\uC601\uC0C1 \uC548\uC815\uD654"

/* irclr dialog */
#define STR_IRCLR                   "\uC758\uC0AC \uC0C9\uCC44"
#define STR_IRCLR_MONO              "\uBAA8\uB178"
#define STR_IRCLR_SEPIA             "\uC801\uAC08\uC0C9"
#define STR_IRCLR_SPECTRUM          "\uC2A4\uD399\uD2B8\uB7FC"
#define STR_IRCLR_ISOTHERM          "\uB4F1\uC628\uC120"

/* irheq dialog */
#define STR_IRHEQ                   "\uC5F4\uC601\uC0C1\uCC98\uB9AC"
#define STR_IRHEQ_GHEQ              "\uC804\uC5ED\uCC98\uB9AC"
#define STR_IRHEQ_LHEQ              "\uC9C0\uC5ED\uCC98\uB9AC"

/* irglv dialog */
#define STR_IRGLV                   "\uC5F4\uC0C1 \uC774\uB4DD/\uB808\uBCA8 \uC870\uC815"
#define STR_IRGLV_GAIN              "\uC5F4\uC0C1 \uC774\uB4DD"
#define STR_IRGLV_LEVEL             "\uC5F4\uC0C1 \uB808\uBCA8"

/* coordsys dialog */
#define STR_COORDSYS                "\uC88C\uD45C \uD615\uC2DD \uC124\uC815"
#define STR_COORDSYS_GEODETIC       "\uC704\uACBD\uB3C4"
#define STR_COORDSYS_MGRS           "MGRS"
#define STR_COORDSYS_UTM            "UTM"

/* timeset dialog */
#define STR_TIMESET                 "\uC2DC\uAC04\uC124\uC815"

/* dmif dialog */
#define STR_DLGTITLE_DMIF_TEST      "\uC120\uB85C \uC810\uAC80"
#define STR_DMIF_TEST               "\uC120\uB85C \uC810\uAC80\uC744 \uC218\uD589\uD569\uB2C8\uB2E4"
#define STR_SUCCESS_DMIF_TEST       "\uC120\uB85C \uC810\uAC80 \uC131\uACF5!"

/* xmit obloc dialog */
#define STR_XMIT_OBLOC              "\uAD00\uCE21 \uC704\uCE58\uB97C \uC804\uC1A1\uD569\uB2C8\uB2E4"
#define STR_CANT_XMIT_OBLOC         "\uAD00\uCE21 \uC704\uCE58 \uC804\uC1A1 \uBD88\uAC00"

/* xmit taloc dialog */
#define STR_XMIT_INFO               "\uD45C\uC801 \uC815\uBCF4 \uC804\uC1A1"
#define STR_XMIT_TALOC              "\uD45C\uC801 \uC815\uBCF4\uB97C \uC804\uC1A1\uD569\uB2C8\uB2E4"

/* obloc dialog */
#define STR_INVALID_OBLOC           "\uC785\uB825 \uC88C\uD45C\uAC00 \uC720\uD6A8\uD558\uC9C0 \uC54A\uC74C"
#define STR_ENABLE_USER_OBLOC       "\uC785\uB825 \uC88C\uD45C\uB97C \uAD00\uCE21 \uC704\uCE58\uB85C \uC124\uC815"
#define STR_ENABLE_AUTO_OBLOC       "\uAD00\uCE21 \uC704\uCE58\uB97C \uC790\uB3D9\uC73C\uB85C \uD68D\uB4DD"

/* user range dialog */
#define STR_ENABLE_USER_RANGE       "\uC785\uB825 \uAC70\uB9AC\uB97C \uD45C\uC801 \uAC70\uB9AC\uB85C \uC124\uC815"
#define STR_ENABLE_AUTO_RANGE       "\uD45C\uC801 \uAC70\uB9AC\uB97C \uC790\uB3D9\uC73C\uB85C \uD68D\uB4DD"
#define STR_INVALID_RANGE           "\uC720\uD6A8\uD558\uC9C0 \uC54A\uC740 \uC785\uB825\uAC12"

/* user gridvar dialog */
#define STR_ENABLE_USER_GRIDVAR     "\uC785\uB825\uAC01\uC744 \uB3C4\uC790\uAC01\uC73C\uB85C \uC124\uC815"
#define STR_ENABLE_AUTO_GRIDVAR     "\uB3C4\uC790\uAC01\uC744 \uC790\uB3D9\uC73C\uB85C \uD68D\uB4DD"
#define STR_INVALID_GRIDVAR         "\uC720\uD6A8\uD558\uC9C0 \uC54A\uC740 \uC785\uB825\uAC12"

/* taqsel dialog */
#define STR_CIRCULAR_TARGET         "\uC6D0\uD615\uD45C\uC801"
#define STR_SQUARE_TARGET           "\uC0AC\uAC01\uD45C\uC801"
#define STR_FOS_CORRECTTION         "\uB099\uD0C4\uC218\uC815"
#define STR_ARROW_UP				"▲"
#define STR_ARROW_DOWN				"▼"
#define STR_ARROW_LEFT				"◀"
#define STR_ARROW_RIGHT				"▶"

/* gps status dialog */
#define STR_LATITUDE                "\uC704\uB3C4"
#define STR_LONGITUDE               "\uACBD\uB3C4"
#define STR_ALTITUDE                "\uACE0\uB3C4"
#define STR_NSV                     "\uC704\uC131\uC218"
#define STR_NSU                     "\uC0AC\uC6A9\uC704\uC131\uC218"
#define STR_MAGDECL                 "\uC790\uD3B8\uAC01"
#define STR_GRIDVAR                 "\uB3C4\uC790\uAC01"

/* termtaq dialog */
#define STR_DLGTITLE_TERMTAQ        "\uD45C\uC801 \uD68D\uB4DD \uC911\uB2E8"
#define STR_TERM_CTAQ               "\uC6D0\uD615 \uD45C\uC801 \uD68D\uB4DD\uC744 \uC911\uB2E8\uD569\uB2C8\uB2E4"
#define STR_TERM_STAQ               "\uC0AC\uAC01 \uD45C\uC801 \uD68D\uB4DD\uC744 \uC911\uB2E8\uD569\uB2C8\uB2E4"
#define STR_TERM_FOS_CORRECTION     "\uB099\uD0C4 \uC218\uC815\uC744 \uC911\uB2E8\uD569\uB2C8\uB2E4"

/* enter standby mode */
#define STR_ENTER_STANDBY_MODE      "\uB300\uAE30 \uBAA8\uB4DC\uB85C \uC804\uD658\uD569\uB2C8\uB2E4"

/* taq information */
#define STR_TAQTIME                 "\uD68D\uB4DD \uC2DC\uAC04"
#define STR_TARGET_COORDINATE       "\uD45C\uC801 \uC88C\uD45C"
#define STR_OBSERVER_COORDINATE     "\uAD00\uCE21 \uC88C\uD45C"
#define STR_HDIST                   "\uC218\uD3C9 \uAC70\uB9AC"
#define STR_TARGET_ATTITUDE         "\uC7A5\uCD95\uAC01"
#define STR_TARGET_LENGTH           "\uC7A5\uCD95 \uAE38\uC774"
#define STR_TARGET_WIDTH            "\uD3ED"
#define STR_TARGET_RADIUS           "\uBC18\uACBD"
#define STR_TARGET_SHIFT            "\uC218\uC815\uB7C9"
#define STR_TARGET_LSHIFT           "\uD3B8\uC774"
#define STR_TARGET_RSHIFT           "\uC0AC\uAC70\uB9AC"
#define STR_COORDINATE              "\uC88C\uD45C"

/* taq error */
#define STR_ERROR_ZERO_RANGE        "\uAC70\uB9AC \uCE21\uC815 \uC2E4\uD328!"
#define STR_ERROR_TARGET_SHIFT      "\uC218\uC815\uB7C9 \uBC94\uC704 \uCD08\uACFC!"
#define STR_ERROR_TARGET_SIZE       "\uD45C\uC801 \uD06C\uAE30 \uBC94\uC704 \uCD08\uACFC!"
#define STR_ERROR_GEODCALC          "\uD45C\uC801 \uC81C\uC6D0 \uACC4\uC0B0 \uC2E4\uD328!"
#define STR_ERROR_DMC_OFFLINE       "\uC804\uC790\uB098\uCE68\uBC18 \uC624\uB958 \uBC1C\uC0DD!"
#define STR_ERROR_GNSS_OFFLINE      "\uAD00\uCE21 \uC704\uCE58 \uD68D\uB4DD \uBD88\uAC00!"
#define STR_ERROR_TAQ               "\uD45C\uC801 \uD68D\uB4DD \uC624\uB958!"
#define STR_ERROR_GRIDVAR           "\uB3C4\uC790\uAC01\uC744 \uC785\uB825\uD558\uC2ED\uC2DC\uC624"

/* transmit target info */
#define STR_XMIT_SUCCESS                "\uC804\uC1A1 \uC131\uACF5"
#define STR_SELECT_TARGET_INFO_TO_XMIT  "\uC804\uC1A1\uD560 \uD45C\uC801 \uC815\uBCF4\uB97C \uC120\uD0DD"
#define STR_XMIT_FAIL                   "\uC804\uC1A1 \uC2E4\uD328"
#define STR_XMIT_TARGET_INFO            "\uD45C\uC801 \uC815\uBCF4\uB97C \uC804\uC1A1\uD588\uC2B5\uB2C8\uB2E4"
#define STR_RESPONSE_TIMEOUT            "\uC751\uB2F5 \uC2DC\uAC04\uC774 \uCD08\uACFC\uB418\uC5C8\uC2B5\uB2C8\uB2E4"
#define STR_NACK_CANPRO1                "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (1)"
#define STR_NACK_CANPRO2                "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (2)"
#define STR_NACK_CANPRO8                "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (8)"
#define STR_NACK_CANPRO13               "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (13)"
#define STR_NACK_CANPRO15               "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (15)"
#define STR_NACK_CANPRO17               "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (17)"
#define STR_NACK_CANPRO19               "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (19)"
#define STR_NACK_CANPRO25               "\uCC98\uB9AC \uBD88\uAC00 \uCF54\uB4DC \uC218\uC2E0 (25)"

/* notice */
#define STR_NOTICE_INIT_MAGCOMP         "\uB098\uCE68\uBC18 \uBCF4\uC815 \uCD08\uAE30\uD654 \uC911"
#define STR_NOTICE_EXIT_MAGCOMP         "\uB098\uCE68\uBC18 \uBCF4\uC815 \uC885\uB8CC \uC911"
#define STR_NOTICE_CHANGE_OBMODE        "\uAD00\uCE21\uBAA8\uB4DC \uBCC0\uACBD \uC911"
#define STR_NOTICE_CHANGE_FILTER        "\uAD11\uB7C9\uC870\uC808\uD544\uD130 \uBCC0\uACBD \uC911"
#define STR_NOTICE_POWEROFF             "\uAD00\uCE21\uC7A5\uBE44\uB97C \uC885\uB8CC\uD558\uB294 \uC911"
#define STR_NOTICE_CALC_MAGCOMP         "\uC790\uAE30 \uBCF4\uC815\uCE58 \uACC4\uC0B0 \uC911"
#define STR_NOTICE_WAIT_RESPONSE        "\uC751\uB2F5 \uB300\uAE30 \uC911"
#define STR_NOTICE_DMIF_TEST            "\uC120\uB85C \uC810\uAC80 \uC218\uD589 \uC911"
#define STR_NOTICE_ENTER_STANDBY_MODE   "\uB300\uAE30 \uBAA8\uB4DC\uB85C \uC804\uD658 \uC911"
#define STR_NOTICE_EXIT_STANDBY_MODE    "\uB300\uAE30 \uBAA8\uB4DC \uD574\uC81C \uC911"

/* magnetic compensation */
#define STR_ENTER_MAGCOMPMODE       "\uB098\uCE68\uBC18 \uBCF4\uC815\uC744 \uC218\uD589\uD569\uB2C8\uB2E4"
#define STR_EXIT_MAGCOMPMODE        "\uB098\uCE68\uBC18 \uBCF4\uC815\uC744 \uC911\uB2E8\uD569\uB2C8\uB2E4"
#define STR_FAIL_INIT_MAGCOMP       "\uB098\uCE68\uBC18 \uBCF4\uC815 \uCD08\uAE30\uD654 \uC624\uB958"
#define STR_FRAME_CURSOR_LEFT		"\u25C1"
#define STR_FRAME_CURSOR_RIGHT		"\u25B7"
#define STR_FRAME_CURSOR_UP			"\u25B3"
#define STR_FRAME_CURSOR_DOWN	    "\u25BD"
#define STR_FILL_CURSOR_LEFT	    "\u25C0"
#define STR_FILL_CURSOR_RIGHT	    "\u25B6"
#define STR_FILL_CURSOR_UP		    "\u25B2"
#define STR_FILL_CURSOR_DOWN	    "\u25BC"
#define STR_TURN_CW                 "\uC6B0\uD5A5"
#define STR_TURN_CCW                "\uC88C\uD5A5"
#define STR_TURN_UP                 "\uC0C1\uD5A5"
#define STR_TURN_DOWN               "\uD558\uD5A5"
#define STR_TILT_CW                 "\uC88C\uC0C1"
#define STR_TILT_CCW                "\uC6B0\uC0C1"
#define STR_STOP_MOVE               "\uC815\uC9C0"
#define STR_SUCCESS_MAGCOMP         "\uB098\uCE68\uBC18 \uBCF4\uC815 \uC131\uACF5"
#define STR_FAIL_MAGCOMP            "\uB098\uCE68\uBC18 \uBCF4\uC815 \uC2E4\uD328"
#define STR_MAGCOMP_ERRCALC         "\uC790\uAE30 \uBCF4\uC815\uCE58 \uACC4\uC0B0 \uC2E4\uD328"
#define STR_MAGCOMP_ERRDAQ          "\uBCF4\uC815 \uB370\uC774\uD130 \uD68D\uB4DD \uC2E4\uD328"
#define STR_APPLY_MAGPARM           "\uC790\uAE30 \uBCF4\uC815\uCE58\uB97C \uC801\uC6A9\uD569\uB2C8\uB2E4"
#define STR_RETRY_MAGCOMP           "\uB098\uCE68\uBC18\uC744 \uC7AC\uBCF4\uC815\uD569\uB2C8\uB2E4"

/* bit */
#define STR_BIT_BIT                 "\uC790\uCCB4 \uC810\uAC80"
#define STR_BIT_STANDBY             "\uC810\uAC80 \uB300\uAE30"
#define STR_BIT_INPROC              "\uC810\uAC80 \uC911"
#define STR_BIT_OK                  "\uC815\uC0C1"
#define STR_BIT_ERROR               "\uACE0\uC7A5"
#define STR_BIT_MEMTEST             "\uBA54\uBAA8\uB9AC \uD14C\uC2A4\uD2B8"
#define STR_BIT_MAINBOARD           "\uC8FC\uC81C\uC5B4 \uD68C\uB85C \uCE74\uB4DC"
#define STR_BIT_LRF                 "\uB808\uC774\uC800 \uAC70\uB9AC\uCE21\uC815\uAE30"
#define STR_BIT_DMC                 "\uC804\uC790\uB098\uCE68\uBC18"
#define STR_BIT_IRCAM               "\uC5F4\uC601\uC0C1 \uCE74\uBA54\uB77C"
#define STR_BIT_GNSS                "\uC704\uC131 \uD56D\uBC95 \uC7A5\uCE58"
#define STR_BIT_DVO                 "\uC8FC\uAC04 \uAD11\uD559\uACC4"
#define STR_BIT_DMIF                "\uC120\uB85C \uC810\uAC80"
#define STR_BIT_LIMIT_FUNCTION      "\uACE0\uC7A5\uC73C\uB85C \uC778\uD574 \uAE30\uB2A5\uC774 \uC81C\uD55C\uB429\uB2C8\uB2E4"
#define STR_BIT_PRIMARY_CELL_VOLT   "1\uCC28 \uC804\uC9C0 \uC804\uC555"
#define STR_BIT_SECONDARY_CELL_VOLT "2\uCC28 \uC804\uC9C0 \uC804\uC555"
#define STR_BIT_EXTDC_VOLT          "\uC678\uBD80 \uC804\uC6D0 \uC804\uC555"
#define STR_BIT_DMIF_ERROR          "\uC5F0\uACB0 \uD655\uC778 \uC694\uB9DD"
#define STR_DMC_ERROR               "\uC804\uC790\uB098\uCE68\uBC18 \uACE0\uC7A5"
#define STR_GNSS_ERROR              "\uC704\uC131 \uD56D\uBC95 \uC7A5\uCE58 \uACE0\uC7A5"
#define STR_EXEC_BIT                "\uC790\uCCB4 \uC810\uAC80\uC744 \uC2E4\uD589\uD569\uB2C8\uB2E4"
#define STR_BIT_PASS                "\uC790\uCCB4 \uC810\uAC80 \uACB0\uACFC \uC815\uC0C1\uC785\uB2C8\uB2E4"

/* config xmit target data */
#define STR_CONF_XMIT_DATA           "\uC81C\uC6D0 \uC804\uC1A1 \uC124\uC815"
#define STR_AUTO                     "\uC790\uB3D9"
#define STR_MANUAL                   "\uC218\uB3D9"

/* auto xmit config */
#define STR_CONF_XMIT_DATA           "\uC81C\uC6D0 \uC804\uC1A1 \uC124\uC815"

/* target list dialog */
#define STR_TARGET_LIST             "\uD45C\uC801 \uBAA9\uB85D"
#define STR_DETAIL_INFO             "\uD45C\uC801 \uC0C1\uC138"
#define STR_XMIT_TARGET             "\uD45C\uC801 \uC804\uC1A1"
#define STR_DELETE_TARGET           "\uD45C\uC801 \uC0AD\uC81C"
#define STR_CONFIRM_DELETE          "\uD45C\uC801\uC744 \uC0AD\uC81C\uD569\uB2C8\uB2E4"
#define STR_NO_TARGET               "\uC800\uC7A5\uB41C \uD45C\uC801\uC774 \uC5C6\uC2B5\uB2C8\uB2E4"
#define STR_DISTANCE                "\uAC70\uB9AC"

/* select shutter dialog */
#define STR_INTERNAL_SHUTTER        "\uB0B4\uBD80 \uC154\uD130"
#define STR_EXTERNAL_SHUTTER        "\uC678\uBD80 \uC154\uD130"

/* cover irlens dialog */
#define STR_IRNUC                   "\uC5F4\uC0C1 \uBCF4\uC815"
#define STR_COVER_IRLENS            "\uC5F4\uC0C1 \uCE74\uBA54\uB77C \uB80C\uC988\uB97C \uAC00\uB9AC\uC2ED\uC2DC\uC624"

/* delete target dialog */
#define STR_CONFIRM_DELETE_TARGET   "\uC120\uD0DD\uD55C \uD45C\uC801 \uC815\uBCF4\uB97C \uC0AD\uC81C\uD569\uB2C8\uB2E4"

/* target detail dialog */
#define STR_TARGET_DETAIL           "\uD45C\uC801 \uC0C1\uC138 \uC815\uBCF4"
#define STR_OBSERVER_ALTITUDE       "\uAD00\uCE21 \uD45C\uACE0"
#define STR_TARGET_ALTITUDE         "\uD45C\uC801 \uD45C\uACE0"
#define STR_FWDAZ                   "\uBC29\uC704\uAC01"
#define STR_FWDEL                   "\uACE0\uB3C4\uAC01"
#define STR_SDIST                   "\uC0AC\uAC70\uB9AC"
#define STR_CIRCULAR_TARGET_INFO    "\uBC18\uACBD"
#define STR_SQUARE_TARGET_INFO      "\uAE38\uC774/\uD3ED"
#define STR_ATTITUDE                "\uC7A5\uCD95\uAC01"
#define STR_ADJUSTMENT              "\uD3B8\uC774/\uC0AC\uAC70\uB9AC"
#define STR_ADJUSTED_AZIMUTH        "\uC218\uC815\uBC29\uC704\uAC01"

/* device info dialog */
#define STR_APP_VERSION             "\uC6B4\uC6A9 S/W \uBC84\uC804"
#define STR_APP_BUILD_DATE          "\uC6B4\uC6A9 S/W \uBE4C\uB4DC\uC77C"
#define STR_APP_CRC                 "\uC6B4\uC6A9 S/W CRC32"
#define STR_RCC_PARAM               "\uAC70\uB9AC \uBCF4\uC815 \uB9E4\uAC1C \uBCC0\uC218"
#define STR_ANGULAR_OFFSET          "\uAC01\uB3C4 \uCE21\uC815 \uC624\uD504\uC14B"
#define STR_EOSYSTEM                "(\uC8FC) \uC774\uC624\uC2DC\uC2A4\uD15C"

/* etc. */
#define STR_ON                      "\uCF2C"
#define STR_OFF                     "\uB054"
#define STR_YES                     "\uC608"
#define STR_NO                      "\uC544\uB2C8\uC624"
#define STR_ERROR                   "\uC624\uB958"
#define STR_NEXT                    "\uB2E4\uC74C"
#define STR_PREV                    "\uC774\uC804"
#define STR_OPEN                    "\uC5F4\uAE30"
#define STR_CLOSE                   "\uB2EB\uAE30"
#define STR_CONFIRM                 "\uD655\uC778"
#define STR_APPLY                   "\uC801\uC6A9"
#define STR_DELETE                  "\uC0AD\uC81C"
#define STR_NOTICE                  "\uC54C\uB9BC"
#define STR_WARNING                 "\uC8FC\uC758"
#define STR_WAIT                    "\uC7A0\uC2DC \uAE30\uB2E4\uB9AC\uC2ED\uC2DC\uC624"
#define STR_CANCEL                  "\uCDE8\uC18C"
#define STR_CURRCONF                "\uD604\uC7AC \uC124\uC815"
#define STR_XMIT_INFO_TAQSEL        "\uC81C\uC6D0\uC804\uC1A1"
#define STR_CURSOR                  "\u25B6"
#define STR_CURSOR_UP               "\u25B2"
#define STR_CURSOR_DOWN             "\u25BC"
#define STR_DOT_CIRCLE              "\u25CF"
#define STR_DOT_SQUARE              "\u25A0"
#define STR_DEGREE                  "\u00B0"
#define STR_SAVE					"\uC800\uC7A5"
#define STR_INIT					"\uCD08\uAE30\uD654"
#define STR_SAVE_END				"\uC800\uC7A5&\uC885\uB8CC"
#define STR_END						"\uC885\uB8CC"
#define STR_HORIZONTAL				"\uC218\uD3C9"
#define STR_VERTICAL				"\uC218\uC9C1"
/* low voltage & dead battery */
#define STR_LOW_VOLTAGE             "\uC800\uC804\uC555 \uACBD\uACE0"
#define STR_CHANGE_BATTERY          "\uC804\uC9C0\uB97C \uAD50\uCCB4\uD558\uC2DE\uC2DC\uC624"
#define STR_DEAD_BATTERY            ""


/* errors */
#define STR_ERROR_ANGLE_LIMIT       "\uAC01\uB3C4 \uCE21\uC815 \uBC94\uC704 \uCD08\uACFC"
#define STR_ERROR_LRF_CTRL          "\uB808\uC774\uC800 \uAC70\uB9AC\uCE21\uC815\uAE30 \uC81C\uC5B4 \uC624\uB958"
#define STR_ERROR_SHTR_CTRL         "\uAD11\uB7C9 \uC870\uC808 \uD544\uD130 \uC81C\uC5B4"
#define STR_ERROR_EVOUT_CTRL        "\uC678\uBD80 \uC601\uC0C1 \uCD9C\uB825 \uC81C\uC5B4"
#define STR_ERROR_IRCAM_POWER       "\uC5F4\uC0C1 \uCE74\uBA54\uB77C \uC804\uC6D0"
#define STR_ERROR_IRCAM_CTRL        "\uB0B4\uBD80 \uC804\uC2DC\uAE30 \uC81C\uC5B4 \uC624\uB958"
#define STR_ERROR_DISP_CTRL         "\uB0B4\uBD80 \uC804\uC2DC\uAE30 \uC81C\uC5B4 \uC624\uB958"
#define STR_UNKNOWN_ERROR           "\uC54C\uC218 \uC5C6\uB294 \uC624\uB958"


#define STR_UPDATE_WARNING			"\uC804\uC6D0\uC744 \uB044\uAC70\uB098 SD\uCE74\uB4DC\uB97C \uC81C\uAC70\uD558\uC9C0 \uB9C8\uC2ED\uC2DC\uC624"
#define STR_UPDATE_BOOTLOADER		"\uBD80\uD2B8\uB85C\uB354 \uC5C5\uB370\uC774\uD2B8 \uC911..."
#define STR_UPDATE_UIMAGE			"\uCEE4\uB110 \uC5C5\uB370\uC774\uD2B8 \uC911..."
#define STR_UPDATE_ROOTFS			"\uB8E8\uD2B8 \uD30C\uC77C \uC2DC\uC2A4\uD15C \uC5C5\uB370\uC774\uD2B8 \uC911..."
#define STR_ERASE_ROOTFS			"\uB8E8\uD2B8 \uD30C\uC77C \uC2DC\uC2A4\uD15C \uC0AD\uC81C \uC911..."

#define STR_NOFILE					"SD\uCE74\uB4DC\uC5D0 \uAD00\uCE21\uACBD S/W \uD30C\uC77C\uC774 \uC5C6\uC2B5\uB2C8\uB2E4"
#define STR_FAIL_FWUPDATE			"F/W \uC5C5\uB370\uC774\uD2B8 \uC2E4\uD328!"
#define STR_SUCCESS_FWUPDATE		"F/W \uC5C5\uB370\uC774\uD2B8 \uC131\uACF5"
#define STR_INSERT_SDCARD			"SD\uCE74\uB4DC\uB97C \uC0BD\uC785\uD558\uC2ED\uC2DC\uC624"


#endif /* _GUI_STR_H_ */
