package main

import (
	"runtime"
	"tzkt_server/http_handle"
)

func main() {
	runtime.GOMAXPROCS(4)
	http_handle.StartServer()
}

/****************************************************************************************************************/
// type Server struct {
// 	ServerName string
// 	ServerIP   string
// }

// type Serverslice struct {
// 	SerVers []Server
// }

// func main() {
// 	var s Serverslice
// 	str := `{"SerVers":[{"serverName":"Shanghai_VPN","serverIP":"127.0.0.1"},
//             {"serverName":"Beijing_VPN","serverIP":"127.0.0.2"}]}`

// 	json.Unmarshal([]byte(str), &s)
// 	fmt.Println(s)
// 	fmt.Println(s.SerVers[0].ServerIP)
// }

// type Server struct {
// 	ServerName string `json:"serverName"`
// 	ServerIP   string `json:"serverIP"`
// }

// type Serverslice struct {
// 	Servers []interface{} `json:"servers"`
// }

// type rspStruct struct {
// 	Status    int64         `json:"status"`
// 	Msg       string        `json:"msg"`
// 	MessageID string        `json:"messageId"`
// 	Data      []interface{} `json:"data"`
// }

// func main() {

// 	// var s Serverslice
// 	// s.Servers = append(s.Servers, Server{ServerName: "Shanghai_VPN", ServerIP: "127.0.0.1"})
// 	// s.Servers = append(s.Servers, Server{ServerName: "Beijing_VPN", ServerIP: "127.0.0.2"})

// 	// b, err := json.Marshal(s)
// 	// if err != nil {
// 	// 	fmt.Println("json err: ", err)
// 	// }

// 	var rspS rspStruct
// 	rspS.Status = 12345
// 	rspS.Msg = "1231大幅度"
// 	rspS.MessageID = "魂牵梦萦魂牵梦萦1231"
// 	rspS.Data = append(rspS.Data, Server{ServerName: "Shanghai_VPN", ServerIP: "127.0.0.1"})
// 	rspS.Data = append(rspS.Data, Server{ServerName: "Beijing_VPN", ServerIP: "127.0.0.2"})
// 	b, _ := json.Marshal(rspS)
// 	fmt.Println(string(b))
// }
