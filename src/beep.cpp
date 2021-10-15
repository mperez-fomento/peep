#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

const std::string DBURL {"https://beep-8f89b-default-rtdb.europe-west1.firebasedatabase.app"};
const std::string USERSURL {"users"};
const int DEFAULT_MESSAGES_NR_PRINTED {5};

std::vector<std::string> split_command_line(std::string command_line) {
    std::vector<std::string> result {};
    std::istringstream command_stream {command_line};
    std::string word {};
    while (command_stream >> word) {
        result.push_back(word);
    }
    return result;
}

bool exists(std::string userid, httplib::Client & http_client) {
    std::string url = "/" + USERSURL + "/" + userid + ".json";
    std::cout << url << "\n";
    auto res = http_client.Get(url.c_str());
    return res->status == 200;
}

std::string get_conversation(std::string a, std::string b, int messagesnr, httplib::Client & http_client)
{
    return "";
}

void connect(std::string source, std::string target, std::string & chat_selected, httplib::Client & http_client)
{
    if (exists(target, http_client)) {
        std::cout << "[Conversation with " << target << "]\n";
        std::cout << get_conversation(source, target, DEFAULT_MESSAGES_NR_PRINTED, http_client);
        chat_selected = target;
    }
    else {
        std::cout << "[Cannot connect to " << target << ": unknown user id]\n";
    }
}

void send_message(const std::string & source, const std::string & target, const std::string & body, httplib::Client & http_client)
{
    std::cout << "(You) -> " << body << "\n";
}

std::string login(const std::string & userid, const std::string & password, httplib::Client & http_client)
{
    std::string url = "/" + USERSURL + ".json";
    auto res = http_client.Get(url.c_str());
    json users {};
    if (res->status == 200) {
        users = json::parse(res->body);
        try {
            if (password == users.at(userid)) {
                return userid;
            }
            else {
                std::cout << "[login failed: incorrect password]\n";
            }
        }
        catch (json::out_of_range & e) {
            std::cout << "[login failed: unknown user id]\n";
        }
    }
    else {
        std::cout << "[error connecting to server: " << res.error() << "]\n";
    }
    return "nobody";
}

int execute_command(std::string command_line,
                    std::string & source,
                    std::string & chat_selected,
                    bool & close_requested,
                    httplib::Client & http_client)
{
    std::vector<std::string> command_words {split_command_line(command_line)};
    if (command_words.size() < 1) {
        return -1;
    }
    std::string command {command_words[0]};
    if (source != "nobody") {
        if (chat_selected == "nobody") {
            if (command == "connect") {
                if (command_words.size() <  2) {
                    std::cout << "['connect' requires a user id to connect to]\n";
                    return -1;
                }
                std::string target {command_words[1]};
                connect(source, target, chat_selected, http_client);
            }
        }
        else if (command.size() > 0){
            if (command[0] == '#') {
                if (command == "#close") {
                    std::cout << "[Conversation closed]\n'";
                    chat_selected = "nobody";
                }
                if (command == "#quit") {
                    close_requested = true;
                }
            }
            else {
                send_message(source, chat_selected, command_line, http_client);
            }
        }
    }
    else if (command == "login") {
        std::cout << "User ID: ";
        std::string userid {};
        std::cin >> userid;
        std::cout << "Password: ";
        std::string password {};
        std::cin >> password;
        source = login(userid, password, http_client);
    }
    if (command == "quit" || command == "q") {
        close_requested = true;
    }
    return 0;
}


int main()
{
    httplib::Client http_client(DBURL);
    bool close_requested {false};
    std::string chat_selected {"nobody"};
    std::string me {"nobody"};
    std::string command_line {};
    while (!close_requested) {
        std::cout << ((chat_selected == "nobody") ? "> " : "[" + me + "-->" + chat_selected + "] ");
        getline(std::cin, command_line);
        execute_command(command_line, me, chat_selected, close_requested, http_client);
    }
}
