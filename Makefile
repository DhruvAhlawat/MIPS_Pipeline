all: run5 run79

5stageFinal: ./5stage.cpp ./MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I .  .\FiveStage\5stage.cpp -o .\FiveStage\5stage

5stage_bypassFinal: ./5stage_bypass.cpp ./MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I .  .\FiveStage\5stage_bypass.cpp -o .\FiveStage\5stage_bypass

bypass: ./5stage_bypassFinal ./5stage_bypass.cpp 
	./FiveStage/5stage_bypass.exe "input.asm"


run5: ./5stageFinal ./5stage.cpp
	./FiveStage/5stage.exe "input.asm"


79stageFinal: ./Seven9Stage/79stage.cpp MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I . .\79stage.cpp  -o .\79stageFinal

run79: ./79stageFinal ./79stage.cpp 
	./79stageFinal "input.asm"

./Original/forwarding.exe: ./Original/forwarding.cpp MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 ./Original/sample.cpp ./Original/forwarding.cpp  -o .\Original\forwarding


compile: 
	g++ -I . ./5stage.cpp -o ./5stageFinal
	g++ -I . ./79stage.cpp -o ./79stageFinal
	g++ -I . ./5stage_bypass.cpp -o ./5stage_bypassFinal

run_5stage: 
	./5stageFinal "input.asm"

run_5stage_bypass:
	./5stage_bypassFinal "input.asm"

run_79stage:
	./79stageFinal "input.asm"

clean:
	rm ./5stageFinal ./5stage_bypassFinal ./79stageFinal
