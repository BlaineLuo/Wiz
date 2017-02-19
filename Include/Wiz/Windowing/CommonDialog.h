// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_COMMONDIALOG_H__
#define __WIZ_WINDOWING_COMMONDIALOG_H__

#include "Wiz/Windowing/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

// ============================================================
#ifndef _WIN32_WCE
class OpenFileName : public Structure< OPENFILENAME >{

public:
	MaxPathT _fileName;

	inline OpenFileName(){
		MemoryReset( *this );
		_fileName.clear();
	}

	bool open(){
		Entry& entry = *this;
		entry.lStructSize = _entrySize;
		entry.hwndOwner = NULL;
		entry.lpstrFile = _fileName;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		entry.lpstrFile[0] = '\0';
		entry.nMaxFile = _fileName._totalSize;
		//entry.lpstrFilter = _T("Executable File\0*.EXE;*.SCR\0");
		entry.nFilterIndex = 1;
		entry.lpstrFileTitle = NULL;
		entry.nMaxFileTitle = 0;
		entry.lpstrInitialDir = NULL;
		entry.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_NOCHANGEDIR;
		//entry.Flags = OFN_EXPLORER;
		return( 0 != ::GetOpenFileName( &entry ) );
	}

	bool save(){
		Entry& entry = *this;
		entry.lStructSize = _entrySize;
		entry.hwndOwner = NULL;
		entry.lpstrFile = _fileName;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		entry.lpstrFile[0] = '\0';
		entry.nMaxFile = _fileName._totalSize;
		//entry.lpstrFilter = _T("Executable File\0*.EXE;*.SCR\0");
		entry.nFilterIndex = 1;
		entry.lpstrFileTitle = NULL;
		entry.nMaxFileTitle = 0;
		entry.lpstrInitialDir = NULL;
		entry.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		return( 0 != ::GetSaveFileName( &entry ) );
	}
};
#endif

}}
// ===================================Namespace Tail==========================================

#endif
