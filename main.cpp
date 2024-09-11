#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <netdb.h>

using namespace std;

const size_t MAX_MESS_LEN = 1000;


void receive_message(int socketf, string l_ip, size_t l_nickname) {
  struct sockaddr_in s_addr;
  socklen_t addr_l = sizeof(s_addr);

  while (true) {
    string buffer(MAX_MESS_LEN+l_nickname, '\0');
    ssize_t len = recvfrom(socketf, &buffer[0], MAX_MESS_LEN, 0, (struct sockaddr*)&s_addr, &addr_l);
    if (len < 0) {
      cerr << "Ошибка при получении сообщения " <<  strerror(errno) <<  endl;
      continue;
    }
    buffer.resize(len);
    string s_ip = inet_ntoa(s_addr.sin_addr);
    if(s_ip == l_ip){
        continue;
    }
    cout << "Ip " << s_ip << " Message " << buffer << endl;
  }
}

void send_message(int socketf, const string& broad_ip, uint16_t port, const string& nickname) {
  struct sockaddr_in broad_addr;
  memset(&broad_addr, 0, sizeof(broad_addr));
  broad_addr.sin_family = AF_INET;
  broad_addr.sin_port = htons(port);
  inet_pton(AF_INET, broad_ip.c_str(), &broad_addr.sin_addr);

  string message;

  while (true) {
    cout <<  "Введите сообщение: " << endl;
    getline(cin, message);

    if (message.length() == 0) {
      cerr <<  "Сообщение пустое" <<  endl;
      continue;
    }
    if(message.length() == MAX_MESS_LEN){
        cerr << "Сообщение слишком длинное" << endl;
    }
    string full_message = nickname + ": " + message;
    ssize_t s_len = sendto(socketf, full_message.c_str(), full_message.size(), 0, (struct sockaddr*)&broad_addr, sizeof(broad_addr));
    if (s_len < 0) {
      cerr << "Ошибка, не удалось отправить: " <<  strerror(errno) <<  endl;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr <<  "Ошибка: Неправильное количество аргументов" <<  endl;
    return EXIT_FAILURE;
  }
  string l_ip = argv[1];
  uint16_t port = stoi(argv[2]);

  // Создадим сокет
  int socketf = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketf < 0) {
    cerr <<  "Не удалось создать сокет" <<  endl;
    return EXIT_FAILURE;
  }


  int broadcastEnable = 1;

    if (setsockopt(socketf, SOL_SOCKET, SO_REUSEADDR, &broadcastEnable, sizeof(broadcastEnable)) < 0){
    cerr <<  "Не удалось установить опцию SO_REUSEADDR" <<  endl;
    close(socketf);
    return 1;
  }
    // Устанавливаем опцию широковещания
  if (setsockopt(socketf, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0){
    cerr <<  "Не удалось установить опцию широковещания" <<  endl;
    close(socketf);
    return 1;
  }

  struct sockaddr_in loc_addr;
  memset(&loc_addr, 0, sizeof(loc_addr));
  loc_addr.sin_family = AF_INET;
  loc_addr.sin_port = htons(port);
  loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socketf, (struct sockaddr*)&loc_addr, sizeof(loc_addr)) < 0) {
    cerr <<  "Не удалось связать сокет с адресом и портом" <<  endl;
    close(socketf);
    return 1;
  }

  string nickname;
  cout <<  "Введите nickname: ";
  getline(cin, nickname);

  thread r_thread(receive_message, socketf, l_ip, nickname.length());
  thread s_thread(send_message, socketf, "255.255.255.255", port, nickname);

  r_thread.join();
  s_thread.join();
  close(socketf);

  return EXIT_SUCCESS;
}
