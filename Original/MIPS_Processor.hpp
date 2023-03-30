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

using namespace std;


struct MIPS_Architecture
{
	public:
	int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
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
		instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

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


	


	// perform add operation
	int add(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a + b; });
	}

	// perform subtraction operation
	int sub(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a - b; });
	}

	// perform multiplication operation
	int mul(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a * b; });
	}

	// perform the binary operation
	int op(std::string r1, std::string r2, std::string r3, std::function<int(int, int)> operation)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = operation(registers[registerMap[r2]], registers[registerMap[r3]]);
		PCnext = PCcurr + 1;
		return 0;
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
					return -3;
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
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
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end(); //shouold probably be !=
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

	// execute the commands sequentially (no pipelining)
	void executeCommandsUnpipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockCycles = 0;
		while (PCcurr < commands.size())
		{
			++clockCycles;
			std::vector<std::string> &command = commands[PCcurr];
			if (instructions.find(command[0]) == instructions.end())
			{
				handleExit(SYNTAX_ERROR, clockCycles);
				return;
			}
			exit_code ret = (exit_code) instructions[command[0]](*this, command[1], command[2], command[3]);
			if (ret != SUCCESS)
			{
				handleExit(ret, clockCycles);
				return;
			}
			++commandCount[PCcurr];
			PCcurr = PCnext;
			printRegisters(clockCycles);
		}
		handleExit(SUCCESS, clockCycles);
	}

	struct IFID //basically the L2 flipflop, used to transfer values between IF and ID stage
	{
		vector<string> currentCommand = {};
		vector<string> nextCommand = {};
		IFID()
		{
			currentCommand = {"","$t0","$t0","$t0","$t0"};
			nextCommand = currentCommand;
		}
		void Update()
		{
			currentCommand = nextCommand; 
			nextCommand = {};
		}
	};

	struct IF
	{
		public:
		MIPS_Architecture *arch;
		IFID *L2; //L2 flipflop or register
		int address;
		vector<string> CurCommand; //the current command after reading the address
		
		IF(MIPS_Architecture *architecture, IFID *l2)
		{
			arch = architecture; L2 = l2;
		}
		void run()
		{
			address = arch->PCcurr;
			CurCommand = arch->commands[address]; //updates to this address
			L2->nextCommand = CurCommand; //updates the value in the L2 at the same time, but for the next time
		}
	};

	int instructionNumber(string s)
	{
		if(s == "add" || s == "and" || s == "sub" || s == "mult" || s == "or" || s == "slt")
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
		string curAddr = "", nextAddr = "";
		void Update()
		{
			//on getting the updated values, we can run the code
			curData = nextData; curAddr = nextAddr;
			curInstructionType = nextInstructionType; curWriteReg = nextWriteReg;
			
			nextInstructionType = ""; //this would ensure that if instruction
			// type does not get updated, then we won't run any new commands in the case of stall
			
		}
        
	};

	struct ID
	{
		public:
		MIPS_Architecture *arch;
		IFID* L2;
		IDEX *L3;
		string r[3] = {0}; //r[0] is the written to register, r[1] and r[2] are the using registers, they can be null
		vector<int>dataValues = vector<int>(2,0); //to be passed onto the EX stage for computation
		string instructionType = "";
		string addr;
		vector<string> curCommand;
		ID(MIPS_Architecture *architecture, IFID *ifid, IDEX *idex)
		{
			arch = architecture;
			L2 = ifid;
			L3 = idex;
		}
		void run()
		{
			curCommand = L2->currentCommand; //we get the command from the L2 flipflop between IF and ID
			//on the basis of the commands we got, we can assign further
			if(curCommand.size() == 0) return;
			else if(curCommand[0] == "") return;
			instructionType = curCommand[0];
			for (int i = 1; i < 4 && i < curCommand.size(); i++)
			{
				if(curCommand[i] != "") 
				r[i-1] = curCommand[i];
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
			else //for lw and sw to be written LATER
			{
				addr = r[1]; //r[2] is just blank in this case
			}
			UpdateL3();
		}
		
		void UpdateL3()
		{
			//on getting the updated values, we can run the code
			L3->nextData = dataValues;
			L3->nextInstructionType = instructionType;
			L3->nextWriteReg = r[0];
			L3->nextAddr = addr;
		}
	};

	struct EX
	{
		public:
		string instructionType;
		int data1, data2; 
		int output;
		string r1; //register to be written into
		//now we decode the instruction from the instructions map
		EX(string InstructionType, string R1, int Data1, int Data2)
		{
			instructionType = InstructionType;
			r1 = R1;
			data1 = Data1;
			data2 = Data2;

		}

		void run()
		{
			
		}

	};
    
	struct DMWB{
            string currRegister = "";
			string nextRegister = "";
			int curr_data;
            int next_data;
			void Update(){
				 currRegister = nextRegister;    //on getting the updated value we can run the code
				 nextRegister = "";
				 curr_data = next_data;
				 next_data = 0;
			}
	};


    struct WB{
          public:
		  string r2;
		  int new_data;
		  MIPS_Architecture *newarch;
		  DMWB* L5;
		  WB(MIPS_Architecture *a,DMWB *dmwb){
             newarch = a;
			 L5 = dmwb;
		  }
        void run(){
             r2 = L5->currRegister;
			 new_data  = L5->curr_data;
			 if(r2!="")
			 newarch->registers[newarch->registerMap[r2]] = new_data;
		}
    };


	void ExecutePipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		} //memory error

		int clockCycles = 0;
		//first we instantiate the stages
		IFID L2;
		IDEX L3 = IDEX();
		IF fetch(this, &L2); //fetch is the IF stage
		ID Decode(this,&L2, &L3); //Decode is the ID stage

		while(PCcurr < commands.size())
		{
			fetch.run();
		// 	//Decode.run();

		// 	//L2.Update(); L3.Update(); //updated the intermittent registers/Flipflops
		// 	clockCycles++; 
		// 	++commandCount[PCcurr];
			PCcurr = PCnext;
			PCnext++;
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