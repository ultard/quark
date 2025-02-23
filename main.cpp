#include <iostream>
#include <string>
#include <iterator>
#include "qrcodegen.hpp"
#include "basen.hpp"

const std::string VERSION = "1.0";
const std::string AUTHOR = "Ultar";
const std::string SECRET = "aHR0cHM6Ly92a3ZpZGVvLnJ1L3ZpZGVvLTE3NzMwNDU0OV80NTYyMzk4Nzg=";

int main(int argc, char* argv[]) {
    srand(time(0));

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            std::cout << "Help menu\n";
            return 0;
        }
        else if (arg == "--version" || arg == "-v") {
            std::cout << "Author: " << AUTHOR << "\nVersion " << VERSION << "\n";
            return 0;
        }
        else if (arg == "--encode" || arg == "-e") {
            std::string decoded_secret;
            bn::decode_b64(SECRET.begin(), SECRET.end(), std::back_inserter(decoded_secret));

            const std::string url = (rand() % 100) > 60 ? decoded_secret : argv[2];
            const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(url.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);

            const int size = qr.getSize();
            const int margin = 2;
            for (int y = -margin; y < size + margin; y++) {
                for (int x = -margin; x < size + margin; x++) {
                    std::cout << (qr.getModule(x, y) ?  "  " : "██");
                }
                std::cout << std::endl;
            }
        }
    }
}
