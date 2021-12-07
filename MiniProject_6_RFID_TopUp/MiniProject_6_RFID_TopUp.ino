/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 */

// Top Up Saldo
// Nama: Albertus Adrian
// NIM: 71190494
// MiniProject 6 RFID

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22         // RST
#define SS_PIN          21         // SDA

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

bool valid = true;
bool isiSaldo = false;
String input;

long saldo; //Nominal ribuan (untuk tampil di serial monitor)
int digit; //Nominal untuk disimpan di RFID (0-255)

long saldoLama; //Nominal ribuan (untuk tampil di serial monitor)
int digitLama; //Nominal untuk disimpan di RFID (0-255)

void setup() {
    Serial.begin(115200); // Initialize serial communications with the PC
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Mempersiapkan key sebanyak 6 byte
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println();
    Serial.println("e-TOLL: Top Up Saldo");
    Serial.println();
    Serial.println("========== Selamat Datang di Menu Top Saldo e-Toll ==========");
    Serial.println();
}

void loop() {
  // Cek apakah di buffer serial masih ada data
  if (Serial.available()){
    isiSaldo = true;
    input = "";
    
    // Selama di buffer serial masih ada data,
    // maka data tersebut akan ditampung dalam variabel input
    while(Serial.available()>0){
      input += char(Serial.read());
    }

    if (valid){
      valid = false;
      Serial.println();
      Serial.println("==================================================");
      Serial.println("Silahkan input jumlah saldo dan tap kartu");
      Serial.println("Contoh input: 5000, Saldo = Rp5.000,00 || Max saldo: Rp255.000,00");
      Serial.println();
    }
    
    saldo = input.toInt();
    digit = saldo/1000;
    if (digit > 255){
      saldo = 0;
      Serial.println("Saldo tidak boleh lebih dari 255000");
    }
    if (digit < 0){
      saldo = 0;
      Serial.println("Saldo tidak boleh kurang dari 0");
    }
    
    Serial.print("Nominal Top Up : ");
    Serial.println(saldo);
    Serial.println("Silahkan tap kartu untuk menambah saldo!");
  }
  
  if ( ! mfrc522.PICC_IsNewCardPresent()){
      return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()){
      return;
  }
  
  Serial.println();
  // Mendapatkan UID dari kartu
  Serial.print("Card UID:");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  
  Serial.println();
  Serial.print("Tipe Kartu : ");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Cek kesesuaian kartu
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Anda dimohon untuk menggunakan MIFARE Classic cards!"));
      delay(2000);
      resetReader();
      return;
  }

  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;
  
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Show the whole sector as it currently is
  //Serial.println("Current data in sector:");
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  if (isiSaldo){
    // Baca Saldo yang ada pada RFID Card
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("Gagal membaca kartu RFID");
        //Serial.println(mfrc522.GetStatusCodeName(status));
        resetReader();
        return;
    }
    digitLama = buffer[0];
    saldoLama = digitLama*1000;
    Serial.print("Saldo Kartu Sebelum Top Up : ");
    Serial.println(saldoLama);
    Serial.println();
  
    // Proses Write Saldo pada RFID Card untuk Tambah Saldo
    saldo += saldoLama; // 5000 + 2000 = 7000
    digit += digitLama; // 5 + 2 = 7
     
    byte dataBlock[]    = {
        //0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
        digit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    // Menulis saldo pada kartu RFID
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("GAGAL Write Saldo pada Kartu RFID");
        //Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    // Membaca ulang isi saldo di kartu
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("Gagal Baca Kartu RFID");
        //Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();
    
    Serial.println("Menambahkan Saldo...");
    if (buffer[0] == dataBlock[0]){
      Serial.print("Saldo kartu sekarang : ");
      Serial.println(saldo);
      Serial.println("------------ TOP UP SALDO SUKSES --------------");
    }else{
      Serial.println("------------ TOP UP SALDO GAGAL --------------");
    }
  } else {
    Serial.println("Silakan masukan input terlebih dahulu pada serial monitor!");
  }

  saldo = 0;
  digit = 0;

  Serial.println();

  // Dump the sector data
  //Serial.println("Current data in sector:");
  //mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  resetReader();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void resetReader(){
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  valid = true;
  isiSaldo = false;
}
