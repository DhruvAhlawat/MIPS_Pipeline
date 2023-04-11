all: 5stage

5stage: ./FiveStage/5stage.cpp MIPS_Processor.hpp
	g++ -I C:/Users/tripl/Documents/boost_1_62_0/boost_1_62_0 -I .  .\FiveStage\5stage.cpp -o .\FiveStage\5stage
	
	
clean:
	rm ./FiveStage/5stage.exe