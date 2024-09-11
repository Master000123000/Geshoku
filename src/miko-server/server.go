package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
)

func write_welcome() {
	fmt.Println("-----------Geshoku------------")
	fmt.Println("--------Miko-Server-----------")
	fmt.Println("------Version 0.001 Alpha-----")
	fmt.Println("------------------------------")
}

func handle_client(conn net.Conn) {
	defer conn.Close()
	buffer := make([]byte, 1024)
	fmt.Println("[Info] Client connected:", conn.RemoteAddr())
	fmt.Println("[Info] Waiting for commands to send to the client")
	//var command string
	for {
		for i := range buffer {
			buffer[i] = 0
		}
		fmt.Print("Shell> ")
		in := bufio.NewReader(os.Stdin)
		command, err := in.ReadString('\n')
		if err != nil {
			fmt.Println("[Err] Failed to read command:", err)
			continue
		}
		if command == "quit" {
			fmt.Println("[Info] Quitting... Closing connection to client.")
			break
		}
		_, err = conn.Write([]byte(command + "\n"))
		if err != nil {
			fmt.Println("[Err] Failed to send command to client:", err)
			break
		}
		fmt.Println("Reading data")
		_, err = conn.Read(buffer)
		if err != nil {
			fmt.Println("[Err] Failed to receive response from client:", err)
			break
		}
		println("data received")
		response := string(buffer[:])
		if response != "" {
			fmt.Println(string(buffer[:]))
		}
	}
}

func start_server() {
	fmt.Println("[Info] Trying to start server")
	listener, err := net.Listen("tcp", ":6666")
	fmt.Println("started server")
	if err != nil {
		fmt.Println("[Err] Server start failed: ", err)
		return
	}
	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("[Err] Connection accept failed: ", err)
			continue
		}
		go handle_client(conn)
	}
}

func main() {
	write_welcome()
	start_server()
}
