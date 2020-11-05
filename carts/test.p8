pico-8 cartridge // http://www.pico-8.com
version 29
__lua__
-- pico clock
--  @cbmakes

function _init()
 bgcol = 0
 fgcol = 7
 panel = "closed"
end

function _update()
 if btnp(❎) then
  if panel == "closed" then
   panel = "open"
  else
   panel = "closed"
  end
 end
end

function _draw()
 cls(bgcol)
 sspr(8,0,24,6,2,0)
 --sspr(8,8,28,6,2,8)

 -- local time
 print("local time: "..stat(93)..":"..stat(94)..":"..stat(95),2,21,7)
 print("date: "..stat(92).."/"..stat(91).."/"..stat(90),2,28,7)

 -- utc time
 print("utc time: "..stat(83)..":"..stat(84)..":"..stat(85),2,40,7)
 print("date: "..stat(82).."/"..stat(81).."/"..stat(80),2,47,7)

 -- developer info.
 if panel == "open" then
  rectfill(0,60,128,128,8)
  rectfill(0,56,62,128,8)
  pset(62,56,0)
  rectfill(117,56,128,128,8)
  pset(117,56,0)
  print("❎",119,58,15)
  print("developer info.",2,58,2)
  print("local time: computer clock",2,73)
  print("utc: coordinated universal time",2,80)

  print("more info on accessing this",2,96)
  print("data can be found in tab 2",2,103)
  print("of the code of this cartridge.",2,110)

 else
  rectfill(0,110,128,128,8)
  rectfill(0,106,62,128,8)
  pset(62,106,0)
  rectfill(117,106,128,128,8)
  pset(117,106,0)
  print("❎",119,108,2)
  print("developer info.",2,108,2)
 end
end
-->8
-------------------------------
-- accessing time via stat() --
-------------------------------
-- you can call stat() with  --
-- any of the values below   --
-- to return that data.      --
-------------------------------
--       stat() values       --
-------------------------------
-- stat(80) = year   (utc)   --
-- stat(81) = month  (utc)   --
-- stat(82) = day    (utc)   --
-- stat(83) = hour   (utc)   --
-- stat(84) = minute (utc)   --
-- stat(85) = second (utc)   --
-- stat(90) = year   (local) --
-- stat(91) = month  (local) --
-- stat(92) = day    (local) --
-- stat(93) = hour   (local) --
-- stat(94) = minute (local) --
-- stat(95) = second (local) --
-------------------------------
-------------------------------
--  thanks for reading!      --
--                -chris :]  --
-------------------------------
-------------------------------
__gfx__
00000000777700777777007777007770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000777770777777077777077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700770770007700077000077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000777770007700077000077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000777700777777077777077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700770000777777007777007770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000077770770000777000777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000777770770007777707777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000770000770007707707700007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000770000770007707707700007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000777770777707777707777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000077770777700777000777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__label__
00777700777777007777007770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777770777777077777077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770770007700077000077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777770007700077000077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777700777777077777077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770000777777007777007770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077770770000777000777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777770770007777707777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770000770007707707700007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770000770007707707700007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777770777707777707777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077770777700777000777707707700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700007700770777070000000777077707770777000000000770077700000777077700000777000000000000000000000000000000000000000000000000000
00700070707000707070000000070007007770700007000000070000700700007000700700707000000000000000000000000000000000000000000000000000
00700070707000777070000000070007007070770000000000070000700000077000700000777000000000000000000000000000000000000000000000000000
00700070707000707070000000070007007070700007000000070000700700007000700700007000000000000000000000000000000000000000000000000000
00777077000770707077700000070077707070777000000000777000700000777000700000007000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770077707770777000000000777000707770007077707770777077700000000000000000000000000000000000000000000000000000000000000000000000
00707070700700700007000000700007007000070000707070007070700000000000000000000000000000000000000000000000000000000000000000000000
00707077700700770000000000777007007770070077707070777070700000000000000000000000000000000000000000000000000000000000000000000000
00707070700700700007000000007007000070070070007070700070700000000000000000000000000000000000000000000000000000000000000000000000
00777070700700777000000000777070007770700077707770777077700000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00707077700770000077707770777077700000000077707700000077707770000077700000000000000000000000000000000000000000000000000000000000
00707007007000000007000700777070000700000000700700070000700070070070700000000000000000000000000000000000000000000000000000000000
00707007007000000007000700707077000000000077700700000007700070000077700000000000000000000000000000000000000000000000000000000000
00707007007000000007000700707070000700000070000700070000700070070000700000000000000000000000000000000000000000000000000000000000
00077007000770000007007770707077700000000077707770000077700070000000700000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00770077707770777000000000777000707770007077707770777077700000000000000000000000000000000000000000000000000000000000000000000000
00707070700700700007000000700007007000070000707070007070700000000000000000000000000000000000000000000000000000000000000000000000
00707077700700770000000000777007007770070077707070777070700000000000000000000000000000000000000000000000000000000000000000000000
00707070700700700007000000007007000070070070007070700070700000000000000000000000000000000000000000000000000000000000000000000000
00777070700700777000000000777070007770700077707770777077700000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
88888888888888888888888888888888888888888888888888888888888888000000000000000000000000000000000000000000000000000000008888888888
88888888888888888888888888888888888888888888888888888888888888800000000000000000000000000000000000000000000000000000088888888888
88228822282828222828888228222822282228888822282288222882288888800000000000000000000000000000000000000000000000000000088822222888
88282828882828288828882828282828882828888882882828288828288888800000000000000000000000000000000000000000000000000000088228282288
88282822882828228828882828222822882288888882882828228828288888888888888888888888888888888888888888888888888888888888888222822288
88282828882228288828882828288828882828888882882828288828288888888888888888888888888888888888888888888888888888888888888228282288
88222822288288222822282288288822282828888822282828288822888288888888888888888888888888888888888888888888888888888888888822222888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888