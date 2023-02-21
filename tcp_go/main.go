package main

import (
	"net"
	"os"
)

func main() {
	//wd := "\x00\x00 hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness."
	strEcho := "FS_CREATE z\x00erui /a/a d         \x00"
	//strEcho := "FS_WRITEBLOCK 0 /foo -8\x00" + wd
	//strEcho := "FS_READBLOCK zerui /a/a/a 10000000000\x00"
	// strEcho := "FS_CREATE zerui /1234\x00"
	//strEcho := "FS_DELETE zerui /b/f_p\x00"
	servAddr := "localhost:8000"
	tcpAddr, err := net.ResolveTCPAddr("tcp", servAddr)
	if err != nil {
		println("ResolveTCPAddr failed:", err.Error())
		os.Exit(1)
	}

	conn, err := net.DialTCP("tcp", nil, tcpAddr)
	if err != nil {
		println("Dial failed:", err.Error())
		os.Exit(1)
	}

	_, err = conn.Write([]byte(strEcho))
	if err != nil {
		println("Write to server failed:", err.Error())
		os.Exit(1)
	}
	conn.Close()
}

