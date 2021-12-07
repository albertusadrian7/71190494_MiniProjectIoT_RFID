/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 */
// Potong Saldo
// Nama: Albertus Adrian S
// NIM: 71190494
// MiniProject 6 RFID

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22          // RST
#define SS_PIN          21          // SDA

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

bool valid = true;
long tagihan = 5000;
int digitTagihan = tagihan/1000;

long saldo; //Nominal ribuan (untuk tampil di serial monitor)
int digit; //Nominal untuk disimpan di RFID (0-255)

long saldoLama; //Nominal ribuan (untuk tampil di serial monitor)
int digitLama; //Nominal untuk disimpan di RFID (0-255)

/**
 * Initialize.
 */
void setup() {
  Serial.begin(115200); // Initialize serial communications with the PC
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Mempersiapkan key sebanyak 6 byte
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  Serial.println();
  Serial.println("e-TOLL: Potong Saldo");
  Serial.println();
  Serial.println("========== Selamat Datang di Gerbang Tol ==========");
  Serial.println("Gerbang TOL akan terbuka setelah Anda melakukan");
  Serial.println("Pembayaran sebesar Rp5.000,00");
  Serial.println();
}

/**
 * Main loop.
 */
void loop() {
  if(valid){
    valid = false;
    Serial.println();
    Serial.println("================================================");
    Serial.println("Silahkan tap kartu");
    Serial.print("Setiap TAP, saldo berkurang sebesar ");
    Serial.println(tagihan);
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }
  
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }

  // Mengetahui UID dari Kartu
  Serial.println();
  Serial.print("Card UID:");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  
  Serial.println();
  Serial.print("Tipe Kartu : ");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Mengecek kesesuaian kartu
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
 
  // Start of read: Baca Saldo yang ada pada RFID Card
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.println("Gagal membaca kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
      resetReader();
      return;
  }
  
  digitLama = buffer[0];
  saldoLama = digitLama*1000;
  Serial.print("Saldo Kartu Anda: ");
  Serial.println(saldoLama);
  delay(2000);
  Serial.println();
  // End of Read: Baca Saldo yang ada pada RFID Card

  // Start of Write: Mengurangi saldo yang ada pada Kartu RFID
  // Kurangi Saldo sebesar tagihan merchant
  if (digitLama < digitTagihan){
    Serial.println("Saldo Anda tidak cukup, silahkan top up saldo terlebih dahulu");
    Serial.println("PEMBAYARAN GAGAL");
    delay(2000); // Mencegah double tap
    Serial.println();
    resetReader();
    return;
  }
  
  digitLama -= digitTagihan;
  byte dataBlock[]    = {
      //0,      1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
      digitLama, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
      Serial.println("Gagal Write Saldo pada Kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.println("Gagal membaca Kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
  }

  Serial.println();
  Serial.println("Mengurangi Saldo...");
  Serial.println();
  if (buffer[0] == dataBlock[0]){
    digit = buffer[0];
    saldo = digit*1000;
    //Serial.print("data digit ke 0 : ");
    //Serial.println(buffer[0]);
    Serial.println("------------ PEMBAYARAN SUKSES --------------");
    Serial.print("======================>>>>>> Saldo kartu sekarang : ");
    Serial.println(saldo);
    delay(2000);
    Serial.println();
    Serial.println();
  }else{
    Serial.println("------------ PEMBAYARAN GAGAL --------------");
  }
  // End of Write: Mengurangi saldo yang ada pada Kartu RFID

  Serial.println();

  // Dump the sector data
  //Serial.println("Current data in sector:");
  //mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  resetReader();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
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
}
