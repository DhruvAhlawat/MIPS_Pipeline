#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__

#include<trial.cpp>
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
	int outputFormat = 1; //output format = 0 is the output format we used for debugging, it shows what each stage is doing at every cycle, and which PC is being executed
	//in each stage. output format = 1 is the output format we used for the final submission, it shows the value of each register at every cycle.

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

	void ExecutePipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		} //memory error

		//registers[registerMap["$sp"]] = (4 * commands.size()); //initializes position of sp. assumes that all the commands are also stored in data and so sp needs to be here
		//the above is optional, but since none of the testcases utilize it, it has been commented out
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

			Decode.run(); //Second Half Cycle. Our implementation requires Decode to run before fetch but what it essentially means in hardware is 
							//that the fetch stage is running in parallel with the decode stage and for stall cases, decode is informing the next IF stage to stall
							//if ID is stalling. This is done through the latch L2, which both IF and ID can access
			fetch.run();
			ALU.run();
			DataMemory.run();
			
			L2.Update(); L3.Update(); L4.Update(); L5.Update(); //updated the intermittent latches
			clockCycles++;
			printRegisters(clockCycles);
			if(DataMemory.memWrite == 1)
			{
				cout << 1 << " " << DataMemory.dataIn/4 << " " << DataMemory.swData;
			}
			else
			{
				cout << 0;
			}
			if(outputFormat == 0) 
			{	
				cout << " dataHazards are : ";
				for(auto i: DataHazards)
				{	
					cout << i.first << " " << i.second << ", ";
				}
			}
			
			HazardUpdate(); //updating the hazards

			//cout << endl << " at clockCycles " << clockCycles << endl;
			cout << endl;
		}
		handleExit(SUCCESS, clockCycles);

	}

	// print the register data in hexadecimal
	void printRegisters(int clockCycle)
	{
		if(outputFormat == 0) 
			std::cout << "Cycle number: " << clockCycle << '\n';
				  
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << std::dec << '\n';
	}

};

#endif