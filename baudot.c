/*
 ccitt converter
 by Hanno Behrens (pebbles@schattenlauf.de)
 published under project Jathene on http://pebbles.schattenlauf.de/ 2007
 Licence MIT
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <wctype.h>
#include <wchar.h>

/*
 *  0 Letters
 * then 3 type of figures
 *  1 ITA-2 version of the figures case.
 *  2 U.S. version of the figures case.
 *  3 A mix of the two. This is what seems to be what people actually use.
 */

int figure=1;
int decode=false;

enum {ORDER_FROMRIGHT, ORDER_BAUDOT, ORDER_FROMLEFT};  // Bits in paper countet as 54·123, 54·321, 12·345

typedef struct{
	char *name;
	unsigned char bitmode;
	unsigned char shifts;
	unsigned char table[4];
	unsigned char shiftcode[4];
}tape;

tape codetable[]={
	{//0
	"CCITT#2",
	ORDER_FROMLEFT,
	2,
	{0,1},
	{0x1f, 0x1b}
},{//1
	"CCITT#2/US",
	ORDER_FROMLEFT,
	2,
	{0,3},
	{0x1f, 0x1b}
},{//2
	"ITA-2",
	ORDER_FROMLEFT,
	2,
	{0,2},
	{0x1f, 0x1b}
},{//3
	"LEO (Lyon's Electronic Office)",
	ORDER_FROMRIGHT,
	1,
	{4},
	{255},
},{//4
	"English Electric Deuce",
	ORDER_FROMRIGHT,
	2,
	{5,6},
	{'\025','\024'}
},{//5
	"Illiac",
	ORDER_FROMRIGHT,
	2,
	{7,8},
	{'\024','\033'}
},{//6
	"Baudot (CCITT#1)",
	ORDER_FROMRIGHT,
	2,
	{9,10},
	{'\x10','\x08'}
},{//7
	"Murray (CCITT#2)",
	ORDER_FROMRIGHT,
	2,
	{11, 12},
	{'\x1f', '\x1b'}
},{//8
	"Baudot (Original UK)",
	ORDER_FROMRIGHT,
	2,
	{35,36},
	{'\x10','\x08'}
},{//9
	"Baudot (Original Continental)",
	ORDER_FROMRIGHT,
	2,
	{37,38},
	{'\x10', '\x08'}
},{//10
	"Baudot-Murray (Variant 0x04/0x1b)",
	ORDER_FROMRIGHT,
	2,
	{39,40},
	{'\x04','\x1b'}
},{//11
	"Baudot (Variant ITA1)",
	ORDER_FROMRIGHT,
	2,
	{45,46},
	{'\x10','\x08'}
},{//12
	"Baudot (Variant ITA2/US-TTY)",
	ORDER_FROMRIGHT,
	2,
	{41,42},
	{'\x1f','\x1b'}
},{//13
	"Baudot (Variant ITA2)",
	ORDER_FROMRIGHT,
	2,
	{41,43},
	{'\x1f','\x1b'}
},{//14
	"Baudot (Variant ITA2/Weather)",
	ORDER_FROMRIGHT,
	2,
	{41,44},
	{'\x1f','\x1b'}
},{//15
	"Baudot (Variant MKT2 Russian)",
	ORDER_FROMRIGHT,
	2,
	{47,48},
	{'\x1f','\x1b'}
},{//16
	"Alcor (Algol 60, DIN)",
	ORDER_FROMRIGHT,
	2,
	{11,13},
	{'\037', '\033'}
},{//17
	"Teletype (US CCITT#2)",
	ORDER_FROMRIGHT,
	2,
	{11, 14},
	{'\037', '\033'}
},{//18
	"AT&T (US Stock Market)",
	ORDER_FROMRIGHT,
	2,
	{11,15},
	{'\037','\033'}
},{//19
	"Flexowriter",
	ORDER_FROMRIGHT,
	2,
	{11,16},
	{'\037','\033'}
},{//20
	"Metro-Vick 950",
	ORDER_FROMRIGHT,
	2,
	{17,18},
	{'\0', '\033'}
},{//21
	"Elliott 405",
	ORDER_FROMRIGHT,
	2,
	{19,20},
	{'\037', '\033'}
},{//22
	"EMI 2400",
	ORDER_FROMRIGHT,
	2,
	{21,22},
	{'\0', '\037'}
},{//23
	"BSI Proposal",
	ORDER_FROMRIGHT,
	2,
	{23,24},
	{'\037','\033'}
},{//24
	"Stantec Zebra",
	ORDER_FROMLEFT,
	1,
	{25},
	{255}
},{//25
	"EMI M/C Tool",
	ORDER_FROMLEFT,
	1,
	{26},
	{255}
},{//26
	"EMI 1100",
	ORDER_FROMLEFT,
	2,
	{27, 28},
	{'\036','\0'}
},{//27
	"Pegasus-Mercury",
	ORDER_FROMLEFT,
	2,
	{29,30},
	{'\033','\0'}
},{//28
	"Pegasus-Flexowriter",
	ORDER_FROMLEFT,
	4,
	{31,32,33,34},
	{'\033','\0','\036','\035'}
}
};

static wchar_t letter[][32] = {
	// Lorenz:
	// --Name   => "/T3O9HNM4LRGIPCVEZDBSYFXAWJ5UQK8",
	// Letter => "iTrO_HNMnLRGIPCVEZDBSYFXAWJ<UQK>",
	// Figure => "i5r9_h,.n)4g80:=3+d?'6f/-2j<71(>"
	{	//0 Letters/Standard
        '\0',   'E',    '\n',   'A',    ' ',    'S',    'I',    'U',
        '\r',   'D',    'R',    'J',    'N',    'F',    'C',    'K',
        'T',    'Z',    'L',    'W',    'H',    'Y',    'P',    'Q',
        'O',    'B',    'G',    L'␏',    'M',    'X',    'V',    L'␎'
	},
	{	//1 fig: Mix of ITA-2 and U.S.
        '\0',   '3',    '\n',   '-',    ' ',    '\'',   '8',    '7',
        '\r',   '$',    '4',    '\a',   ',',    '!',    ':',    '(',
        '5',    '+',    ')',    '2',    '#',    '6',    '0',    '1',
        '9',    '?',    '&',    L'␏',    '.',    '/',    '=',   L'␎'
	},
	{	//2 fig: ITA-2
        '\0',   '3',    '\n',   '-',    ' ',    '\'',   '8',    '7',
        '\r',   L'·',    '4',    '\a',   ',',    L'·',    ':',    '(',
        '5',    '+',    ')',    '2',    L'·',    '6',    '0',    '1',
        '9',    '?',    L'·',    L'␏',    '.',    '/',    '=',    L'␎'
	},
	{	//3 fig: U.S.
        '\0',   '3',    '\n',   '-',    ' ',    '\a',   '8',    '7',
        '\r',   '$',    '4',    '\'',   ',',    '!',    ':',    '(',
        '5',    '"',    ')',    '2',    '#',    '6',    '0',    '1',
        '9',    '?',    '&',    L'␏',    '.',    '/',    ';',    L'␎'
	},
	{	//4 LEO
		'\0', '-', '\002', L'·', L'·', L'·', '\006', L'·',
		L'·', L'·', L'·', L'·', L'·', L'·', L'·', L'·', 
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 0x247d, 0x247e, L'·', L'·', L'·', '\b'
	},
	{	//5 EED Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'I', '.', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', L'␏', L'␎', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z', '\n', '\b'
	},
	{	//6 EED Fig
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 0x247d, 0x247e, 0x247f, 0x2480, 0x2481, 0x2482,
		0x2483, 0x2484, 0x2485, 0x2486, L'␏', L'␎', L'·', L'·',
		L'·', '+', '-', ',', '\034', ' ', '\n', '\b'
	},
	{	//7 Illiac Let 
		'P', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 
		'I', 'O', 'K', 'S', 'N', 'J', 'F', 'L', 
		'\b', 'D', '\n', 'B', L'␎', 'V', 'A', 'X', 
		'\b', 'G', 'M', L'␏', 'H', 'C', 'Z', ' '

	},
	{	//8 Illiac Fig
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', '+', '-', 'N', 'J', 'F', 'L', 
		'\b', '$', '\n', '(', L'␎', ',', ')', '/', 
		'\b', '=', '.', L'␏', '\'', ':', '*', ' '
	},
	{	// 9 Baudot Let 0x20 0x10
		'\0', 'Y', 'E', 'I', 'A', 'U', L'É', 'O', // É=/?
		L'␏', 'B', 'G', 'F', 'J', 'C', 'H', 'D', 
		' ', 'S', 'X', 'W', '-', 'T', 'Z', 'V', 
		'\b', 'R', 'M', 'N', 'K', 'Q', 'L', 'P'
	},
	{	// 10 Baudot Fig
		'\0', '3', '2', L'³', '1', '4', L'¹', '5', 
		' ', '8', '7', 0x2075, '6', '9', 0x2074, '0', 
		L'␎', 0x2077, 0x2079, '?', '.', L'²', ':', '\'', 
		'\b', '-', ')', L'£', '(', '/', '=', '+'
	},
	{	// 11 Murray Let
		'\0', 'T', '\r', 'O', ' ', 'H', 'N', 'M', 
		'\n', 'L', 'R', 'G', 'I', 'P', 'C', 'V', 
		'E', 'Z', 'D', 'B', 'S', 'Y', 'F', 'X', 
		'A', 'W', 'J', L'␏', 'U', 'Q', 'K', L'␎'
	},
	{	// 12 Murray Fig
		'\0', '5', '\r', '9', ' ', L'·', ',', '.', 
		'\n', ')', '4', L'·', '8', '0', ':', ';', 
		'3', '"', '\022', '?', '\'', '6', L'·', '/', 
		'-', '2', '\a', L'␏', '7', '1', '(', L'␎'
	},
	{	// 13 Alcor Fig
		'\0', '5', '\r', '9', ' ', 0x2469, ',', '.', 
		'\n', ')', '4', ']', '8', '0', ':', '=', 
		'3', '+', 0x2670, '*', '\'', '6', '[', '/', 
		'-', '2', ';', L'␏', '7', '1', '(', L'␎'
	},
	{	// 14 Teletype Fig
		'\0', '5', '\r', '9', ' ', '#', ',', '.', 
		'\n', ')', '4', '&', '8', '0', ':', ';', 
		'3', '"', '$', '?', '\'', '6', '!', '/', 
		'-', '2', '\a', L'␏', '7', '1', '(', L'␎'
	},
	{	// 15 AT&T Fig
		'\0', '5', '\r', '9', ' ', '#', '%', '.', 
		'\n', L'¾', '4', '&', '8', '0', 0x215b, 0x215c, 
		'3', '\"', '$', 0x215d, '\a', '6', L'¼', '/', 
		'-', '2', ',', L'␏', '7', '1', L'½', L'␎'
	},
	{	// 16 Flexowriter Fig
		'\0', '5', '\r', '9', ' ', '(', ',', '.', 
		'\n', ')', '4', '&', '8', '0', '\016', '\017', 
		'3', '\021', '\022', '$', '/', '6', ',', '\027', 
		'-', '2', '\032', L'␏', '7', '1', '\036', L'␎'
	},
	{	// 17 Metro-Vick950 Let
		L'␎', 'T', '\r', 'O', ' ', 'H', 'N', 'M', 
		'\n', 'L', 'R', 'G', 'I', 'P', 'C', 'V', 
		'E', 'Z', 'D', 'B', 'S', 'Y', 'F', 'X', 
		'A', 'W', 'J', L'␏', 'U', 'Q', 'K', '\b'
	},
	{	// 18 Metro-Vick950 Fig
		L'␎', '?', '\r', '+', ' ', L'£', ',', '.', 
		'\n', ')', '*', L'&', '%', '-', L'·', '=', 
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', L'·', L'␏', '!', '/', '(', '\b'
	},
	{	// 19 Elliott405 Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', L'␏', ' ', '\r', '\n', L'␎'
	},
	{	// 20 Elliott405 Fig
		'\0', '1', '2', '*', '4', '$', '=', '7', 
		'8', '!', ',', '+', ':', '-', '.', '%', 
		'0', '(', ')', '3', '?', '5', '6', '/', 
		L'·', '9', L'£', L'␏', ' ', '\r', '\n', L'␎'
	},
	{	// 21 EMI2400 Let
		L'␎', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '&', ' ', '\n', '\'', L'␏'
	},
	{	// 22 EMI2400 Fig
		L'␎', '1', '2', ')', '4', L'½', '%', '7', 
		'8', L'£', '+', 0x247e, '/', '-', '.', '\0', 
		'0', L'→', '(', '3', 0x2153, '5', '6', '*', 
		L'¼', '9', 0x247d, L'¾', L'×', '\n', '\n', L'␏'
	},
	{	// 23 BSI Proposal Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', L'␏', ' ', '\n', '\'', L'␎'
	},
	{	// 24 BSI Proposal Fig
		'\0', '1', '2', L'·', '4', L'·', L'·', '7',
		'8', L'·', '+', 0x247e, L'·', '-', '.', L'·', 
		'0', L'·', '\022', '3', L'·', '5', '6', L'·', 
		L'·', '9', 0x247d, L'␏', ' ', '\r', '\n', L'␎' 
	},
	{	// 25 Stantec Zebra
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', 'K', 'Q', '.', 'L', 'R', 'I', 
		'B', 'C', 'D', 'E', 'T', 'U', 'V', 'N', 
		'A', 'X', '+', '-', 'Y', 'Z', 'P', '#'
	},
	{	// 26 EMI M/C Tool
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', 'S', 'T', 'U', 'V', ' ', '\n', 
		'\020', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
		'J', 'K', 'L', 'M', 'N', 'A', 'R', '\b'
	},
	{	// 27 EMI 1100 Let
		L'␏', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 
		'X', 'Y', 'Z', 'D', 'M', 'U', '&', ' ', 
		'A', 'B', 'C', 'E', 'F', 'G', 'H', 'I', 
		'J', 'K', 'L', 'N', 'O', '\n', L'␎', '\b'
	},
	{	// 28 EMI 1100 Fig
		L'␏', '%', L'£', '-', '*', '/', '.', '%', 
		L'¾', '\'', '(', ')', '+', L'×', L'½', ' ', 
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', 0x247d, 0x247e, 0x247f, '\n', L'␎', '\b'
	},
	{	// 29 Pegasus-Mercury Let
		L'␏', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', L'␎', '.', '?', L'£', '\b'
	},
	{	//30 Pegasus-Mercury Fig
		L'␏', '1', '2', '*', '4', '(', ')', '7', 
		'8', L'≠', '=', '-', L'υ', '\n', ' ', ',', 
		'0', '>', 0x2265, '3', L'→', '5', '6', '/', 
		L'×', '9', '+', L'␎', '.', L'η', '\r', '\b'
	},
	{	//31 Pegasus-Flexowriter Let Lower
		L'␏', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 
		'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 
		'x', 'y', 'z', L'␎', ' ', L'␏', L'␏', '\b'
	},
	{	//32 Pegasus-Flexowriter Fig Lower
		L'␏', '1', '2', '\003', '4', '\005', '\006', '7', 
		'8', '+', '-', 0x247e, 0x2409, '\n', '.', L'·', 
		'0', '\021', '\022', '3', '\024', '5', '6', '\b', 
		'\030', '9', 0x247d, L'␎', ' ', L'␏', L'␏', '\b'
	},
	{	//33 Pegasus-Flexowriter Let Upper
		L'␏', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', L'␎', ' ', L'␏', L'␏', '\b'
	},
	{	//34 Pegasus-Flexowriter Fig Upper
		L'␏', L'¼', L'½', '\003', '/', '\005', '\006', '<', 
		'>', '\'', '\r', ':', '\t', '\n', ',', L'·', 
		L'£', '\021', '\022', L'¾', '\024', '&', '?', '\b', 
		'\030', '%', '=', L'␎', ' ', L'␏', L'␏', '\b'
	},
	{	// 35 Baudot Original UK Let
		'\0', 'A', 'E', '/', 'Y', 'U', 'I', 'O', 
		L'␏', 'J', 'G', 'H', 'B', 'C', 'F', 'D', 
		' ', '-', 'X', 'Z', 'S', 'T', 'W', 'V', 
		'\b', 'K', 'M', 'L', 'R', 'Q', 'N', 'P'
	},
	{	// 36 Baudot Original UK Fig
		'\0', '1', '2', L'⅟', '3', '4', L'³', '5', 
		' ', '6', '7', L'¹', '8', '9', L'⁵', '0', 
		L'␎', '.', L'⁹', ':', L'⁷', L'²', '?', '\'', 
		'\b', '(', ')', '=', '-', '/', L'£', '+'
	},
	{	// 37 Baudot Original Continental Let
		'\0', 'A', 'E', L'É', 'Y', 'U', 'I', 'O', 
		L'␏', 'J', 'G', 'H', 'B', 'C', 'F', 'D', 
		' ', '-', 'X', 'Z', 'S', 'T', 'W', 'V', 
		'\b', 'K', 'M', 'L', 'R', 'Q', 'N', 'P'
	},
	{	// 38 Baudot Original Continental Fig
		'\0', '1', '2', '&', '3', '4', L'°', '5', 
		' ', '6', '7', L'ʰ', '8', '9', L'ᶠ', '0', 
		L'␎', '.', ',', ':', ';', '!', '?', '\'', 
		'\b', '(', ')', '=', '-', '/', L'№', '%'
	},
	{	// 39 Baudot-Murray Let
		' ', 'E', '\t', 'A', L'␎', 'S', 'I', 'U', 
		'\n', 'D', 'R', 'J', 'N', 'F', 'C', 'K', 
		'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 
		'O', 'B', 'G', L'␏', 'M', 'X', 'V', '\b'
	},
	{	// 40 Baudot-Murray Fig
		' ', '3', '\t', L'·', L'␎', '\'', '8', '7', 
		'\n', L'²', '4', L'⁷', '-', L'⅟', '(', L'⁹', 
		'5', '.', '/', '2', L'⁵', '6', '0', '1', 
		'9', '?', L'³', L'␏', ',', L'£', ')', '\b'
	},
	{	// 41 Baudot ITA2 Let
		'\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U', 
		'\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K', 
		'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 
		'O', 'B', 'G', L'␏', 'M', 'X', 'V', '\b'
	},
	{	// 42 Baudot ITA2/US Fig
		'\0', '3', '\n', '-', ' ', '\a', '8', '7', 
		'\r', '$', '4', '\'', ',', '!', ':', '(', 
		'5', '"', ')', '2', '#', '6', '0', '1', 
		'9', '?', '?', '&', L'␏', '.', '/', L'␎'
	},
	{	// 43 Baudot ITA2 Fig
		'\0', '3', '\n', '-', ' ', '\'', '8', '7', 
		'\r', L'␅', '4', '\a', ',', '!', ':', '(', 
		'5',  '+', ')', '2',  L'£','6', '0', '1', 
		'9', '?', '&', L'␏', '.', '/', '=', L'␎' 
	},
	{	// 44 Baudot ITA2 Weather Fig
		'-', '3', '\n', L'↑', ' ', '\a', '8', '7', 
		'\r', L'↗', '4', L'↙', L'◍', L'→', L'○', L'←', 
		'5',  '+', L'↖', '2',  L'↓','6', '0', '1', 
		'9', L'⊕', L'↘', L'·', '.', '/', L'⦶', L'␎' 
	},
	{	// 45 Baudot ITA1 Let
		'\0', 'A', 'E', '\r', 'Y', 'U', 'I', 'O', 
		L'␏', 'J', 'G', 'H', 'B', 'C', 'F', 'D', 
		' ', '\n', 'X', 'Z', 'S', 'T', 'W', 'V', 
		'\b', 'K', 'M', 'L', 'R', 'Q', 'N', 'P'
	},
	{	// 46 Baudot ITA1 Fig
		'\0', '1', '2', '\r', '3', '4', L'␑', '5', 
		' ', '6', '7', '+', '8', '9', L'␒', '0', 
		L'␎', '\n', ',', ':', '.', L'␓', '?', '\'', 
		'\b', '(', ')', '=', '-', '/', L'␔', '%'
	},
	{	// 47 Baudot Russian MKT2 Let
		'\0', L'Е', '\n', L'А', ' ', L'С', L'И', L'У', 
		'\r', L'Д', L'Р', L'Й', L'Н', L'Ф', L'Ц', L'К', 
		L'Т', L'З', L'Л', L'В', L'Х', L'Ы', L'П', L'Я', 
		L'О', L'Б', L'Г', L'␏', L'М', L'Ь', L'Ж', '\b'
	},
	{	// 48 Baudot Russian MKT2 Fig
		'\0', '3', '\n', '-', ' ', '\'', '8', '7', 
		'\r', L'␅', '4', '\a', ',', L'Э', ':', '(', 
		'5',  '+', ')', '2',  L'Щ','6', '0', '1', 
		'9', '?', L'Ш', L'␏', '.', '/', '=', L'␎'
	},
};
//		'', '', '', '', '', '', '', '', 

wint_t fgetutf8c(FILE *fp){
	wint_t utf;
	int ch,i;

	ch=fgetc(fp);
	if(ch==EOF) {
		return WEOF;
	}
	if((0x80 & ch) == 0) {
		return ch;
	}
	for(i=0; (ch & 0x40>>i) != 0;i++);
	if(i>4) i=4;
	utf=ch&(0x3f>>i);
	do{
		ch=fgetc(fp);
		if(ch==EOF) return WEOF;
		if((0xc0&ch) != 0x80) return WEOF;
		utf=(utf<<6)|(ch&0x3f);
	}while(--i > 0);
	return utf;
}

wint_t fpututf8c(wint_t utf, FILE *fp){
	int res, i;
	if(utf<128) {
		return fputc(utf, fp);
	}else
	if(utf<2048) {
		res=fputc(0xc0|(utf>>6), fp);
		if(res==EOF) return WEOF;
		for(i=0;i>=0;i--){
			res=fputc(0x80|((utf>>(i*6))&0x3f), fp);
			if(res==EOF) return WEOF;
		}
	}else
	if(utf<65536) {
		res=fputc(0xe0|(utf>>12), fp);
		if(res==EOF) return WEOF;

		for(i=1;i>=0;i--){
			res=fputc(0x80|((utf>>(i*6))&0x3f), fp);
			if(res==EOF) return WEOF;
		}
	}else{//>=65536
		res=fputc(0xf0|(utf>>18), fp);
		if(res==EOF) return WEOF;

		for(i=2;i>=0;i--){
			res=fputc(0x80|((utf>>(i*6))&0x3f), fp);
			if(res==EOF) return WEOF;
		}
	}
	return utf;
}
// Bits in paper countet as BAUDOT 54·123, FROMRIGHT 54·321, FROMLEFT 12·345
unsigned char changebitmode(unsigned char c, int from, int to){
	if(from==to || to==-1 || from==-1) return c;
	//assume normal case is ORDER_FROMRIGHT
	switch(from){
	case ORDER_BAUDOT:	//Change bits 0+2
		c=c&0x1a|(c&1)<<2|(c&4)>>2;
		break;
	case ORDER_FROMLEFT:	//Inverse direction of all bits (but 2 cause its in the middle
		c=c&4|(c&1)<<4|(c&2)<<2|(c&8)>>2|(c&0x10)>>4;
		break;
	default:
		break;
	}
	//c is normalized not change
	switch(to){
	case ORDER_BAUDOT:
		c=c&0x1a|(c&1)<<2|(c&4)>>2;
		break;
	case ORDER_FROMLEFT:
		c=c&4|(c&1)<<4|(c&2)<<2|(c&8)>>2|(c&0x10)>>4;
		break;
	default:
		break;
	}	
	return c;
}

size_t baudot_enc(wint_t utf, size_t *mode, int code, int bitmode, int autoreset){
	size_t i,tab;

	if(iswlower(utf) && codetable[code].shifts<4) utf = towupper(utf); //most are uppercase only
	if(utf>=0x0430 && utf<=0x044f) utf-=0x20; // а-я Russian to upper
	for(i=0; i<32; i++){ // check active codetab first
		if(utf == letter[codetable[code].table[*mode]][i]) goto found;
	}
	for(tab=0; tab<codetable[code].shifts; tab++){
		if(tab == *mode) continue; // we checked already
		for(i=0; i<32; i++){ // check the other tabs
			//printf("%d %d\n", utf, letter[codetable[code].table[tab]][i]);
			if(utf == letter[codetable[code].table[tab]][i]) goto settab;
		}
	}
settab:
	putchar(changebitmode(codetable[code].shiftcode[tab], codetable[code].bitmode, bitmode));
	*mode=tab;
found:
	putchar(changebitmode(i, codetable[code].bitmode, bitmode));
/*
	//printf("%02x ",c);
	//if same char in current map stay on map
	if(utf==letter[codetable[code].table[*mode]][i]) tab=*mode;
	
	//Spaces as Let/Fig shifts as in Baudot(6)
	if(utf==letter[codetable[code].table[*mode]][codetable[code].shiftcode[*mode]]){
		c=codetable[code].shiftcode[*mode];
	}

	//Let <> Fig
	if((*mode&1)!=(tab&1)) {
		*mode^=1; 
		putchar(changebitmode(codetable[code].shiftcode[tab&1],codetable[code].bitmode,bitmode));
	}
	//lc <> uc
	if((*mode&2)!=(tab&2)) {
		*mode^=2; 
		putchar(changebitmode(codetable[code].shiftcode[2|(tab>>1)],codetable[code].bitmode,bitmode));
	}
	putchar(changebitmode(c,codetable[code].bitmode,bitmode));
*/
	return i;
}

wint_t baudot_dec(wchar_t baudot, size_t *mode, int code, int bitmode, int autoreset){
	wint_t c=0;

	baudot = changebitmode(baudot&0x1f, bitmode, codetable[code].bitmode);
	
	if(letter[codetable[code].table[*mode]][baudot]==L'␎' || letter[codetable[code].table[*mode]][baudot]==L'␏'){
		for(size_t m=0; m<codetable[code].shifts; m++){
			if(codetable[code].shiftcode[m]==baudot){
				*mode=m;
				//fprintf(stdout, "mode %ld\n",m);
				return c;
			}
		}
	}
/*
	for(tab=0; tab<codetable[code].shifts; tab++){
		if(baudot==codetable[code].shiftcode[tab]) {
			if(L'␏'!=letter[codetable[code].table[*mode]][baudot] && L'␎'!=letter[codetable[code].table[*mode]][baudot]) 
				fpututf8c(letter[codetable[code].table[*mode]][baudot], stdout);
			if(tab<2) 
				*mode=(*mode&2)|tab;
			else 
				*mode=(*mode&1)|((tab&1)<<1);
			return c;
		}
	}
*/
	c=letter[codetable[code].table[*mode]][baudot];
	//fprintf(stderr, "%02X %d %c\n", baudot, c, c);//
	fpututf8c(c, stdout);
	return c;
}

void list(){
	char *mode[]={"FromRight", "Baudot", "FromLeft"};
	int i;
	for(i=0; i<sizeof(codetable)/sizeof(tape); i++){
		fprintf(stderr,"%2d %s (Bitmode=%d - %s)\n", i, codetable[i].name,codetable[i].bitmode,mode[codetable[i].bitmode]);
	}
}

void help(char *name){
	fprintf(stderr,"%s [OPTION]\n"
		"  Encodes or decodes UTF-8 to and from Baudot-like 5-tape text encoding\n"
		"  uses stdin and stdout as input and output\n"
		"  © 2007-2025 Hanno Behrens DL7HH <behrens.hanno@gmail.com> CC-BY\n\n"
		"  Available Options:\n"
		"   -h	this help\n"
		"   -l	list of available codings\n"
		"   -v  print verbose tables on stderr\n"
		"   -d	decode from 5-tape to utf\n"
		"   -cNUM	used coding (default=7)\n"	
		"   -bBIT	bitmode (default -1=standard for coding)\n",name);
}


int main(int argc, char **argv){
	int opt, code=7,bitmode=-1,autoreset=0;
	size_t mode;
	bool verbose=false;
	wchar_t ch;
//	extern char *optarg;

	while(-1 != (opt=getopt(argc, argv, "dvlc:b:h"))){
		switch(opt){
		case 'd':
			decode=true;
			break;
		case 'l':
			list();
			exit(0);
			break;
		case 'c':
			code=atoi(optarg);
			if(code>=sizeof(codetable)/sizeof(tape) || code<0){
				fprintf(stderr, "***Error: no table %d\n", code);
				return 20;
			}
			break;
		case 'b':
			bitmode=atoi(optarg);
			break;
		case 'v':
			verbose=true;
			break;
		case 'h':
		case '-':
			help(argv[0]);
			exit(0);
			break;
		}
	}
	if(verbose) fprintf(stderr, "Table %d: %s %s Bitmode %d\n", code, codetable[code].name, decode?"decoding":"encoding", bitmode);
	mode=0;
	while((ch=fgetutf8c(stdin))!=WEOF){
		if(decode){
			baudot_dec(ch, &mode, code, bitmode, autoreset);
		}else{
			baudot_enc(ch, &mode, code, bitmode, autoreset);
		}
	}
}
