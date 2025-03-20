@echo off

set "SOURCE_DIR=%~dp0"

set "TARGET1=D:\GameEngine_DirectX11\GameEngine\GameEngine\ThirdParty\coremodule"
set "TARGET2=D:\GameEngine_DirectX11\GameEngine\Client\ThirdParty\coremodule"

if not exist %TARGET1% mkdir %TARGET1%
if not exist %TARGET2% mkdir %TARGET2%

echo Source Dir : %SOURCE_DIR%
echo Target1 : %TARGET1%
echo Target2 : %TARGET2%
echo.

robocopy %SOURCE_DIR% %TARGET1% /E /XF Copy.bat
robocopy %SOURCE_DIR% %TARGET2% /E /XF Copy.bat

echo.
echo 모든 파일(하위 폴더 포함)을 복사 완료했습니다.
pause