OUT_EXE=dbms 
D_FLAG=#-DDEBUGX #this is very very detailed debug option,looks messy

#change the following boost library as necessary
I_FLAG=-I include
L_FLAG=lib/libboost_serialization.a

FILE = sql.tab.o lex.yy.o main.o parser.o query.o catalog.o table.o buffer_mng.o buffer.o file.o bplustree.o
build: $(FILE) 
	$(CXX) $^ -o $(OUT_EXE) -Wall $(I_FLAG) $(L_FLAG)
main.o: main.c sql.tab.h catalog.h query.h parser.h
	$(CXX) -c main.c $(I_FLAG)
parser.o: parser.c parser.h sql.tab.h sql.lex.h
	$(CC) -c parser.c 
query.o: query.c query.h catalog.h
	$(CXX) $(D_FLAG) -c query.c $(I_FLAG)
catalog.o: catalog.cpp table.h
	$(CXX) -c catalog.cpp $(I_FLAG)
table.o: table.cpp table.h parser.h
	$(CXX) -c table.cpp $(I_FLAG)
buffer_mng.o: buffer_mng.cpp buffer_mng.h
	$(CXX) $(D_FLAG) -c buffer_mng.cpp $(I_FLAG)
bplustree.o: bplustree.cpp bplustree.h file.h
	$(CXX) $(D_FLAG) -c bplustree.cpp
buffer.o: buffer.cpp buffer.h
	$(CXX) -c buffer.cpp
file.o: file.cpp file.h
	$(CXX) -c file.cpp

lex.yy.o: lex.yy.c sql.tab.h
	$(CC) -g -std=c99 -D_POSIX_SOURCE -c $<
sql.tab.c sql.tab.h: sql.y
	bison --defines sql.y
lex.yy.c: sql.lex
	flex --header-file=sql.lex.h sql.lex
clean: 
	rm -rf data *.o *.index dbms sql.lex.h lex.yy.c sql.tab.h sql.tab.c
clean_:
	rm *.o dbms sql.lex.h lex.yy.c sql.tab.h sql.tab.c
rebuild: clean build
	
