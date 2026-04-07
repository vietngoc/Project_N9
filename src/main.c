#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include <time.h> 

enum Status {TURN_ON, TURN_OFF};
//Kiểu dữ liệu mô tả trạng thái
enum BaterryRequire {HIGH, MEDIUM, LOW, EMPTY};
//Kiểu dữ liệu mức pin

typedef struct {
   char subName[50];
   enum Status status;
   enum BaterryRequire baterryRequire;
} SubModule;
//Kiểu mô tả các chức năng nhỏ của thiết bị( tên chức năng, trạng thái hoạt động, mức pin)

typedef struct {
   char name[50];
   enum Status status;
   int limitPower;
   int powerPerHour;
   int remainPower;
   SubModule listSubmodules[3];
} Thietbi;
//Kiểu mô tả thiết bị( tên, trạng thái hoạt động,mức pin, năng lượng tiêu hao mỗi giờ, năng lượng còn lại) 

// hàm ghi log ra file
void writeLog(const char *message){
    FILE *f = fopen("system_log.txt","a"); // mở file log ở chế độ append
    if(f==NULL) return; // nếu lỗi thì bỏ qua

    time_t now = time(NULL); // lấy thời gian hiện tại
    char *t = ctime(&now);   // chuyển sang chuỗi
    t[strlen(t)-1] = '\0';   // xóa ký tự xuống dòng

    fprintf(f,"[%s] %s\n",t,message); // ghi log kèm timestamp
    fclose(f); // đóng file
}

void noteTimeRequire(float maxTime) {
   FILE *noteTimeRequire = fopen("note_time_require.txt", "wb");
   if (noteTimeRequire == NULL) {
       printf("Unable to identify list device!");
       return;
   }
   fwrite(&maxTime, sizeof(float), 1, noteTimeRequire);
   fclose(noteTimeRequire);
}
// ghi giá trị maxTime (thời gian chạy lâu nhất) vào file note_time_require.txt

float getTimeRequire() {

   float timeRequire;
   FILE *noteTimeRequire = fopen("note_time_require.txt", "rb");
   if (noteTimeRequire == NULL) {
       printf("Unable to identify list device!");
       return 0;
   }
   fread(&timeRequire, sizeof(float), 1, noteTimeRequire);
   fclose(noteTimeRequire);
   return timeRequire;
}
// Lấy lại thời gian chạy tối đa (maxTime) từ file để dùng cho hệ thống chạy.

void runProcess() {
    
    writeLog("Nguoi dung bat dau he thong"); // log khi user chạy
    showAllDevice();

    char request[1];
    printf("Ban co muon bat dau chay tat ca cac thiet bi? (Y/N) ");
    scanf("%s",&request);

    if (request[0] == 'Y' || request[0] == 'y') {
       while (getchar() != '\n');
            writeLog("Nguoi dung khoi dong tat ca thiet bi"); // log khi user chạy
            runAllDevice();
    } else {
        printf("He thong chua duoc khoi dong! ");
        writeLog("Nguoi dung ket thuc he thong"); // log khi user tắt}
    }
}

void showAllDevice() {
    
   Thietbi thietbidangchay;
   FILE *listDeviceRunning = fopen("../test/listDevices.txt", "rb+");
   if (listDeviceRunning == NULL) {
       printf("Khởi chạy hệ thống thất bại");
   }
   while (fread(&thietbidangchay, sizeof(Thietbi), 1, listDeviceRunning) == 1) {

       thietbidangchay.remainPower = thietbidangchay.remainPower - thietbidangchay.powerPerHour;
       fseek(listDeviceRunning, -sizeof(Thietbi), SEEK_CUR);
       fwrite(&thietbidangchay, sizeof(Thietbi), 1, listDeviceRunning);
       fseek(listDeviceRunning, 0, SEEK_CUR);

       int ratioPower = thietbidangchay.remainPower * 100/thietbidangchay.limitPower;

       char msg[120]; 

       int pinStatus = 0; // HIGHT
       if (ratioPower > 80) {
           pinStatus = HIGH;
           printf("Thiet bi %s dang chay day du chuc nang (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: HIGH (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else if (ratioPower <= 80 && ratioPower > 50) {
           pinStatus = MEDIUM;
           printf("Thiet bi %s con muc pin dam bao hoat dong tot (%d %c).\n", thietbidangchay.name, ratioPower,'%');

           sprintf(msg,"%s: MEDIUM (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else if (ratioPower <= 50 && ratioPower > 20) {
           pinStatus = LOW;
           printf("Thiet bi %s dang co muc pin thap (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: LOW (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else {
           printf("Thiet bi %s da tat.\n", thietbidangchay.name);

           sprintf(msg,"%s: OFF",thietbidangchay.name); 
           writeLog(msg);
       }
   }
}

void showBatteryDevice() {

    printf("--- DANH SACH THIET BI DUNG PIN ---\n");
    Thietbi thietbidangchay;
    FILE *listDeviceRunning = fopen("../test/listDevices.txt", "rb");
    if (listDeviceRunning == NULL) {
       printf("Khởi chạy hệ thống thất bại");
    }
    const char* FORMAT = "%-20s %-20s %-20s\n";
    printf(FORMAT, "Thiet bi", "Trang thai hoat dong", "Muc pin con lai (%)");
    printf("--------------------------------------------\n");

    while (fread(&thietbidangchay, sizeof(Thietbi), 1, listDeviceRunning) == 1) {
        if (thietbidangchay.status == TURN_ON) {
            int ratioPower = thietbidangchay.remainPower * 100/thietbidangchay.limitPower;
            char stt[10];
            switch (thietbidangchay.status) {
               case TURN_OFF:
                   strcpy(stt, "TURN_OFF");
                   break;
               case TURN_ON:
                   strcpy(stt, "TURN_ON");
                   break;
           }
           printf(FORMAT, thietbidangchay.name, stt, ratioPower);
       }
   }
   fclose(listDeviceRunning);
}

void showDetailDevice() {

    Thietbi thietbi;
    FILE *listDevice = fopen("../test/listDevices.txt", "rb");
    const char* FORMAT = "%-20s %-20s %-20s %-20s\n";
    const char* FORMATNUMBER = "%-20s %-20s %-20d %-20d\n";
    printf("--- THONG TIN THIET BI DA LUU ---\n");
    printf(FORMAT, "Thiet bi", "Trang thai hoat dong", "Han muc tieu thu (kWh)", "Cong suat moi gio (kW)");
    printf("--------------------------------------------\n");

    while (fread(&thietbi, sizeof(Thietbi), 1, listDevice) == 1) {
        enum Status status = thietbi.status;
        char stt[10];
        switch (status) {
            case TURN_OFF:
               strcpy(stt, "TURN_OFF");
               break;
           case TURN_ON:
               strcpy(stt, "TURN_ON");
               break;
       }

       printf(FORMATNUMBER, thietbi.name, stt, thietbi.limitPower, thietbi.powerPerHour);
   }
   fclose(listDevice);
}

void importDevice() {
    FILE *listDevice = fopen("../test/listDevices.txt", "w+");
    if (listDevice == NULL) {
       printf("Khong the tai danh sach thiet bị da luu !");
       return;
    }

    writeLog("Khoi tao danh sach thiet bi");

    Thietbi list[5];
    //danh sách gồm 5 thiết bị 
    list[0] = (Thietbi) {
       "Tu lanh", TURN_OFF, 1000, 100,1000,
       {
           {"Lanh nhanh", TURN_ON, MEDIUM},
           {"Khang khuan", TURN_ON, LOW }
           }
    };

    list[1] = (Thietbi) {
       "Tivi", TURN_OFF, 2000, 150,2000,
       {
           {"Retina", TURN_ON, MEDIUM}
           }
    };
    list[2] = (Thietbi) {
       "May giat", TURN_ON, 1500, 150,1500,
       {
           {"Giat ngam", TURN_ON, LOW},
           {"Tu dong say sau khi giat", TURN_ON, EMPTY}
           }
    };
    list[3] = (Thietbi) {
       "Dieu hoa", TURN_OFF, 1300, 110,1300,
       {
           {"Cap am", TURN_ON, MEDIUM},
           {"Diệt khuẩn không khí", LOW},
           {"Canh bao o nhiem", EMPTY}
           }
    };
    list[4] = (Thietbi) {
       "Bep dien", TURN_OFF, 1800, 200,1800,
       {
           {"Sieu toc", TURN_ON, LOW}
       }
    };
// định nghĩa các thiết bị 

    float maxTime = 0; //Thời gian chạy toàn hệ thống( thiết bị chạy lâu nhất)
    for (int i = 0; i < 5; i++) {
       fwrite(&list[i], sizeof(Thietbi), 1, listDevice);

       char msg[100]; 
       sprintf(msg,"Them thiet bi: %s",list[i].name); 
       writeLog(msg); // ghi log thêm thiết bị

       float runTimeRequire = list[i].limitPower/list[i].powerPerHour;
       if (runTimeRequire > maxTime) {
           maxTime = runTimeRequire;
       }
    }
    noteTimeRequire(maxTime);

    rewind(listDevice);
    Thietbi thietbi;

    const char* FORMAT = "%-20s %-20s %-20s %-20s\n";
    const char* FORMATNUMBER = "%-20s %-20s %-20d %-20d\n";
    printf("--- DANH SACH THIET BI DA LUU ---\n");
    printf(FORMAT, "Thiet bi", "Trang thai hoat dong", "Han muc tieu thu (kWh)", "Cong suat moi gio (kW)");
    printf("--------------------------------------------\n");

    while (fread(&thietbi, sizeof(Thietbi), 1, listDevice) == 1) {
       enum Status status = thietbi.status;
       char stt[10];
       switch (status) {
           case TURN_OFF:
               strcpy(stt, "TURN_OFF");
               break;
           case TURN_ON:
               strcpy(stt, "TURN_ON");
               break;
       }

       printf(FORMATNUMBER, thietbi.name, stt, thietbi.limitPower, thietbi.powerPerHour);
   }
   fclose(listDevice);
}

void showCurrentDevice() {

    printf("\n======= CHI TIET THIET BI =======\n");
   
    Thietbi thietbidangchay;
    FILE *listDeviceRunning = fopen("../test/listDevices.txt", "rb+");
    if (listDeviceRunning == NULL) {
        printf("Khởi chạy hệ thống thất bại");
    }
    while (fread(&thietbidangchay, sizeof(Thietbi), 1, listDeviceRunning) == 1) {

        thietbidangchay.remainPower = thietbidangchay.remainPower - thietbidangchay.powerPerHour;
        fseek(listDeviceRunning, -sizeof(Thietbi), SEEK_CUR);
        fwrite(&thietbidangchay, sizeof(Thietbi), 1, listDeviceRunning);
        fseek(listDeviceRunning, 0, SEEK_CUR);

        int ratioPower = thietbidangchay.remainPower * 100/thietbidangchay.limitPower;

        char msg[120]; 

        int pinStatus = 0; // HIGHT
        if (ratioPower > 80) {
           pinStatus = HIGH;
           printf("Thiet bi %s dang chay day du chuc nang (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: HIGH (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

        } else if (ratioPower <= 80 && ratioPower > 50) {
           pinStatus = MEDIUM;
           printf("Thiet bi %s con muc pin dam bao hoat dong tot (%d %c).\n", thietbidangchay.name, ratioPower,'%');

           sprintf(msg,"%s: MEDIUM (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

        } else if (ratioPower <= 50 && ratioPower > 20) {
           pinStatus = LOW;
           printf("Thiet bi %s dang co muc pin thap (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: LOW (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

        } else {
           printf("Thiet bi %s da tat.\n", thietbidangchay.name);

           sprintf(msg,"%s: OFF",thietbidangchay.name); 
           writeLog(msg);
        }
   }
}

void runAllDevice() {
    
   Thietbi batThietbi;
   FILE *listDeviceRunning = fopen("../test/listDevices.txt", "rb+");
   if (listDeviceRunning == NULL) {
       printf("Khoi chay he thong that bai!");
   }

   writeLog("Khoi dong tat ca thiet bi"); // log bật hệ thống

    //bật trạng thái TURN_ON cho tất cả các thiết bị
    while (fread(&batThietbi, sizeof(Thietbi), 1, listDeviceRunning) == 1) {
       if (batThietbi.status == TURN_OFF) {
           batThietbi.status = TURN_ON;
       }

       char msg[100]; 
       sprintf(msg,"Bat thiet bi: %s",batThietbi.name); 
       writeLog(msg); // log bật từng thiết bị

       fseek(listDeviceRunning, -sizeof(Thietbi), SEEK_CUR);
       fwrite(&batThietbi, sizeof(Thietbi), 1, listDeviceRunning);
       fseek(listDeviceRunning, 0, SEEK_CUR);
   }
   fclose(listDeviceRunning);

   int currentTime = 0;
   int timeRequire = (int) getTimeRequire();
   while (currentTime < timeRequire) {
       sleep(10);
       showCurrentDevice();
       currentTime++;
       printf("--------------------------------------------\n");
   }

}

void displayReport() {

    printf("\n======= BAO CAO TONG HOP =======\n");
    float timeRequire;
    FILE *noteTimeRequire = fopen("note_time_require.txt", "rb");
    if (noteTimeRequire == NULL) {
       printf("Unable to identify list device!");
    }

    fread(&timeRequire, sizeof(float), 1, noteTimeRequire);
    printf("Thoi gian chay toan bo he thong: %.2f gio\n", timeRequire);

    fclose(noteTimeRequire);
}

int main() {
    
    int lua_chon;
    do {
        printf("\n======= CHUONG TRINH QUAN LY NANG LUONG =======\n");
        printf("1. Danh sach thiet bi dang chay\n");
        printf("2. Danh sach thiet bi dung pin\n");
        printf("3. Chi tiet thiet bi\n");
        printf("4. Them moi thiet bi\n");
        printf("5. Hien thi bao cap\n");
        printf("0. Thoat\n");
        printf("-----------------------------------------------\n");
        printf("Nhap lua chon: ");
        scanf("%d", &lua_chon);

        switch (lua_chon) {
            case 1: runProcess(); break;
            case 2: showBatteryDevice(); break;
            case 3: showDetailDevice(); break;
            case 4: importDevice(); break;
            case 5: displayReport(); break;
            case 0: printf("Tam biet!\n"); break;
            default: printf("Lua chon sai!\n");
        }
    } while (lua_chon != 0);

    return 0;
}