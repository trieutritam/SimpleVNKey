//============================================================================
// Name        : kbengine.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
using namespace std;

#include "kbengine/kbengine.h"
#include "kbengine/platforms/macos.h"

int main()
{
		cout << "!!!Hello World 123!!!" << endl; // prints !!!Hello World!!!
																						//	char ch = 1;
		kbengine engine;
		engine.resetBuffer();

		UInt16 seq[] = {KEY_V, KEY_I, KEY_E, KEY_T, KEY_6, KEY_5};
		UInt16 CharCode[] = {'v', 'i', 'e', 't', '6', '5'};
		int size = 6;

		for (int i = 0; i < size; i++)
		{
			engine.process(CharCode[i], seq[i], 0, false);
			vector<UInt32> keystrokes = engine.getOutputBuffer();

			if (keystrokes.size() == 0)
				continue;

			cout << endl
					<< "send keystroke: ";
			for (auto it = keystrokes.begin(); it != keystrokes.end(); it++)
			{
				cout << (*it) << " ";
			}
			cout << endl;
		}

		BufferEntry *buffer = engine.getBuffer();
		for (int i = 0; i < 32; i++)
		{
				cout << (buffer + i)->keyCode << " ";
		}

		return 0;
}
