socat -d -d pty,raw,echo=0 pty,raw,echo=0			- 내부 serial 포트 열기

gcc -o serv serv.c -lmodbus -I/usr/local/include/modbus

gcc -o cli cli.c func.c -lmodbus -I/usr/local/include/modbus	- build

export LD_LIBRARY_PATH=/usr/local/lib				- 고정 라이브러리 포함 변수

