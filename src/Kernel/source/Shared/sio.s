;	Altirra - Atari 800/800XL/5200 emulator
;	Modular Kernel ROM - Serial Input/Output Routines
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

.proc SIOInit
	;turn off POKEY init mode so polynomial counters and audio run
	mva		#3 skctl
	sta		sskctl
	rts
.endp

;==============================================================================
.proc SIO
	tsx
	stx		stackp
	
	;set retry counters
	mva		#$0d cretry
	mva		#$01 dretry
	
	;enter critical section
	sta		critic
	
.if _KERNEL_PBI_SUPPORT
	;attempt PBI transfer
	jsr		PBIAttemptSIO
	scc:jmp	xit
.endif
	
	jsr		SIOInitBaseTransfers

	;check for cassette
	lda		ddevic
	cmp		#$5f
	sne:jmp	SIOCassette	
	
retry_command:
	;init command buffer
	lda		ddevic
	clc
	adc		dunit
	sec
	sbc		#1
	sta		cdevic

	mva		dcomnd		ccomnd
	mva		daux1		caux1
	mva		daux2		caux2
		
	;assert command line
	mva		#$34		pbctl

	;clear stray bytes
	lda		serin

	;send command frame
	mva		#0			nocksm
	mwa		#cdevic-1	bufrlo
	mwa		#caux2+1	bfenlo
	jsr		SIOSend
	bmi		xit
	
	;negate command line
	mva		#$3c		pbctl
	
	;setup 3 frame delay for ack
	mva		#$ff		timflg
	lda		#1
	ldx		#>3
	ldy		#<3
	jsr		setvbv

	;setup for receiving ACK
	mwa		#temp		bufrlo
	mwa		#temp+1		bfenlo
	mva		#$ff		nocksm
	jsr		SIOReceive
	bmi		xit
	lda		temp
	cmp		#'A'
	bpl		ackOK
	
	ldy		#SIOErrorNAK
	
command_error:
	dec		cretry
	bne		retry_command
	jmp		xit
ackOK:
	
	;setup 90 frame delay for complete
	mva		#$ff		timflg
	lda		#1
	ldx		#>90
	ldy		#<90
	jsr		setvbv

	;setup for receiving complete
	mva		#$ff		nocksm
	mwa		#temp		bufrlo
	jsr		SIOReceive
	bmi		xit
	lda		temp
	cmp		#'C'
	beq		completeOK
	
	dec		dretry
	beq		device_retries_exhausted
	jmp		retry_command

device_retries_exhausted:	
	ldy		#SIOErrorNAK
xit:
	lda		#0
	sta		audc1
	sta		audc2
	sta		audc3
	sta		audc4
	sta		critic

	ldx		stackp
	txs
	tya
	sty		dstats
	rts

completeOK:

	;setup buffer pointers
	jsr		SIOSetupBufferPointers
		
	;check if we should send a data frame
	bit		dstats
	bmi		sendFrame
	bvc		dataSendOK
	
	jsr		SIOReceive
	bmi		xit
	ldy		#SIOSuccess
	jmp		xit

sendFrame:
	;send data frame
	dew		bufrlo				;must be -1 for (vseror)
	
	jsr		SIOSend
	bmi		xit
		
dataSendOK:	
	ldy		#SIOSuccess
	jmp		xit
.endp

;==============================================================================
;SIO base init routine
;
.proc SIOInitBaseTransfers
	;set timeout timer address -- MUST be done on each call to SIO, or
	;Cross-Town Crazy Eight hangs on load due to taking over this vector
	;previously
	mwa		#SIOCountdown1Handler	cdtma1

	;clock channel 3 and 4 together at 1.79MHz
	mva		#$28	audctl

	;configure pokey timers 3 and 4 for 19200 baud (1789773/(2*40+7) = 19040)
	mva		#40		audf3
	mva		#0		audf4
	mva		#$a0	audc3
	mva		#$a8	audc4
	
	;reset serial status
	sta		skres
	rts
.endp

;==============================================================================
.proc SIOSetupBufferPointers
	clc
	lda		dbuflo
	sta		bufrlo
	adc		dbytlo
	sta		bfenlo
	lda		dbufhi
	sta		bufrhi
	adc		dbythi
	sta		bfenhi
	rts
.endp

;==============================================================================
;SIO send routine
;
.proc SIOSend
	;configure serial port for synchronous transmission
	;enable transmission IRQs
	sei
	lda		sskctl
	and		#$8f
	ora		#$20
	sta		sskctl
	sta		skctl
	lda		pokmsk
	ora		#$10
	and		#$f7
	sta		pokmsk
	sta		irqen
	cli
	
	lda		#0
	sta		xmtdon
	sta		status
	sta		chksum
	sta		chksnt
	
	;send first byte
	lda		#>wait
	pha
	lda		#<wait
	pha
	php
	sei
	pha
	jmp		(vseror)
	
	;wait for transmit to complete
wait:
	ldy		status
	bmi		error
	lda		xmtdon
	beq		wait
	ldy		status
	
	;shut off transmission IRQs
error:
	sei
	lda		pokmsk
	and		#$e7
	sta		pokmsk
	sta		irqen
	cli

	;we're done
	tya
	rts
.endp

;==============================================================================
;SIO receive routine
;
.proc SIOReceive
	lda		#0
use_checksum:
	sta		chksum
	lda		#0
	sta		recvdn
	sta		bufrfl
	lda		#1
	sta		status

	;configure serial port for asynchronous receive
	;enable receive IRQ
	sei
	lda		sskctl
	and		#$8f
	ora		#$10
	sta		sskctl
	sta		skctl
	lda		pokmsk
	ora		#$20
	sta		pokmsk
	sta		irqen
	cli
	
	;wait for receive to complete
wait:
	lda		timflg
	beq		timeout
	ldy		status
	bmi		error
	lda		recvdn
	beq		wait
	ldy		status
	
	;shut off receive IRQs
error:
	sei
	lda		pokmsk
	and		#$d7
	sta		pokmsk
	sta		irqen
	cli
	
	;we're done
	tya
	rts
	
timeout:
	ldy		#SIOErrorTimeout
	jmp		error
.endp

;==============================================================================
; SIO serial input routine
;
; DOS 2.0S replaces (VSERIN), so it's critical that this routine follow the
; rules compatible with DOS. The rules are as follows:
;
;	BUFRLO/BUFRHI:	Points to next byte to read. Note that this is different
;					from (VSEROR)!
;	BFENLO/BFENHI:	Points one after last byte in buffer.
;	BUFRFL:			Set when all data bytes have been read.
;	NOCKSM:			Set if no checksum byte is expected. Cleared after checked.
;	RECVDN:			Set when receive is complete, including any checksum.	
;
.proc SIOInputReadyHandler
	lda		bufrfl
	bne		receiveChecksum

	;receive data byte
	tya
	pha
	lda		serin
	ldy		#$00
	sta		(bufrlo),y
	clc
	adc		chksum
	adc		#$00
	sta		chksum
	
	;bump buffer pointer
	inw		bufrlo
	
	;check for EOB
	lda		bufrlo
	cmp		bfenlo
	beq		possiblyEnd
xit:
	pla
	tay
	pla
	rti
	
receiveChecksum:
	;read and compare checksum
	lda		serin
	cmp		chksum
	beq		checksumOK
	
	mva		#SIOErrorChecksum	status
checksumOK:
	
	;set receive done flag
	mva		#$ff	recvdn

	;exit
	pla
	rti
	
possiblyEnd:	
	lda		bufrhi
	cmp		bfenhi
	bne		xit

	mva		#$ff	bufrfl
	
	;should there be a checksum?
	lda		nocksm
	bne		skipChecksum
	jmp		xit

skipChecksum:
	;set receive done flag
	sta		recvdn
	
	;clear no checksum flag
	lda		#0
	sta		nocksm
	jmp		xit
.endp

;==============================================================================
; SIO serial output ready routine
;
; DOS 2.0S replaces (VSEROR), so it's critical that this routine follow the
; rules compatible with DOS. The rules are as follows:
;
;	BUFRLO/BUFRHI:	On entry, points to one LESS than the next byte to write.
;	BFENLO/BFENHI:	Points to byte immediately after buffer.
;	CHKSUM:			Holds running checksum as bytes are output.
;	CHKSNT:			$00 if checksum not yet sent, $FF if checksum sent.
;	POKMSK:			Used to enable the serial output complete IRQ after sending
;					checksum.
;
.proc SIOOutputReadyHandler
	;increment buffer pointer
	inc		bufrlo
	bne		addrcc
	inc		bufrhi
addrcc:

	;compare against buffer end
	lda		bufrlo
	cmp		bfenlo
	lda		bufrhi
	sbc		bfenhi			;set flags according to (dst - end)
	bcs		doChecksum

	;save Y
	tya
	pha

	;send out next byte
	ldy		#0
	lda		(bufrlo),y
	sta		serout
	
	;update checksum
	adc		chksum
	adc		#0
	sta		chksum

	;restore registers and exit
	pla
	tay
	pla
	rti
	
doChecksum:
	;send checksum
	lda		chksum
	sta		serout
	
	;set checksum sent flag
	mva		#$ff	chksnt
	
	;enable output complete IRQ and disable serial output IRQ
	lda		pokmsk
	ora		#$08
	and		#$ef
	sta		pokmsk
	sta		irqen
	
	pla
	rti
.endp

;==============================================================================
.proc SIOOutputCompleteHandler
	;check that we've sent the checksum
	lda		chksnt
	beq		xit
	
	;we're done sending the checksum
	sta		xmtdon
	
	;need to shut off this interrupt as it is not latched
	lda		pokmsk
	and		#$f7
	sta		pokmsk
	sta		irqen

xit:
	pla
	rti
.endp

;==============================================================================
.proc SIOCountdown1Handler
	;signal operation timeout
	mva		#0	timflg
	rts
.endp

;==============================================================================
.proc SIOCassette
	;check if it's read sector
	lda		dcomnd
	cmp		#$52
	beq		isread

	;nope, bail	
	ldy		#SIOErrorNAK
	jmp		xit
	
isread:
	jsr		SIOCassetteReadFrame
xit:
	jmp		SIO.xit
	
.endp

;==============================================================================
.proc SIOCassetteReadFrame
	;set to 600 baud, turn on async read to shut off annoying tone
	mva		#$cc audf3
	mva		#$05 audf4
	lda		#0
	sta		audc3
	sta		audc4
	
	lda		sskctl
	and		#$8f
	ora		#$10
	sta		sskctl

	;set timeout (approx; no NTSC/PAL switching yet)
	mva		#$ff timflg
	lda		#1
	ldx		#>3600
	ldy		#<3600
	jsr		VBISetVector

	;wait for beginning of frame
	lda		#$10		;test bit 4 of SKSTAT
waitzerostart:
	bit		timflg
	bpl		timeout
	bit		skstat
	bne		waitzerostart
	
	;take first time measurement
	jsr		readtimer
	sty		timer1+1
	sta		timer1

	;wait for 19 bit transitions
	lda		#$10		;test bit 4 of SKSTAT
	ldx		#10			;test 10 pairs of bits
waitone:
	bit		timflg
	bpl		timeout
	bit		skstat
	beq		waitone
	dex
	beq		waitdone
waitzero:
	bit		timflg
	bpl		timeout
	bit		skstat
	bne		waitzero
	beq		waitone
	
timeout:
	ldy		#SIOErrorTimeout
	jmp		SIO.xit	

waitdone:
	
	;take second time measurement
	jsr		readtimer
	sta		timer2
	sty		timer2+1
	
	;compute baud rate and adjust pokey divisor
	;
	; counts = (pal ? 156 : 131)*rtdelta + vdelta;
	; lines = counts * 2
	; lines_per_bit = lines / 19
	; cycles_per_bit = lines_per_bit * 114
	; pokey_divisor = cycles_per_bit / 2 - 7
	;
	; -or-
	;
	; pokey_divisor = counts * 2 * 114 / 19 / 2 - 7
	;               = counts * 114 / 19 - 7
	;
	;16 bits at 600 baud is nominally 209 scanline pairs. This means that we
	;don't have to worry about more than two frames, which is at least 262
	;scanline pairs or less than 480 baud. However, since we're using HLE,
	;we can cheat and do this in C++.
	
	lda		#124
	sta		temp3
	
	;compute line difference
	lda		timer1
	jsr		correct_time
	sta		temp1
	
	lda		timer2
	jsr		correct_time
	sub		temp1
	sta		temp1
	lda		#0
	sbc		#0
	tay
	
	;compute frame difference
	lda		timer2+1
	sub		timer1+1
	tax
	
	;accumulate frame difference
	beq		no_frames
	lda		temp1
add_frame_loop:
	clc
	adc		temp3
	scc:iny
	dex
	bne		add_frame_loop
	sta		temp1
no_frames:
	sty		temp1+1

	;compute lines*6 - 7
	asl		temp1				;lines*2 (lo)
	rol		temp1+1				;lines*2 (hi)
	lda		temp1				;
	ldy		temp1+1				;
	asl		temp1				;lines*4 (lo)
	rol		temp1+1				;lines*4 (hi)
	adc		temp1				;lines*6 (lo)
	tax							;
	tya							;
	adc		temp1+1				;
	tay
	txa
	sec
	sbc		#7
	sta		audf3
	tya
	sbc		#0
	sta		audf4
		
	;kick pokey into init mode to reset serial input shift hw
	ldx		sskctl
	txa
	and		#$fc
	sta		skctl
	
	;reset serial port status
	sta		skres
	
	;re-enable serial input hw
	stx		skctl
	
	jsr		SIOSetupBufferPointers
	
	;stuff two $55 bytes into the buffer, which we "read" above
	lda		#$55
	ldy		#0
	ldx		#2
aaloop:
	sta		(bufrlo),y
	inw		bufrlo
	dex:bne	aaloop
	
	;reset checksum for two $55 bytes and receive frame
	lda		#$aa
	sta		chksum
	
	jmp		SIOReceive.use_checksum

readtimer:
	ldy		rtclok+2
	lda		vcount
	cpy		rtclok+2
	bne		readtimer
	rts
	
correct_time:
	sec
	sbc		#124
	bcs		time_ok
	adc		temp3
time_ok:
	rts
		
.endp

