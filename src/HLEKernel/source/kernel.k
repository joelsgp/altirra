;	Altirra - Atari 800/800XL emulator
;	HLE Kernel ROM
;	Copyright (C) 2008-2009 Avery Lee
;
;	This program is free software; you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation; either version 2 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program; if not, write to the Free Software
;	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

trig0	equ	$d010
trig1	equ	$d011
trig2	equ	$d012
colpm0	equ	$d012
trig3	equ	$d013
colpm1	equ	$d013
pal		equ	$d014
colpm2	equ	$d014
colpm3	equ	$d015
colpf0	equ	$d016
colpf1	equ	$d017
colpf2	equ	$d018
colpf3	equ	$d019
colbk	equ	$d01a
prior	equ	$d01b
consol	equ	$d01f
pot0	equ	$d200
audf1	equ	$d200
pot1	equ	$d201
audc1	equ	$d201
pot2	equ	$d202
audf2	equ	$d202
pot3	equ	$d203
audc2	equ	$d203
pot4	equ	$d204
audf3	equ	$d204
pot5	equ	$d205
audc3	equ	$d205
pot6	equ	$d206
audf4	equ	$d206
pot7	equ	$d207
audc4	equ	$d207
audctl	equ	$d208
kbcode	equ	$d209
skres	equ	$d20a
potgo	equ	$d20b
serin	equ	$d20d
serout	equ	$d20d
irqen	equ	$d20e
irqst	equ	$d20e
skctl	equ	$d20f
skstat	equ	$d20f
porta	equ	$d300
portb	equ	$d301
pactl	equ	$d302
pbctl	equ	$d303
dmactl	equ	$d400
chactl	equ	$d401
dlistl	equ	$d402
dlisth	equ	$d403
hscrol	equ	$d404
vscrol	equ	$d405
chbase	equ	$d409
wsync	equ	$d40a
vcount	equ	$d40b
nmien	equ	$d40e
nmist	equ	$d40f
nmires	equ	$d40f

	opt		o-

casini	= $0002			;cassette initialization vector
ramlo	= $0004			;
tramsz	= $0006			;temporary ram size / cart A flag
tstdat	= $0007			;RAM test data register / cart B flag
warmst	= $0008			;warmstart flag
boot?	= $0009			;boot flag; 0 if none, 1 for disk, 2 for cassette
dosvec	= $000a
dosini	= $000c
pokmsk	= $0010
brkkey	= $0011			;set on [BREAK]
rtclok	= $0012
a0		= $0015
bufadr	= $0015			;indirect buffer address pointer (temp for disk buffer)
		; $0016
iccomt	= $0017			;CIO: command byte
ziocb	= $0020			;zero-page IOCB
ichidz	= $0020			;Zero page IOCB: device index ($FF = not open)
icdnoz	= $0021			;Zero page IOCB: device number
iccomz	= $0022			;Zero page IOCB: command byte
icstaz	= $0023			;Zero page IOCB: status byte
icbalz	= $0024			;Zero page IOCB: address of device/filename spec lo
icbahz	= $0025			;Zero page IOCB: address of device/filename spec hi
icptlz	= $0026			;Zero page IOCB: put byte address lo (-1)
icpthz	= $0027			;Zero page IOCB: put byte address hi (-1)
icbllz	= $0028			;Zero page IOCB: buffer length/byte count lo (-1)
icblhz	= $0029			;Zero page IOCB: buffer length/byte count hi (-1)
icax1z	= $002a			;Zero page IOCB: device-specific information 1
icax2z	= $002b			;Zero page IOCB: device-specific information 2
icax3z	= $002c			;Zero page IOCB: device-specific information 3
icax4z	= $002d			;Zero page IOCB: device-specific information 4
icax5z	= $002e			;Zero page IOCB: device-specific information 5
icidno	= $002e			;CIO: call X register save/restore
icax6z	= $002f			;Zero page IOCB: device-specific information 6
ciochr	= $002f			;CIO: call A register save/restore
status  = $0030			;SIO: temporay status byte
chksum	= $0031			;SIO: temporary checksum byte (used by IRQ routines)
bufrlo	= $0032			;SIO: buffer pointer lo (incremented by IRQ routines)
bufrhi	= $0033			;SIO: buffer pointer hi (incremented by IRQ routines)
bfenlo	= $0034			;SIO: buffer end lo
bfenhi	= $0035			;SIO: buffer end hi
bufrfl	= $0038			;SIO: buffer full flag
recvdn	= $0039			;SIO: receive completed flag
xmtdon	= $003a			;SIO: transmit completed flag
chksnt	= $003b			;SIO: checksum sent flag
nocksm	= $003c			;SIO: no-checksum flag
bptr	= $003d			;Cassette: Buffer pointer
ftype	= $003e			;Cassette: Interrecord gap (IRG) type; bit7=1 means continuous mode
feof	= $003f			;Cassette: EOF flag
critic	= $0042			;critical flag for vbi
ckey	= $004a			;cassette boot key
atract	= $004d			;screen attract counter
drkmsk	= $004e			;screen attract mask
colrsh	= $004f			;screen attract color shift
lmargn	= $0052			;left margin column
rmargn	= $0053			;right margin column
rowcrs	= $0054			;cursor row
colcrs	= $0055			;cursor column
		; $0056
dindex	= $0057			;display mode index (0-15)
savmsc	= $0058			;lowest address of screen region
oldchr	= $005d			;cursor character save/restore
oldadr	= $005e			;cursor memory address
palnts	= $0062			;[XL/XE] PAL/NTSC flag; 0=NTSC
logcol	= $0063			;logical line column
adress	= $0064			;temporary storage (display code)
		; $0065
a1		= adress
ramtop	= $006a			;ram size
bufcnt	= $006b
bufstr	= $006c			;row/column of start of logical line
keydat	= $0079			;keyboard translation table (XL/XE only; required by SDX ED)
		; $007a
holdch	= $007c			;temporary key hold area prior to shift/control lock logic

fr0		= $00d4			;floating point register 0
fr1		= $00e0			;floating point register 1

vdslst	= $0200			;display list interrupt vector
vprced	= $0202			;serial bus proceed interrupt vector
vinter	= $0204			;serial bus interrupt vector
vbreak	= $0206			;BRK instruction vector
vkeybd	= $0208			;keyboard interrupt vector
vserin	= $020a			;serial input ready interrupt vector
vseror	= $020c			;serial output ready interrupt vector
vseroc	= $020e			;serial output completed interrupt vector
vtimr1	= $0210			;pokey timer 1 interrupt vector
vtimr2	= $0212			;pokey timer 2 interrupt vector
vtimr4	= $0214			;pokey timer 4 interrupt vector
vimirq	= $0216			;immediate IRQ vector
cdtmv1	= $0218			;countdown timer 1
cdtmv2	= $021a			;countdown timer 2
cdtmv3	= $021c			;countdown timer 3
cdtmv4	= $021e			;countdown timer 4
cdtmv5	= $0220			;countdown timer 5
vvblki	= $0222			;vertical blank immediate vector
vvblkd	= $0224			;vertical blank deferred vector
cdtma1	= $0226			;timer 1 vector
cdtma2	= $0228			;timer 2 vector
cdtmf3	= $022a			;timer 3 flag
srtimr	= $022b			;autorepeat timer
cdtmf4	= $022c			;timer 4 flag
intemp	= $022d			;temp value used by SETVBV
cdtmf5	= $022e			;timer 5 flag
sdmctl	= $022f			;shadow for DMACTL ($D400)
sdlstl	= $0230			;shadow for DLISTL ($D402)
sdlsth	= $0231			;shadow for DLISTH ($D403)
sskctl	= $0232			;shadow for SKCTL ($D20F)
brkky	= $0236			;break key interrupt vector (OS B+ only)
cdevic	= $023a			;SIO: command frame device ID
ccomnd	= $023b			;SIO: command frame command ID
caux1	= $023c			;SIO: command aux byte 1
caux2	= $023d			;SIO: command aux byte 2
temp	= $023e			;SIO: temp space
errflg	= $023f			;SIO: error flag
dflags	= $0240			;disk boot flags
dbsect	= $0241			;disk boot sector count
bootad	= $0242			;disk boot address
coldst	= $0244			;cold start flag
gprior	= $026f			;shadow for PRIOR ($D01B)
paddl0	= $0270			;shadow for POT0 ($D200)
paddl1	= $0271			;shadow for POT1 ($D201)
paddl2	= $0272			;shadow for POT2 ($D202)
paddl3	= $0273			;shadow for POT3 ($D203)
paddl4	= $0274			;shadow for POT4 ($D204)
paddl5	= $0275			;shadow for POT5 ($D205)
paddl6	= $0276			;shadow for POT6 ($D206)
paddl7	= $0277			;shadow for POT7 ($D207)
stick0	= $0278			;shadow for PORTA lo ($D300)
stick1	= $0279			;shadow for PORTA hi ($D300)
stick2	= $027a			;shadow for PORTB lo ($D302)
stick3	= $027b			;shadow for PORTB hih ($D302)
ptrig0	= $027c			;paddle 0 trigger
ptrig1	= $027d			;paddle 1 trigger
ptrig2	= $027e			;paddle 2 trigger
ptrig3	= $027f			;paddle 3 trigger
ptrig4	= $0280			;paddle 4 trigger
ptrig5	= $0281			;paddle 5 trigger
ptrig6	= $0282			;paddle 6 trigger
ptrig7	= $0283			;paddle 7 trigger
strig0	= $0284			;shadow for TRIG0 ($D001)
strig1	= $0285			;shadow for TRIG1 ($D002)
strig2	= $0286			;shadow for TRIG2 ($D003)
strig3	= $0287			;shadow for TRIG3 ($D004)
wmode	= $0289			;Cassette: Write mode (00 = read, 80 = write)
blim	= $028a			;Cassette: Buffer limit
txtmsc	= $0294			;text window pointer
		; $0295
escflg	= $02a2			;display next character verbatim if non-EOL
logmap	= $02b2			;line logical map
shflok	= $02be			;shift/control lock flags
botscr	= $02bf			;number of text rows in text window
pcolr0	= $02c0			;shadow for COLPM0 ($D012)
pcolr1	= $02c1			;shadow for COLPM1 ($D013)
pcolr2	= $02c2			;shadow for COLPM2 ($D014)
pcolr3	= $02c3			;shadow for COLPM3 ($D015)
color0	= $02c4			;shadow for COLPF0 ($D016)
color1	= $02c5			;shadow for COLPF1 ($D017)
color2	= $02c6			;shadow for COLPF2 ($D018)
color3	= $02c7			;shadow for COLPF3 ($D019)
color4	= $02c8			;shadow for COLBK ($D01A)
dsctln	= $02d5			;disk sector length (1200XL and newer only)
		; $02d6
noclik	= $02db			;key click disable (XL/XE only)
ramsiz	= $02e4			;ram size in pages
memtop	= $02e5			;highest location for programs and data
memlo	= $02e7			;base of application memory
dvstat	= $02ea			;DISK: Status request buffer area (4 bytes)
		; $02eb
		; $02ec
		; $02ed
keydel	= $02f1			;keyboard debounce delay (set to 3 vblanks per key)
ch1		= $02f2			;last keyboard code read
chact	= $02f3			;shadow for CHACTL ($D401)
chbas	= $02f4			;shadow for CHBASE ($D409)
atachr	= $02fb			;
ch		= $02fc			;keyboard FIFO byte
fildat	= $02fd			;color for screen XIO fill operation
dspflg	= $02fe			;enable/disable of control codes by screen editor (E:)
ssflag	= $02ff			;display/screen editor suspend flag
ddevic	= $0300			;serial bus ID
dunit	= $0301			;device number
dcomnd	= $0302			;command byte
dstats	= $0303			;status byte
dbuflo	= $0304			;buffer address lo
dbufhi	= $0305			;buffer address hi
dtimlo	= $0306			;disk timeout value
dbytlo	= $0308			;byte count lo
dbythi	= $0309			;byte count hi
daux1	= $030a			;sector number lo
daux2	= $030b			;sector number hi
timer1	= $030c			;baud rate determination - first timer value
timer2	= $0310			;baud rate determination - second timer value
timflg	= $0317			;SIO: operation timeout flag (set by countdown timer 1 IRQ)
stackp	= $0318			;SIO: stack pointer save
hatabs	= $031a			;handler table
ichid	= $0340			;IOCB #0 handler ID
icdno	= $0341			;IOCB #0 device number
iccmd	= $0342			;IOCB #0 command byte
icsta	= $0343			;IOCB #0 status
icbal	= $0344			;IOCB #0 buffer address lo
icbah	= $0345			;IOCB #0 buffer address hi
icptl	= $0346			;IOCB #0 PUT address lo
icpth	= $0347			;IOCB #0 PUT address hi
icbll	= $0348			;IOCB #0 buffer length/byte count lo
icblh	= $0349			;IOCB #0 buffer length/byte count hi
icax1	= $034a			;IOCB #0 auxiliary information lo
icax2	= $034b			;IOCB #0 auxiliary information hi
icax3	= $034c			;
icax4	= $034d			;
icax5	= $034e			;
icax6	= $034f			;
basicf	= $03f8			;XL/XE: BASIC flag (0 = enabled)
gintlk	= $03fa			;XL/XE: Cartridge interlock (clone of TRIG3). 
casbuf	= $03fd			;Cassette buffer (03FD-047F)

	opt		h-o+f+

;	icl		'loader.xasm'

;============================================================================
;	C000-CFFF	LOWER OS ROM
;
;
		#include <xlext.k>
		
:$cc00-*	dta		$12
		ins		'..\..\src\kernel\source\Shared\atariifont.bin',$0,$400

;============================================================================
;	5000-57FF	SELF-TEST ROM
;
;
		org		$5000, $d000
		jsr		BugCheck
:$5800-*	dta		$12
	
;============================================================================
;	D800-DFFF	MATH PACK
;
		org		$d800
		#include <mathpack.k>
:$e000-*	dta		$12

;============================================================================
;	E000-FFFF	UPPER OS ROM
;
	org		$e000
	ins		'..\..\src\kernel\source\Shared\atarifont.bin',$0,$400

		org		$e400
editrv	dta		a(EditorOpen-1)
		dta		a(EditorClose-1)
		dta		a(EditorGetByte-1)
		dta		a(EditorPutByte-1)
		dta		a(EditorGetStatus-1)
		dta		a(EditorSpecial-1)
		jmp		EditorInit
		dta		$00

screnv	dta		a(ScreenOpen-1)
		dta		a(ScreenClose-1)
		dta		a(ScreenGetByte-1)
		dta		a(ScreenPutByte-1)
		dta		a(ScreenGetStatus-1)
		dta		a(ScreenSpecial-1)
		jsr		BugCheck
		dta		$00

keybdv	dta		a(KeyboardOpen-1)
		dta		a(KeyboardClose-1)
		dta		a(KeyboardGetByte-1)
		dta		a(KeyboardPutByte-1)
		dta		a(KeyboardGetStatus-1)
		dta		a(KeyboardSpecial-1)
		jmp		KeyboardInit
		dta		$00
	
printv	dta		a(BugCheck-1)
		dta		a(BugCheck-1)
		dta		a(BugCheck-1)
		dta		a(BugCheck-1)
		dta		a(BugCheck-1)
		dta		a(BugCheck-1)
		jsr		BugCheck
		dta		$00

casetv	dta		a(CassetteOpen-1)
		dta		a(CassetteClose-1)
		dta		a(CassetteGetByte-1)
		dta		a(CassettePutByte-1)
		dta		a(CassetteGetStatus-1)
		dta		a(CassetteSpecial-1)
		jmp		CassetteInit
		dta		$00
	
		;vector table
		org	$e450
diskiv	jmp		DiskInit			;$E450
dskinv	jmp		DiskHandler			;$E453
ciov	jmp		CIO					;$E456
siov	jmp		SIO					;$E459
setvbv	jmp		SetVBlankVector		;$E45C
sysvbv	jmp		VBlankImmediate		;$E45F
xitvbv	jmp		VBlankDeferred		;$E462
sioinv	jmp		SIOInit				;$E465
sendev	jsr		BugCheck			;$E468
intinv	jmp		InitInterrupts		;$E46B
cioinv	jmp		CIOInit				;$E46E
blkbdv	jmp		Blackboard			;$E471
warmsv	jmp		WarmStart			;$E474
coldsv	jmp		ColdStart			;$E477
rblokv	jmp		CassetteReadBlock	;$E47A
csopiv	jmp		CassetteOpenRead	;$E47D
pupdiv	jsr		BugCheck			;$E480	1200XL: Power-on display; XL/XE: self-test
slftsv	jmp		$5000				;$E483	XL: Self-test ($5000)
pentv	jmp		PBIAddHandler		;$E486	XL: add handler to HATABS
phunlv	jsr		BugCheck			;$E489	XL: 
phiniv	jsr		BugCheck			;$E48C	XL:
gpdvv	jsr		BugCheck			;$E48F	XL: Generic device vector

		;handler table
HandlerTable:
		dta		c'P',a(printv)
		dta		c'C',a(casetv)
		dta		c'E',a(editrv)
		dta		c'S',a(screnv)
		dta		c'K',a(keybdv)
		
;==============================================================================
; $E4C0  Known RTS instruction
;
; The Atari 850 handler uses this as a "known RTS" instruction, as does
; Altirra's internal R: handler emulation.
;
		org		$e4c0
		
.nowarn .proc KnownRTS
		rts
.endp

;==============================================================================
;HDM requires SIO and interrupt handlers to be within either $C000-CFFF
;or $DFF0-EFFF.
#include <sio.k>
#include <nmi.k>
#include <irq.k>

;==============================================================================
DefaultHandler_A_RTI:
	pla
DefaultHandler_RTI:
	rti
;==============================================================================

.print 'Disk support end: ',*
.if * >= $f000
.error 'Disk support extends beyond $F000. 
.endif

;==============================================================================

#include <bugcheck.k>
#include <cio.k>
#include <disk.k>
#include <editor.k>
#include <keyboard.k>
#include <screen.k>
#include <cassette.k>

;==============================================================================
; Hacks
;==============================================================================
	org		$f3f6
	jmp		ScreenOpen

#include <boot.k>
#include <init.k>
#include <blackboard.k>
#include <pbi.k>
	
;==============================================================================
.proc ResetRoutine
	jmp		ColdStart
	.endp

;==============================================================================
; Dispatched from INTINV. Used by SpartaDOS X.
.proc InitInterrupts
	mva		#$40 nmien
	mva		trig3 gintlk		;required by all XEGS carts
	rts
	.endp

;==============================================================================
; CPU vectors
;==============================================================================
	org		$fffa
nmivec	.wo		NmiRoutine
resvec	.wo		ResetRoutine
irqvec	.wo		IrqRoutine
