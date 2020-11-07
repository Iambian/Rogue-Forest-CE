

XDEF _asm_LoadMinimap
XDEF _asm_SetTile2ColorStart

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
            
            
            
            
            
            
            
            
            
            
            call  _asm_SetTile2ColorStart
            push  hl
            pop   iy          ;
            
            
            
            
            
            
            
            














