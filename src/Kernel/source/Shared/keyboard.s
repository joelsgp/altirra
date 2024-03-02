;	Altirra - Atari 800/800XL/5200 emulator
;	Modular Kernel ROM - Keyboard Handler
;	Copyright (C) 2008-2012 Avery Lee
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

.proc	KeyboardInit
	mva		#0		keydel
	lda		#$ff
	sta		ch
	sta		ch1
	
	;enable keyboard interrupts
	php
	sei
	lda		pokmsk
	ora		#$c0
	sta		pokmsk
	sta		irqen
	plp
	
	;turn on shift lock
	mva		#$40	shflok
	rts
.endp

KeyboardOpen = CIOExitSuccess
KeyboardClose = CIOExitSuccess

.proc	KeyboardGetByte
waitForChar:
	ldx		#$ff
	lda		brkkey
	beq		isBreak
	lda		ch
	cmp		#$ff
	beq		waitForChar
	
	;invalidate char
	stx		ch
	
	;do keyboard click (we do this even for ignored ctrl+shift+keys)
	ldy		#12
	jsr		Bell

	;ignore char if both ctrl and shift are pressed
	cmp		#$c0
	bcs		waitForChar
	
	;trap Ctrl-3 and return EOF
	cmp		#$9a
	beq		isCtrl3
	
	;trap Caps Lock and alter shift/caps lock
	tax
	and		#$3f
	cmp		#$3c
	beq		isCapsLock
		
	;translate char
	lda		KeyCodeToATASCIITable,x
	
	;check for invalid char
	cmp		#$f0
	beq		waitForChar
	
	;check for alpha key
	cmp		#'a'
	bcc		notAlpha
	cmp		#'z'+1
	bcs		notAlpha
	
	;check for shift/control lock
	bit		shflok
	bvs		doShiftLock
	bpl		notAlpha
	
	;do control lock logic
	and		#$1f

notAlpha:
	;return char
	ldy		#1
	rts
	
doShiftLock:
	and		#$df
	bne		notAlpha
	
isBreak:
	stx		brkkey
	ldy		#CIOStatBreak
	rts
	
isCtrl3:
	ldy		#CIOStatEndOfFile
	rts
	
isCapsLock:
	txa
	and		#$c0
	sta		shflok
	jmp		waitForChar
.endp

;==============================================================================
KeyboardPutByte = CIOExitNotSupported
KeyboardGetStatus = CIOExitSuccess
KeyboardSpecial = CIOExitNotSupported

;==============================================================================
.proc	KeyboardIRQ
	;reset screen timer
	mva		#$30	srtimr
	
	;read new key
	lda		kbcode
	
	;check if it is the same as the prev key
	cmp		ch1
	bne		debounced

	;reject key if debounce timer is still running	
	lda		keydel
	bne		xit
	lda		ch1	
debounced:

	;check for Ctrl+1 to toggle display activity
	cmp		#$9f
	beq		isctrl_1

	;store key
	sta		ch

	;reset attract
	mva		#0		atract
	
xit:
	;all done
	pla
	rti	
	
isctrl_1:
	;toggle stop/start flag
	lda		ssflag
	eor		#$ff
	sta		ssflag
	jmp		xit
.endp

;==============================================================================
.proc	KeyboardBreakIRQ
	mva		#0		brkkey
	pla
	rti
.endp
