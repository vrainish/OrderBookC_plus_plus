SHELL = /usr/bin/tcsh
CC = g++-4.7
CFLAGS =  -O3 -std=gnu++0x -I.
CFLAGS_TS =  -DSIMPLE -O3 -std=gnu++0x -I.
SOURCES = main.cpp Pricer.cpp Line.cpp Tags.cpp Chunks.cpp
SOURCES1 = main.cpp Pricer.cpp Line.cpp TagsSimple.cpp ChunksSimple.cpp
	
test:
	$(CC) $(CFLAGS) -o pricer $(SOURCES)
	./pricer 200 < pricer.in > yt.200
	diff yt.200 out/pricer.out.200
	rm yt.200
	./pricer 1 < pricer.in > yt.1
	diff yt.1 out/pricer.out.1
	rm yt.1
	./pricer 10000 < pricer.in > yt.10000
	diff yt.10000 out/pricer.out.10000
	rm yt.10000

test_ts:
	$(CC) $(CFLAGS_TS) -o pricer_ts $(SOURCES1)
	./pricer_ts 200 < pricer.in > yt.200
	diff yt.200 out/pricer.out.200
	rm yt.200
	./pricer_ts 1 < pricer.in > yt.1
	diff yt.1 out/pricer.out.1
	rm yt.1
	./pricer_ts 10000 < pricer.in > yt.10000
	diff yt.10000 out/pricer.out.10000
	rm yt.10000

ftest:
	$(CC) $(CFLAGS) -o pricer_f $(SOURCES)
	
	@echo "\nPricer 200"
	@echo -n "       time ./pricer_f 200 < pricer.in > out.200\n"
	@time ./pricer_f 200 < pricer.in > out.200
	@time ./pricer_f 200 < pricer.in > out.200
	@time ./pricer_f 200 < pricer.in > out.200
	@echo -n "       time ./pricer_f 200 < pricer.in > /dev/null\n"
	@time ./pricer_f 200 < pricer.in > /dev/null
	@time ./pricer_f 200 < pricer.in > /dev/null
	@time ./pricer_f 200 < pricer.in > /dev/null
	@diff out.200 out/pricer.out.200; rm out.200
	
	@echo "\nPricer 1"
	@echo -n "       time ./pricer_f 1 < pricer.in > out.1\n"
	@time ./pricer_f 1 < pricer.in > out.1
	@time ./pricer_f 1 < pricer.in > out.1
	@time ./pricer_f 1 < pricer.in > out.1
	@echo -n "       time ./pricer_f 1 < pricer.in > /dev/null\n"
	@time ./pricer_f 1 < pricer.in > /dev/null
	@time ./pricer_f 1 < pricer.in > /dev/null
	@time ./pricer_f 1 < pricer.in > /dev/null
	@diff out.1 out/pricer.out.1; rm out.1
	
	@echo "\nPricer 10000"
	@echo -n "       time ./pricer_f 10000 < pricer.in > out.10000\n"
	@time ./pricer_f 10000 < pricer.in > out.10000
	@time ./pricer_f 10000 < pricer.in > out.10000
	@time ./pricer_f 10000 < pricer.in > out.10000
	@echo -n "       time ./pricer_f 10000 < pricer.in > /dev/null\n"
	@time ./pricer_f 10000 < pricer.in > /dev/null
	@time ./pricer_f 10000 < pricer.in > /dev/null
	@time ./pricer_f 10000 < pricer.in > /dev/null
	@diff out.10000 out/pricer.out.10000; rm out.10000
	
	@echo "\n"

ftest_ts:
	$(CC) $(CFLAGS) -DSIMPLE -o pricer_f_ts $(SOURCES1)
	
	@echo "\nPricer 200"
	@echo -n "       time ./pricer_f_ts 200 < pricer.in > out.200\n"
	@time ./pricer_f_ts 200 < pricer.in > out.200
	@time ./pricer_f_ts 200 < pricer.in > out.200
	@time ./pricer_f_ts 200 < pricer.in > out.200
	@echo -n "       time ./pricer_f_ts 200 < pricer.in > /dev/null\n"
	@time ./pricer_f_ts 200 < pricer.in > /dev/null
	@time ./pricer_f_ts 200 < pricer.in > /dev/null
	@time ./pricer_f_ts 200 < pricer.in > /dev/null
	@diff out.200 out/pricer.out.200; rm out.200
	
	@echo "\nPricer 1"
	@echo -n "       time ./pricer_f_ts 1 < pricer.in > out.1\n"
	@time ./pricer_f_ts 1 < pricer.in > out.1
	@time ./pricer_f_ts 1 < pricer.in > out.1
	@time ./pricer_f_ts 1 < pricer.in > out.1
	@echo -n "       time ./pricer_f_ts 1 < pricer.in > /dev/null\n"
	@time ./pricer_f_ts 1 < pricer.in > /dev/null
	@time ./pricer_f_ts 1 < pricer.in > /dev/null
	@time ./pricer_f_ts 1 < pricer.in > /dev/null
	@diff out.1 out/pricer.out.1; rm out.1
	
	@echo "\nPricer 10000"
	@echo -n "       time ./pricer_f_ts 10000 < pricer.in > out.10000\n"
	@time ./pricer_f_ts 10000 < pricer.in > out.10000
	@time ./pricer_f_ts 10000 < pricer.in > out.10000
	@time ./pricer_f_ts 10000 < pricer.in > out.10000
	@echo -n "       time ./pricer_f_ts 10000 < pricer.in > /dev/null\n"
	@time ./pricer_f_ts 10000 < pricer.in > /dev/null
	@time ./pricer_f_ts 10000 < pricer.in > /dev/null
	@time ./pricer_f_ts 10000 < pricer.in > /dev/null
	@diff out.10000 out/pricer.out.10000; rm out.10000
	
	@echo "\n"


compile:
	$(CC) $(CFLAGS) -o pricer $(SOURCES)

compile_ts:
	$(CC) $(CFLAGS_TS) -o pricer_ts $(SOURCES1)
