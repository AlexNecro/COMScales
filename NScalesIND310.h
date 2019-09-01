#pragma once
#include "IScalesHW.h"
//Mettler Toledo IND310
//working mode: "Multicontinuous 1 Mode Output" (unnatural mode for PC!), 8bits, no parity
//format (from IND310_ETM_71207916_R4.pdf)
//18 bytes in message:
//---------------------------------------------------------------------------
//|	ADR	|SWA	|SWB	|SWC	|X X X X X X		|X X X X X X		|CR	|CKS|
//|	1	|			2			|		3			|		4			|5	|6	|
//|		|STATUS BYTES			|GROSS / NET WEIGHT	|	TARE WEIGHT		|	|	|
//|	01	|29		|30		|21		|20 20 20 20 30 30	|20 20 20 20 30 30	|0D	| seems we have no checksum
//---------------------------------------------------------------------------
//1. ASCII Character in hex that represents the scale address 01 = scale A, 02 = scale
//B, 03 = scale C, 04 = scale D, 05 = scale E(sum)
//2.  <SWA>, <SWB>, <SWC> Status Word Bytes A, B, and C.Refer to the
//Standard Bit Identification Tables for individual bit definition.
//3. Displayed weight, either Gross or Net weight.Six digits, no decimal point or
//sign.Non significant leading zeros are replaced with spaces.
//4. Tare weight.Six digits, no decimal point or sign.
//5. <CR> ASCII Carriage Return, Hex 0d.
//6. <CKS> Checksum character, 2’s complement of the 7 low order bits of the
//binary sum of all characters on a line preceding the checksum, including the
//STX and CR
//
//SWA contains multiplier in bits 0-2: 1(001)=10, 2(010)=1, 3(011)=0.1, 4(100)=0.01, 5(101)=0.001
//SWB contains: bit1: sign(0=+, 1=-), bit2: out of range, bit3: motion, bit6: zero not captured (?), bit4: 1==kg
//SWC contains units setup
//SWA[5]==1, SWA[6]==0. SWB[5]==1, SWC[5]==1, SWC[6]==0
//sample(weight is 0):
//01 29 30 21 20 20 20 20 30 30 20 20 20 20 30 30 0D
//1380kg:
//01 29 30 21 20 20 31 33 38 30 20 20 20 20 30 30 0D
//here Scale=01 (A)
//SWA=29 (00101001): 001: multiplier is 10
//SWB=30 (00110000): using kilograms
//SWC=21 (00100001): 001: grams (strange!)
//WEIGHT:	....00
//TARE:		....00
//<CR>

class NScalesIND310 :
	public IScalesHW
{
protected:
	byte scaleAddress;
public:
	NScalesIND310(NLog* log);
	virtual ~NScalesIND310();		
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual double GetWeight();	
};

