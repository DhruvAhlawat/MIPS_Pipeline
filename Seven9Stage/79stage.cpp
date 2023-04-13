#include<MIPS_Processor.hpp>
#include<map>
#include<string>
#include<set>
#include<vector>
#include<iostream>
using namespace std;
map<string,int> DataHazards;

int stallNumber = 0;
set<int> pcs;
struct IFID //basically the L2 latch, used to transfer values between IF and ID stage
{
	vector<string> currentCommand = {};
	vector<string> nextCommand = {};
	int curPc, nextPc;
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
		curPc = nextPc;
		nextPc = 0;
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
		pcs.insert(arch->PCcurr); //inserted the pc into the set
		//else we will work
		//then we check if the current instruction is a branch
		LIF->nextPc = arch->PCcurr;
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
		L2->nextPc = LIF->curPc;
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
	vector<string> curCommand = {}, nextCommand = {};
	bool curIsWorking = true, nextIsWorking = true;
	int curPc, nextPc;
	void Update()
	{
		//on getting the updated values, we can run the code
		// curData = nextData;
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		curPc = nextPc;
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
		L3->nextPc = L2->curPc;
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
	vector<string> curCommand = {}, nextCommand = {};
	bool curIsWorking = true, nextIsWorking = true;
	int curOffset = 0, nextOffset = 0;
	int nextPc= -1, curPc = -1;
	void Update()
	{
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		curPc = nextPc;
	}
};

struct ID1
{
	MIPS_Architecture *arch;
	IDID *LID; 	IDRR *L4; 
	vector<string> curCommand = {};
	//ID0 will be responsible for decoding the instruction
	ID1(MIPS_Architecture *mips, IDID *lid, IDRR *l4)
	{
		arch = mips;
		LID = lid;
		L4 = l4;
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
		L4->nextPc = LID->curPc;
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
	vector<int> curData = {}, nextData = {};
	vector<string> curCommand,nextCommand;
	bool curIsWorking = true, nextIsWorking = true;
	string curWriteReg, nextWriteReg;
	int curPC = -1, nextPC = 0;
	void Update()
	{
		//on getting the updated values, we can run the code
		curData = nextData;
		curCommand = nextCommand;
		curIsWorking = nextIsWorking;
		curWriteReg = nextWriteReg;
		nextCommand = {};
		curPC = nextPC;
		//nextInstructionType = ""; //this would ensure that if instruction
		// type does not get updated, then we won't run any new commands
	}
};


struct RR
{
	MIPS_Architecture *arch;
	IDRR *L4; RREX *L5r, *L5i;
	vector<int> regVal; int nextOffset = 0;
	string writeReg = "";
	vector<string> curCommand;
	//RR is responsible for reading the register values and passing them to EX for working
	RR(MIPS_Architecture *mips, IDRR *l4, RREX *l5a, RREX *l5b)
	{
		arch = mips;
		regVal = vector<int>(3,0);
		L4 = l4;
		L5r = l5a;
		L5i = l5b;
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
		if(curCommand[3] == "")
			regVal[1] = 0;
		else if(arch->instructionNumber(curCommand[0]) == 0)
			regVal[1] = arch->registers[arch->registerMap[curCommand[3]]];
		else if(arch->instructionNumber(curCommand[0]) == 1)
			regVal[1] = stoi(curCommand[3]);
		// else
		// 	regVal[1] = arch->registers[arch->registerMap[curCommand[3]]]; //getting its value normally

		writeReg = curCommand[1]; 
		if(curCommand[0] == "beq" || curCommand[0] == "bne")
		{
			//then we need to stall the pipeline
			stallNumber = 5; //so the next RR instruction gets stalled as well.
			//and pass the commands forward as well	
		}
		
		else if(arch->instructionNumber(curCommand[0]) == 2)
		{
			//then we need to take the 9 stage pipeline path
			if(!arch->outputFormat)
				cout << "Itype ";
			nextOffset = L4->nextOffset;
			regVal[1] = nextOffset; 
			// L5r->nextCommand = {}; //passing a no-op
			L5i->nextPC = L4->curPc;
			L5i->nextWriteReg = writeReg;
			L5i->nextData = regVal; //passing the data to ALU of the i type (9 stage) instruction
			L5i->nextCommand = curCommand; 

			cout << curCommand[0] << " " << regVal[0] << "+" << regVal[1]; // << "data-" <<  << " ";
		}
		else
		{
			//otherwise we need to take the the 7 stage pipeline path
			// L5i->nextCommand = {};
			L5r->nextPC = L4->curPc;
			L5r->nextCommand = curCommand;
			L5r->nextWriteReg = writeReg;
			L5r->nextData = regVal; //passing the data to ALU of the r type (7 stage) instruction
			if(!arch->outputFormat) {
				cout << "Rtype ";
				cout << "passed " << regVal[0] << " " << regVal[1] << " "; 
			}
		}
	}
};

struct EXDM
{
	vector<string> curCommand, nextCommand;
	string curReg, nextReg;
	int curSWdata, nextSWdata;
	int curAddr, nextAddr;
	int curPC = -1, nextPC = -1;
	bool curIsWorking = true, nextIsWorking = true;
	void Update()
	{
		// curAddr = nextAddr; curReg = nextReg;
		curCommand = nextCommand; nextCommand = {};
		curAddr = nextAddr;
		curReg = nextReg; curIsWorking = nextIsWorking;
		curSWdata = nextSWdata;
		curPC = nextPC; nextPC = -1;
	}
};

struct LWB
{
	string curReg, nextReg;
	int curDataOut, nextDataOut;
	bool curIsWorking = true, nextIsWorking = true;
	bool curIsUsingWriteBack = false, nextIsUsingWriteBack = false;
	int curPC = -1, nextPC = -1;
	void Update()
	{
		curReg = nextReg; curDataOut = nextDataOut;
		nextReg = "";
		curIsWorking = nextIsWorking;
		curIsUsingWriteBack = nextIsUsingWriteBack;
		nextIsUsingWriteBack = false;
		curPC = nextPC; nextPC = -1;
	}
};

struct DM0
{
	MIPS_Architecture *arch;
	EXDM *L7, *L8; //L8 here will be used for to transferring data from this stage to the DM1 stage
	DM0(MIPS_Architecture *architecture, EXDM *l7, EXDM *l8)
	{
		arch = architecture; L7 = l7; L8 = l8;
	}
	void run()
	{
		 //transporting the value from the DM0 stage to the DM1 stage, where all the computation will happen
		if(arch->outputFormat==0)
			cout << "|DM0|=>";	
		L8->nextPC = L7->curPC; //PC update
		L8->nextAddr = L7->curAddr;
		L8->nextCommand = L7->curCommand;
		L8->nextIsWorking = L7->curIsWorking;
		L8->nextReg = L7->curReg;
		L8->nextSWdata = L7->curSWdata; 							
	}
};
	

struct DM1
{
	MIPS_Architecture *arch;
	EXDM *L8; LWB *L6; int Addr = 0;
	string writeReg = "";
	DM1(MIPS_Architecture *architecture, EXDM *l8, LWB *l6)
	{
		arch = architecture; L8 = l8; L6 = l6;
	}
	void run()
	{
		if(arch->outputFormat==0)
			cout << "|DM1|=>";
		if(L8->curIsWorking == false)
		{
			return;
		}
		if(L8->curCommand.size() == 0)
		{
			// L6->nextIsWorking = false;
			return;
		}
		Addr = L8->curAddr;
		L6->nextPC = L8->curPC;
		if(L8->curCommand[0] == "lw")
		{
			L6->nextReg = L8->curReg;
			L6->nextIsUsingWriteBack = true;
			L6->nextDataOut = arch->data[Addr];
			if(!arch->outputFormat)
				cout << "lw " << L8->curReg << " " << L6->nextDataOut << " ";
		}
		else if(L8->curCommand[0] == "sw")
		{
			arch->data[Addr] = L8->curSWdata;
			L6->nextIsUsingWriteBack = false;
			if(!arch->outputFormat)
				cout << "sw " << L8->curSWdata << " " << Addr << " ";
		}
	}
};

struct EX
{	
	public:
	bool isWorking = true; int swData;
	MIPS_Architecture *arch; RREX *L5; EXDM *L7; LWB *L6;
	string iType = "";
	vector<int> dataValues; 
	int result = 0;
	string r0; //register to be written into, this will not be used in this step but passed forward till the WriteBack stage where it will be written into
	//now we decode the instruction from the instructions map
	int checkforPC;
	EX(MIPS_Architecture *architecture, RREX *l5, EXDM *l7, LWB *l6)
	{
		arch = architecture; L5 = l5; L7 = l7; L6 = l6;//the latch reference and architecture reference is stored at initialization
		dataValues = vector<int>(3,0);
	}

	void run()
	{	
		if(arch->outputFormat == 0)
			cout << "|EX|=>";
		if(stallNumber > 5)
		{
			//then we are supposed to stall and effectively do nothing
			if(!arch->outputFormat)
				cout << "**";
			return;
		}
		else
		{
			if(L5->curCommand.size() == 0)
			{
				return; //a no-op
			}
			iType = L5->curCommand[0];
			dataValues = L5->curData; //getting the data from L3 in the nonforwarding case
			r0 = L5->curCommand[1]; //the register to be written into
		}
		if(L5->curIsWorking == false)
		{
			L7->nextIsWorking = false;
			return;
		}
		
		//else we will work
		if(arch->instructionNumber(L5->curCommand[0]) == 2)
		{
			//then this EX is of the 9 stage pipeline path
			L7->nextPC = L5->curPC; //PC update
			//in this case the address can be calculated by adding dataValues[0] and dataValues[1] 
			//and dataValues[2] will be the data to be written into the memory incase of sw
			int address = dataValues[0] + dataValues[1]; //this is indeed the address
			if(address%4 != 0) cerr << "Error: Address not word aligned" << endl;
			address = address/4;
			if(arch->outputFormat == 0) cout << "address: " << address << " ";
			L7->nextCommand = L5->curCommand;
			L7->nextAddr = address;
			L7->nextReg = r0;
			L7->nextSWdata = dataValues[2]; //this is the data to be written into the memory incase of sw
		}
		else
		{
			//then this EX is of the 7stage pipeline path
			L6->nextPC = L5->curPC; //PC update
			int result = calc();
			if(arch->outputFormat == 0) cout << "Result: " << result << " ";
			L6->nextIsUsingWriteBack = true;
			L6->nextReg = r0;
			L6->nextDataOut = result;
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

struct WB
{	public:
	bool isWorking = true;
	MIPS_Architecture *arch; LWB *dmwb, *exwb, *usingLatch;
	int dataOut = 0; string reg = ""; int curPc = -1;
	WB(MIPS_Architecture *architecture, LWB *lwb1, LWB *lwb2)
	{
		arch = architecture; dmwb = lwb1; exwb = lwb2;
	}
	void run()
	{
		if(arch->outputFormat == 0)
			cout << "|WB|=> ";
		//check which one of these requires the writeback port, or if none require it.
		if(dmwb->curIsUsingWriteBack && !(exwb->curIsUsingWriteBack))
		{
			usingLatch = dmwb;
		}
		else if(exwb->curIsUsingWriteBack && !(dmwb->curIsUsingWriteBack))
		{
			usingLatch = exwb;
		}
		else if(!(dmwb->curIsUsingWriteBack) && !(exwb->curIsUsingWriteBack))
		{
			return; //do nothing this cycle
		}
		else
		{
			//then both are using the writeback port, so we would have stalled before in ID stage
			//so we do nothing
			// cerr << "both writing??";
		}
		pcs.erase(dmwb->curPC); pcs.erase(exwb->curPC); //erasing both of those pcs
		reg = usingLatch->curReg; dataOut = usingLatch->curDataOut;
		curPc = usingLatch->curPC; //with this we get the pc 
		cout << "pcI:" << dmwb->curPC << "pcR:" << exwb->curPC  << " ";
		if(reg != "")
		{
			arch->registers[arch->registerMap[reg]] = dataOut;
			if(arch->outputFormat == 0)
				cout << reg << ":" << dataOut << " ";
		}
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
		IDID L3; IDRR L4; RREX L5i, L5r; //The Latches
		EXDM L7, L9; LWB L6r, L8i; //The Latches 
		IF0 fetch0(arch,&LIF); IF1 fetch1(arch,&LIF,&L2);
		ID0 decode0(arch,&L2,&L3);
		ID1 decode1(arch,&L3,&L4);
		RR readReg(arch, &L4, &L5i, &L5r); //IDRR (arch,&L4);
		EX ALUi(arch,&L5i,&L7, &L6r); //RREX (arch,&L5);
		EX ALUr(arch,&L5r,&L7, &L6r); //RREX (arch,&L5);
		WB writeBack(arch,&L8i,&L6r); //LWB (arch,&L6);
		DM0 dataMem0(arch,&L7,&L9); //EXDM (arch,&L7);
		DM1 dataMem1(arch,&L9,&L8i); //LWB (arch,&L8);
		int i = 12;
		do
		{
			fetch0.run(); 
			fetch1.run(); 
			decode0.run(); 
			decode1.run(); 
			readReg.run();
			ALUi.run(); ALUr.run();
			dataMem0.run(); dataMem1.run();
			writeBack.run();

			L2.Update(); 
			LIF.Update(); 
			L3.Update(); 
			L4.Update();
			L5i.Update();
			L5r.Update();
			L6r.Update();
			L7.Update();
			L8i.Update();
			L9.Update();
			// if(arch->outputFormat == 0) 
			// {	
			// 	std::cout << " dataHazards are : ";
			// 	for(auto i: DataHazards)
			// 	{	
			// 		std::cout << i.first << " " << i.second << ", ";
			// 	}
			// }
			
			HazardUpdate(6); //updating the hazards
			clockCycles++;
			arch->printRegisters(clockCycles);
			//cout << endl << " at clockCycles " << clockCycles << endl;
			std::cout << endl;
			
			
		} while((pcs.size() > 0));
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

