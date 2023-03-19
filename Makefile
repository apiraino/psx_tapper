prog=main
param=-DCDROM_RELEASE
CFLAGS=-Wall -g #-O2
CC=ccpsx
OTHERFLAGS= # -DDONTBLOCK # -DPXFINDER # -DDEBUG

# Movie e BSdec non devono essere compilati come oggetti
# si compilano insieme al programma principale (verificare...)
PSXLIB_OBJS = 	System.o 		\
				DataManager.o 	\
				Sprite.o

TAPPER_OBJS =	menu.o			\
				beer.o			\
				drunkard.o		\
				game.o

all: clean tapper

System.o:
	$(CC) $(CFLAGS) $(param) -I$(PSXLIB_HEADERS)\. -c $(PSXLIB_HEADERS)\System.c -o System.o

DataManager.o:
	$(CC) $(CFLAGS) $(param) -I$(PSXLIB_HEADERS)\. -c $(PSXLIB_HEADERS)\DataManager.c -o DataManager.o

Sprite.o:
	$(CC) $(CFLAGS) $(param) -I$(PSXLIB_HEADERS)\. -c $(PSXLIB_HEADERS)\Sprite.c -o Sprite.o

Prim2D.o:
	$(CC) $(CFLAGS) $(param) -I$(PSXLIB_HEADERS)\. -c $(PSXLIB_HEADERS)\Prim2D.c -o Prim2D.o

beer.o:
	$(CC) $(CFLAGS) $(OTHERFLAGS) -I$(PSXLIB_HEADERS)\. -c beer.c -o beer.o

drunkard.o:
	$(CC) $(CFLAGS) $(OTHERFLAGS) -I$(PSXLIB_HEADERS)\. -c drunkard.c -o drunkard.o

game.o:
	$(CC) $(CFLAGS) $(OTHERFLAGS) -I$(PSXLIB_HEADERS)\. -c game.c -o game.o

menu.o:
	$(CC) $(CFLAGS) $(OTHERFLAGS) -I$(PSXLIB_HEADERS)\. -c menu.c -o menu.o

# Compilation targets

tapper: $(PSXLIB_OBJS) $(TAPPER_OBJS)
	$(CC) $(CFLAGS) $(param) $(OTHERFLAGS) -I$(PSXLIB_HEADERS)\. -Xo$$80100000 $(PSXLIB_OBJS) $(TAPPER_OBJS) MMGMNEW.OBJ pxfinder.obj main.c -o MAIN.cpe,MAIN.sym,MAIN.map
	@echo ======= COPYING DATA =======
#cp2ex /CE MAIN.cpe
	cpe2psx MAIN.cpe
	copy MAIN.PSX C:\psyq\tools\makecd\ISOROOT\MAIN.PSX
	copy data\bg.TIM C:\psyq\tools\makecd\ISOROOT\data\bg.TIM
	copy data\beer_st.tim C:\psyq\tools\makecd\ISOROOT\data\beer_st.tim
	copy data\beer_sh.tim C:\psyq\tools\makecd\ISOROOT\data\beer_sh.tim
	copy data\tip.tim C:\psyq\tools\makecd\ISOROOT\data\tip.tim
	copy data\d_sheet.tim C:\psyq\tools\makecd\ISOROOT\data\d_sheet.tim
	copy data\d_drink.tim C:\psyq\tools\makecd\ISOROOT\data\d_drink.tim
	copy data\BARMAN.TIM C:\psyq\tools\makecd\ISOROOT\data\BARMAN.TIM
	copy data\menu.TIM C:\psyq\tools\makecd\ISOROOT\data\menu.TIM
	copy data\FONT8X8.TIM C:\psyq\tools\makecd\ISOROOT\data\FONT8X8.TIM
	copy data\pad.TIM C:\psyq\tools\makecd\ISOROOT\data\pad.TIM

clean_psxlib_obj:
	del $(PSXLIB_OBJS)

clean:
	del $(TAPPER_OBJS)
	del *.cpe
	del *.sym
	del *.map
#del *.exe
	del *.psx