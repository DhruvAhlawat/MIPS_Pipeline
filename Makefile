all: run5 run79

./FiveStage/5stage.exe: ./FiveStage/5stage.cpp 
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I .  .\FiveStage\5stage.cpp -o .\FiveStage\5stage

./FiveStage/5stage_bypass.exe: ./FiveStage/5stage_bypass.cpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I .  .\FiveStage\5stage_bypass.cpp -o .\FiveStage\5stage_bypass

bypass: ./FiveStage/5stage_bypass.exe ./FiveStage/5stage_bypass.cpp
	./FiveStage/5stage_bypass.exe "sample.asm"


run5: ./FiveStage/5stage.exe ./FiveStage/5stage.cpp
	./FiveStage/5stage.exe "sample.asm"


./Seven9Stage/79stage.exe: ./Seven9Stage/79stage.cpp MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I . ./Seven9Stage/79stage.cpp  -o .\Seven9Stage\79stage

run79: ./Seven9Stage/79stage.exe ./Seven9Stage/79stage.cpp MIPS_Processor.hpp
	./Seven9Stage/79stage.exe "sample.asm"

./Original/forwarding.exe: ./Original/forwarding.cpp MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 ./Original/sample.cpp ./Original/forwarding.cpp  -o .\Original\forwarding



forwardRun: ./Original/forwarding.exe ./Original/forwarding.cpp MIPS_Processor.hpp
	./Original/forwarding.exe "sample.asm"

clean:
	rm ./FiveStage/5stage.exe ./Seven9Stage/79stage.exe