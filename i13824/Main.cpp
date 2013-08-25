/*
 * Mini DirectShow Player 
 *
 */
#include <iostream>
#include <string>
using namespace std;

#include <Windows.h>
#include <tchar.h>

#include "MiniPlayer.h"

BOOL OpenFile(LPTSTR szFile, size_t len)
{
	OPENFILENAME ofn;       // common dialog box structure

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = len;
	ofn.lpstrFilter = _T("Media Files\0*.avi;*.mpg;*.mpeg;*.wmv;*.mp4;*.rm;*.rmvb;*.mkv\0All Files\0*.*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 

	return GetOpenFileName(&ofn);
}

int main()
{
	TCHAR filename[MAX_PATH] = {0};

	if(!OpenFile(filename, MAX_PATH)) return 0;

	MiniPlayer mp;
	mp.prepare(filename);
	mp.play();
	long volume = mp.volume();
	long balance = mp.balance();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if(msg.message == WM_KEYUP)
		{
			if(msg.wParam == VK_UP)
			{
			}
			switch(msg.wParam)
			{
			case VK_UP:
				volume += 100;
				if(volume > 0) volume = 0;
				mp.setVolume(volume);
				std::cout << "volume up: " << volume << std::endl;
				break;
			case VK_DOWN:
				volume -= 100;
				mp.setVolume(volume);
				std::cout << "volume down: " << volume << std::endl;
				break;
			case VK_LEFT:
				balance -= 100;
				mp.setBalance(balance);
				std::cout << "balance left: " << balance << std::endl;
				break;
			case VK_RIGHT:
				balance += 100;
				mp.setBalance(balance);
				std::cout << "balance right: " << balance << std::endl;
				break;
			}
			
		}
	}

Cleanup:
	CoUninitialize();

	return 0;
}