<<<<<<< HEAD
=======
/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
 * 
 */

>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
#include <map>
// #include<trial.cpp>

using namespace std;

//hello
struct MIPS_Architecture
{
	public:
	int outputFormat = 0; //output format = 0 is the output format we used for debugging, it shows what each stage is doing at every cycle, and which PC is being executed
	//in each stage. output format = 1 is the output format we used for the final submission, it shows the value of each register at every cycle.

	int registers[32] = {0}, PCcurr = 0, PCnext = 0;
	//std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
=======

using namespace std;


struct MIPS_Architecture
{
	public:
	int registers[32] = {0}, PCcurr = 0, PCnext = 1;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
		//instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};
=======
		instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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

<<<<<<< HEAD
=======

	


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

>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
	//hello
=======

>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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

<<<<<<< HEAD
	pair<int,string> decodeAddress(string addr)
	{    //8($s0)
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

=======
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
				{	
					return -3;
				}
					
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 != 0 || address < (4 * commands.size()) || address >= MAX)
				{	
					return -3;
				}
					
=======
					return -3;
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
														   { return (bool)isalnum(c); });
			   //instructions.find(str) != instructions.end(); //shouold probably be != (WE CHANGED THIS)
=======
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end(); //shouold probably be !=
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
	}

	// checks if the register is a valid one
	inline bool checkRegister(std::string r)
	{
<<<<<<< HEAD
		
=======
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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
<<<<<<< HEAD
		if(outputFormat == 0)
		{
			std::cout << "\nFollowing are the non-zero data values:\n";
			for (int i = 0; i < MAX / 4; ++i)
				if (data[i] != 0)
					std::cout << 4 * i << '-' << 4 * i + 3 << ": " << data[i] << '\n'
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
	}
	int instructionNumber(string s)
	{
		if(s == "add" || s == "and" || s == "sub" || s == "mul" || s == "or" || s == "slt")
			return 0;
		else if(s == "addi" || s == "andi" || s == "ori" || s == "srl" || s == "sll")
			return 1;
		else if(s == "lw" || s == "sw") //I type₹
			return 2;
		return 3; //branch/jump type instructions
	}
=======
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

>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
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

<<<<<<< HEAD

=======
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}

<<<<<<< HEAD
	


	//the execution of commands is left to the pipeline that is using this architecture
=======
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
			currentCommand = {"","$t0","$t0","$t0"};
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
			if(arch->PCcurr >= arch->commands.size())
				return; //since we must be done with all the commands at this point
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
			// type does not get updated, then we won't run any new commands
			
		}

	};

	struct ID
	{
		public:
		MIPS_Architecture *arch;
		IFID* L2;
		IDEX *L3;
		string r[3] = {""}; //r[0] is the written to register, r[1] and r[2] are the using registers, they can be null
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
		MIPS_Architecture *arch; IDEX *L3; //The L3 Latch
		string iType = "";
		vector<int> dataValues; 
		int result = 0;
		string addr; //possible address that we can calculate using LocateAddress
		string r1; //register to be written into, this will not be used in this step but passed forward till the WriteBack stage where it will be written into
		//now we decode the instruction from the instructions map
		EX(MIPS_Architecture *architecture, IDEX *l3)
		{
			arch = architecture; L3 = l3; //the latch reference and architecture reference is stored at initialization
			dataValues = vector<int>(2,0);
		}

		void run()
		{
			dataValues = L3->curData; iType = L3->curInstructionType; 
			addr = L3->curAddr; r1 = L3->curWriteReg; 
			result = calc(); 
			cout << result << " ";
		}

		int calc()
		{
			if(iType == "") return -1;
			if(iType == "add" || iType == "addi")
				return dataValues[0] + dataValues[1];
			else if(iType == "sub")
				return dataValues[0] - dataValues[1];
			else if(iType == "mult")
				return dataValues[0] * dataValues[1];
			else if(iType == "and" || iType == "andi")
				return (dataValues[0] & dataValues[1]);
			else if(iType == "or" || iType == "ori")
				return (dataValues[0] | dataValues[1]);
			else if(iType == "srl")
				return (dataValues[0] >> dataValues[1]);
			else if(iType == "sll")
				return (dataValues[0] << dataValues[1]);
			else if(iType == "slt")
				return (dataValues[0] < dataValues[1]); 
			else  											//because otherwise it must be a lw or sw instruction
				return arch->locateAddress(addr);
		}

	};

	struct EXDM
	{
		string curAddr, curReg, nextAddr, nextReg;
		int curDataIn, nextDataIn;
		bool curMemWrite, nextMemWrite = true;

		void Update()
		{
			curAddr = nextAddr; curReg = nextReg;
			curMemWrite = nextMemWrite; curDataIn = nextDataIn;

			nextAddr = ""; nextReg = "";
		}
	};

	struct DM
	{
		public:
		MIPS_Architecture *arch; EXDM *L4; //the references to architecture and the L4 Latch 
		string addr;	//when addr is null, then we assume no DM stage to be applied
		string reg;
		bool memWrite;  //when memWrite is true, then we write from register into the memory
						//when memWrite is false. then we read from memory into register, so it is passed onto the WB stage to do that
						
		int dataIn;
		DM(MIPS_Architecture *architecture, EXDM *exdm)
		{
			arch = architecture; L4 = exdm; 
		}
		
		void run()
		{
			addr = L4->curAddr; reg = L4->curReg; memWrite = L4->curMemWrite; dataIn = L4->curDataIn;
			//updated all the values using the latch L4
			

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
		IDEX L3;
		L2.currentCommand = {"","","",""};
		IF fetch(this, &L2); //fetch is the IF stage
		ID Decode(this,&L2, &L3); //Decode is the ID stage
		EX ALU(this, &L3);
		while(PCcurr < commands.size()+3)
		{
			fetch.run();
			Decode.run();
			ALU.run();

			L2.Update(); L3.Update(); //updated the intermittent registers/Flipflops
			clockCycles++; 
			++commandCount[PCcurr];
			PCcurr = PCnext;
			PCnext++;
			cout << endl << " at clockCycles " << clockCycles << endl;
		}
		handleExit(SUCCESS, clockCycles);

	}
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9

	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
<<<<<<< HEAD
		if(outputFormat == 0) 
			std::cout << "Cycle number: " << clockCycle << '\n';
				  
=======
		std::cout << "Cycle number: " << clockCycle << '\n'
				  << std::hex;
>>>>>>> a8e2144f3b27dfcad3cdcac6a0bd92050e3fa4f9
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}

};

#endif