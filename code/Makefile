# CC=g++ -march=native -w -O3
CC=g++ -O3 --std=c++0x
# CC=g++

file = file.o file.h
insert = insert.o insert.h
# common = common.h tool/debug.h tool/myIntA.h graph.h
common = common.h tool/debug.h tool/myIntAHash.h tool/myAHash.h tool/myPriQueue.h tool/myRandom.h graph.h
myG = myG.o myG.h
Comp = Comp.o Comp.h
backtrack = backtrack.o backtrack.h
DAG = DAG.o DAG.h

total: targetPCFR
	
checkAnswer.o: checkAnswer.cpp $(common) file.h myG.h backtrack.h
	$(CC) -rdynamic -c checkAnswer.cpp

targetPCFR: maxKGLayerDAG.o $(common) $(file) $(myG) $(insert) $(Comp) $(backtrack) $(DAG)
	$(CC)  -rdynamic -o PCFR.out maxKGLayerDAG.o file.o myG.o insert.o Comp.o backtrack.o DAG.o
	
maxKGLM: maxKGLayerDAG.o $(common) $(file) $(myG) $(insert) $(Comp) $(backtrack) $(DAG)
	$(CC)  -rdynamic -o maxKGLayerDAG.out maxKGLayerDAG.o file.o myG.o insert.o Comp.o backtrack.o DAG.o
	cp ./maxKGLayerDAG.out ../../prog/

maxKDebugDP: maxKDebugDP.o $(common) $(file) $(myG) $(insert) $(Comp) $(backtrack) $(DAG)
	$(CC)  -rdynamic -o maxKDebugDP.out maxKDebugDP.o file.o myG.o insert.o Comp.o backtrack.o DAG.o
	cp ./maxKDebugDP.out ../../prog/
	
maxKDebugDP.o: maxKDebugDP.cpp $(common) file.h myG.h Comp.h backtrack.h
	$(CC) -rdynamic -c maxKDebugDP.cpp

maxKGLayerDAG.o: maxKGLayerDAG.cpp $(common) file.h myG.h Comp.h backtrack.h
	$(CC) -rdynamic -c maxKGLayerDAG.cpp

maxKGLR: maxKGLayerRandom.o $(common) $(file) $(myG) $(insert) $(Comp) $(backtrack) $(DAG)
	$(CC)  -rdynamic -o maxKGLayerRandom.out maxKGLayerRandom.o file.o myG.o insert.o Comp.o backtrack.o DAG.o
	cp ./maxKGLayerRandom.out ../../prog/

maxKGLayerRandom.o: maxKGLayerRandom.cpp $(common) file.h myG.h Comp.h backtrack.h
	$(CC) -rdynamic -c maxKGLayerRandom.cpp

file.o: file.cpp $(common) file.h myG.h Comp.h
	$(CC) -rdynamic -c file.cpp

insert.o: insert.cpp $(common) insert.h myG.h
	$(CC) -rdynamic -c insert.cpp
	
myG.o: myG.cpp myG.h $(common)
	$(CC) -rdynamic -c myG.cpp
	
Comp.o: Comp.cpp Comp.h $(common) myG.h insert.h backtrack.h DAG.h
	$(CC) -rdynamic -c Comp.cpp
	
backtrack.o: backtrack.cpp backtrack.h $(common) myG.h insert.h
	$(CC) -rdynamic -c backtrack.cpp
	
DAG.o: DAG.cpp DAG.h $(common)
	$(CC) -rdynamic -c DAG.cpp
	
clean:
	rm *.o