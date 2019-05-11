#ifndef _KEY_ASCII_H
#define _KEY_ASCII_H

#define K_PRIVATE 0x100	

#define K_MOUSE_LFET  	0x1
#define K_MOUSE_RIGHT 	0x2
#define K_CANCEL				0x3
#define K_MOUSE_MIDDLE	0x4
#define K_BACKSPACE			0x8
#define K_TAB						0x9
#define K_CLEAR			  	0xC
#define K_ENTER				 	0xA
#define K_SHIFT					0x10
#define K_CTRL					0x11
#define K_MENU					0x12
#define K_PAUSE					0x13
#define K_CAPS_LOCK		 	0x14
#define K_ESC					 	0x1B
#define K_SPACEBAR			0x20


#define K_PAGE_UP				(K_PRIVATE|0x21)
#define K_PAGE_DOWN			(K_PRIVATE|0x22)
#define K_END						(K_PRIVATE|0x23)
#define K_HOME			  	(K_PRIVATE|0x24)
#define K_LEFT_ARROW	 	(K_PRIVATE|0x25)
#define K_UP_ARROW			(K_PRIVATE|0x26)
#define K_RIGHT_ARROW		(K_PRIVATE|0x27)
#define K_DOWN_ARROW		(K_PRIVATE|0x28)
#define K_SELECT				(K_PRIVATE|0x29)
#define K_PRINT_SCREEN 	(K_PRIVATE|0x2A)
#define K_EXECUTE			 	(K_PRIVATE|0x2B)
#define K_SNAPSHOT			(K_PRIVATE|0x2C)
#define K_INSERT				(K_PRIVATE|0x2D)
#define K_DELETE				(K_PRIVATE|0x2E)
#define K_HELP					(K_PRIVATE|0x2F)
#define K_DRAG					(K_PRIVATE|0x30)


#define K_0		48
#define K_1		49
#define K_2		50
#define K_3		51
#define K_4		52
#define K_5		53
#define K_6		54
#define K_7		55
#define K_8		56
#define K_9		57

#define K_A  	65
#define K_B 	66
#define K_C		67
#define K_D		68
#define K_E		69
#define K_F		70
#define K_G		71
#define K_H		72
#define K_I		73
#define K_J		74
#define K_K		75
#define K_L		76
#define K_M		77
#define K_N		78
#define K_O		79
#define K_P		80
#define K_Q  	81
#define K_R 	82
#define K_S		83
#define K_T		84
#define K_U		85
#define K_V		86
#define K_W		87
#define K_X		88
#define K_Y		89
#define K_Z		90

#define K_N_0								0x60
#define K_N_1								0x61
#define K_N_2								0x62
#define K_N_3								0x63
#define K_N_4								0x64
#define K_N_5								0x65
#define K_N_6								0x66
#define K_N_7								0x67
#define K_N_8								0x68
#define K_N_9								0x69
#define K_N_MULTIPLICATION	0x6A
#define K_N_PLUS						0x6B
#define K_N_ENTER						0x6C
#define K_N_MINUS						0x6D
#define K_N_DECIMAL					0x6E
#define K_N_DIVISION				0x6F

#define K_F1  	0x70
#define K_F2  	0x71
#define K_F3  	0x72
#define K_F4  	0x73
#define K_F5  	0x74
#define K_F6  	0x75
#define K_F7  	0x76
#define K_F8  	0x77
#define K_F9  	0x78
#define K_F10  	0x79
#define K_F11  	0x7A
#define K_F12  	0x7B
#define K_F13 	0x7C
#define K_F14  	0x7D
#define K_F15  	0x7E
#define K_F16  	0x7F

#define K_NUM_LOCK	  	0x90
#define K_SCROLL_LOCK		0x91


#define K_a  	K_PRIVATE 
#define K_b 	(K_PRIVATE|1)
#define K_c		(K_PRIVATE|2)
#define K_d		(K_PRIVATE|3)
#define K_e		(K_PRIVATE|4)
#define K_f		(K_PRIVATE|5)
#define K_g		(K_PRIVATE|6)
#define K_h		(K_PRIVATE|7)
#define K_i		(K_PRIVATE|8)
#define K_j		(K_PRIVATE|9)
#define K_k		(K_PRIVATE|10)
#define K_l		(K_PRIVATE|11)
#define K_m		(K_PRIVATE|12)
#define K_n		(K_PRIVATE|13)
#define K_o		(K_PRIVATE|14)
#define K_p		(K_PRIVATE|15)
#define K_q  	(K_PRIVATE|16)
#define K_r 	(K_PRIVATE|17)
#define K_s		(K_PRIVATE|18)
#define K_t		(K_PRIVATE|19)
#define K_u		(K_PRIVATE|20)
#define K_v		(K_PRIVATE|21)
#define K_w		(K_PRIVATE|22)
#define K_x		(K_PRIVATE|23)
#define K_y		(K_PRIVATE|24)
#define K_z		(K_PRIVATE|25)

#define K_TILDE  				(0x7e) //~
#define K_DUNHAO 				(0x27) //`
#define K_EXCLAMATION			(0x21) //!
#define K_ALPHA					(0x40) //@
#define K_POUND 				(0x23) //#
#define K_DOLLAR				(0x24) //$
#define K_PERCENT				(0x25) //%
#define K_OR					(0x5e) //^
#define K_AND					(0x26) //&
#define K_MULTIPLICATION		(0x2a) //*
#define K_PARENTHESE_OPEN		(0x28) //(
#define K_PARENTHESE_CLOSE		(0x29) //)
#define K_DASH					(0x5f) //_
#define K_HYPHEN				(0x2d) //-
#define K_PLUS					(0x2b) //+
#define K_EQUAL					(0x3d) //=
#define K_BRACE_OPEN			(0x7b) //{
#define K_BRACE_CLOSE			(0x7d) //}
#define K_BRACKET_OPEN			(0x5b) //[
#define K_BRACKET_CLOSE			(0x5d) //]
#define K_VERTICAL_BAR			(0x7c) //|
#define K_BACKSLASH				(0x5c) //\\/
#define K_COLON					(0x3a) //:
#define K_SEMICOLON				(0x3b) //;
#define K_DOUBLE_QUOTATION		(0x22) //"
#define K_SINGLE_QUOTATION		(0x60) //'
#define K_ANGLE_BRACKET_OPEN	(0x3c) //<
#define K_ANGLE_BRACKET_CLOSE	(0x3e) //>
#define K_COMMA					(0x2c) //,
#define K_FULL_STOP				(0x2e) //.
#define K_QUESTION				(0x3f) //?
#define K_SLASH					(0x2f) ///

#define K_ZHO	(K_PRIVATE|58)
#define K_ENG	(K_PRIVATE|59)



#endif
