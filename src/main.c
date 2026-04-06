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

int main(void) {
   importDevice();
   return 0;
}

void importDevice() {
   FILE *listDevice = fopen("listDevices.txt", "w+");
   if (listDevice == NULL) {
       printf("UKhong the xac dinh thiet !");
       return;
   }
// Không mở được file : in ra thông báo 

   writeLog("Khoi tao danh sach thiet bi"); // log khi bắt đầu import

   Thietbi list[5];
//danh sách gồm 5 thiết bị 
   list[0] = (Thietbi) {
       "Tủ lạnh", TURN_OFF, 1000, 100,1000,
       {
           {"Làm lạnh nhanh", TURN_ON, MEDIUM},
           {"Kháng khuẩn", TURN_ON, LOW }
           }
   };

   list[1] = (Thietbi) {
       "Tivi", TURN_OFF, 2000, 150,2000,
       {
           {"Retina", TURN_ON, MEDIUM}
           }
   };
   list[2] = (Thietbi) {
       "Máy giặt", TURN_ON, 1500, 150,1500,
       {
           {"Giặt ngâm", TURN_ON, LOW},
           {"Tự động sấy sau khi giặt", TURN_ON, EMPTY}
           }
   };
   list[3] = (Thietbi) {
       "Điều hoà", TURN_OFF, 1300, 110,1300,
       {
           {"Cấp ẩm", TURN_ON, MEDIUM},
           {"Diệt khuẩn không khí", LOW},
           {"Cảnh báo ô nhiễm", EMPTY}
           }
   };
   list[4] = (Thietbi) {
       "Bếp điện ", TURN_OFF, 1800, 200,1800,
       {
           {"Nấu siêu tốc", TURN_ON, LOW}
       }
   };
// định nghĩa các thiết bị 

   float maxTime = 0;//Thời gian chạy toàn hệ thống( thiết bị chạy lâu nhất)
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
   printf("--- DANH SÁCH THIẾT BỊ ĐÃ LƯU TRỮ ---\n");
   printf(FORMAT, "Tên sản phẩm", "Trạng thái hoạt động", "Hạn mức tiêu thụ(kW)", "Lượng tiêu thụ mỗi giờ(kWh)");
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
//Đọc từng thiết bị từ file, chuyển trạng thái enum → chuỗi,in ra danh sách thiết bị,đóng file

   runProcess();
//Bắt đầu chương trình chính (runProcess)

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
//ghi giá trị maxTime (thời gian chạy lâu nhất) vào file note_time_require.txt

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
//Lấy lại thời gian chạy tối đa (maxTime) từ file để dùng cho hệ thống chạy.

void runProcess() {
   char request[1];
   printf("Bạn có muốn bắt đầu chạy tất cả các thiết bị không? (Y/N)\n");
   scanf("%s",&request);

   writeLog("Nguoi dung bat dau he thong"); // log khi user chạy

   while (request[0] == 'Y' || request[0] == 'y') {
       while (getchar() != '\n');
       runAllDevice();
   }
}

void runAllDevice() {
   Thietbi batThietbi;
   FILE *listDeviceRunning = fopen("listDevices.txt", "rb+");
   if (listDeviceRunning == NULL) {
       printf("Khởi chạy hệ thống thất bại");
   }

   writeLog("Bat tat ca thiet bi"); // log bật hệ thống

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
       checkAllDevice();
       currentTime++;
       printf("--------------------------------------------\n");
   }

}

void checkAllDevice() {
   Thietbi thietbidangchay;
   FILE *listDeviceRunning = fopen("listDevices.txt", "rb+");
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
           printf("Thiết bị %s đang chạy đầy đủ chức năng (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: HIGH (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else if (ratioPower <= 80 && ratioPower > 50) {
           pinStatus = MEDIUM;
           printf("Thiết bị %s còn mức pin đảm bảo hoạt động tốt (%d %c).\n", thietbidangchay.name, ratioPower,'%');

           sprintf(msg,"%s: MEDIUM (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else if (ratioPower <= 50 && ratioPower > 20) {
           pinStatus = LOW;
           printf("Thiết bị %s đang có mức pin thấp (%d %c).\n", thietbidangchay.name, ratioPower, '%');

           sprintf(msg,"%s: LOW (%d%%)",thietbidangchay.name,ratioPower); 
           writeLog(msg);

       } else {
           printf("Thiết bị %s đã tắt.\n", thietbidangchay.name);

           sprintf(msg,"%s: OFF",thietbidangchay.name); 
           writeLog(msg);
       }
   }
}