# baudot
converts encoding to 20 ancient 5-bit text codes like baudot, murray to and from UTF-8/ASCII

This little utility is something I was missing on the internet in 2007. So I simply 
coded some short lines to solve this problem once and for all. It does not do much, 
but it does it well.

baudot is a text convert like recode or iconv but it's specialized on ancient formats. 
Like 5 bit BAUDOT code or classic TELETYPE code and endodes it to and from ASCII or 
UTF-8. 

No Makefile for it's trivial to compile. It's thought to be an easy tool. I don't care 
if it's ugly or not or if there are compiler warning or not. It does what it intended to.

Guess some cryptologists and HAM operators will find this useful. And folks working 
in computer museums. And of course those scientists working at Aperture Science Labs that
think that 8 bit is a waste of storage and bandwidth for transmission and that 60% enhanced 
efficiency isn't something that science can ignore.

##Compile:
```
$> gcc -O2 baudot.c -o baudot
```
##Install
```
$> sudo ln -s SOURCEPATH/baudot/baudot /usr/local/bin/
```

##Help:
```
$> baudot -h
baudot [OPTION]
  Encodes or decodes UTF-8 to and from Baudot-like 5-tape text encoding
  uses stdin and stdout as input and output
  © 2007 Hanno Behrens (pebbles@schattenlauf.de) Licence GPL

  Available Options:
   -h   this help
   -l   list of available codings
   -d   decode from 5-tape to utf
   -cNUM        used coding (default=7)
   -bBIT        bitmode (default -1=standard for coding)
```
##Encodings:
```
$> baudot -l
 0 CCITT#2 (Bitmode=2 - FromLeft)
 1 CCITT#2/US (Bitmode=2 - FromLeft)
 2 ITA-2 (Bitmode=2 - FromLeft)
 3 LEO (Lyon's Electronic Office) (Bitmode=0 - FromRight)
 4 English Electric Deuce (Bitmode=0 - FromRight)
 5 Illiac (Bitmode=0 - FromRight)
 6 Baudot (CCITT#1) (Bitmode=0 - FromRight)
 7 Murray (CCITT#2) (Bitmode=0 - FromRight)
 8 Alcor (Algol 60, DIN) (Bitmode=0 - FromRight)
 9 Teletype (US CCITT#2) (Bitmode=0 - FromRight)
10 AT&T (US Stock Market) (Bitmode=0 - FromRight)
11 Flexowriter (Bitmode=0 - FromRight)
12 Metro-Vick 950 (Bitmode=0 - FromRight)
13 Elliott 405 (Bitmode=0 - FromRight)
14 EMI 2400 (Bitmode=0 - FromRight)
15 BSI Proposal (Bitmode=0 - FromRight)
16 Stantec Zebra (Bitmode=2 - FromLeft)
17 EMI M/C Tool (Bitmode=2 - FromLeft)
18 EMI 1100 (Bitmode=2 - FromLeft)
19 Pegasus-Mercury (Bitmode=2 - FromLeft)
20 Pegasus-Flexowriter (Bitmode=2 - FromLeft)
```
##Examples

Simple test, will output in shifted letters for 
MURRAY code has no small ones defined. Default is 
Number 7 "Murray" also known as CCITT#2
```
$> echo "hello THIS is a test"|baudot | baudot -d 
Table 7: Murray (CCITT#2) encoding Bitmode -1
Table 7: Murray (CCITT#2) decoding Bitmode -1
HELLO THIS IS A TEST
```

Encode to binary printout, like paperpunch 
```
$> echo "the enemy attacks at dawn"|baudot -c6|xxd -b -c 1 -E|awk '{print $2,$3}'
Table 7: Murray (CCITT#2) encoding Bitmode 6
00000001 .
00000101 .
00010000 .
00000100 .
00010000 .
00000110 .
00010000 .
00000111 .
00010101 .
00000100 .
00011000 .
00000001 .
00000001 .
00011000 .
00001110 .
00011110 .
00010100 .
00000100 .
00011000 .
00000001 .
00000100 .
00010010 .
00011000 .
00011001 .
00000110 .
00001000 .
```
Print on simulated paper punch
The ppt program is part of the "bsdgames" package.
```
5070> echo "the enemy attacks at dawn" | baudot -c9|ppt >&2|ppt -d|baudot -c9 -d    
Table 9: Teletype (US CCITT#2) encoding Bitmode -1
___________
|     .  o|
|     .o o|
|   o .   |
|     .o  |
|   o .   |
|     .oo |
|   o .   |
|     .ooo|
|   o .o o|
|     .o  |
|   oo.   |
|     .  o|
|     .  o|
|   oo.   |
|    o.oo |
|   oo.oo |
|   o .o  |
|     .o  |
|   oo.   |
|     .  o|
|     .o  |
|   o . o |
|   oo.   |
|   oo.  o|
|     .oo |
|    o.   |
___________
Table 9: Teletype (US CCITT#2) decoding Bitmode -1
THE ENEMY ATTACKS AT DAWN
```
Print UTF8 over TELETYPE and back
The trick is to convert the text into Punycode, which is
a pure ASCII code for international domain names. This 
will still lose the small characters but I found it 
amusing to put UTF8 on ancient TELETYPE lines.

This is especially for Aperture Labs! UTF-8? Ha! We do it
with 5 bit! The whole 8-Bit thing was barking up the wrong
tree! Never mind the small characters. Who needs them anyway?
Seymour Cray never did mind the small characters. Why should 
we?
```
$> echo "Über den Hügel"|idn | baudot -c9|ppt >&2|ppt -d|baudot -c9 -d |idn -u           
Table 9: Teletype (US CCITT#2) encoding Bitmode -1
___________
|   o .ooo|
|     .oo |
|   oo. oo|
|   oo.   |
|   oo.   |
|   oo.ooo|
|   o . oo|
|   o .   |
|    o. o |
|     .o  |
|   o . o |
|   o .   |
|     .oo |
|     .o  |
|     .o o|
|    o. oo|
|   o .   |
|    o.  o|
|   oo. oo|
|   oo.   |
|   oo.ooo|
|    o.oo |
|    o.  o|
|   o . oo|
|   oo. o |
|    o.   |
___________
Table 9: Teletype (US CCITT#2) decoding Bitmode -1
üBER DEN HüGEL
```
