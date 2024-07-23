#!/bin/bash

function menu {
	clear
	echo "您可以选择以下选项："
	echo
	echo "1. 发送在线通知广播"
	echo "2. 选择用户进行聊天"
	echo "3. 查看聊天记录"
	echo "4. 退出聊天程序"
	echo
	read -n1 -p "请输入你的选择：" option
}

function sendUDP {
	clear
	./sendUDP $name
}

function client {
	clear
	read -p "请输入要连接的用户的IP地址：" IP
	read -p "请输入聊天记录只要保存的文件名：" file
	./client $IP $name $file
}

function History {
	clear
	cat $file
}

function Exit {
	./sendUDP "_EXIT"
	./client "127.0.0.1" "_EXIT" $file
	rm $file
}

clear
echo "欢迎使用命令行局域网P2P聊天程序"
read -p "请输入您的昵称：" name

while [ 1 ]
do
	menu
	case $option in
	1 )
	   	sendUDP ;;
	2 )
		client ;;
	3 )
		History ;;
	4 )
		Exit 
		break ;;
	* )
		clear
		echo "您输入的选项错误，请重新输入：" ;;
	esac
	echo
	echo -n "按下任意字符返回菜单"
	read -n1 asd
done

clear
