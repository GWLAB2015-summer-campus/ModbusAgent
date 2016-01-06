# Serial 통신 프로그램 작성해보기
	> socat 명령을 통해 가상의 연결통로를 만들고 시리얼통신을 하는 프로그램을 간단히 만들자.
	> socat -d -d pty,raw,echo=0 pty,raw,echo=0


# 사용법
	terminal 1
	> terminal 1 $ socat -d -d pty,raw,echo=0 pty,raw,echo=0
	
	2016/01/06 10:14:10 socat[26237] N PTY is /dev/pts/3		//
	2016/01/06 10:14:10 socat[26237] N PTY is /dev/pts/4		// remember this number
	2016/01/06 10:14:10 socat[26237] N starting data transfer loop with FDs [3,3] and [5,5]

	
	
	terminal 2
	> terminal 2 $ ./cli 3
	> hi
	> hello
	
	
	
	terminal 3
	> terminal 3 $ ./serv 4
	> buf is [hi]
	> buf is [hello]
	
	
	
