.binary "Hydlide (U) [!].nes"

	* = $10
	.logical $8000
	
	
	
	
	
	
	
	* = $e398
	
sfx_tbl	.byte $90,$08,$00,$88,$84,$82,$80,$88,$84,$8A,$C0,$88,$82,$82,$40,$88,$8C,$CA,$FF,$A0,$8C,$CA,$FF,$A0,$A1,$AC,$FF,$8A,$BF,$08,$40,$08,$BF,$08,$6B,$08,$BF,$08,$55,$08,$BF,$08,$47,$08,$A1,$08,$40,$88,$10,$00,$00,$88,$38,$00,$0E,$B0,$0F,$00,$08,$40,$1F,$00,$0D,$B0,$23
	
	.include "exported-tracks.txt"
	
	
	
	* = $e330
make_ptr	.macro
		.word \1,\1+size(\1)
	.endm
	
	.word $e396,$e398
	#make_ptr song_data_0_0
	#make_ptr song_data_1_0
	#make_ptr song_data_2_0
	#make_ptr song_data_3_0
	#make_ptr song_data_0_0
	
	.word $e396,$e398
	#make_ptr song_data_0_1
	#make_ptr song_data_1_1
	#make_ptr song_data_2_1
	#make_ptr song_data_3_1
	#make_ptr song_data_0_1
	
	
	
	* = $e27a
	lda sfx_tbl + $00,y
	sta $4000
	lda sfx_tbl + $01,y
	sta $4001
	lda sfx_tbl + $02,y
	sta $4002
	lda sfx_tbl + $03,y
	sta $4003
	
	* = $e29c
	lda sfx_tbl + $30,y
	sta $400c
	lda sfx_tbl + $31,y
	sta $400d
	lda sfx_tbl + $32,y
	sta $400e
	lda sfx_tbl + $33,y
	sta $400f
	
	
	.here
	
	