call ..\config.cmd

call:create 2600_0_32
call:create 2600_1_32
call:create 2600_2_32
call:create 2600_3_32
call:create 3790_0_32
call:create 3790_1_32
call:create 3790_1_64
call:create 3790_2_32
call:create 3790_2_64
call:create 6000_0_32
call:create 6000_0_64
call:create 6001_1_32
call:create 6001_1_64
call:create 6002_2_32
call:create 6002_2_64
call:create 7600_0_32
call:create 7600_0_64
call:create 7601_1_32
call:create 7601_1_64
call:create 9200_0_32
call:create 9200_0_64
call:create 9600_0_32
call:create 9600_0_64

exit

:create

rm -f sqlite3 ..\build\db\%~1.db3
%SQLITE_PATH%\sqlite3 ..\build\db\%~1.db3 < %~1.sql

GOTO:EOF