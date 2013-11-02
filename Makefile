play_libs=`pkg-config --libs libavcodec libavformat libavutil libswscale` -lSDL2_image -lSDL2
cc=clang
all:
	$(cc) -oplay play.c $(play_libs)
clean:clean_tmp clean_exe
	
clean_tmp:
	rm *~
clean_exe:
	rm play

