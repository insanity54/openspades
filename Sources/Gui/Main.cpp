//
//  main.cpp
//  OpenSpades
//
//  Created by yvt on 7/11/13.
//  Copyright (c) 2013 yvt.jp. All rights reserved.
//

#include "MainWindow.h"
#include "../Core/FileManager.h"
#include "../Core/DirectoryFileSystem.h"
#include "../Core/Debug.h"
#include "../Core/Settings.h"
#include "../Core/ConcurrentDispatch.h"
#include "../Core/ZipFileSystem.h"

#include "../Core/VoxelModel.h"
#include "../Draw/GLOptimizedVoxelModel.h"

//using namespace spades::gui;
#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef __APPLE__
#include <xmmintrin.h>
#endif

#include <FL/fl_ask.H>

int main(int argc, char ** argv)
{
	
	// Enable FPE
#if 0
#ifdef __APPLE__
	short fpflags = 0x1332; // Default FP flags, change this however you want.
	__asm__("fnclex\n\tfldcw %0\n\t": "=m"(fpflags));
	
	_MM_SET_EXCEPTION_MASK(_MM_GET_EXCEPTION_MASK() & ~_MM_MASK_INVALID);
#endif
	
#endif
	
	
	Fl::scheme("gtk+");
	
	spades::reflection::Backtrace::StartBacktrace();
	
	SPADES_MARK_FUNCTION();
	spades::DispatchQueue::GetThreadQueue()->MarkSDLVideoThread();
	
#ifdef PACKAGE_STRING
	SPLog("Package: %s", PACKAGE_STRING);
#else
	SPLog("Package: (unknown)");
#endif
	
	// default resource directories
#ifdef WIN32
	static char buf[4096];
	GetModuleFileName(NULL, buf, 4096);
	std::string appdir = buf;
	appdir = appdir.substr(0, appdir.find_last_of('\\')+1);
	
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem(appdir + "Resources", false));
	
	if(SUCCEEDED(SHGetFolderPath(NULL,
								 CSIDL_APPDATA,
								 NULL, 0,
								 buf))){
		
		
		std::string datadir = buf;
		datadir += "\\OpenSpades\\Resources";
		spades::FileManager::AddFileSystem
		(new spades::DirectoryFileSystem(datadir, true));
	}
#elif defined(__APPLE__)
	std::string home = getenv("HOME");
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem("./Resources", false));
	
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem(home+"/Library/Application Support/OpenSpades/Resources", true));
#else
	std::string home = getenv("HOME");
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem("./Resources", false));
	
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem(home+"/.openspades/Resources", true));
#endif
	
	try{
		spades::StartLog();
	}catch(const std::exception& ex){
		fl_alert("Failed to start recording log because of the following error:\n%s\n\n"
				 "OpenSpades will continue to run, but any critical events are not logged.", ex.what());
	}
	SPLog("Log Started.");
	
#ifdef RESDIR
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem(RESDIR, false));
#endif
	
	/*
	spades::FileManager::AddFileSystem
	(new spades::DirectoryFileSystem("/Users/tcpp/Programs/MacPrograms2/OpenSpades/Resources", false));
	*/
	// add all zip files
	{
		std::vector<spades::IFileSystem*> fss;
		
		std::vector<std::string> files = spades::FileManager::EnumFiles("");
		
		for(size_t i = 0; i < files.size(); i++){
			std::string name = files[i];
			
			
			// check extension
			if(name.size() < 4 ||
			   name.rfind(".pak") != name.size() - 4){
				continue;
			}
			
			if(spades::FileManager::FileExists(name.c_str())) {
				spades::IStream *stream = spades::FileManager::OpenForReading(name.c_str());
				spades::ZipFileSystem *fs = new spades::ZipFileSystem(stream);
				SPLog("Pak Registered: %s\n", name.c_str());
				fss.push_back(fs);
			}
		}
		for(size_t i = 0; i < fss.size(); i++){
			spades::FileManager::AddFileSystem(fss[i]);
		}
	}
	
	MainWindow win;
	win.Init();
	win.show(argc, argv);
	win.CheckGLCapability();
	
	SPLog("Entering FLTK main loop");
	Fl::run();
	
	SPLog("Leaving FLTK main loop");
	spades::Settings::GetInstance()->Flush();
	
    return 0;
}

