;	Altirra - Atari 800/800XL emulator
;	Kernel ROM replacement
;	Copyright (C) 2008 Avery Lee
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

.proc ColdStart
	; 1. initialize CPU
	sei
	cld
	ldx		#$ff
	txs
	
	; 2. clear warmstart flag
	mva		#0 warmst
	
	; 3. test for diagnostic cartridge
	; 4. measure memory -> tramsz
	; 5. clear all hardware registers
	jsr		InitMemory
	
	; 6. clear memory from $0008 up to [tramsz,0]
	ldy		#8
	mva		#0 a1
	sta		a1+1
clearloop:
	lda		#0
clearloop2:
	sta		(a1),y
	iny
	bne		clearloop2
	inc		a1+1
	lda		a1+1
	cmp		tramsz
	bne		clearloop
	
	; 7. set dosvec to blackboard routine
	mwa		#Blackboard dosvec
	
	; 8. set coldstart flag
	mva		#$ff coldst

	; 9. set screen margins
	; 10. initialize RAM vectors
	; 11. set misc database values
	; 12. enable IRQ interrupts
	; 13. initialize device table
	; 14. initialize cartridges
	; 15. use IOCB #0 to open screen editor (E)
	; 16. wait for VBLANK so screen is initialized
	; 17. do cassette boot, if it was requested
	; 18. do disk boot
	; 19. reset coldstart flag
	; 20. run cartridges or blackboard
	jmp		InitEnvironment
	
.endp

;==============================================================================
.proc WarmStart
	; A. initialize CPU
	sei
	cld
	ldx		#$ff
	txs
	
	; B. set warmstart flag
	stx		warmst
	
	; C. check for diag, measure memory, clear hw registers
	jsr		InitMemory
	
	; D. zero 0010-007F and 0200-03FF.
	ldx		#$5f
	lda		#0
zpclear:
	sta		$0010,x
	dex
	bpl		zpclear
	
	ldx		#0
dbclear:
	sta		$0200,x
	sta		$0300,x
	inx
	bne		dbclear
	
	; E. steps 9-16 above
	; F. if cassette boot was successful on cold boot, execute cassette init
	; G. if disk boot was successful on cold boot, execute disk init
	; H. same as steps 19 and 20
	jmp		InitEnvironment
.endp

;==============================================================================
.proc InitMemory
	; 3. test for diagnostic cartridge
	lda		$bffc
	bne		notdiag
	ldx		#0
	mva		#$ff $bffc
	sta		$bffc
	cmp		notdiag
	bne		notdiag
	
	; is it enabled?
	bit		$bffd
	bpl		notdiag
	
	; start diagnostic cartridge
	jmp		($bffe)
	
notdiag:

	;(Altirra specific) Check OPTION and enable BASIC rom.
	bit		warmst			;is warmstart set?
	bpl		checkoption		;yes, check the OPTION key
	
	lda		basicf
	jmp		setoption
	
checkoption:
	lda		#$3c
	sta		pbctl
	sta		pactl
	lda		#$00
	sta		porta

	lda		consol
setoption:
	and		#$04
	sta		basicf
	lsr
	eor		#$02
	ora		#$b1
	sta		portb
	
	lda		#$38
	sta		pbctl
	lda		#$ff
	sta		portb		;set port B lines to output
	lda		#$3c
	sta		pbctl		;swap back to data register (required for disk version of Numen)
	
	; 4. measure memory -> tramsz
	ldy		#$00
	sty		a0
	ldx		#$02
pageloop:
	stx		a0+1
	lda		(a0),y
	eor		#$ff
	sta		(a0),y
	cmp		(a0),y
	bne		notRAM
	eor		#$ff
	sta		(a0),y
	inx
	cpx		#$c0
	bne		pageloop
notRAM:
	stx		tramsz

	; 5. clear all hardware registers
	ldx		#$1f
	lda		#$00
hwclear:
	sta		$d000,x
	sta		$d200,x
	sta		$d400,x
	dex
	bpl		hwclear
		
	rts
.endp

;==============================================================================
.proc InitEnvironment
	;required for Translator to work
	mva		tramsz ramsiz
	
	; 9. set screen margins
	mva		#2 lmargn
	mva		#39 rmargn
	
	; 10. initialize RAM vectors
	mwa		#DefaultHandler_RTI		vdslst
	mwa		#DefaultHandler_A_RTI	vprced
	mwa		#DefaultHandler_A_RTI	vinter
	mwa		#DefaultHandler_A_RTI	vbreak
	mwa		#KeyboardIRQ			vkeybd
	mwa		#SerialInputReady		vserin
	mwa		#SerialOutputReady		vseror
	mwa		#SerialOutputComplete	vseroc
	mwa		#DefaultHandler_A_RTI	vtimr1
	mwa		#DefaultHandler_A_RTI	vtimr2
	mwa		#DefaultHandler_A_RTI	vtimr4
	mwa		#IrqHandler				vimirq
	mwa		#VBlankImmediate		vvblki
	mwa		#VBlankDeferred			vvblkd
	mwa		#KeyboardBreakIRQ		brkky
	mwa		#0						cdtma1
	
	; 11. set misc database values
	mva		#$ff brkkey
	mva		#0 memtop
	mva		tramsz memtop+1
	mwa		#$0700 memlo
	
	jsr		DiskInit
	jsr		ScreenInit
	;jsr	DisplayInit
	jsr		KeyboardInit
	;jsr	PrinterInit
	;jsr	CassetteInit
	jsr		cioinv
	jsr		SIOInit
	jsr		InitInterrupts
	
	; check for START key, and if so, set cassette boot flag
	lda		consol
	ror
	bcc		nocasboot
	mva		#1 ckey
nocasboot:

	; 12. enable IRQ interrupts
	cli
	
	; 13. initialize device table
	ldx		#14
htabinit:
	lda		HandlerTable,x
	sta		hatabs,x
	dex
	bpl		htabinit

	; 14. initialize cartridges
	mva		#0 tstdat
	lda		$9ffc
	bne		skipCartBInit
	mvx		#$ff $9ffc
	cpx		$9ffc
	sta		$9ffc
	beq		skipCartBInit
	jsr		InitCartB
	mva		#1 tstdat
skipCartBInit:
	
	mva		#0 tramsz
	lda		$bffc
	bne		skipCartAInit
	mvx		#$ff $bffc
	cpx		$bffc
	sta		$bffc
	beq		skipCartAInit
	jsr		InitCartA
	mva		#1 tramsz
skipCartAInit:

	; 15. use IOCB #0 ($0340) to open screen editor (E)
	mva		#$03 iccmd		;OPEN
	mva		#$c0 icax1		;read/write, no forced read
	mva		#0 icax2		;mode 0
	mwa		#ScreenEditorName icbal
	ldx		#0
	jsr		ciov
	
	; 16. wait for VBLANK so screen is initialized
	
	lda		rtclok+2
waitvbl:
	cmp		rtclok+2
	beq		waitvbl

;-----------------------------------------------------------

	; 17. do cassette boot, if it was requested
	; F. if cassette boot was successful on cold boot, execute cassette init
	
	; The cold boot path must check the warm start flag and switch paths if
	; necessary. SpartaDOS X relies on being able to set the warm start
	; flag from its cart init handler.
	
	lda		warmst
	bne		reinitcas
	
	lda		ckey
	bne		postcasboot
	jsr		BootCassette
	jmp		postcasboot

reinitcas:	
	lda		boot?
	cmp		#2
	bne		postcasboot
	jsr		initCassette
postcasboot:

	; 18. do disk boot
	; G. if disk boot was successful on cold boot, execute disk init
	lda		warmst
	bne		reinitDisk
	
	;check for cart B requesting boot
	lda		tstdat
	beq		noCartBBoot
	lda		#$01
	bit		$9ffd
	bne		doBootDisk
	lda		tramsz
	beq		postDiskBoot
noCartBBoot:

	;check for cart A requesting boot
	lda		tramsz
	beq		noCartABoot
	lda		#$01
	bit		$bffd
	beq		postDiskBoot
noCartABoot:
doBootDisk:
	jsr		BootDisk
	jmp		postDiskBoot
	
reinitDisk:
	lda		boot?
	cmp		#1
	bne		postDiskBoot
	jsr		initDisk
postDiskBoot:

	; H. same as steps 19 and 20
	; 19. reset coldstart flag
	
	mva		#0 coldst
	
	; 20. run cartridges or blackboard
	
	; try to boot cart A
	lda		tramsz
	beq		NoBootCartA
	lda		#$04
	bit		$bffd
	beq		NoBootCartA
	jmp		($bffa)
NoBootCartA:

	; try to boot cart B
	lda		tstdat
	beq		NoBootCartB
	bit		$9ffd
	beq		NoBootCartB
	jmp		($9ffa)
NoBootCartB:

	; run blackboard
	jmp		(dosvec)

initCassette:
	jmp		(casini)

initDisk:
	jmp		(dosini)
	
InitCartA:
	jmp		($bffe)

InitCartB:
	jmp		($9ffe)
	
ScreenEditorName:
	dta		c"E:",$9B

.endp

;==============================================================================

	nop
