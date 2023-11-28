#include <iostream>
#include <string>

std::string encryptString(const std::string& input) {
    std::string encrypted = input;

    for (char& c : encrypted) {
        if (isalpha(c)) {
            char base = (isupper(c)) ? 'A' : 'a';
            c = (((c - base) + 5) % 26) + base;
        } else if (isdigit(c)) {
            c = (((c - '0') + 5) % 10) + '0';
        }
    }

    return encrypted;
}

int main() {
    std::string input1 = "Welcome to EE450!";
    std::string input2 = "199@$";
    std::string input3 = "0.27#&";
    std::string input4 = "gourab";
    std::string input5= "John";
    std::string input6= "Utkarsh";

    std::cout << "Encrypted 1: " << encryptString(input1) << std::endl;
    std::cout << "Encrypted 2: " << encryptString(input2) << std::endl;
    std::cout << "Encrypted 3: " << encryptString(input3) << std::endl;
    std::cout << "Encrypted 4: " << encryptString(input4) << std::endl;
    std::cout << "Encrypted 5: " << encryptString(input5) << std::endl;
    std::cout << "Encrypted 6: " << encryptString(input6) << std::endl;

    return 0;
}
