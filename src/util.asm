

XDEF _asm_LoadMinimap
XDEF _asm_SetTile2ColorStart
XDEF _asm_InterpolateMap

XREF _gfx_SetColor
XREF _gfx_Rectangle_NoClip


flags             EQU $D00080 ;As defined in ti84pce.inc
_LoadPattern      EQU $021164 ;''
_FindAlphaDn      EQU $020E90 ;''
_FindAlphaUp      EQU $020E8C ;''
_ChkFindSym       EQU $02050C ;''
_ChkInRam         EQU $021F98 ;'' NC if in RAM, C if in arc
_PopRealO1        EQU $0205DC ;''
_PopRealO2        EQU $0205D8 ;''
_PopRealO4        EQU $0205D0 ;''
_PushRealO1       EQU $020614 ;''
_PushRealO4       EQU $020608 ;''
_SetLocalizeHook  EQU $0213F0 ;''
_ClrLocalizeHook  EQU $0213F4 ;''
_SetFontHook      EQU $021454 ;''
_ClrFontHook      EQU $021458 ;''
_MovFrOp1         EQU $02032C ;''
_Mov9ToOp1        EQU $020320 ;''
_DelVarArc        EQU $021434 ;''
_CreateAppVar     EQU $021330 ;''
_CreateProtProg   EQU $021334 ;''

prevDData         EQU $D005A1 ;''
lFont_record      EQU $D005A4 ;''
sFont_record      EQU $D005C5 ;''
Op1               EQU $D005F8 ;''
Op2               EQU $D00603 ;''
Op3               EQU $D0060E ;''
Op4               EQU $D00619 ;''
Op5               EQU $D00624 ;''
Op6               EQU $D0062F ;''

DRAW_BUFFER       EQU $E30014


      

XREF _curmap
XREF _tile2color

;Perform page-alignment shenanigans for faster accesses with _asm_LoadMinimap
_asm_SetTile2ColorStart:
      ld    hl,_tile2color
      inc   L
      dec   L
      ret   z
      inc   h
      ld    L,0
      ret

;screenx = DRAW_BUFFER + (320*ypos)+xpos -- xpos=234 ypos=37
;arg0 = x (+0), arg1 = y (+3)
_asm_LoadMinimap:       ;-3
      push  ix          ;-6
            ld    c,2   ;color red
            push  bc
                  call _gfx_SetColor
            pop   bc
            ld    ix,6
            add   ix,sp
            ;Get offset into curmap, adjusted.
            sbc   hl,hl
            ex    de,hl       ;Zero DE
            ld    L,(ix+3)    ;get Y offset
            srl   L
            ld    B,L         ;back up Y into B.
            ld    H,128
            mlt   hl
            ld    e,(ix+0)
            srl   e
            ld    c,e         ;back up X into C. BC=YXofs.
            add   hl,de       ;offsetted.
            push  bc          ;-9
                  ld    bc,(_curmap)
                  add   hl,bc       ;address completed
                  ex    de,hl       ;address to curmap in DE
                  ;Get screen render address. Oh. This offset is hardcoded. Woo.
                  ld    bc,(DRAW_BUFFER)
                  ld    iy,(320*37)+234
                  add   iy,bc
                  ;Get mapper
                  call  _asm_SetTile2ColorStart
                  ;DE=mapper, HL=curmap
                  ex    de,hl
                  ld    c,64
loadminimap_mainloop:
                  ld    b,64
loadminimap_subloop:
                  ld    e,(hl)
                  ld    a,(de)
                  ld    (iy+0),a
                  inc   hl
                  inc   iy
                  djnz  loadminimap_subloop
                  ld    a,c
                  ld    bc,320-64
                  add   iy,bc
                  ld    bc,64
                  add   hl,bc
                  ld    c,a
                  dec   c
                  jr    nz,loadminimap_mainloop
                  ;Prepare render of camera-bounded box
                  ;ld    a,2
                  ;ld    (-1),a
                  ld    c,14
                  push  bc                      ;h
                        push  bc                ;w
                              ld    e,(ix+3)
                              ld    d,(ix-8)
                              call  loadminimap_clamp
                              add   a,37
                              ld    c,a
                              push  bc
                                    ld    e,(ix+0)
                                    ld    d,(ix-9)
                                    call  loadminimap_clamp
                                    ld    c,a
                                    ld    hl,234
                                    add   hl,bc
                                    push  hl
                                          call _gfx_Rectangle_NoClip
                                    pop   af
                              pop   af
                        pop   af
                  pop   af
            pop   bc
      pop   ix
      ret
            
            
            
loadminimap_clamp:
      ld    a,e
      ;srl   a
      sub   a,7
      sub   a,d
      cp    a,200             ;if nc, is lt 0
      jr    c,$+3
      xor   a,a
      cp    a,63-14
      jr    c,$+4
      ld    a,63-14
      ret
            
;0xFF = path
;Everything else = wall
_asm_InterpolateMap:
      push  ix
      ld    hl,(_curmap)
      inc   hl
      inc   hl
      ld    bc,16384
      ld    de,($E30014)
      inc   d     ;making it so there's no I/O errors at boundaries
      push  hl
            push  de
                  push  bc
                        ldir
                  pop   bc
            pop   hl    ;reading from
      pop   ix          ;writing to
interpolatemap_loop:
      push  bc
            ld    a,(hl)
            inc   a
            ld    a,9
            jr    z,interpolatemap_skip
            ld    b,1
            ld    de,-129     ;get curmap into position
            add   hl,de       ;
            ;topleft
            ld    a,(hl)
            inc   hl
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 0      
            ld    a,(hl)
            inc   hl
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 1
            ld    a,(hl)
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 2
            ld    de,126
            add   hl,de
            ;left
            ld    a,(hl)
            inc   hl
            inc   hl
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 3
            ld    a,(hl)
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 4
            add   hl,de
            ;btm-left
            ld    a,(hl)
            inc   hl
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 5
            ld    a,(hl)
            inc   hl
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 6
            ld    a,(hl)
            add   a,b         ;carly if 0xFF
            rr    c           ;bitpos 7
            push  hl
                  ld    e,c
                  ld    hl,interpolate_xform1
                  add   hl,de
                  ld    a,(hl)
            pop   hl
            ld    de,-129     ;move hl to next position in loop
            add   hl,de
interpolatemap_skip:
            ld    (ix+0),a
            inc   hl
            inc   ix
      pop   bc
      dec   bc
      ld    a,c
      or    a,b
      jr    nz,interpolatemap_loop
      pop   ix
      ret


            
            
            
            
            



;top: 07, btm: 03
interpolate_xform1:
db $07  ;0
db $07  ;1
db $01  ;2
db $01  ;3
db $07  ;4
db $07  ;5
db $01  ;6
db $01  ;7
db $06  ;8
db $06  ;9
db $07  ;10
db $00  ;11
db $07  ;12
db $07  ;13
db $07  ;14
db $00  ;15
db $08  ;16
db $07  ;17
db $07  ;18
db $07  ;19
db $08  ;20
db $07  ;21
db $02  ;22
db $02  ;23
db $07  ;24
db $07  ;25
db $07  ;26
db $07  ;27
db $07  ;28
db $03  ;29
db $07  ;30
db $07  ;31
db $07  ;32
db $07  ;33
db $07  ;34
db $07  ;35
db $07  ;36
db $07  ;37
db $07  ;38
db $07  ;39
db $06  ;40
db $06  ;41
db $07  ;42
db $00  ;43
db $07  ;44
db $07  ;45
db $07  ;46
db $00  ;47
db $07  ;48
db $07  ;49
db $07  ;50
db $07  ;51
db $07  ;52
db $07  ;53
db $07  ;54
db $07  ;55
db $07  ;56
db $07  ;57
db $07  ;58
db $07  ;59
db $07  ;60
db $03  ;61
db $07  ;62
db $07  ;63
db $0D  ;64
db $07  ;65
db $07  ;66
db $07  ;67
db $07  ;68
db $07  ;69
db $07  ;70
db $07  ;71
db $07  ;72
db $07  ;73
db $07  ;74
db $07  ;75
db $07  ;76
db $07  ;77
db $07  ;78
db $07  ;79
db $07  ;80
db $07  ;81
db $07  ;82
db $07  ;83
db $07  ;84
db $07  ;85
db $07  ;86
db $07  ;87
db $07  ;88
db $07  ;89
db $07  ;90
db $07  ;91
db $07  ;92
db $07  ;93
db $07  ;94
db $07  ;95
db $0D  ;96
db $07  ;97
db $07  ;98
db $03  ;99
db $07  ;100
db $07  ;101
db $07  ;102
db $03  ;103
db $0C  ;104
db $0C  ;105
db $07  ;106
db $07  ;107
db $07  ;108
db $07  ;109
db $07  ;110
db $07  ;111
db $07  ;112
db $07  ;113
db $07  ;114
db $07  ;115
db $07  ;116
db $07  ;117
db $07  ;118
db $07  ;119
db $07  ;120
db $07  ;121
db $07  ;122
db $07  ;123
db $07  ;124
db $07  ;125
db $07  ;126
db $07  ;127
db $07  ;128
db $07  ;129
db $07  ;130
db $07  ;131
db $07  ;132
db $07  ;133
db $07  ;134
db $07  ;135
db $07  ;136
db $07  ;137
db $07  ;138
db $07  ;139
db $07  ;140
db $07  ;141
db $07  ;142
db $07  ;143
db $08  ;144
db $07  ;145
db $07  ;146
db $07  ;147
db $08  ;148
db $07  ;149
db $02  ;150
db $02  ;151
db $07  ;152
db $07  ;153
db $07  ;154
db $07  ;155
db $07  ;156
db $03  ;157
db $07  ;158
db $07  ;159
db $07  ;160
db $07  ;161
db $07  ;162
db $07  ;163
db $07  ;164
db $07  ;165
db $07  ;166
db $07  ;167
db $07  ;168
db $07  ;169
db $07  ;170
db $07  ;171
db $07  ;172
db $07  ;173
db $07  ;174
db $07  ;175
db $07  ;176
db $07  ;177
db $07  ;178
db $07  ;179
db $07  ;180
db $07  ;181
db $07  ;182
db $07  ;183
db $03  ;184
db $03  ;185
db $07  ;186
db $07  ;187
db $03  ;188
db $03  ;189
db $07  ;190
db $07  ;191
db $0D  ;192
db $07  ;193
db $07  ;194
db $07  ;195
db $07  ;196
db $07  ;197
db $03  ;198
db $03  ;199
db $07  ;200
db $07  ;201
db $07  ;202
db $07  ;203
db $07  ;204
db $07  ;205
db $07  ;206
db $07  ;207
db $0E  ;208
db $07  ;209
db $07  ;210
db $07  ;211
db $0E  ;212
db $07  ;213
db $07  ;214
db $07  ;215
db $07  ;216
db $07  ;217
db $07  ;218
db $07  ;219
db $07  ;220
db $07  ;221
db $07  ;222
db $07  ;223
db $0D  ;224
db $07  ;225
db $07  ;226
db $03  ;227
db $07  ;228
db $07  ;229
db $03  ;230
db $03  ;231
db $0C  ;232
db $0C  ;233
db $07  ;234
db $07  ;235
db $07  ;236
db $07  ;237
db $07  ;238
db $07  ;239
db $0E  ;240
db $07  ;241
db $07  ;242
db $07  ;243
db $0E  ;244
db $07  ;245
db $07  ;246
db $07  ;247
db $07  ;248
db $07  ;249
db $07  ;250
db $07  ;251
db $07  ;252
db $07  ;253
db $07  ;254
db $03  ;255









