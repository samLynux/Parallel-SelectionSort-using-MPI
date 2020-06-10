// Encryption using Vigenere Cipher
// Input: Message in UPPER CASE, Key in lowercase

#include "mpi.h"
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>

#define id rank
#define TAG 0

using namespace std;

//RUMUS TABLE VIGENERE
char vigenere(char msg, char k)
{
	if (!isalpha(msg)) //Jika msg berupa angka
		return msg; //plaintext berupa angka apapun, hasil ciphertextnya juga berupa angka

	//Jika char yang ingin dienkripsi  + ascii key - 96(ascii sebelum a) > 91 (ascii sesudah Z)
	//Guna: karakter 91-96 yang bkn abjad ga ikt di encrypt
	//Karakter apapun hasil ciphertextnya juga beruoa karakter tsb 
	//Z=90 , 96 -> sebelum a (a=97) 
	else if(msg + k - 96 > 91) 
		return char((msg + k) - 123); // z= 122
	else
		return char(msg + k - 97); // a = 97 
}

int main(int a, char* b[])
{
	//Inisiasi environment MPI -> biar parallel
	MPI_Init(&a, &b);

	int rank, size; //rank-> id proses, size -> jumlah proses

	//Untuk dapat rank dari setiap proses -> proses 0 ranknya brp
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Jumlah processor -> by default sesuai dg jumlah processor di laptop masing
	//Size of processor bisa di assign saat run dengan menggunakan -> mpiexec -n size namaproject
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	//Untuk kasih status udah berhasil nerima msg atau blm
	MPI_Status status;

	string str; //plaintext
	char key[50]; //key maks memiliki panjang 49
	string encrypted; //ciphertext

	int len, keylen; //Panjang data 
	int spasi, kecil, besar; //untuk constraint

	char key_character, encrypted_character, character_to_encrypt; //data per karakter
	double start, finish, time; //waktu eksekusi 

	//Proses 0 
	//Proses 0 bertugas untuk menerima input dan menampilkan semua output
	if (rank == 0)
	{
		cout << "\n----------------------------------------------------\n";
		cout << "ENCRYPTION USING VIGENERE CIPHER";
		cout << "\n----------------------------------------------------\n";

		while (1) {
			spasi = 0;
			kecil = 0;

			cout << "\nEnter the message to be encrypted: ";

			getline(cin, str); //input str 
			fflush(stdin);

			len = str.length(); //panjang str

			for (int i = 0; str[i]; i++)
			{
				if (isspace(str[i]) || ispunct(str[i]))
				{
					spasi++;
				}
				if (str[i] < 65 || str[i] > 90) {
					kecil++;
				}
			}

			if (spasi == 0 && kecil == 0) {
				break;
			}
		}

		while (1) {
			spasi = 0;
			besar = 0;
			cout << "\nEnter the cipher key: ";
			cin >> key; //input key

			keylen = strlen(key); //panjang key 
			key[keylen] = '\0'; //assign doang untuk array terakhir adalah end of file

			for (int i = 0; key[i]; i++)
			{
				if (isspace(key[i]) || ispunct(key[i]))
				{
					spasi++;
				}
				if (key[i] >= 65 && key[i] <= 90) {
					besar++;
				}
			}
			if (keylen < size && spasi == 0 && besar == 0) {
				break;
			}
		}
	}

	//mulai perhitungan waktu execution
	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	//Untuk menyebarkan data keylen bertipe int dari root (0) dengan komunikasi MPI_COMM_WORLD
	MPI_Bcast(&keylen, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Untuk menyebarkan data len bertipe int dari root (0) dengan komunikasi MPI_COMM_WORLD
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//!!! Setiap proses akan memiliki panjang key dan panjang msg !!!

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Proses root
	if (rank == 0)
	{
		//cout << "Inside root process\n";

		// Kirim karakter key - 1 character ke 1 proses
		//looping sepanjang key untuk kirim 1 char key ke 1 proses
		for (int i = 1; i <= keylen; i++)
		{
			if (str[i - 1] == '0') {
				cout << "Exit";
				break;
			}

			cout << "Sending " << key[i - 1] << " to " << i << endl;

			//Send -> langsung kirim
			//Barrier -> nunggu semua proses sampai pada titik yang ditentukan
			//Ssend -> nunggu apa yang mau dikirim sampai / didapat terlebih dahulu baru send

			//Kirim key[0] sampai key[n-1] bertipe data char dengan komunikasi MPI_COMM_WORLD
			MPI_Ssend(&key[i - 1], 1, MPI_CHAR, i, TAG, MPI_COMM_WORLD);
		}

		// Kirim message, min karaketer (len/keylen) ke sebuah proses
		// Klo message = 10 character dan key = 5 character -> maka 10/5 = 2 character per proses 
		for (int i = 0; i < len; i++)
		{
			cout << "Sending " << str[i] << " to " << (i % keylen) + 1 << endl;

			//Untuk kirim str[i] ke proses (i % keylen)+1 dengan komunikasi MPI_COMM_WORLD
			//Misal: iterasi 0 
			//Kirim str[0] ke proses 1
			MPI_Ssend(&str[i], 1, MPI_CHAR, (i % keylen) + 1, TAG, MPI_COMM_WORLD);

			cout << "Receiving encrypted character: ";
			//Menerima encrypted character yang bertipe data char yang telah dikirim dengan komunikasi MPI_COMM_WORLD 
			//Misal: i=0 
			//Menerima str[0] ke proses 1 dengan komunikasi MPI_COMM_WORLD 
			MPI_Recv(&encrypted_character, 1, MPI_CHAR, (i % keylen) + 1, TAG, MPI_COMM_WORLD, &status);


			cout << "Received encrypted: " << encrypted_character << " in root\n";
			//masukkin encrypted character ke encrypted yang bertipe data array of string
			encrypted.push_back(encrypted_character);
		}
		//cout << "Exiting root process\n";
	}

	//Proses selain root
	else
	{
		//cout << "Inside process-" << id << endl;

		//Menerima karakter key 
		//Proses ke-id menerima karakter key
		MPI_Recv(&key_character, 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &status);
		cout << "Process-" << id << " received " << key_character << endl;

		//!!! Menentukan jumlah proses yang digunakan !!!
		//Membagi data ke proses sebanyak keylen proses
		//Kalau keylen 5 maka process yang digunakan juga 5, klo 8 maka 8

		//Untuk menentukan jumlah data di setiap proses
		//Jika rank <= (len % keylen) -> rank ini memiliki jumlah data yang lebih bnyak drpd rank lainnya
		if (id <= len % keylen)
		{
			//looping sebanyak karakter yang ada di setiap proses, yaitu len/keylen + 1
			for (int i = 0; i < len / keylen + 1; i++)
			{
				cout << "Process-" << id << " is trying to receive " << endl;

				//Menerima karakter yang mau di encrypt 
				MPI_Recv(&character_to_encrypt, 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &status);

				//Masukkin karakter yang mau di enkripsi dan kuncinya ke function vigenere
				encrypted_character = vigenere(character_to_encrypt, key_character);

				//Mengirim karakter enkripsi bertipe data char ke proses 0 dengan menggunakan komunikasi MPI_COMM_WORLD
				MPI_Ssend(&encrypted_character, 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
			}
		}
		// Jika rank > (len % keylen) -> rank ini memiliki jumlah data yang lebih sedikit dari rank lainnya
		else
		{
			//looping sebanyak karakter yang ada di setiap proses
			for (int i = 0; i < len / keylen; i++)
			{
				//cout << "Process-" << id << " is trying to receive " << endl;

				//Menerima karakter untuk enkripsi dengan tipe data char ke proses 0 dengan menggunakan komunikasi MPI_COMM_WORLD
				MPI_Recv(&character_to_encrypt, 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &status);

				//Masukkin karakter yang mau di enkripsi dan kuncinya ke function vigenere
				encrypted_character = vigenere(character_to_encrypt, key_character);

				//Mengirim karakter enkripsi bertipe data char ke proses 0 dengan menggunakan komunikasi MPI_COMM_WORLD
				MPI_Ssend(&encrypted_character, 1, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
			}
		}
		//cout << "Exiting process-" << id << endl;
	}


	//Proses 0 -> menampilkan hasil enkripsi dan waktu eksekusi
	if (rank == 0)
	{
		finish = MPI_Wtime();

		cout << "\nThe encrypted string is: " << encrypted << "\n";

		time = finish - start;
		cout << "Execution Time = " << time;
		cout << endl;
	}
	//Untuk menutup environment MPI 
	MPI_Finalize();
	return 0;
}