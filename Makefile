export CPMDrive_D = ./
export CPMDefault = d:

CFLAGS  = +cpm -Wall -clib=8080 --list --c-code-in-asm -s -m -pragma-include:zpragma.inc 
LINKOP  = +cpm -create-app -clib=8080 -s -m -pragma-include:zpragma.inc
CC = zcc

all: te_ansi tecf

tecf: tecf.o
	$(CC) $(LINKOP) -otecf tecf.o

te_ansi: te_ansi.o
	$(CC) $(LINKOP) -ote te_ansi.o

clean:
	rm -f *.o *.lis *.sym TE.COM te  *.map *.c~ *.h~

lessclean:
	rm -f *.o *.lis *.sym te  *.map *.c~ *.h~

install:
	sudo cp TE.COM /var/www/html/TETEST.COM

patch:
	mv TE.COM te.com
	vcpm d:TECF.COM patch d:TE.COM d:TE_ANSI.CF
	mv te.com TE.COM
