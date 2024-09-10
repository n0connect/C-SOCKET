#!/bin/bash

# Derlenecek dosyalar (kullanıcıdan alınabilir)
server_file="TCPServer.c"
client_file="TCPClient.c"
database_file="database.c"
socketutil_file="socketutil.c"

CFLAGS="-g -Wvarargs -Wall -Wextra"

# Derleme komutu
gcc  -o server $CFLAGS $server_file $database_file $socketutil_file
gcc  -o client $CFLAGS $client_file $socketutil_file

# Derlemenin başarılı olup olmadığını kontrol et
if [ $? -eq 0 ]; then
    echo "-----------------------"
    echo "Build succeeded."
    echo "Running the program..."
    echo "-----------------------"
    echo " "
    # Programı çalıştır, bellek sızıntısını test et (valgrind)
    # ./server
    
    
else
    echo "Build failed."
    # Derleme hatası mesajını göster
    echo "gcc: $?"
fi
