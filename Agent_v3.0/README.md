#실행 방법
  > 프로그램 실행 이전에 export LD_LIBRARY_PATH=/usr/local/lib 명령어 입력<br>
  > 포함 라이브러리 고정주소 입력<br><br>
  > serv : ./serv<br>
  > serv_option.txt 파일에서 설정을 읽어들어와 프로그램 실행<br>
  > PortName, SlaveID, Bit rate, Parity, Debug option 순서대로 파일에 저장되어 있음<br><br><br>

  > cli  : ./cli<br>
  >   실행 후 새로운 연결을 만든다. 연결을 선택하고 query 문을 이용하여 통신한다.<br><br><br>

  > cli_test : ./cli_test <PortName> <Slave ID><br>
  > 설정된 포트 이름과 slave id 에 1초마다 랜덤으로 query를 날린다. server 의 정상작동을 알아보기 위한 test program<br><br><br>

#modbus-private.h
  > modbus library의 modbus_reply 함수를 꺼내 변경하기 위해 가져옴<br>
  > modbus_reply 에서 사용되는 다양한 상수값(define) 이 정의되어 있음<br>
  
#unit-test.h
  > 현재 구현된 server, client 모델에서 공유하는 상수 값 및 const 변수들<br>
  > register memory 가 RO 인지 RW 인지 구별하기 위한 flag 존재<br>
  
#serv.c
  > Slave module 을 표현하기 위한 server program<br>
  > 현재 기본값으로 register 값들이 초기화 되어 있음<br>
  > modbus library 의 modbus_reply 함수에 memory flag 를 넣어 RO, RW memory 를 구분 -> RW 일 시에만 data write 되도록 분기 설정<br>
  >     -> modbus_reply_userdef 로 새로운 함수를 정의하여 사용  : func.h 에 정의됨<br>
  >     -> 다른 함수들은 변경점 x, register write 에 연결되는 0x06, 0x10 함수에만 flag 값을 이용한 분기가 추가됨<br>
  >   
  >     void init_register_data( uint16_t *regi ) 함수<br>
  >           : register 내부의 값을 기본값으로 초기화 시킨다.<br>
  
#usrdef_modbus_receive.h      usrdef_modbus_receive.c
  > odbus library 의 modbus_reply 함수에 memory flag 를 넣기 위해 만든 header 및 source<br>
  > modbus_reply 함수와 기본적으로 동일하나 register 에 값 입력 시 호출되는 함수 ( 0x06, 0x10 ) 에서 flag 값 검사로 인한 분기 설정<br>
  
  
#cli.c
  > Master module 을 표현하기 위한 client program<br>
  > 최대 247개 까지의 연결을 만들 수 있다.<br>
  > 연결을 선택하고 선택된 Slave module 에 명령어를 입력하여 read, write 가능<br>
  > modbus library 의 function 0x03, 0x06, 0x01 을 이용하여 기능 구현<br>
  

#cli_test.c
  > server 가 문제없이 정상 작동 하는가를 테스트 하기 위한 test client<br>
  > PortName 과 SlaveID를 파라미터로 받아 한개의 server 에 1초다마 random query 를 전달, 통신을 수행한다.
