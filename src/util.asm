

XDEF _asm_LoadMinimap
XDEF _asm_SetTile2ColorStart
XDEF _asm_InterpolateMap
XDEF _asm_GetNumpad

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
_KbdScan          EQU $020148 ;''
_GetCSC           EQU $02014C ;''


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
                  ld    iy,(320*39)+234
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
                              add   a,39
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
      ;ld    a,2
      ;ld    (-1),a
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
            
            ld    e,c
            ld    bc,-129
            add   hl,bc
            ld    bc,interpolate_xform1
            ld    a,(hl)
            inc   a
            jr    nz,$+3      ;check if current tile is a path (z)
            inc   b           ;next table forward if current tile is a path
            ex    de,hl       ;now: HL = tile neighborhood state -> tbl offset
            add   hl,bc       ;add table to get complete address for new tile state
            ld    a,(hl)      ;retrieve tile from table
            ex    de,hl       ;HL is now back to being curpos in 0/-1 reader
            ld    (ix+0),a    ;write result
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
db $07,$0B,$01,$01,$0A,$03,$01,$01,$06,$06,$03,$00,$06,$06,$03,$00
db $08,$08,$03,$03,$08,$08,$02,$02,$03,$03,$03,$03,$03,$03,$03,$03
db $05,$03,$01,$01,$11,$03,$01,$01,$06,$06,$03,$00,$06,$06,$03,$00
db $08,$08,$03,$03,$08,$08,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $0D,$0D,$03,$03,$0D,$0D,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $0D,$0D,$03,$03,$0D,$0D,$03,$03,$0C,$0C,$03,$03,$03,$03,$03,$03
db $03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $04,$10,$01,$01,$03,$03,$01,$01,$06,$06,$03,$03,$06,$06,$03,$03
db $08,$08,$03,$03,$08,$08,$02,$02,$03,$03,$03,$03,$03,$03,$03,$03
db $03,$03,$01,$01,$03,$03,$01,$01,$06,$06,$03,$03,$06,$06,$03,$03
db $08,$08,$03,$03,$08,$08,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $0D,$0D,$03,$03,$0D,$0D,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $0E,$03,$03,$03,$0E,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03
db $0D,$0D,$03,$03,$0D,$0D,$03,$03,$0C,$0C,$03,$03,$03,$03,$03,$03
db $0E,$03,$03,$03,$0E,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03,$03

interpolate_xform2:
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$50,$48,$48,$48,$50
db $48,$48,$48,$48,$48,$48,$4E,$4E,$48,$48,$4F,$4F,$48,$48,$4F,$4F
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$50,$48,$48,$48,$50
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$4F,$4F,$48,$48,$4F,$4F
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$49,$49,$48,$48,$49,$49
db $48,$48,$47,$47,$48,$48,$47,$47,$41,$41,$48,$48,$41,$41,$48,$48
db $48,$48,$48,$48,$48,$48,$48,$48,$42,$42,$49,$49,$48,$48,$49,$49
db $48,$48,$47,$47,$48,$48,$47,$47,$41,$41,$48,$48,$41,$41,$48,$48
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48
db $48,$48,$48,$48,$48,$48,$4E,$4E,$48,$48,$4F,$4F,$48,$48,$4F,$4F
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$48
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$4F,$4F,$48,$48,$4F,$4F
db $48,$48,$48,$48,$48,$48,$48,$48,$48,$48,$49,$49,$48,$48,$49,$49
db $40,$48,$47,$47,$40,$48,$47,$47,$41,$41,$48,$48,$41,$41,$48,$48
db $48,$48,$48,$48,$48,$48,$48,$48,$42,$42,$49,$49,$48,$48,$49,$49
db $40,$48,$47,$47,$40,$48,$47,$47,$41,$41,$48,$48,$41,$41,$48,$48


;returns 0 for no press. 1-10 for keys [1-9,0]. Blocks until keyrelease
_asm_GetNumpad:
      push  ix
            call  _KbdScan
            call  _GetCSC
      pop   ix
      or    a,a
      ret   z
      ld    hl,getnumpad_datatable
      ld    bc,10
      cpir
      jr    z,$+4
      xor   a,a
      ret
      ld    a,c
      cpl
      add   a,10+1
      ret
getnumpad_datatable:
;  1  2  3  4  5  6  7  8  9  0
db 34,26,18,35,27,19,36,28,20,33
      
      
      
