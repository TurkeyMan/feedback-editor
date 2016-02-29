@ECHO OFF

ECHO Select the type of project you would like to create:
ECHO 1. Visual Studio 2015 Solution
ECHO 2. Visual Studio 2013 Solution
ECHO 3. Visual Studio 2012 Solution
ECHO 4. Visual Studio 2010 Solution
ECHO 5. Visual Studio 2008 Solution
ECHO 6. CodeLite Project
ECHO 7. GNU Makefile

CHOICE /N /C:12345678 /M "[1-8]:"

IF ERRORLEVEL ==7 GOTO SEVEN
IF ERRORLEVEL ==6 GOTO SIX
IF ERRORLEVEL ==5 GOTO FIVE
IF ERRORLEVEL ==4 GOTO FOUR
IF ERRORLEVEL ==3 GOTO THREE
IF ERRORLEVEL ==2 GOTO TWO
IF ERRORLEVEL ==1 GOTO ONE
GOTO END

:SEVEN
 ECHO Creating GNU Makefile...
 ..\..\Fuji\dist\bin\premake5.exe gmake
 GOTO END
:SIX
 ECHO Creating CodeLite Project...
 ..\..\Fuji\dist\bin\premake5.exe codelite
 GOTO END
:FIVE
 ECHO Creating VS2008 Project...
 ..\..\Fuji\dist\bin\premake5.exe vs2008
 GOTO END
:FOUR
 ECHO Creating VS2010 Project...
 ..\..\Fuji\dist\bin\premake5.exe vs2010
 GOTO END
:THREE
 ECHO Creating VS2012 Project...
 ..\..\Fuji\dist\bin\premake5.exe vs2012
 GOTO END
:TWO
 ECHO Creating VS2013 Project...
 ..\..\Fuji\dist\bin\premake5.exe vs2013
 GOTO END
:ONE
 ECHO Creating VS2015 Project...
 ..\..\Fuji\dist\bin\premake5.exe vs2015
 GOTO END

:END
