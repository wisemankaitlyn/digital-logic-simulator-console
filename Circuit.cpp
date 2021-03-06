/*
Circuit.cpp   Implementation of the Circuit class
Author:       Kaitlyn Wiseman
Modified:     12 Nov 2020
*/

#pragma once

#include "Circuit.h"

// constructor
Circuit::Circuit() {
	
}


// TO USE IN MAIN()

// step 1.
bool Circuit::ReadCircuit(std::string filename) {
	std::ifstream in;
	std::string keyword;
	std::string name;
	std::string sNumber;
	int number;
	std::string delay;
	std::string sInput0;
	int input0;
	std::string sInput1;
	int input1;
	std::string sOutput;
	int output;

	in.open(filename);
	if (!in.is_open())
	{
		std::cerr << std::endl;
		std::cerr << "Could not open file " << filename << std::endl;
		return false;
	}

	in >> keyword;
	while (!in.eof())
	{
		// circuit name
		if (keyword == "CIRCUIT")
		{
			getline(in, this->name, '\n');
		}
		// io wires
		else if (keyword == "INPUT" || keyword == "OUTPUT")
		{
			in >> name >> sNumber;

			if (isdigit(sNumber[0]))
			{
				number = atoi(sNumber.c_str());
				MakeWire(number, name);
				ioWires.push_back(wires.at(number));

				name = "";
				sNumber = "";
			}
			else
			{
				// invalid input, clear and skip the current line
				std::cerr << "Issue with wire number: " << sNumber << std::endl;
				getline(in, keyword, '\n');

				name = "";
				sNumber = "";
			}
		}
		// not gate
		else if (keyword == "NOT")
		{
			in >> delay >> sInput0 >> sOutput;

			if (isdigit(sInput0[0]))
			{
				input0 = atoi(sInput0.c_str());
				MakeWire(input0);
			}
			else
			{
				std::cerr << "Issue with gate: " << sInput0 << std::endl;
				getline(in, keyword, '\n');
				in >> keyword;
				
				delay = "";
				sInput0 = "";
				sOutput = "";

				continue;
			}

			if (isdigit(sOutput[0]))
			{
				output = atoi(sOutput.c_str());
				MakeWire(output);
			}
			else
			{
				std::cerr << "Issue with gate: " << sOutput << std::endl;
				getline(in, keyword, '\n');
				in >> keyword;

				delay = "";
				sInput0 = "";
				sOutput = "";

				continue;
			}

			gates.push_back(new Gate(keyword, atoi(delay.c_str()), 
				wires.at(input0), wires.at(output)));

			wires.at(input0)->AddGate(gates.back());

			delay = "";
			sInput0 = "";
			sOutput = "";
		}
		// other gates
		else if (keyword == "AND" || keyword == "OR" || keyword == "XOR"
			|| keyword == "NAND" || keyword == "NOR" || keyword == "XNOR")
		{
			in >> delay >> sInput0 >> sInput1 >> sOutput;

			if (isdigit(sInput0[0]))
			{
				input0 = atoi(sInput0.c_str());
				MakeWire(input0);
			}
			else
			{
				std::cerr << "Issue with gate: " << sInput0 << std::endl;
				getline(in, keyword, '\n');
				in >> keyword;

				delay = "";
				sInput0 = "";
				sInput1 = "";
				sOutput = "";

				continue;
			}

			if (isdigit(sInput1[0]))
			{
				input1 = atoi(sInput1.c_str());
				MakeWire(input1);
			}
			else
			{
				std::cerr << "Issue with gate: " << sInput1 << std::endl;
				getline(in, keyword, '\n');
				in >> keyword;

				delay = "";
				sInput0 = "";
				sInput1 = "";
				sOutput = "";

				continue;
			}
			
			if (isdigit(sOutput[0]))
			{
				output = atoi(sOutput.c_str());
				MakeWire(output);
			}
			else
			{
				std::cerr << "Issue with gate: " << sOutput << std::endl;
				getline(in, keyword, '\n');
				in >> keyword;

				delay = "";
				sInput0 = "";
				sInput1 = "";
				sOutput = "";

				continue;
			}

			gates.push_back(new Gate(keyword, atoi(delay.c_str()),
				wires.at(input0), wires.at(input1), wires.at(output)));

			wires.at(input0)->AddGate(gates.back());
			wires.at(input1)->AddGate(gates.back());

			delay = "";
			sInput0 = "";
			sInput1 = "";
			sOutput = "";
		}
		// invalid input
		else
		{
			std::cerr << "invalid keyword " << keyword 
				<< ". skipping this line." << std::endl;

			// clear the line
			getline(in, keyword, '\n');
		}

		in >> keyword;
	}

	in.close();
	return true;
}

// step 2.
bool Circuit::ReadVector(std::string filename) {
	std::ifstream in;
	std::string keyword;
	std::string wireName;
	int time;
	std::string tempval;
	int val;

	in.open(filename);
	if (!in.is_open())
	{
		std::cerr << std::endl;
		std::cerr << "Could not open file " << filename << std::endl;
		return false;
	}

	in >> keyword;
	while (!in.eof())
	{
		if (keyword == "VECTOR")
		{
			getline(in, keyword, '\n');
		}
		else if (keyword == "INPUT")
		{
			in >> wireName >> time >> tempval;

			if (tempval == "X")
			{
				val = -1;
			}
			else if (tempval == "")
			{
				std::cerr << "Ill-formed input line in vector file." << std::endl;
				getline(in, keyword, '\n');
				in >> keyword; 

				continue;
			}
			else
			{
				val = atoi(tempval.c_str());
			}

			for (Wire* w : ioWires)
			{
				if (w->GetName() == wireName)
				{
					q.push(Event(w->GetWireNo(), time, val));
					break;
				}
			}

			tempval = "";
		}
		else
		{
			// this line is invalid input...
			std::cerr << "Invalid keyword in vector file: " << keyword << std::endl;
			getline(in, keyword, '\n');
		}

		in >> keyword;
	}

	in.close();
	return true;
}

// step 3.
void Circuit::Simulate() {
	while (!q.empty())
	{
		// get the next event
		Event e = q.top();
		q.pop();

		Wire* w = wires.at(e.GetWire());
		int val = e.GetValue();
		int time = e.GetTime();
		// implement it
		w->SetValue(time, val);

		// find out if it has any lingering effects
		// + add addl events accordingly
		int i = 0;
		while (true)
		{
			try {
				Gate* g = w->GetGate(i);

				// find out whether the value changed
				if (g->Evaluate(time) != 
					g->GetOutput()->at(time + g->GetDelay()))
				{
					// yes, the value has changed.

					// find out if there's a duplicate event in the
					// queue
					Event n(g->GetOutput()->GetWireNo(), time + g->GetDelay(),
						g->Evaluate(time));

					// to make sure we don't simulate past 60 ns
					if (n.GetTime() <= 60)
					{
						if (!IsInQueue(n))
						{
							// if there's an earlier-created event in the queue for the
							// same wire + time but a different value (i.e. a conflict),
							// update the event with the new value
							if (IsConflict(n))
							{
								ReplaceInQueue(n);
							}
							// if there's no duplicate or conflict, put the event in
							else 
							{
								q.push(n);
							}
							wires.at(n.GetWire())->SetValue(n.GetTime(), n.GetValue());
						}						
					}
				}
			}
			// if we index out of the vector range, we've exhausted
			// the number of gates
			catch (std::out_of_range)
			{
				break;
			}
			i++;
		}
	}
}

// step 4.
std::string Circuit::Print() {
	std::string output = "";
	//std::cout << "Circuit: " << name << std::endl;
	output += "Circuit: " + name + "\n";
	output += "Wire traces: \n";
	//std::cout << "Wire traces:" << std::endl;

	int time = ioWires.at(0)->GetValuesSize();
	int nameLen = ioWires.at(0)->GetName().length();

	for (Wire* w : ioWires)
	{
		if (w->GetValuesSize() > time)
		{
			time = w->GetValuesSize();
		}

		if (w->GetName().length() > nameLen)
		{
			nameLen = w->GetName().length();
		}
	}

	for (Wire* w : ioWires)
	{
		output += w->Print(time, nameLen);
	}

	// print the numbers at the bottom
	for (int i = 0; i < nameLen + 2; i++)
	{
		output += " ";
	}
	output += "0";
	while (time > 0)
	{
		if (time >= 10)
		{
			output += "    5    0";
			time -= 10;
		}
		else if (time >= 5)
		{
			output += "    5";
			time = 0;
		}
		else
		{
			time = 0;
		}
		
	}

	output += "\n \n";

	return output;
}


// OTHER

void Circuit::MakeWire(int wireNo, std::string iname) {
	if (wires.size() <= wireNo)
	{
		wires.resize(wireNo + 1, NULL);
	}
	if (wires.at(wireNo) == NULL)
	{
		wires.at(wireNo) = new Wire(iname, wireNo);
	}
}


bool Circuit::IsInQueue(Event& e) const
{
	Queue temp = q;

	while (!temp.empty())
	{
		Event qe = temp.top();
		temp.pop();

		if (qe == e)
		{
			return true;
		}
	}

	return false;
}


bool Circuit::IsConflict(Event& e) const
{
	Queue temp = q;

	while (!temp.empty())
	{
		Event qe = temp.top();
		temp.pop();

		if (qe.GetWire() == e.GetWire()
			&& qe.GetTime() == e.GetTime()
			&& qe.GetValue() != e.GetValue())
		{
			return true;
		}
	}

	return false;
}


void Circuit::ReplaceInQueue(Event& e)
{
	Queue temp;
	Queue copy = q;

	Event current;

	while (!copy.empty())
	{
		current = copy.top();
		copy.pop();

		if (current.GetWire() == e.GetWire()
			&& current.GetTime() == e.GetTime())
		{
			temp.push(e);

			while (!copy.empty())
			{
				temp.push(copy.top());
				copy.pop();
			}
			q = temp;

			return;
		}

		temp.push(current);
	}

}


// for debugging - to test the contents of the queue
void Circuit::PrintWires() const {
	std::cout << "Printing all wires: " << std::endl;

	for (Wire* w : wires)
	{
		if (w == NULL) continue;

		std::cout << w->GetWireNo() << " drives ";

		int i = 0;
		while (true)
		{
			try {
				std::cout << w->GetGate(i)->GetType() << " ";
				i++;
			}
			catch (std::out_of_range)
			{
				break;
			}
		}

		std::cout << std::endl;
	}
	
	std::cout << std::endl;

	std::cout << "Printing I/O wires: " << std::endl;

	for (Wire* w : ioWires)
	{
		std::cout << w->GetName() << " " << w->GetWireNo() << " drives ";

		int i = 0;
		while (true)
		{
			try {
				std::cout << w->GetGate(i)->GetType() << " ";
				i++;
			}
			catch (std::out_of_range)
			{
				break;
			}
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;
}

void Circuit::PrintGates() const {
	std::cout << "Printing gates: " << std::endl;

	for (Gate* g : gates)
	{
		std::cout << g->GetType() << " " << g->GetDelay() << " "
			<< g->GetInput0()->GetWireNo() << " ";
		if (g->GetType() != "NOT")
		{
			std::cout << g->GetInput1()->GetWireNo() << " ";
		}

		std::cout << g->GetOutput()->GetWireNo() << std::endl;
	}

	std::cout << std::endl;
}

void Circuit::PrintQueue() const {
	Queue temp = q;

	std::cout << "Printing contents of queue: " << std::endl;

	while (!temp.empty())
	{
		Event e = temp.top();
		std::cout << e.GetWire() << " " << e.GetTime() << " "
			<< e.GetValue() << std::endl;

		temp.pop();
	}
}