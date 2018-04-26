/*
 uppt unicode paper punch converter
 by Hanno Behrens (pebbles@schattenlauf.de)
 published 2016
 Licence GPL
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wctype.h>
#include <wchar.h>

int figure=1;
int decode=false;

static wchar_t paper[][3]{
	{' ', 'o', '.'}, 	//0 ASCII
	{' ', L'●', L'·'},	// 1 UTF-8
}
enum {BORDER_TL, BORDER_TM, BORDER_TR, BORDER_ML, BORDER_MR, BORDER_BL, BORDER_BM, BORDER_BR};
static wchar_t border[][8]{
	{'-', '-', '-', '|', '|', '-', '-', '-'},	// 0 ASCII
	{L'┌', L'─', L'┐', L'│', L'│', L'└', L'─', L'┘'}, // 1 UTF slim Border
	{L'▛', L'▀', L'▜', L'▌', L'▐', L'▙', L'▄', L'▟'} ,
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


void help(char *name){
	printf("%s [OPTION]\n"
		"  Prints encoded paper punch\n"
		"  uses stdin and stdout as input and output\n"
		"  © 2016 Hanno Behrens (pebbles@schattenlauf.de) Licence GPL\n\n"
		"  Available Options:\n"
		"   -h	this help\n"
		"   -l	list of available codings\n"
		"   -d	decode\n"
		"   -cNUM	used coding (default=7)\n"	
		"   -bBIT	bitmode (default -1=standard for coding)\n",name);
}

int main(int argc, char **argv){
	int opt,mode,code=7,bitmode=-1,autoreset=0;
	wint_t ch;
	extern char *optarg;

	while(-1 != (opt=getopt(argc, argv, "dlc:b:h"))){
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
		case 'h':
		case '-':
			help(argv[0]);
			exit(0);
			break;
		}
	}
	fprintf(stderr, "Table %d: %s %s Bitmode %d\n", code, codetable[code].name, decode?"decoding":"encoding", bitmode);
	mode=0;
	while((ch=fgetutf8c(stdin))!=WEOF){
		if(decode){
			baudot_dec(ch, &mode, code, bitmode, autoreset);
		}else{
			baudot_enc(ch, &mode, code, bitmode, autoreset);
		}
	}
}
