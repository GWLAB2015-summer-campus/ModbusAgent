#modbus-private.h
  > modbus library의 modbus_reply 함수를 꺼내 변경하기 위해 가져옴\n
  > modbus_reply 에서 사용되는 다양한 상수값(define) 이 정의되어 있음
  
#unit-test.h
  > 현재 구현된 server, client 모델에서 공유하는 상수 값 및 const 변수들
  
#serv.c
  > Slave module 을 표현하기 위한 server program
  > 현재 기본값으로 register 값들이 초기화 되어 있음
  > modbus library 의 modbus_reply 함수에 memory flag 를 넣어 RO, RW memory 를 구분 -> RW 일 시에만 data write 되도록 분기 설정
  >     -> modbus_reply_userdef 로 새로운 함수를 정의하여 사용  : func.h 에 정의됨
  >     -> 다른 함수들은 변경점 x, register write 에 연결되는 0x06, 0x10 함수에만 flag 값을 이용한 분기가 추가됨
  >   
  >     void init_register_data( uint16_t *regi ) 함수
  >           : register 내부의 값을 기본값으로 초기화 시킨다.
  
#func.h func.c
  > odbus library 의 modbus_reply 함수에 memory flag 를 넣기 위해 만든 header 및 source
  > modbus_reply 함수와 기본적으로 동일하나 register 에 값 입력 시 호출되는 함수 ( 0x06, 0x10 ) 에서 flag 값 검사로 인한 분기 설정
  
  
#cli.c
  > Master module 을 표현하기 위한 client program
  > Slave module 에 명령어를 입력하여 read, write 가능
  > modbus library 의 function 0x03, 0x06, 0x01 을 이용하여 기능 구현
  
