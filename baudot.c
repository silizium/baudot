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
	{'\020','\010'}
},{//7
	"Murray (CCITT#2)",
	ORDER_FROMRIGHT,
	2,
	{11, 12},
	{'\037', '\033'}
},{//8
	"Alcor (Algol 60, DIN)",
	ORDER_FROMRIGHT,
	2,
	{11,13},
	{'\037', '\033'}
},{//9
	"Teletype (US CCITT#2)",
	ORDER_FROMRIGHT,
	2,
	{11, 14},
	{'\037', '\033'}
},{//10
	"AT&T (US Stock Market)",
	ORDER_FROMRIGHT,
	2,
	{11,15},
	{'\037','\033'}
},{//11
	"Flexowriter",
	ORDER_FROMRIGHT,
	2,
	{11,16},
	{'\037','\033'}
},{//12
	"Metro-Vick 950",
	ORDER_FROMRIGHT,
	2,
	{17,18},
	{'\0', '\033'}
},{//13
	"Elliott 405",
	ORDER_FROMRIGHT,
	2,
	{19,20},
	{'\037', '\033'}
},{//14
	"EMI 2400",
	ORDER_FROMRIGHT,
	2,
	{21,22},
	{'\0', '\037'}
},{//15
	"BSI Proposal",
	ORDER_FROMRIGHT,
	2,
	{23,24},
	{'\037','\033'}
},{//16
	"Stantec Zebra",
	ORDER_FROMLEFT,
	1,
	{25},
	{255}
},{//17
	"EMI M/C Tool",
	ORDER_FROMLEFT,
	1,
	{26},
	{255}
},{//18
	"EMI 1100",
	ORDER_FROMLEFT,
	2,
	{27, 28},
	{'\036','\0'}
},{//19
	"Pegasus-Mercury",
	ORDER_FROMLEFT,
	2,
	{29,30},
	{'\033','\0'}
},{//20
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
        'O',    'B',    'G',    L'·',    'M',    'X',    'V',    L'·'
	},
	{	//1 fig: Mix of ITA-2 and U.S.
        '\0',   '3',    '\n',   '-',    ' ',    '\'',   '8',    '7',
        '\r',   '$',    '4',    '\a',   ',',    '!',    ':',    '(',
        '5',    '+',    ')',    '2',    '#',    '6',    '0',    '1',
        '9',    '?',    '&',    L'·',    '.',    '/',    '=',   L'·'
	},
	{	//2 fig: ITA-2
        '\0',   '3',    '\n',   '-',    ' ',    '\'',   '8',    '7',
        '\r',   L'·',    '4',    '\a',   ',',    L'·',    ':',    '(',
        '5',    '+',    ')',    '2',    L'·',    '6',    '0',    '1',
        '9',    '?',    L'·',    L'·',    '.',    '/',    '=',    L'·'
	},
	{	//3 fig: U.S.
        '\0',   '3',    '\n',   '-',    ' ',    '\a',   '8',    '7',
        '\r',   '$',    '4',    '\'',   ',',    '!',    ':',    '(',
        '5',    '"',    ')',    '2',    '#',    '6',    '0',    '1',
        '9',    '?',    '&',    L'·',    '.',    '/',    ';',    L'·'
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
		'O', 'P', 'Q', 'R', '\025','\024', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z', '\n', '\b'
	},
	{	//6 EED Fig
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 0x247d, 0x247e, 0x247f, 0x2480, 0x2481, 0x2482,
		0x2483, 0x2484, 0x2485, 0x2486, '\025','\024', L'·', L'·',
		L'·', '+', '-', ',', '\034', ' ', '\n', '\b'
	},
	{	//7 Illiac Let
		'P', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 
		'I', 'O', 'K', 'S', 'N', 'J', 'F', 'L', 
		'\b', 'D', '\n', 'B', '\024', 'V', 'A', 'X', 
		'\b', 'G', 'M', '\033', 'H', 'C', 'Z', ' '

	},
	{	//8 Illiac Fig
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', '+', '-', 'N', 'J', 'F', 'L', 
		'\b', '$', '\n', '(', '\024', ',', ')', '/', 
		'\b', '=', '.', '\033', '\'', ':', '*', ' '
	},
	{	// 9 Baudot Let
		'\0', 'Y', 'E', 'I', 'A', 'U', L'É', 'O', // É=/?
		' ', 'B', 'G', 'F', 'J', 'C', 'H', 'D', 
		' ', 'S', 'X', 'W', '-', 'T', 'Z', 'V', 
		'\b', 'R', 'M', 'N', 'K', 'Q', 'L', 'P'
	},
	{	// 10 Baudot Fig
		'\0', '3', '2', L'³', '1', '4', L'¹', '5', 
		' ', '8', '7', 0x2075, '6', '9', 0x2074, '0', 
		' ', 0x2077, 0x2079, '?', '.', L'²', ':', '\'', 
		'\b', '-', ')', L'£', '(', '/', '=', '+'
	},
	{	// 11 Murray Let
		'\0', 'T', '\r', 'O', ' ', 'H', 'N', 'M', 
		'\n', 'L', 'R', 'G', 'I', 'P', 'C', 'V', 
		'E', 'Z', 'D', 'B', 'S', 'Y', 'F', 'X', 
		'A', 'W', 'J', '\033', 'U', 'Q', 'K', '\037'
	},
	{	// 12 Murray Fig
		'\0', '5', '\r', '9', ' ', L'·', ',', '.', 
		'\n', ')', '4', L'·', '8', '0', ':', ';', 
		'3', '"', '\022', '?', '\'', '6', L'·', '/', 
		'-', '2', '\a', '\033', '7', '1', '(', '\037'
	},
	{	// 13 Alcor Fig
		'\0', '5', '\r', '9', ' ', 0x2469, ',', '.', 
		'\n', ')', '4', ']', '8', '0', ':', '=', 
		'3', '+', 0x2670, '*', '\'', '6', '[', '/', 
		'-', '2', ';', '\033', '7', '1', '(', '\037'
	},
	{	// 14 Teletype Fig
		'\0', '5', '\r', '9', ' ', '#', ',', '.', 
		'\n', ')', '4', '&', '8', '0', ':', ';', 
		'3', '"', '$', '?', '\'', '6', '!', '/', 
		'-', '2', '\a', '\033', '7', '1', '(', '\037'
	},
	{	// 15 AT&T Fig
		'\0', '5', '\r', '9', ' ', '#', '%', '.', 
		'\n', L'¾', '4', '&', '8', '0', 0x215b, 0x215c, 
		'3', '\"', '$', 0x215d, '\a', '6', L'¼', '/', 
		'-', '2', ',', '\033', '7', '1', L'½', '\037'
	},
	{	// 16 Flexowriter Fig
		'\0', '5', '\r', '9', ' ', '(', ',', '.', 
		'\n', ')', '4', '&', '8', '0', '\016', '\017', 
		'3', '\021', '\022', '$', '/', '6', ',', '\027', 
		'-', '2', '\032', '\033', '7', '1', '\036', '\037'
	},
	{	// 17 Metro-Vick950 Let
		'\0', 'T', '\r', 'O', ' ', 'H', 'N', 'M', 
		'\n', 'L', 'R', 'G', 'I', 'P', 'C', 'V', 
		'E', 'Z', 'D', 'B', 'S', 'Y', 'F', 'X', 
		'A', 'W', 'J', '\033', 'U', 'Q', 'K', '\b'
	},
	{	// 18 Metro-Vick950 Fig
		'\0', '?', '\r', '+', ' ', L'£', ',', '.', 
		'\n', ')', '*', L'&', '%', '-', L'·', '=', 
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', L'·', '\033', '!', '/', '(', '\b'
	},
	{	// 19 Elliott405 Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '\033', ' ', '\r', '\n', '\037'
	},
	{	// 20 Elliott405 Fig
		'\0', '1', '2', '*', '4', '$', '=', '7', 
		'8', '!', ',', '+', ':', '-', '.', '%', 
		'0', '(', ')', '3', '?', '5', '6', '/', 
		L'·', '9', L'£', '\033', ' ', '\r', '\n', '\037'
	},
	{	// 21 EMI2400 Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '&', ' ', '\n', '\'', '\037'
	},
	{	// 22 EMI2400 Fig
		'\0', '1', '2', ')', '4', L'½', '%', '7', 
		'8', L'£', '+', 0x247e, '/', '-', '.', '\0', 
		'0', L'→', '(', '3', 0x2153, '5', '6', '*', 
		L'¼', '9', 0x247d, L'¾', L'×', '\n', '\n', '\037'
	},
	{	// 23 BSI Proposal Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '&', ' ', '\n', '\'', '\037'
	},
	{	// 24 BSI Proposal Fig
		'\0', '1', '2', L'·', '4', L'·', L'·', '7',
		'8', L'·', '+', 0x247e, L'·', '-', '.', L'·', 
		'0', L'·', '\022', '3', L'·', '5', '6', L'·', 
		L'·', '9', 0x247d, '\033', ' ', '\r', '\n', '\037' 
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
		'\0', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 
		'X', 'Y', 'Z', 'D', 'M', 'U', '&', ' ', 
		'A', 'B', 'C', 'E', 'F', 'G', 'H', 'I', 
		'J', 'K', 'L', 'N', 'O', '\n', '\036', '\b'
	},
	{	// 28 EMI 1100 Fig
		'\0', '%', L'£', '-', '*', '/', '.', '%', 
		L'¾', '\'', '(', ')', '+', L'×', L'½', ' ', 
		'0', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', 0x247d, 0x247e, 0x247f, '\n', '\036', '\b'
	},
	{	// 29 Pegasus-Mercury Let
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '\033', '.', '?', L'£', '\b'
	},
	{	//30 Pegasus-Mercury Fig
		'\0', '1', '2', '*', '4', '(', ')', '7', 
		'8', L'≠', '=', '-', L'υ', '\n', ' ', ',', 
		'0', '>', 0x2265, '3', L'→', '5', '6', '/', 
		L'×', '9', '+', '\n', '.', L'η', '\r', '\b'
	},
	{	//31 Pegasus-Flexowriter Let Lower
		'\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 
		'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 
		'x', 'y', 'z', '\033', ' ', '\035', '\036', '\b'
	},
	{	//32 Pegasus-Flexowriter Fig Lower
		'\0', '1', '2', '\003', '4', '\005', '\006', '7', 
		'8', '+', '-', 0x247e, 0x2409, '\n', '.', L'·', 
		'0', '\021', '\022', '3', '\024', '5', '6', '\b', 
		'\030', '0', 0x247d, '\033', ' ', '\035', '\036', '\b'
	},
	{	//33 Pegasus-Flexowriter Let Upper
		'\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
		'X', 'Y', 'Z', '\033', ' ', '\035', '\036', '\b'
	},
	{	//34 Pegasus-Flexowriter Fig Upper
		'\0', L'¼', L'½', '\003', '/', '\005', '\006', '<', 
		'>', '\'', '\r', ':', '\t', '\n', ',', L'·', 
		L'£', '\021', '\022', L'¾', '\024', '&', '?', '\b', 
		'\030', '%', '=', '\033', ' ', '\035', '\036', '\b'
	}
};
//		'', '', '', '', '', '', '', '', 

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

int baudot_enc(wint_t utf, int *mode, int code, int bitmode, int autoreset){
	int i,tab, c;
	
	if (iswlower(utf) && codetable[code].shifts<4) utf = towupper(utf); //most are uppercase only
	for(tab=0; tab<codetable[code].shifts; tab++){
		for(i=0, c=-1; i<32; i++){ 
			//printf("%d %d\n", utf, letter[codetable[code].table[tab]][i]);
			if(utf == letter[codetable[code].table[tab]][i]) {c=i; break;}
		}
		if(c != -1) break;
	}
	if(c == -1) return c;

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
	return c;
}

int baudot_dec(wint_t baudot, int *mode, int code, int bitmode, int autoreset){
	wint_t c=0;
	int tab;

	baudot = changebitmode(baudot&0x1f, bitmode, codetable[code].bitmode);

	for(tab=0; tab<codetable[code].shifts; tab++){
		if(baudot==codetable[code].shiftcode[tab]) {
			if(tab<2) *mode=(*mode&2)|tab;
			else *mode=(*mode&1)|((tab&1)<<1);
			if(' '==letter[codetable[code].table[*mode]][baudot]) putchar(' ');
			return c;
		}
	}
	c=letter[codetable[code].table[*mode]][baudot];
	fputwc(c, stdout);
	return c;
}

void list(){
	char *mode[]={"FromRight", "Baudot", "FromLeft"};
	int i;
	for(i=0; i<sizeof(codetable)/sizeof(tape); i++){
		printf("%2d %s (Bitmode=%d - %s)\n", i, codetable[i].name,codetable[i].bitmode,mode[codetable[i].bitmode]);
	}
}

void help(char *name){
	fprintf(stderr,"%s [OPTION]\n"
		"  Encodes or decodes UTF-8 to and from Baudot-like 5-tape text encoding\n"
		"  uses stdin and stdout as input and output\n"
		"  © 2007 Hanno Behrens (pebbles@schattenlauf.de) Licence GPL\n\n"
		"  Available Options:\n"
		"   -h	this help\n"
		"   -l	list of available codings\n"
		"   -v  print verbose tables on stderr\n"
		"   -d	decode from 5-tape to utf\n"
		"   -cNUM	used coding (default=7)\n"	
		"   -bBIT	bitmode (default -1=standard for coding)\n",name);
}

wint_t fgetutf8c(FILE *fp){
	wint_t utf, ch;
	int i;
	
	ch=fgetc(fp);
	if(ch==EOF) {
		return WEOF;
	}
	if((0x80&ch) == 0) {
		return ch;
	}
	for(i=0;ch&(0x40>>i)!=0;i++);
	if(i>4) i=4;
	utf=ch&(0x3f>>i);
	do{
		ch=getc(fp);
		if(ch==EOF) return WEOF;
		if((0xc0&ch) != 0x80) return WEOF;
		utf=(utf<<6)|(ch&0x3f);
	}while(--i > 0);
	return utf;
}

int fpututf8c(wint_t utf, FILE *fp){
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

int main(int argc, char **argv){
	int opt,mode,code=7,bitmode=-1,autoreset=0;
	bool verbose=false;
	wint_t ch;
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
