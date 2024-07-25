@echo off

set buildDir=bin\win\dbg\
if not exist %buildDir% mkdir %buildDir%

cl /nologo /I..\okc\src\Game\ /I..\okc\vendor\raylib\src /I..\okc\vendor\box2d\include\ -c /Z7 /D DBG=1 src\main.cc /Fo:%buildDir%\game.obj
link /NOLOGO /DLL %buildDir%\game.obj /DEBUG /PDB:%buildDir%\game.pdb /OUT:%buildDir%\game.dll
