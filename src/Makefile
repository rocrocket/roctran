all: rocsend rocreceive
#which compiler
CC = cc
#what libs
LIBLIST = 
APPENDIX = 

#rocsend
rocsend: rocsend.c
	$(CC) -o rocsend rocsend.c

#rocreceive
rocreceive: rocreceive.c
	$(CC) -o rocreceive rocreceive.c

#clean
clean:
	-rm -f rocsend rocreceive
