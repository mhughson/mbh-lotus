@echo off

:: Options
:: audio
:: code
:: run
 
::echo %audio%
::echo %code%
::echo %run%

set name="lotus"
set name_no_quote=lotus

set path=..\bin\;%path%

set CC65_HOME=..\

IF DEFINED audio (
	MUSIC\text2vol5.exe MUSIC\songs.txt -ca65
	MUSIC\nsf2data5.exe MUSIC\sounds.nsf -ca65
)

IF DEFINED maps (
	py NES_ST/meta.py tiles_temp.nam ..\maps\nametable_temp.json temp
	py NES_ST/meta.py tiles_overworld.nam ..\maps\nametable_overworld.json overworld
	py MAPS\generate_maps_header.py
)

IF DEFINED code (
	REM -g adds debug information, but the end result .nes file is not
	REM affected, so leave it in all the time.
	cc65 -g -Oirs main.c --add-source
	cc65 -g -Oirs PRG0.c --add-source
	cc65 -g -Oirs PRG1.c --add-source
	cc65 -g -Oirs PRG2.c --add-source
	cc65 -g -Oirs PRG3.c --add-source
	cc65 -g -Oirs PRG4.c --add-source
	cc65 -g -Oirs PRG5.c --add-source
	cc65 -g -Oirs PRG6.c --add-source
	cc65 -g -Oirs PRG7.c --add-source
	cc65 -g -Oirs mmc3\mmc3_code.c --add-source
	ca65 crt0.s
	ca65 main.s -g
	ca65 PRG0.s -g
	ca65 PRG1.s -g
	ca65 PRG2.s -g
	ca65 PRG3.s -g
	ca65 PRG4.s -g
	ca65 PRG5.s -g
	ca65 PRG6.s -g
	ca65 PRG7.s -g
	ca65 mmc3\mmc3_code.s -g

	REM -dbgfile does not impact the resulting .nes file.
	ld65 -C mmc3_128_128.cfg --dbgfile %name%.dbg -o %name%.nes crt0.o main.o mmc3\mmc3_code.o PRG0.o PRG1.o PRG2.o PRG3.o PRG4.o PRG5.o PRG6.o PRG7.o nes.lib -Ln labels.txt -m map.txt

	del *.o
	del mmc3\*.o

	mkdir BUILD\
	move /Y %name%.nes BUILD\ 
	move /Y %name%.dbg BUILD\ 
	move /Y labels.txt BUILD\ 
	move /Y main.s BUILD\ 
REM	move /Y mmc3\mmc3_code.s BUILD\ 
	move /Y PRG0.s BUILD\ 
	move /Y PRG1.s BUILD\ 
	move /Y PRG2.s BUILD\
	move /Y PRG3.s BUILD\
	move /Y PRG4.s BUILD\
	move /Y PRG5.s BUILD\
	move /Y PRG6.s BUILD\
	move /Y PRG7.s BUILD\
)

if DEFINED run (
	BUILD\%name%.nes
)

if DEFINED deploy (
	N8\edlink-n8 ..\game\BUILD\%name_no_quote%.nes
)

::set audio
::set code
::set run