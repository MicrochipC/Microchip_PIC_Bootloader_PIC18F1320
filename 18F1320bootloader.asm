;================== User setting section ======================================
	list		p=PIC18f1320	;this directive must come first
					;Set it for the kind of PIC you are using!

	#include <p18f1320.inc>
	

    __CONFIG  _CONFIG1H, _IESO_ON_1H & _FSCMEN_OFF_1H & _HS_OSC_1H
    __CONFIG  _CONFIG2L, _BORV_20_2L & _BOR_ON_2L & _PWRT_OFF_2L
    __CONFIG  _CONFIG2H, _WDT_ON_2H & _WDTPS_256_2H
    __CONFIG  _CONFIG3H, _MCLRE_OFF_3H
    __CONFIG  _CONFIG4L, _BKBUG_OFF_4L & _LVP_OFF_4L & _STVR_ON_4L
    __CONFIG  _CONFIG5L, _CP0_OFF_5L & _CP1_OFF_5L ;& _CP2_OFF_5L & _CP3_OFF_5L
    __CONFIG  _CONFIG5H, _CPB_OFF_5H & _CPD_OFF_5H
    __CONFIG  _CONFIG6L, _WRT0_OFF_6L & _WRT1_OFF_6L ;& _WRT2_OFF_6L & _WRT3_OFF_6L
    __CONFIG  _CONFIG6H, _WRTC_OFF_6H & _WRTB_OFF_6H & _WRTD_OFF_6H
    __CONFIG  _CONFIG7L, _EBTR0_OFF_7L & _EBTR1_OFF_7L ;& _EBTR2_OFF_7L & _EBTR3_OFF_7L
    __CONFIG  _CONFIG7H, _EBTRB_OFF_7H

													  ;     set same microcontroller in the project
	#define FOSC D'20000000' 	; <<< set quartz frequence [Hz], max. 20 MHz
	#define BAUD D'19200'			; <<< set baud rate [bit/sec]
	#define	BAUD_ERROR	D'2'	;	<<< set baud rate error [%]
	#define TIME							; <<< set method of bootloader start PIN/TIME
														;     PIN	: start on low level of trigger pin
                            ;     TIME: start on receive IDENT byte in TIMEOUT
	#define	TIMEOUT		D'1'		; <<< only for TIME - set time [0.1s], max. 25 sec

;=================== Configuration ============================================

;	__IDLOCS H'2100'					; version ID of bootloader
;=============== End of user setting section ==================================


;================== Check User Constants ======================================

;========================== Constants =========================================

	IF ((FOSC/(D'16' * BAUD))-1) < D'256'
		#define DIVIDER (FOSC/(D'16' * BAUD))-1
		#define HIGH_SPEED 1
	ELSE
		#define DIVIDER (FOSC/(D'64' * BAUD))-1
		#define HIGH_SPEED 0
	ENDIF

BAUD_REAL	EQU	FOSC/((D'64'-(HIGH_SPEED*D'48'))*(DIVIDER+1))

	IF BAUD_REAL > BAUD
		IF (((BAUD_REAL - BAUD)*D'100')/BAUD) > BAUD_ERROR
			ERROR	"wrong baud rate"
		ENDIF
	ELSE
		IF (((BAUD - BAUD_REAL)*D'100')/BAUD) > BAUD_ERROR
			ERROR	"wrong baud rate"
		ENDIF
	ENDIF
	
	;//T1CON:
	;//bit 7: read/write as one 16 bit operation enable (0)
	;//bit 6: timer 1 system clock status bit			(0)
	;//bit 5: prescalar select							(1)
	;//bit 4: prescalar select							(1)
	;//bit 3: timer 1 oscillator enable bit				(0)
	;//bit 2: external clock input syncronization		(0)
	;//bit 1: timer 1 clock select						(0)
	;//bit 0: timer 1 on bit							(1)

	IF FOSC > D'10240000'
		#define	T1PS 8
		#define	T1SU 0x31	
	ELSE
		IF FOSC > D'5120000'
			#define T1PS 4
			#define T1SU 0x21
		ELSE
			IF FOSC > D'2560000'
				#define T1PS 2
				#define T1SU 0x11
			ELSE
				#define T1PS 1
				#define T1SU 0x01
			ENDIF
		ENDIF
	ENDIF

TIMER	EQU	(D'65538'-(FOSC/(D'10'*4*T1PS))); reload value for TIMER1 (0.1s int)

#define	NumRetries	1												; number of writing retries


#define WRITE       0xE3										; communication protocol
#define WR_OK       0xE4
#define WR_BAD      0xE5

#define DATA_OK     0xE7
#define DATA_BAD    0xE8

#define IDENT       0xEA
#define IDACK       0xEB

#define DONE        0xED

;=============== Variables ====================================================

buff			EQU	0x20
; RAM address 0x70 reserved for MPLAB-ICD
amount		EQU	0x71
chk1			EQU	0x72
chk2			EQU	0x73
retry			EQU	0x74
address		EQU 0x75
address_high	EQU	0x76
address_upper	EQU 0x77

tmpaddr		EQU 0x78

temp			EQU	0x79
time			EQU	0x7A
count			EQU 0x7B
counter			EQU 0x7C

COUNTER_HI		EQU	0x7D
COUNTER			EQU 0x7E

;//just for my fun
temp2			EQU 0x7F

;//code address of the start of the 64 byte block
;//in which address resides
CODE_ADDR_UPPER		EQU 0x84
CODE_ADDR_HIGH		EQU 0x85
CODE_ADDR_LOW		EQU 0x86

offset				EQU	0x87
num_groups			EQU 0x88
;//memory address to load the code into before modification
BUFFER_ADDR_HIGH  	EQU 0x00
BUFFER_ADDR_LOW		EQU 0xC0

;------------------------------------------------------------------------------
	ORG     0x0000                ; reset vector of microcontroller
	nop							 ; for compatibility with ICD
	goto    Main

;------------------------------------------------------------------------------

		ORG		0x1DF0
UserStart:
	
;------------------------------------------------------------------------------
		ORG     0x1E00
Main:
	btfss	RCON, 3		;//test to see if a WDT reset
		goto	UserStart	;//it did, so run user program
start:

	CLRF 	PORTB ; Initialize PORTB by
	MOVLW 	0x70 ; Set RB0, RB1, RB4 as
	MOVWF 	ADCON1 ; digital I/O pins

	;//make sure interrupt doesn't get called
	clrf	PIE1
	clrf	INTCON

	movlw	b'10010000'											; SPEN = 1, CREN = 1
	movwf	RCSTA
	
	;//set up for 20.0mhz 19.2k baud. SYNC=0,BRGH=1,BRG16=1,  SPBRG=259
	movlw	b'00100100'		;TXEN, BRGH enabled 
	movwf	TXSTA

	clrf	BAUDCTL
	bsf		BAUDCTL, BRG16
	
	;//259 = 0x01, 0x03
	movlw	0x01
	movwf	SPBRGH
	movlw	0x03										; baud rate generator
	movwf	SPBRG

	;//clear all errors by reading 3 bytes
	call	overerror
	
	movlw	TIMEOUT+1								; for TIME: set timeout
	movwf	time
	movlw	T1SU
	movwf	T1CON										; TIMER1 on, internal clock, prescale T1PS
	bsf   	PIR1,TMR1IF
	call	getbyte									; wait for IDENT
	xorlw	IDENT
	btfss	STATUS,Z
	 goto	user_restore
	clrf	time										; no more wait for IDENT
	goto	inst_ident							; bootloader identified, send of IDACK

;------------------------------------------------------------------------------

receive														; programming
	call	getbyte										; get byte from USART
	movwf	temp
	xorlw	WRITE
	btfsc	STATUS,Z
	 goto	inst_write								; write instruction
	movf	temp,w
	xorlw	IDENT
	btfsc	STATUS,Z
	 goto	inst_ident								; identification instruction
	movf	temp,w
	xorlw	DONE
	btfss	STATUS,Z									; done instruction ?
	 goto	receive

;------------------------------------------------------------------------------
inst_done													; very end of programming
;------------------------------------------------------------------------------
	movlw	WR_OK
  call	putbyte										; send of byte
	movlw	TIMEOUT+1
	movwf	time
  call	getbyte                   ; has built in timeout - waits until done
;------------------------------------------------------------------------------
user_restore
	clrf  T1CON											; shuts off TIMER1
	clrf  RCSTA
	clrf  TXSTA											; restores USART to reset condition
	clrf  PIR1
	goto	UserStart								  ; run user program

;------------------------------------------------------------------------------
inst_ident
	;//clear all errors by reading 3 bytes
	call	overerror

	movlw	IDACK											; send IDACK
	goto	send_byte
;------------------------------------------------------------------------------
inst_write
	call	getbyte
	movwf	address_upper

	call	getbyte
	movwf	address_high									; high byte of address
	call	getbyte
	movwf	address										; low byte of address
	call	getbyte
	movwf	amount										; number of bytes -> amount -> count
	movwf 	count
	call	getbyte										; checksum -> chk2
	movwf	chk2
	clrf	chk1											; chk1 = 0
	lfsr	FSR0, 0x20		; Point to the buffer

receive_data
	call	getbyte										; receive next byte -> buff[FSR]
	movwf	POSTINC0		; Store the data
	addwf	chk1,f										; chk1 := chk1 + buff[FSR]
	decfsz amount,f
		goto receive_data							; repeat until (--count==0)
checksum
	movf	chk1,w
	xorwf	chk2,w										; if (chk1 != chk2)
	movlw 	DATA_BAD
	btfss	STATUS,Z
	 goto	send_byte									; checksum WRONG
checksum_ok
	movlw	DATA_OK										; checksum OK
	call	putbyte
write_byte
	call	Write_Mem							; write to eeprom
	iorlw	0
	movlw 	WR_OK											; writing OK
	btfsc	STATUS,Z
	movlw	WR_BAD										; writing WRONG

;------------------------------------------------------------------------------
send_byte
	call	putbyte										; send of byte
	goto	receive										; go to receive from UART
;------------------------------------------------------------------------------

;************************* putbyte subroutine *********************************
putbyte
	clrwdt
	btfss	PIR1,TXIF									; while(!TXIF)
	 goto	putbyte
	movwf	TXREG											; TXREG = byte
	return

;************************* getbyte subroutine *********************************
getbyte
	clrwdt

		movf	time,w
		btfsc	STATUS,Z								; check for time==0
		 goto	getbyte3
		btfss	PIR1,TMR1IF							; check for TIMER1 overflow
		 goto	getbyte3								; no overflow
		bcf		T1CON,TMR1ON						; timeout 0.1 sec
		decfsz	time,f								; time--
			goto	getbyte2
		retlw 0												; if time==0 then return
getbyte2
		bcf		PIR1,TMR1IF
		movlw	high TIMER
		movwf	TMR1H										; preset TIMER1 for 0.1s timeout
		bsf		T1CON,TMR1ON

getbyte3
	;//check for an error
	btfsc	RCSTA,OERR
	goto	overerror	;if overflow error...

	btfss	PIR1,RCIF									; while(!RCIF)
	 goto	getbyte
uart_gotit:
	movf	RCREG,w		;recover uart data

	return

overerror:
	movf	RCREG,w		;flush fifo
	movf	RCREG,w		; all three elements.
	movf	RCREG,w
	retlw 0x00
	
	
	;//assume filled:
	;//address, address_high, address_upper
	;//count
Write_Mem:
	movf	address_upper, W
	iorwf	address_high, W
	iorwf	address, W
	btfss	STATUS, Z
	goto	Calc_Mem_Addys
	;//if we get here, then we are trying to write to
	;//address 0, which is bad, so add 0x1DF0 to it
	movlw	0x1D
	movwf	address+1
	movlw	0xF0
	movwf	address
	

Calc_Mem_Addys:
	movf	address_upper, W
	movwf	CODE_ADDR_UPPER
	movf	address_high, W
	movwf	CODE_ADDR_HIGH
	movf	address, W
	andlw	b'11000000'
	movwf	CODE_ADDR_LOW
	
	movf	address, W
	andlw	b'00111111'	;//we only want the low 6 bits
	movwf	offset
	
	;//now calculate the number of 8 byte groups (normally 2)
	MOVLW 0x40 		; d'64' number of bytes in erase block
	MOVWF COUNTER

	MOVLW BUFFER_ADDR_HIGH 	; point to buffer
	MOVWF FSR0H
	MOVLW BUFFER_ADDR_LOW
	MOVWF FSR0L
	
	MOVF CODE_ADDR_UPPER, W 	; Load TBLPTR with the base, used to be 0x00
	MOVWF TBLPTRU 		; address of the memory block
	MOVF CODE_ADDR_HIGH, W
	MOVWF TBLPTRH
	MOVF CODE_ADDR_LOW, W 	; 6 LSB = 0
	MOVWF TBLPTRL

READ_BLOCK:
	TBLRD*+ 		; read into TABLAT, and inc
	MOVF TABLAT, W 		; get data
	MOVWF POSTINC0 		; store data and increment FSR0
	DECFSZ COUNTER		; done?
	GOTO READ_BLOCK 	; repeat

	;//64 bytes are now in memory at BUFFER_ADDR
	
MODIFY_WORD:
	;//move 'count' bytes from 'buff' to 'BUFFER_ADDR + offset'
	lfsr	FSR0, 0x20
	
	movlw	BUFFER_ADDR_HIGH
	movwf	FSR1H
	movlw	BUFFER_ADDR_LOW
	addwf	offset, W
	movwf	FSR1L
	
	movf	count, W
	movwf	amount

MODIFY_WORD_LOOP:
	movf	POSTINC0, W
	movwf	POSTINC1
	decfsz	amount, F
	goto	MODIFY_WORD_LOOP


ERASE_BLOCK:
	MOVF CODE_ADDR_UPPER, W 	; load TBLPTR with the base
	MOVWF TBLPTRU 		; address of the memory block
	MOVF CODE_ADDR_HIGH, W
	MOVWF TBLPTRH
	MOVF CODE_ADDR_LOW, W 	; 6 LSB = 0
	MOVWF TBLPTRL
	BCF EECON1,CFGS 	; point to PROG/EEPROM memory
	BSF EECON1,EEPGD 	; point to FLASH program memory
	BSF EECON1,WREN 	; enable write to memory
	BSF EECON1,FREE 	; enable Row Erase operation
	BCF INTCON,GIE 		; disable interrupts
	MOVLW 0x55 		; Required sequence
	MOVWF EECON2 		; write 55H
	MOVLW 0xAA
	MOVWF EECON2 		; write AAH
	BSF EECON1,WR 		; start erase (CPU stall)
	NOP
	BSF INTCON,GIE 		; re-enable interrupts
WRITE_BUFFER_BACK:
	clrf	offset

	movlw 0x08 		; number of write buffer groups of 8 bytes
	MOVWF COUNTER_HI
	lfsr	FSR0, 0xC0

PROGRAM_LOOP:
	MOVLW 0x08 		; number of bytes in holding register
	MOVWF COUNTER
WRITE_WORD_TO_HREGS:
	MOVF POSTINC0, W 	; get low byte of buffer data and increment FSR0
	MOVWF TABLAT 		; present data to table latch
	TBLWT*+ 		; short write
				; to internal TBLWT holding register, increment TBLPTR
	DECFSZ COUNTER 		; loop until buffers are full
	GOTO WRITE_WORD_TO_HREGS

	;//since the above code increments us 8 bytes
	;//past the position in which we actually want to
	;//write it at, reload the low address of the code
	;//pointer with the correct offset added
	MOVF CODE_ADDR_LOW, W 	; 6 LSB = 0
	addwf	offset, W
	MOVWF TBLPTRL
	
	movlw	0x08
	addwf	offset, F


PROGRAM_MEMORY:
	BCF INTCON,GIE 		; disable interrupts
	MOVLW 0x55 		; required sequence
	MOVWF EECON2 		; write 55H
	MOVLW 0xAA
	MOVWF EECON2 		; write AAH
	BSF EECON1,WR 		; start program (CPU stall)
	NOP
	BSF INTCON,GIE 		; re-enable interrupts
	DECFSZ COUNTER_HI 	; loop until done
	GOTO PROGRAM_LOOP
	BCF EECON1,WREN 	; disable write to memory

	retlw	0x01	;//return success


;******************************************************************************

	END

