mkdir PatchConfig_DIST
mkdir PatchConfig_DIST\res

copy *.cpp PatchConfig_DIST\
copy *.h PatchConfig_DIST\
copy PatchConfig.dsw PatchConfig_DIST\
copy PatchConfig.dsp PatchConfig_DIST\
copy PatchConfig.rc PatchConfig_DIST\
copy PatchConfig.manifest PatchConfig_DIST\
copy default.conf PatchConfig_DIST\
copy res\PatchConfig.rc2 PatchConfig_DIST\res\
copy res\PatchConfig.ico PatchConfig_DIST\res\
copy dist.bat PatchConfig_DIST\

pause