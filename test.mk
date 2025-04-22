ESP32(控制馬達) - 內含有藍芽:
	ESP32 -->->->->-- Motor
	1. 寫入 BLE 接收與發送功能。
	2. 寫入 控制馬達的功能。
	3. 寫入一個 function ，名為： 	
		init_motor_pin()：初始化馬達要使用的Pin角，以及初始數據。
		setMotorStatus(int motor_pin, int value) ： 設定馬達的狀態與數據
		getMotorStatus(int motor_pin) 取得馬達狀態與數據


BLE:
1. Use Arduino-IDE
2. Arduino Project: BLE Server ()
3. 



NRF24:
	switch data Byte:
		byte[0] = port
		byte[1] = channel
		byte[0] = port
		byte[0] = port
		byte[0] = port
		byte[0] = port
		byte[0] = port
		byte[0] = port
		byte[0] = port

