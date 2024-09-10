#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Sabit kullanıcı adı ve şifre listesi
const char *usernames[] = {"n0n0", "z0z0", "root", "admin"};
const char *passwords[] = {"n0n0", "z0z0", "root", "admin"};
const int user_count = 4;

// Kullanıcı adı ve şifre doğrulama fonksiyonu
bool check_credentials(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        fprintf(stdout, "[LOG]: Nickname: %s ?? %s  || Password: %s ?? %s\n", usernames[i], username,
        passwords[i], password);
        if (strcmp(usernames[i], username) == 0 && strcmp(passwords[i], password) == 0) {
            return true; // Giriş bilgileri doğru
        }
    }
    return false; // Giriş bilgileri yanlış
}
