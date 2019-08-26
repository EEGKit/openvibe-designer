#pragma once

#include <string>

#if defined TARGET_OS_Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <fs/IEntryEnumerator.h>
#include <fs/Files.h>
#include <iostream>
#include <fstream>
#endif

#if defined TARGET_OS_Linux || defined TARGET_OS_MacOS
namespace
{
	class CProcessEnumeratorCB : public FS::IEntryEnumeratorCallBack
	{
	public:
		CProcessEnumeratorCB(std::string sProcessName, bool& rbFoundProcess)
			: m_sProcessName(sProcessName),
			  m_rbFoundProcess(rbFoundProcess)
		{}

		virtual FS::boolean callback(FS::IEntryEnumerator::IEntry& rEntry,
									 FS::IEntryEnumerator::IAttributes& rAttributes)
		{
			std::ifstream l_fsInputStream;
			FS::Files::openIFStream(l_fsInputStream, rEntry.getName());
			std::string l_sFileContents;

			if (l_fsInputStream.is_open())
			{
				while(!l_fsInputStream.eof()) { l_fsInputStream >> l_sFileContents; }
				size_t l_stFound = l_sFileContents.rfind(m_sProcessName);
				if (l_stFound != std::string::npos && l_stFound == l_sFileContents.length() - m_sProcessName.length() - 1 )
				{
					m_rbFoundProcess = true;
				}

			}
			l_fsInputStream.close();
			return true;
		}

		std::string m_sProcessName;
		bool& m_rbFoundProcess;
	};
}
#endif

namespace OpenViBE
{
	namespace ProcessUtilities
	{
		bool doesProcessExist(std::string sProcessName)
		{
#if defined TARGET_OS_Windows
			HANDLE hProcessSnap;
			PROCESSENTRY32 pe32;

			// Take a snapshot of all processes in the system.
			hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
			if( hProcessSnap == INVALID_HANDLE_VALUE ) { return false; }

			// Set the size of the structure before using it.
			pe32.dwSize = sizeof( PROCESSENTRY32 );

			// Retrieve information about the first process,
			// and exit if unsuccessful
			if( !Process32First( hProcessSnap, &pe32 ) )
			{
				CloseHandle( hProcessSnap );          // clean the snapshot object
				return false;
			}

			bool bReturnValue = false;
			// Now walk the snapshot of processes
			do
			{
				if (sProcessName + ".exe" == std::string(pe32.szExeFile))
				{
					bReturnValue = true;
				}
			} while( Process32Next( hProcessSnap, &pe32 ) );

			CloseHandle( hProcessSnap );

			return bReturnValue;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS

			bool l_bFoundProcess = false;
			CProcessEnumeratorCB* l_rCB = new CProcessEnumeratorCB(sProcessName, l_bFoundProcess);

			FS::IEntryEnumerator* l_pEntryEnumerator = FS::createEntryEnumerator(*l_rCB);
			l_pEntryEnumerator->enumerate("/proc/*/cmdline");
			l_pEntryEnumerator->release();

			delete l_rCB;
			return l_bFoundProcess;
#endif
		}

		bool launchCommand(const char* sCommandLine)
		{
		#if defined TARGET_OS_Windows
			/*
			LPTSTR szCmdline = TEXT(const_cast<char*>(sCommandLine));

			STARTUPINFO lpStartupInfo;
			ZeroMemory(&lpStartupInfo,sizeof(lpStartupInfo));
			lpStartupInfo.cb = sizeof(lpStartupInfo);
			lpStartupInfo.dwFlags |= CREATE_NEW_CONSOLE;

			PROCESS_INFORMATION lpProcessInfo;
			// Create the process
			if (!CreateProcess(nullptr,szCmdline, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) { exit(1); }
			*/
			std::string l_sCommand = "start \"\" \"" + std::string(sCommandLine) + "\"";
			system(l_sCommand.c_str());
		#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS

			if (fork() == 0)
			{
				system(sCommandLine);
				exit(0);
			}

			// FIXME: temporary solution using system()
		#endif

			return true;
		}
	}
}
