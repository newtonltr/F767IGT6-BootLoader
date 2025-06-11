1. 网络波动的情况下，会出现实际接收多几包的问题，查看是python端多发了还是单片机端的bug
2. 添加热烧录功能
	· python与单片机连接后，如果是bootloader程序，则单片机会发送char connected[] = "client connected \r\n"
		如果是app程序，则会发送char connected_app[] = "client connected app \r\n"
	·如果收到的是app的，python程序只需等待tco断开后重连即可，app程序会在tcp连接上之后自动跳转到bootloader
	·python程序在检测到连接断开之后就可以判断app执行了跳转到bootloader的程序，此时一直尝试连接，直至连接成功即可
	·python如果连接成功后直接收到了bootloader程序的连接成功字符串则直接开始正常的流程，也就是等待用户输入bin文件路径
	·后续流程不变

3. python程序添加一个检测，如果最后一包数据发送后收到的不是start_program而是recv_success那么说明实际接收的包数与正常情况下接收的包数不符合

4. program_complete发送后要延迟100ms不要立刻跳转app，不然上位机无法收到
