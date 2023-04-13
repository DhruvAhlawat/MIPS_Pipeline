#include<MIPS_Processor.hpp>
#include<map>
#include<string>
using namespace std;
map<string,int> DataHazards;

int stallNumber = 0;

struct IFID //basically the L2 latch, used to transfer values between IF and ID stage
{
	vector<string> currentCommand = {};
	vector<string> nextCommand = {};
	bool curIsWorking = true, nextIsWorking = true; 
	IFID()
	{
		currentCommand = {};
		nextCommand = currentCommand;
	}
	void Update()
	{
		currentCommand = nextCommand; 
		curIsWorking = nextIsWorking;
	}
};

struct IF0
{
	MIPS_Architecture *arch;
	IFID *LIF;
	IF0(MIPS_Architecture *mips, IFID *lif)
	{
		arch = mips;
		LIF = lif;
	}

	void run()
	{
		//checks if we are out of instructions
		if(arch->outputFormat == 0)
			cout << "|IF0|=>";

		//checks if we are supposed to stall
		if(stallNumber > 0)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		if(arch->PCnext >= arch->commands.size())
		{
			if(!arch->outputFormat)
				cout << "done";
			LIF->nextIsWorking = false;
			return;
		}
		arch->PCcurr = arch->PCnext; arch->PCnext++;
		if(arch->outputFormat==0)
			cout << "fetch0ed: " << arch->PCcurr;
		//else we will work
		//then we check if the current instruction is a branch
		LIF->nextCommand = arch->commands[arch->PCcurr];
		if(arch->commands[arch->PCcurr][0] == "beq" || arch->commands[arch->PCcurr][0] == "bne")
		{
			//then we need to stall the pipeline
			stallNumber = 1; //so the next IF instruction gets stalled
			//and pass the commands forward as well
		}
	}
};

struct IF1
{
	MIPS_Architecture *arch;
	IFID *LIF, *L2;
	IF1(MIPS_Architecture *mips, IFID *lif, IFID *l2)
	{
		arch = mips;
		LIF = lif;
		L2 = l2;
	}

	void run()
	{
		if(arch->outputFormat==0)
			cout << "|IF1|=>";

		if(stallNumber > 1)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		if(LIF->curIsWorking == false)
		{
			if(arch->outputFormat==0)
				cout << "done";
			L2->nextIsWorking = false;
			return;
		}
		//else we will work
		
		L2->nextCommand = LIF->currentCommand;
		if(LIF->currentCommand.size() == 0)
		{
			//then actually it hasnt been passed a command yet, so we just return
			return;
		}
		if(arch->outputFormat==0)
			cout << "fetched1 " << LIF->currentCommand[0];
		if(LIF->currentCommand[0] == "beq" || LIF->currentCommand[0] == "bne")
		{
			stallNumber = 2; //so the next IF1 instruction gets stalled as well.
		}
	} 
};

struct IDID //the  latch lying between the latch lying between ID0 and ID1
{
	// vector<int> curData, nextData;
	vector<string> curCommand, nextCommand;
	bool curIsWorking = true, nextIsWorking = true;
	void Update()
	{
		//on getting the updated values, we can run the code
		// curData = nextData;
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		//nextInstructionType = ""; //this would ensure that if instruction
		// type does not get updated, then we won't run any new commands
	}
};

struct ID0
{
	MIPS_Architecture *arch;
	IFID *L2; IDID *L3;
	//ID0 will be responsible for decoding the instruction
	ID0(MIPS_Architecture *mips, IFID *l2, IDID *l3)
	{
		arch = mips;
		L2 = l2;
		L3 = l3;
	}
	void run()
	{
		if(arch->outputFormat==0)
			cout << "|ID0|=>";
		if(stallNumber > 2)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		if(L2->curIsWorking == false)
		{
			if(arch->outputFormat==0)
				cout << "done";
			L3->nextIsWorking = false;
			return;
		}
		//else we will work
		//we will first check if the instruction is a branch, sent from IF1
		L3->nextCommand = L2->currentCommand;
		if(L2->currentCommand.size() == 0)
		{ 	//we haven't been passed a command yet, so we just return. This is basically a no-op
			return;
		}
		if(L2->currentCommand[0] == "beq" || L2->currentCommand[0] == "bne")
		{
			//then we need to stall the pipeline
			stallNumber = 3; //so the next ID0 instruction gets stalled as well.
			//and pass the commands forward as well
		}
	}
};

struct IDRR
{	public:
	vector<string> curCommand, nextCommand;
	bool curIsWorking = true, nextIsWorking = true;
	int curOffset = 0, nextOffset = 0;
	void Update()
	{
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		
	}
};

struct ID1
{
	MIPS_Architecture *arch;
	IDID *LID;
	IDRR *L4; 
	vector<string> curCommand;
	//ID0 will be responsible for decoding the instruction
	ID1(MIPS_Architecture *mips, IDID *lid, IDRR *l4)
	{
		arch = mips;
		LID = lid;
		L4 = L4;
	}
	
	void run()
	{
		if(arch->outputFormat==0)
			cout << "|ID1|=>";
		//first we check the stall condition
		if(stallNumber > 3)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		else
		{
			curCommand = LID->curCommand;
		}
		if(LID->curIsWorking == false)
		{
			if(arch->outputFormat==0)
				cout << "done";
			L4->nextIsWorking = false;
			return;
		}
		
		//else we will work
		//we will first check if the instruction is a branch, sent from IF1
		if(curCommand.size() == 0)
		{
			L4->nextCommand = curCommand; //passing a no-op
			return;
		}
		if (LID->curCommand[0] == "lw" || LID->curCommand[0] == "sw")
		{
			//then we need to do address calculation as well, so we parse the address first
			pair<int,string> val = arch->decodeAddress(curCommand[2]);	
			L4->nextOffset = val.first;
			curCommand[2] = val.second; //we replace the address with the register name

		}
		L4->nextCommand = curCommand;
		if(LID->curCommand[0] == "beq" || LID->curCommand[0] == "bne")
		{
			//then we need to stall the pipeline
			stallNumber = 4; //so the next ID1 instruction gets stalled as well.
			//and pass the commands forward as well	
		}
	}
};
struct RREX //the latch lying between RR and EX
{
	vector<int> curData, nextData;
	vector<string> curCommand,nextCommand;
	bool curIsWorking = true, nextIsWorking = true;
	void Update()
	{
		//on getting the updated values, we can run the code
		curData = nextData;
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		//nextInstructionType = ""; //this would ensure that if instruction
		// type does not get updated, then we won't run any new commands
	}
};


struct RR
{
	MIPS_Architecture *arch;
	IDRR *L4; RREX *L5a, *L5b;
	vector<int> regVal;
	vector<string> curCommand;
	//RR is responsible for reading the register values and passing them to EX for working
	RR(MIPS_Architecture *mips, IDRR *l4, RREX *l5a, RREX *l5b)
	{
		arch = mips;
		regVal = vector<int>(2,0);
		L4 = l4;
		L5a = l5a;
		L5b = l5b;
	}
	void run()
	{
		if(arch->outputFormat==0)
			cout << "|RR|=>";
		if(stallNumber > 4)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		else
		{
			curCommand = L4->curCommand;
		}
		if(L4->curIsWorking == false)
		{
			if(arch->outputFormat==0)
				cout << "done";
			L4->nextIsWorking = false;
			return;
		}
		//else we will work
		if(curCommand.size() == 0)
		{
			// L4->nextCommand = curCommand; //passing a no-op
			return;
		}
		regVal[0] = arch->registers[arch->registerMap[curCommand[2]]];
		if(curCommand[3] != "")
			regVal[1] = arch->registers[arch->registerMap[curCommand[3]]];
		else
			regVal[1] = 0;
		if(curCommand[0] == "beq" || curCommand[0] == "bne")
		{
			//then we need to stall the pipeline
			stallNumber = 5; //so the next RR instruction gets stalled as well.
			//and pass the commands forward as well	
		}
		else if(arch->instructionNumber(curCommand[0]) == 2)
		{
			//then we need to take the 9 stage pipeline path
			cout << " taken 9 staged path";
		}
		else
		{
			//otherwise we need to take the the 7 stage pipeline path
			cout << " taken 7 staged path";
		}
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

struct EXWB
{
	string curReg, nextReg;
	int curDataOut, nextDataOut;
	bool curIsWorking = true, nextIsWorking = true;
	void Update()
	{
		curReg = nextReg; curDataOut = nextDataOut;
		curIsWorking = nextIsWorking;
	}
};


struct EX
{	
	public:
	bool isWorking = true; int swData;
	MIPS_Architecture *arch; RREX *L5; EXDM *L6;
	string iType = "";
	vector<int> dataValues; 
	int result = 0;
	string r0; //register to be written into, this will not be used in this step but passed forward till the WriteBack stage where it will be written into
	//now we decode the instruction from the instructions map
	int checkforPC;
	EX(MIPS_Architecture *architecture, RREX *l5, EXDM *l6)
	{
		arch = architecture; L5 = l5; L6 = l6; //the latch reference and architecture reference is stored at initialization
		dataValues = vector<int>(3,0);
	}

	void run()
	{	
		if(stallNumber > 5)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		else
		{
			iType = L5->curCommand[0];
			dataValues[0] = L5->curData[0]; //getting the data from L3 in the nonforwarding case
			dataValues[1] = L5->curData[1];
			r0 = L5->curCommand[1]; //the register to be written into
		}
		if(L5->curIsWorking == false)
		{
			L5->nextIsWorking = false;
			return;
		}
		//else we will work
		if(arch->instructionNumber(L5->curCommand[0]) == 2)
		{
			//then this EX is of the 9 stage pipeline path
			//in this case the address can be calculated by adding dataValues[0] and dataValues[1] 
			//and dataValues[2] will be the data to be written into the memory incase of sw
			int address = dataValues[0] + dataValues[1];
			
		}
		else
		{
			//then this EX is of the 7stage pipeline path
			 
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


void HazardUpdate(int maxVal)
	{
		auto i = DataHazards.begin();
		while(i != DataHazards.end())
		{
			if(++(i->second) >= maxVal)
			{
				auto t = i;
				i++; DataHazards.erase(t);
			}
			else{
				i++;
			}
		}
	}

void ExecutePipelined(MIPS_Architecture *arch)
	{
		if (arch->commands.size() >= arch->MAX / 4)
		{
			arch->handleExit(arch->MEMORY_ERROR, 0);
			return;
		} //memory error

		//registers[registerMap["$sp"]] = (4 * commands.size()); //initializes position of sp. assumes that all the commands are also stored in data and so sp needs to be here
		//the above is optional, but since none of the testcases utilize it, it has been commented out
		int clockCycles = 0;
		//first we instantiate the stages
		IFID L2, LIF; //The Latches
		IDID L3; IDRR L4; RREX L5a, L5b; //The Latches
		IF0 fetch0(arch,&LIF); IF1 fetch1(arch,&LIF,&L2);
		ID0 decode0(arch,&L2,&L3);
		ID1 decode1(arch,&L3,&L4);
		RR readReg(arch,&L4, &L5a, &L5b); //IDRR (arch,&L4);
		
		int i = 5;
		while(i-- > 0)
		{
			cout << " at clockCycles " << clockCycles << endl;
			fetch0.run(); 
			fetch1.run(); 
			decode0.run(); 
			decode1.run(); 
			readReg.run();
			L2.Update(); 
			LIF.Update(); 
			L3.Update(); 
			L4.Update();
			if(arch->outputFormat == 0) 
			{	
				std::cout << " dataHazards are : ";
				for(auto i: DataHazards)
				{	
					std::cout << i.first << " " << i.second << ", ";
				}
			}
			
			HazardUpdate(6); //updating the hazards
			clockCycles++;
			//cout << endl << " at clockCycles " << clockCycles << endl;
			std::cout << endl;
		}
		arch->handleExit(arch->SUCCESS, clockCycles);

	}

//here the commands are being actually executed.
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Required argument: file_name\n./MIPS_interpreter <file name>\n";
		return 0;
	}
	std::ifstream file(argv[1]);
	MIPS_Architecture *mips;
	if (file.is_open())
		mips = new MIPS_Architecture(file);
	else
	{
		std::cerr << "File could not be opened. Terminating...\n";
		return 0;
	}

	ExecutePipelined(mips);
	return 0;
}

