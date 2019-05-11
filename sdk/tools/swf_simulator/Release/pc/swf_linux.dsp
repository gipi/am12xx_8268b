# Microsoft Developer Studio Project File - Name="swf_linux" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=swf_linux - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "swf_linux.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "swf_linux.mak" CFG="swf_linux - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "swf_linux - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "swf_linux - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "swf_linux - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../inc" /I "../" /I "../zlib" /I "../swf" /I "../app" /I "./" /I "swfext/" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cv.lib cxcore.lib highgui.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "swf_linux - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../zlib" /I "../swf" /I "../app" /I "./" /I "swfext/" /I "../freetype/include" /I "../freetype" /I "../keyboard" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 freepy.lib freepy_lib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cv.lib cxcore.lib highgui.lib Debug\adler32.obj Debug\autofit.obj Debug\buddy.obj Debug\crc32.obj Debug\dbg_dump.obj Debug\dump.obj Debug\fontdrv.obj Debug\freepy.obj Debug\ftbase.obj Debug\ftbbox.obj Debug\ftbitmap.obj Debug\ftdebug.obj Debug\ftfstype.obj Debug\ftgasp.obj Debug\ftglyph.obj Debug\ftgxval.obj Debug\ftinit.obj Debug\ftlcdfil.obj Debug\ftmm.obj Debug\ftotval.obj Debug\ftstroke.obj Debug\ftsynth.obj Debug\ftsystem.obj Debug\fui_common.obj Debug\fui_input.obj Debug\Gb2312ToUni.obj Debug\Gb2312ToUtf8.obj Debug\gzio.obj Debug\inffast.obj Debug\inflate.obj Debug\inftrees.obj Debug\keyboard.obj Debug\linear.obj Debug\qsort.obj Debug\raster.obj Debug\sfnt.obj Debug\smooth.obj Debug\sprintf.obj Debug\swf_aa.obj Debug\swf_action.obj Debug\swf_action_array.obj Debug\swf_action_asBroadcaster.obj Debug\swf_action_bitmapdata.obj Debug\swf_action_boolean.obj Debug\swf_action_color.obj Debug\swf_action_colorTransform.obj Debug\swf_action_date.obj Debug\swf_action_default.obj Debug\swf_action_external.obj Debug\swf_action_function.obj Debug\swf_action_global.obj Debug\swf_action_key.obj Debug\swf_action_loadVars.obj Debug\swf_action_math.obj Debug\swf_action_matrix.obj Debug\swf_action_mouse.obj Debug\swf_action_movieClipLoader.obj Debug\swf_action_number.obj Debug\swf_action_object.obj Debug\swf_action_point.obj Debug\swf_action_rectangle.obj Debug\swf_action_selection.obj Debug\swf_action_shared_object.obj Debug\swf_action_sound.obj Debug\swf_action_sprite.obj Debug\swf_action_stage.obj Debug\swf_action_string.obj Debug\swf_action_text_format.obj Debug\swf_action_text_snapshot.obj Debug\swf_action_types.obj Debug\swf_action_xml.obj Debug\swf_action_xmlnode.obj Debug\swf_bitmap.obj Debug\swf_button.obj Debug\swf_cache.obj Debug\swf_dec.obj Debug\swf_dictionary.obj Debug\swf_displaylist.obj Debug\swf_enviroment.obj Debug\swf_font.obj Debug\swf_geometry.obj Debug\swf_gradient.obj Debug\swf_hash.obj Debug\swf_keyboard.obj Debug\swf_mem.obj Debug\swf_morph.obj Debug\swf_render.obj Debug\swf_render_2d.obj Debug\swf_render_422.obj Debug\swf_render_565.obj Debug\swf_render_888.obj Debug\swf_render_low.obj Debug\swf_shape.obj Debug\swf_sound.obj Debug\swf_sprite.obj Debug\swf_sqrt.obj Debug\swf_stat.obj Debug\swf_system.obj Debug\swf_text.obj Debug\swf_time.obj Debug\swf_util.obj Debug\swf_vvd.obj Debug\swfdec.obj Debug\table_read.obj Debug\test.obj Debug\truetype.obj Debug\ttfont.obj Debug\Utf8ToGb2312.obj Debug\work.obj Debug\zutil.obj /nologo /subsystem:console /pdb:"./Debug/swf.pdb" /debug /machine:I386 /out:"./Debug/swf.exe" /pdbtype:sept /libpath:"../keyboard"
# SUBTRACT LINK32 /pdb:none /map

!ENDIF 

# Begin Target

# Name "swf_linux - Win32 Release"
# Name "swf_linux - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app\act_plugin.h
# End Source File
# Begin Source File

SOURCE=.\swfext\audioengine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\audioengine.h
# End Source File
# Begin Source File

SOURCE=.\swfext\cmmb_engine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\ebook.c
# End Source File
# Begin Source File

SOURCE=.\swfext\ebook.h
# End Source File
# Begin Source File

SOURCE=.\swfext\filelist.c
# End Source File
# Begin Source File

SOURCE=.\swfext\flashengine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\locale.c
# End Source File
# Begin Source File

SOURCE=.\swfext\locale.h
# End Source File
# Begin Source File

SOURCE=.\swfext\photoengine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\swfext.c
# End Source File
# Begin Source File

SOURCE=.\swfext\systeminfo.c
# End Source File
# Begin Source File

SOURCE=.\swfext\usbuiengine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\videoengine.c
# End Source File
# Begin Source File

SOURCE=.\swfext\vvdengine.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\am_types.h
# End Source File
# Begin Source File

SOURCE=..\freetype\include\ft2build.h
# End Source File
# Begin Source File

SOURCE=..\app\linear.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_action.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_base.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_dec.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_displaylist.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_geometry.h
# End Source File
# Begin Source File

SOURCE=..\zlib\swf_gzio.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_io.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_list.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_mem.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_opt.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_render.h
# End Source File
# Begin Source File

SOURCE=..\app\swf_system.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_time.h
# End Source File
# Begin Source File

SOURCE=.\swf_types.h
# End Source File
# Begin Source File

SOURCE=..\swf\swf_util.h
# End Source File
# Begin Source File

SOURCE=.\swfdec.h
# End Source File
# Begin Source File

SOURCE=.\swfext\swfext.h
# End Source File
# Begin Source File

SOURCE=..\keyboard\table_read.h
# End Source File
# Begin Source File

SOURCE=..\freetype\ttfont.h
# End Source File
# Begin Source File

SOURCE=..\app\work.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
