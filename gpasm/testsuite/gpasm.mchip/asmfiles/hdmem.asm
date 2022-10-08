; HDMEM2.ASM

; This file tests header file usage for the specified processor.

	LIST	P=EEPROM16
;;;; Begin: Changed in gputils
;;;;	INCLUDE	"memory.inc"
	INCLUDE	"MEMORY.INC"
;;;; End: Changed in gputils

;==========================================================================
;
;       2-Wire Bus Protocol
;
;==========================================================================
;--------------------------------------------------------------------------
;
;       Standard C-Series
;
;--------------------------------------------------------------------------

	DATA	_24C01A  , H'007F'
	DATA	_24C02A  , H'00FF'
	DATA	_24C04A  , H'01FF'
	DATA	_24C08B  , H'03FF'
	DATA	_24C16B  , H'07FF'

	DATA	_85C72   , H'007F'
	DATA	_85C82   , H'00FF'
	DATA	_85C92   , H'01FF'

;--------------------------------------------------------------------------
;
;       Low-Voltage LC Series and AA Series
;
;--------------------------------------------------------------------------

	DATA	_24LC01B , H'007F'
        DATA    _24AA01	 , H'007F'
        DATA    _24LC02B , H'00FF'
        DATA    _24AA02	 , H'00FF'
        DATA    _24LC04B , H'01FF'
        DATA    _24AA04	 , H'01FF'
        DATA    _24LC08B , H'03FF'
        DATA    _24AA08	 , H'03FF'
        DATA    _24LC16B , H'07FF'
        DATA    _24AA16	 , H'07FF'
        DATA    _24FC16	 , H'07FF'
        DATA    _24LC164 , H'07FF'
        DATA    _24AA164 , H'07FF'
        DATA    _24LC174 , H'07FF'
        DATA    _24AA174 , H'07FF'

;--------------------------------------------------------------------------
;
;       High Density
;
;--------------------------------------------------------------------------

        DATA    _24C32	 , H'0FFF'
        DATA    _24LC32	 , H'0FFF'
        DATA    _24AA32	 , H'0FFF'
        DATA    _24FC32	 , H'0FFF'

;--------------------------------------------------------------------------
;
;       Smart Serial (tm) Family
;
;--------------------------------------------------------------------------

        DATA    _24C65   , H'1FFF'
        DATA    _24LC65	 , H'1FFF'
        DATA    _24AA65	 , H'1FFF'
        DATA    _24FC65	 , H'1FFF'

;--------------------------------------------------------------------------
;
;       VESA (tm) / DDC (tm)
;
;--------------------------------------------------------------------------

        DATA    _24LC21	 , H'007F'
        DATA    _24LCS21 , H'007F'

;==========================================================================
;
;       3-Wire/4-Wire Bus Protocol
;
;==========================================================================
;--------------------------------------------------------------------------
;
;       G.I.
;
;--------------------------------------------------------------------------

        DATA    _59C11   , H'007F'

;--------------------------------------------------------------------------
;
;       Standard C-Series
;
;--------------------------------------------------------------------------

        DATA    _93C06   , H'001F'
        DATA    _93C46   , H'007F'

;--------------------------------------------------------------------------
;
;       Low-Voltage LC Series
;
;--------------------------------------------------------------------------

        DATA    _93LC46  , H'007F'
        DATA    _93LC56  , H'00FF'
        DATA    _93LC66  , H'01FF'

;==========================================================================
;
;       3-Wire Bus Protocol
;
;==========================================================================
;--------------------------------------------------------------------------
;
;       Low-Voltage AA Series
;
;--------------------------------------------------------------------------

        DATA    _93AA46  , H'007F'
        DATA    _93AA56  , H'00FF'
        DATA    _93AA66  , H'01FF'
        DATA    _93AA76  , H'03FF'
        DATA    _93AA86  , H'07FF'

;--------------------------------------------------------------------------
;
;       Low-Voltage LC Series
;
;--------------------------------------------------------------------------

        DATA    _93LC46B , H'007F'
        DATA    _93LC56B , H'00FF'
        DATA    _93LC66B , H'01FF'
;;;; Begin: Changed in gputils
;;;;        DATA    _93LC76B , H'03FF'
;;;;        DATA    _93LC86B , H'07FF'
        DATA    _93LC76  , H'03FF'
        DATA    _93LC86  , H'07FF'
;;;; End: Changed in gputils

;--------------------------------------------------------------------------
;
;       Secure LCS-Series
;
;--------------------------------------------------------------------------

        DATA    _93LCS56 , H'00FF'
        DATA    _93LCS66 , H'01FF'

	END
		