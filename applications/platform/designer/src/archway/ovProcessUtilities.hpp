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
		CProcessEnumeratorCB(std::string processName, bool& rbFoundProcess) : m_name(processName), m_found(rbFoundProcess) { }

		virtual FS::boolean callback(FS::IEntryEnumerator::IEntry& entry, FS::IEntryEnumerator::IAttributes& /*attributes*/)
		{
			std::ifstream fsInputStream;
			FS::Files::openIFStream(fsInputStream, entry.getName());
			std::string fileContents;

			if (fsInputStream.is_open())
			{
				while(!fsInputStream.eof()) { fsInputStream >> fileContents; }
				size_t pos = fileContents.rfind(m_name);
				if (pos != std::string::npos && pos == fileContents.length() - m_name.length() - 1 ) { m_found = true; }

			}
			fsInputStream.close();
			return true;
		}

		std::string m_name;
		bool& m_found;
	};
}
#endif

namespace OpenViBE {
namespace ProcessUtilities {
bool doesProcessExist(const std::string& name)
{
#if defined TARGET_OS_Windows
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	const HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (processSnap == INVALID_HANDLE_VALUE) { return false; }

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(processSnap, &pe32))
	{
		CloseHandle(processSnap);          // clean the snapshot object
		return false;
	}

	bool bReturnValue = false;
	// Now walk the snapshot of processes
	do { if (name + ".exe" == std::string(pe32.szExeFile)) { bReturnValue = true; } } while (Process32Next(processSnap, &pe32));

	CloseHandle(processSnap);

	return bReturnValue;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS

			bool found = false;
			CProcessEnumeratorCB* cb = new CProcessEnumeratorCB(name, found);

			FS::IEntryEnumerator* entry = FS::createEntryEnumerator(*cb);
			entry->enumerate("/proc/*/cmdline");
			entry->release();

			delete cb;
			return found;
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
	const std::string cmd = "start \"\" \"" + std::string(sCommandLine) + "\"";
	system(cmd.c_str());
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
}  // namespace ProcessUtilities
}  // namespace OpenViBE
