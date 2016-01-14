#실행 방법
  > serv : ./serv <portName> <SlaveID> <option><br>
  >   ex) ./serv /dev/pts/1 17 ON               // debug option 사용하면서 실행<br>
  >       ./serv /dev/pts/1 17                  // debug option 사용하지 않으면서 실행<br>
  >       ./serv /dev/pts/1 17 asdfx            // debuf option 사용하지 않으면서 실행, ON 에만 반응함<br><br><br>
  > cli  : ./cli<br>
  >   실행 후 새로운 연결을 만든다. 연결을 선택하고 query 문을 이용하여 통신한다.<br><br><br>


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
