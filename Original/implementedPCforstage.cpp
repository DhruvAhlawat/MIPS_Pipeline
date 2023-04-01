/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
 * 
 */

#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <map>

using namespace std;


struct MIPS_Architecture
{
	public:
	int registers[32] = {0}, PCcurr = 0, PCnext = 0;
	//std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	std::vector<std::vector<std::string>> commands;
	std::vector<int> commandCount;
	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};

	// constructor to initialise the instruction set
	MIPS_Architecture(std::ifstream &file)
	{
		//instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

		for (int i = 0; i < 32; ++i)
			registerMap["$" + std::to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + std::to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}

	// perform the beq operation
	int beq(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a == b; });
	}

	// perform the bne operation
	int bne(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a != b; });
	}

	// implements beq and bne by taking the comparator
	int bOP(std::string r1, std::string r2, std::string label, std::function<bool(int, int)> comp)
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		if (!checkRegisters({r1, r2}))
			return 1;
		PCnext = comp(registers[registerMap[r1]], registers[registerMap[r2]]) ? address[label] : PCcurr + 1;
		return 0;
	}

	// implements slt operation
	int slt(std::string r1, std::string r2, std::string r3)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = registers[registerMap[r2]] < registers[registerMap[r3]];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform the jump operation
	int j(std::string label, std::string unused1 = "", std::string unused2 = "")
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		PCnext = address[label];
		return 0;
	}

	// perform load word operation
	int lw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		registers[registerMap[r]] = data[address];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform store word operation
	int sw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r))
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		data[address] = registers[registerMap[r]];
		PCnext = PCcurr + 1;
		return 0;
	}

	pair<int,string> decodeAddress(string addr)
	{
		if(addr.back() == ')')
		{
			try
			{
				int lparen = addr.find('('), offset = stoi(lparen == 0 ? "0" : addr.substr(0, lparen));
				std::string reg = addr.substr(lparen + 1);
				reg.pop_back();
				return {offset, reg};
			}
			catch (std::exception &e)
			{
				return {-4,""};
			}
		}
		try
		{
			int address = stoi(addr);
			if (address % 4 || address >= MAX)
				return {-3,"$0"};
			return {address/4, "$0"};
		}
		catch (std::exception &e)
		{
			return {-4, "$0"};
		}
	}

	int locateAddress(std::string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
				{	
					return -3;
				}
					
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 != 0 || address < (4 * commands.size()) || address >= MAX)
				{	
					return -3;
				}
					
				return address / 4;
			}
			catch (std::exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (std::exception &e)
		{
			return -4;
		}
	}

	// perform add immediate operation
	int addi(std::string r1, std::string r2, std::string num)
	{
		if (!checkRegisters({r1, r2}) || registerMap[r1] == 0)
			return 1;
		try
		{
			registers[registerMap[r1]] = registers[registerMap[r2]] + stoi(num);
			PCnext = PCcurr + 1;
			return 0;
		}
		catch (std::exception &e)
		{
			return 4;
		}
	}

	// checks if label is valid
	inline bool checkLabel(std::string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); });
			   //instructions.find(str) != instructions.end(); //shouold probably be != (WE CHANGED THIS)
	}

	// checks if the register is a valid one
	inline bool checkRegister(std::string r)
	{
		
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the registers are valid or not
	bool checkRegisters(std::vector<std::string> regs)
	{
		return std::all_of(regs.begin(), regs.end(), [&](std::string r)
						   { return checkRegister(r); });
	}

	/*
		handle all exit codes:
		0: correct execution
		1: register provided is incorrect
		2: invalid label
		3: unaligned or invalid address
		4: syntax error
		5: commands exceed memory limit
	*/
	void handleExit(exit_code code, int cycleCount)
	{
		std::cout << '\n';
		switch (code)
		{
		case 1:
			std::cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			std::cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			std::cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			std::cerr << "Syntax error encountered\n";
			break;
		case 5:
			std::cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			std::cerr << "Error encountered at:\n";
			for (auto &s : commands[PCcurr])
				std::cerr << s << ' ';
			std::cerr << '\n';
		}
		std::cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				std::cout << 4 * i << '-' << 4 * i + 3 << std::hex << ": " << data[i] << '\n'
						  << std::dec;
		std::cout << "\nTotal number of cycles: " << cycleCount << '\n';
		std::cout << "Count of instructions executed:\n";
		for (int i = 0; i < (int)commands.size(); ++i)
		{
			std::cout << commandCount[i] << " times:\t";
			for (auto &s : commands[i])
				std::cout << s << ' ';
			std::cout << '\n';
		}
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(std::string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		std::vector<std::string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			std::string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = std::vector<std::string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != std::string::npos)
		{
			int idx = command[0].find(':');
			std::string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);
	}


	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}

	map<string,int> DataHazards;


	struct IFID //basically the L2 latch, used to transfer values between IF and ID stage
	{
		vector<string> currentCommand = {};
		vector<string> nextCommand = {};
		bool IDisStalling = false;
		bool curIsWorking = true, nextIsWorking = true; 
		int PC;
		int PCrun;
		IFID()
		{
			currentCommand = {};
			nextCommand = currentCommand;
			PCrun = PC;
		}
		void Update()
		{
			currentCommand = nextCommand; 
			// nextCommand = {}; 
			curIsWorking = nextIsWorking;
			PC = PCrun;
		}
	};

	struct IF
	{
		public:
		MIPS_Architecture *arch;
		bool isWorking = true;
		IFID *L2; //L2 latch
		int address;
		vector<string> CurCommand; //the current command after reading the address
		
		IF(MIPS_Architecture *architecture, IFID *l2)
		{
			arch = architecture; L2 = l2; isWorking = true;
		}
		void run()
		{
			cout << " |IF|=> ";
			if(arch->PCnext >= arch->commands.size())
			{
				L2->nextCommand = {};
				isWorking = false;
				L2->nextIsWorking = false;
				return; //since we must be done with all the commands at this point
			} 
			if(L2->IDisStalling == false)
			{
				arch->PCcurr = arch->PCnext;
				arch->PCnext++;
				++arch->commandCount[arch->PCcurr];
				address = arch->PCcurr;
				cout << "Fetched Command No. " << arch->PCcurr;
				L2->PCrun = arch->PCcurr;
				CurCommand = arch->commands[address]; //updates to this address
				L2->nextCommand = CurCommand; //updates the value in the L2 at the same time, but for the next time
			}
			else
			{
				cout << "** ";
			}
		}
	};

	int instructionNumber(string s)
	{
		if(s == "add" || s == "and" || s == "sub" || s == "mul" || s == "or" || s == "slt")
			return 0;
		else if(s == "addi" || s == "andi" || s == "ori" || s == "srl" || s == "sll")
			return 1;
		else if(s == "lw" || s == "sw")
			return 2;
		return 3;
	}

	struct IDEX //the L3 register lying between ID and EX
	{
		vector<int> curData, nextData;
		string curWriteReg = "", nextWriteReg = "";
		string curInstructionType = "", nextInstructionType = "";
		bool IDisStalling = false;
		bool curIsWorking = true, nextIsWorking = true;
		int PC;
		int PCrun;
		void Update()
		{
			//on getting the updated values, we can run the code
			curData = nextData;
			curInstructionType = nextInstructionType; curWriteReg = nextWriteReg;
			curIsWorking = nextIsWorking;
			PC = PCrun;
			nextInstructionType = ""; //this would ensure that if instruction
			// type does not get updated, then we won't run any new commands
		}

	};

	struct ID
	{
		public:
		bool isWorking = true;
		bool isStalling = false;
		MIPS_Architecture *arch;
		IFID* L2;
		IDEX *L3;
		string r[3] = {""}; //r[0] is the written to register, r[1] and r[2] are the using registers, they can be null
		vector<int>dataValues = vector<int>(3,0); //to be passed onto the EX stage for computation. only 2 will be sent most of the time, but for sw instruction, the value of the register would be sent as the index=2 element
		string instructionType = "";
		string addr;
		vector<string> curCommand;
		int checkforPC;
		ID(MIPS_Architecture *architecture, IFID *ifid, IDEX *idex)
		{
			arch = architecture;
			L2 = ifid;
			L3 = idex;
		}
        void stall()
		{
			cout << "**";
			isStalling = true;	//then we should stall this stage right now.
			L2->IDisStalling = true;
			L3->nextInstructionType = ""; //sending null as instruction
			return;				//in the stall stage, we will not do any updated to the L3 latch, 
								//so the next values for the next stage will be the default blanks	
		}
		void run()
		{
			if(!isStalling) //if it is stalling, then we do not update the current command and isWorking status
			{
				curCommand = L2->currentCommand; //we get the command from the L2 flipflop between IF and ID
				isWorking = L2->curIsWorking; 
				checkforPC = L2->PC; 
				L3->PCrun = checkforPC;
			}
			if(isWorking == false)
			{
				L3->nextInstructionType = "";
				L3->nextIsWorking = false;
			}
			//on the basis of the commands we got, we can assign further
			cout << " |ID|=> ";
			
			if(curCommand.size() == 0)
				return;
			else if(curCommand[0] == "afterJump") //if the previous instruction was a jump, it will have been calculated by now so we can stop the stalling
			{
				L2->IDisStalling = false;
				isStalling = false; 
				return;
			}
			else if(curCommand[0] == "")
				return;
			instructionType = curCommand[0];
			for (int i = 1; i < 4 && i < curCommand.size(); i++)
			{
				r[i-1] = curCommand[i];
			}
			
			if(instructionType == "j") //then its a jump instruction, in which case we should jump to the address label, using the label
			{
				cout << "jumped to instruction number " << arch->address[curCommand[1]];
				arch->j(curCommand[1],"", ""); //and then we must introduce a stall after this stage so as to 
				//not let a wrong instruction go by.
				//the above code ensures that arch->PCnext has been updated correctly.
				cout << "PC= " << checkforPC;
				curCommand[0] = "afterJump";
				stall();
				return;
			}

			//Checking Dependencies first
			if((arch->DataHazards.count(r[1]) && arch->DataHazards[r[1]] < 5) ||
			 (arch->DataHazards.count(r[2]) && arch->DataHazards[r[2]] < 5) || 
			 ((instructionType == "beq" || instructionType == "bne") && arch->DataHazards.count(r[0]) && arch->DataHazards[r[0]] < 5))
			{
				stall(); 
				return;
			}
			else if(instructionType == "sw" && (arch->DataHazards.count(r[0]) && arch->DataHazards[r[0]] < 5))
			{
				//this is again a stall case, a rare one where the r[0] register's value needs to be used for sw 
				stall();
				return;
			}
			else 
			{	
				cout << " decoded " << instructionType << " ";
				arch->DataHazards.insert({r[0],2});
				L2->IDisStalling = false;
				isStalling = false; 
			}

			if(instructionType == "beq" || instructionType == "bne") //doing the entire BEQ and BNE process in ID step itself, while introducing a bubble in the pipeline where nothing gets done
			{
				bool isEqual = (arch->registers[arch->registerMap[r[0]]]  == arch->registers[arch->registerMap[r[1]]]);
				if((isEqual^(instructionType == "bne")))
				{
					cout << "branched to instruction number " << arch->address[r[2]];
					arch->j(r[2],"", ""); 
					cout << "PC= " << checkforPC;
					curCommand[0] = "afterJump";
					stall();
					return;
				}
				else
				{
					cout << "did not branch- bubbled ";
					L3->nextInstructionType = "";
					curCommand[0] = "afterJump";
					stall();
				}
				return;
			}

			int curInstruction = arch->instructionNumber(instructionType);
			if(curInstruction == 0) //then r2 and r3 need to be added/subtracted/whatever
			{
				dataValues[0] = arch->registers[arch->registerMap[r[1]]];
				dataValues[1] = arch->registers[arch->registerMap[r[2]]];
			}
			else if(curInstruction == 1)
			{
				dataValues[0] = arch->registers[arch->registerMap[r[1]]];
				dataValues[1] = stoi(r[2]); //since this in string form and not a register
			}
			else if(curInstruction == 2)//for lw and sw to be written LATER
			{
				pair<int,string> res = arch->decodeAddress(r[1]);    // we implemented a new function decodeAddress which correctly decodes as string and int 
				dataValues[0] = res.first;
				dataValues[1] = arch->registers[arch->registerMap[res.second]];	
				dataValues[2] = arch->registers[arch->registerMap[r[0]]];
				cout << " passed " << dataValues[2] << " for sw ";		
			}
			else
			{
				cout << "Jumped to " << curCommand[1];
			}
			if(!isStalling)
				cout << dataValues[0] << " and " << dataValues[1] << " "<<"PC="<< checkforPC;
			UpdateL3();
		}
		
		

		void UpdateL3()
		{
			//on getting the updated values, we can run the code
			L3->nextData = dataValues;
			L3->nextInstructionType = instructionType;
			L3->nextWriteReg = r[0];
			
		}
	};

	
	struct EXDM
	{
		string curReg, nextReg;
		int curSWdata, nextSWdata;
		int curDataIn, nextDataIn;
		int curMemWrite, nextMemWrite = 0;
		bool curIsWorking = true, nextIsWorking = true;
		int PC;
		int PCrun;
		void Update()
		{
			// curAddr = nextAddr; curReg = nextReg;
			curMemWrite = nextMemWrite; curDataIn = nextDataIn;
			curReg = nextReg; curIsWorking = nextIsWorking;
			PC = PCrun;
			nextMemWrite = -1; curSWdata = nextSWdata;
		}
	};

	void HazardUpdate()
	{
		auto i = DataHazards.begin();
		while(i != DataHazards.end())
		{
			if(++(i->second) >= 6)
			{
				auto t = i;
				i++; DataHazards.erase(t);
			}
			else{
				i++;
			}
		}
	}

	struct EX
	{	
		public:
		bool isWorking = true; int swData;
		MIPS_Architecture *arch; IDEX *L3; EXDM *L4;//The L3 and L4 Latchs
		string iType = "";
		vector<int> dataValues; 
		int result = 0;
		string r1; //register to be written into, this will not be used in this step but passed forward till the WriteBack stage where it will be written into
		//now we decode the instruction from the instructions map
		int checkforPC;
		EX(MIPS_Architecture *architecture, IDEX *l3, EXDM *l4)
		{
			arch = architecture; L3 = l3; L4 = l4; //the latch reference and architecture reference is stored at initialization
			dataValues = vector<int>(3,0);
		}

		void run()
		{	
			iType = L3->curInstructionType; 
			isWorking = L3->curIsWorking;
			cout << " |EX|=> ";
            checkforPC = L3->PC;
			L4->PCrun = checkforPC;
			if(!isWorking)
			{
				L4->curIsWorking = false;
			}
			if(iType == "")
			{
				L4->nextReg = ""; L4->nextDataIn = -1;
				return;
			}
			dataValues = L3->curData; 
			r1 = L3->curWriteReg; 
			
			result = calc(); 
			if(iType == "sw")
			{
				L4->nextSWdata = dataValues[2];
			}
			L4->nextReg = r1; L4->nextDataIn = result; 
			L4->nextMemWrite = (iType == "sw")? 1 : (iType == "lw") ? 0 : -1;
			if(iType!="sw" || iType !="lw"){
			cout << " did " << iType << " " << dataValues[0] << " " << dataValues[1] << " "<<"PC "<<checkforPC;}
			else{
				cout<<"address"<<dataValues[0] << "+" << dataValues[1] << "calculated"; 
			}
		}

		int calc()
		{
			if(iType == "") return -1;
			if(iType == "add" || iType == "addi" || iType == "lw" || iType == "sw")
				return dataValues[0] + dataValues[1];
			else if(iType == "sub")
				return dataValues[0] - dataValues[1];
			else if(iType == "mul")
				return dataValues[0] * dataValues[1];
			else if(iType == "and" || iType == "andi")
				return (dataValues[0] & dataValues[1]);
			else if(iType == "or" || iType == "ori")
				return (dataValues[0] | dataValues[1]);
			else if(iType == "srl")
				return (dataValues[0] >> dataValues[1]);
			else if(iType == "sll")
				return (dataValues[0] << dataValues[1]);
			else  //if slt
				return (dataValues[0] < dataValues[1]); 

		}

	};

	

	struct DMWB{
            string currRegister = "";
			string nextRegister = "";
			int curr_data;
            int next_data;
			bool curIsWorking = true, nextIsWorking = true;
			int PC;
			int PCrun;
			void Update(){
				 currRegister = nextRegister;    //on getting the updated value we can run the code
				 nextRegister = "";
				 curr_data = next_data;
				 PC = PCrun;
				 next_data = 0; curIsWorking = nextIsWorking;
			}
	};




	struct DM
	{
		public:
		bool isWorking = true;
		MIPS_Architecture *arch; EXDM *L4; DMWB *L5; //the references to architecture and the L4 Latch 
		string reg; 
		int checkforPc;
		int memWrite;  //when memWrite is 1, then we write from register into the memory
						//when memWrite is 0. then we read from memory into register, so it is passed onto the WB stage to do that
						//when it is -1, then we skip this stage and move onto the Writeback stage
		int swData;
		int dataIn; 	//dataIn is an address if memWrite is 1 or 0, otherwise its value will be directly stored onto the register in WB stage
		DM(MIPS_Architecture *architecture, EXDM *exdm, DMWB *dmwb)
		{
			arch = architecture; L4 = exdm; L5 = dmwb; //initialisation
		}

		void run()
		{
			isWorking = L4->curIsWorking;
            checkforPc = L4->PC;
			L5->PCrun = checkforPc;
			if(!isWorking)
			{
				L5->nextIsWorking = false;
			}
			reg = L4->curReg; memWrite = L4->curMemWrite; dataIn = L4->curDataIn;
			swData = L4->curSWdata;
			cout << " |DM|=> "; 
			//updated all the values using the latch L4

			if(reg == "")
			{
				L5->nextRegister = "";
				return; //nothing to do here
			}

			if(memWrite == 1)
			{
				//then we write into the memory

				arch->data[dataIn] = swData; //storing into the register what we decoded from a register file back in the ID stage
				cout << " sent val " << swData << " into memory at " << dataIn<< "PC="<<checkforPc;
				L5->next_data = -1; L5->nextRegister = ""; //since we dont need to write anything onto the register, the reg is passed as ""
			}
			else
			{
				//we must read from the memory into the register, so
				if(memWrite == 0)
				{
					dataIn = arch->data[dataIn]; 
					cout<< "reading value" << dataIn << "from Memory to  register" << reg<<"PC="<<checkforPc; 
				}
				//if memWrite is instead -1, then we simply pass on the value of dataIn directly.
				L5->nextRegister = reg;
				L5->next_data = dataIn; 
			}
		}

	};


    struct WB{
		public:
		bool isWorking = true;
		string r2;
		int checkForPC;
		int new_data;
		MIPS_Architecture *newarch;
		DMWB* L5;
		WB(MIPS_Architecture *a,DMWB *dmwb){
			newarch = a;
			L5 = dmwb;
		}
        void run(){
			isWorking = L5->curIsWorking;
            r2 = L5->currRegister;
			new_data  = L5->curr_data;
			checkForPC = L5->PC;
			cout << " |WB|=> ";

			if(r2 != "")
			{
				newarch->registers[newarch->registerMap[r2]] = new_data;
				cout << "wrote " << new_data << " into reg " << r2 << " "<<"currPC"<<L5->PC;
			}
			
		}
    };



	void ExecutePipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		} //memory error

		registers[registerMap["$sp"]] = (4 * commands.size()); //initializes position of sp. assumes that all the commands are also stored in data and so sp needs to be here
		int clockCycles = 0;
		//first we instantiate the stages
		IFID L2; //The Latches
		IDEX L3;
		EXDM L4;
		DMWB L5;
		IF fetch(this, &L2); //fetch is the IF stage
		ID Decode(this,&L2, &L3); //Decode is the ID stage
		EX ALU(this, &L3, &L4); //all are self explanatory actually
		DM DataMemory(this,&L4,&L5);
		WB WriteBack(this,&L5);

		while(WriteBack.isWorking)
		{
			WriteBack.run(); //First half Cycle
			Decode.run(); //Second Half Cycle, Decode running before IF so it can detect stalls and make IF stall
			fetch.run();
			ALU.run();
			DataMemory.run();
			 
		
			
			
			L2.Update(); L3.Update(); L4.Update(); L5.Update(); //updated the intermittent latches
			clockCycles++; 
			HazardUpdate(); //updating the hazards

			//cout << endl << " at clockCycles " << clockCycles << endl;
			cout << endl;
			printRegisters(clockCycles);
		}
		handleExit(SUCCESS, clockCycles);

	}

	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
		std::cout << "Cycle number: " << clockCycle << '\n'
				  << std::hex;
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}

};

#endif