/*

clé usb:
Number of possible configurations: 1 
Device Class: 0
VendorID: 1921
ProductID: 21863
Interfaces: 1
Number of alternate settings: 1
Interface Number: 0 Number of endpoints: 2
	Descriptor Type: 5 EP Address: 129
	Descriptor Type: 5 EP Address: 2

Scanner:
Number of possible configurations: 1 
Device Class: 255
VendorID: 1193
ProductID: 6400
Interfaces: 1
Number of alternate settings: 1 
Interface Number: 0 Number of endpoints: 3 
	Descriptor Type: 5 	EP Address: 129
	Descriptor Type: 5 	EP Address: 2
	Descriptor Type: 5 	EP Address: 131

cat /sys/kernel/debug/usb/devices:
T:  Bus=01 Lev=01 Prnt=01 Port=01 Cnt=01 Dev#= 23 Spd=480  MxCh= 0
D:  Ver= 2.00 Cls=ff(vend.) Sub=ff Prot=ff MxPS=64 #Cfgs=  1
P:  Vendor=04a9 ProdID=1900 Rev= 3.07
S:  Manufacturer=Canon
S:  Product=CanoScan
C:* #Ifs= 1 Cfg#= 1 Atr=a0 MxPwr=500mA
I:* If#= 0 Alt= 0 #EPs= 3 Cls=ff(vend.) Sub=ff Prot=ff Driver=(none)
E:  Ad=81(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=02(O) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=83(I) Atr=03(Int.) MxPS=   1 Ivl=16ms

usb.src == "1.23.1" || usb.dst == "1.23.1"
usb.src contains "1.23" || usb.dst contains "1.23"

*/
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scan.h"
#include <unistd.h>

struct libusb_device_handle *dev_handle; //a device handle

int main() {
//	libusb_device **devs = NULL; //pointer to pointer of device, used to retrieve a list of devices
	libusb_context *ctx = NULL; //a libusb session
	int r; //for return values
	//ssize_t cnt; //holding number of devices in list
	r = libusb_init(&ctx); //initialize a library session
	uchar data[20480]; //data to write
//	int len=0;

/*	printf("Starting***********************************************\n");
	if(r < 0) {
		printf("Init Error %d\n",r); //there was an error
				return 1;
	}
	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		printf("Get Device Error\n"); //there was an error
	}
	printf("%d Devices in list.\n\n",(int)cnt); //print total number of usb devices
		ssize_t i; //for iterating through the list
	for(i = 0; i < cnt; i++) {
				printdev(devs[i]); //print specs of this device
		}
    data[0]='a';data[1]='b';data[2]='c';data[3]='d'; //some dummy values
*/

	dev_handle = libusb_open_device_with_vid_pid(ctx, 1193, 6400); //these are vendorID and productID I found for my usb device
    if(dev_handle == NULL)
	{
        	printf("Cannot open device\n");
		return 1;
	}
   else
        printf("Device Opened\n");

//    libusb_free_device_list(devs, 1); //free the list, unref the devices in it


//    int actual; //used to find out how many bytes were written
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }

    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) {
        printf("Cannot Claim Interface\n");
        return 1;
    }
    printf("Claimed Interface\n");
    printf("Writing Data...\n");

	uchar STR1[1]={0x41};
	ControlOut8385(STR1,1);
	ControlIn84();

	uchar STR2[3]={0xe,0x1,0x41};
	ControlOut8385(STR2,3);
	ControlIn84();

	uchar STR3[1]={0x6b};
	ControlOut8385(STR3,1);
	ControlIn84();

	uchar STR4[13]={0x6b,0,0x6c,0,0x6D,0,0X6E, 0XFF, 0X6F, 0XE0, 0X6, 0X10, 0X6D};
	ControlOut8385(STR4,13);
	ControlIn84();

	uchar STR5[3]={0x6D,0x9F,0x6B};
	ControlOut8385(STR5,3);
	ControlIn84();

	uchar STR6[3]={0x6B,0x02,0x6B};
	ControlOut8385(STR6,3);
	ControlIn84();

	uchar STR7[3]={0x6B,0x03,0x6C};
	ControlOut8385(STR7,3);
	ControlIn84();

	uchar STR8[3]={0x6C,0x02,0xD4};
	ControlOut8385(STR8,3);

	uchar STR9[1]={0xD4};
	ControlOut8c(STR9,1);

	ControlOutList83(STR10,64);
	ControlOutList83(STR11,64);
	ControlOutList83(STR12,64);
	ControlOutList83(STR13,20);

	uchar STR14[44]={0x10,0x06,0x11,0x13,0x12,0x55,0x13,0x02
					,0x14,0x34,0x15,0x04,0x30,0x06,0x31,0x13
					,0x32,0x55,0x33,0x02,0X34,0x34,0x35,0x04
					,0x36,0xB0,0x37,0xC2,0x38,0x5A,0x39,0xF0
					,0x60,0x06,0x61,0x13,0x62,0x55,0x63,0x02
					,0x64,0x34,0x65,0x04};

	for(int i=0; i<43; i+=2)
	{
		data[0]=STR14[i];
		data[1]=STR14[i+1];
		data[2]=STR14[i];
		ControlOut8385(data,3);
		ControlIn84();
	}

	uchar STR15[5]={0x2A,0x00,0x2B,0,0x3C};
	ControlOut8385(STR15,5);

	uchar STR16[8]={0x01,0x00,0x82,0x00,0x00,0x02,0x00,0x00};
	for(int i=0; i<521; i++)
	{
		ControlOutList82(STR16,8);
		IntCheck();
		BulkOut(STRU,512);
		usleep(8000);
	}

	uchar STR16a[5]={0x2A,0x00,0x2B,0,0x45};
	ControlOut8385(STR16a,5);

	uchar STR17[8]={0x00,0x00,0x82,0x00,0x02,0x00,0x04,0x00};
	ControlOutList82(STR17,8);

	for(int i=0; i<4; i++)
	{
		BulkIn(data,19968);
		BulkIn(data,16384);
		BulkIn(data,16384);
		BulkIn(data,12800);
	}
		printf("dddd %d\n",BulkIn(data,512));

	uchar STR18[5]={0x2A,0x00,0x2B,0,0x3C};
	ControlOut8385(STR18,5);

	for(int i=0; i<521; i++)
	{
		ControlOutList82(STR16,8);
		IntCheck();
		BulkOut(STRA,512);
		usleep(8000);
	}

	ControlOut8385(STR16a,5);

	ControlOutList82(STR17,8);

	for(int i=0; i<4; i++)
	{
		BulkIn(data,19968);
		BulkIn(data,16384);
		BulkIn(data,16384);
		BulkIn(data,12800);
	}
		printf("dddd %d\n",BulkIn(data,512));

	uchar STR19[5]={0x5B,0x00,0x5C,0,0x28};
	ControlOut8385(STR19,5);

	for(int i=0; i<3; i++)
	{
		ControlOutList82(STR16,8);
		IntCheck();
		BulkOut(STRU,512);
		usleep(8000);
	}

	uchar STR20[5]={0x5B,0x00,0x5C,0,0x4E};
	ControlOut8385(STR20,5);
//ligne 6856




//requette Interrupt sur 128 recup 1 octet
//	int out;
//    r = libusb_interrupt_transfer(dev_handle, 131, data, 1, &out,0);
//	printf("Message de sortie: %s\n",libusb_strerror(r));
//	printf("Touche appuyé: %d\n",data[0]);














    r = libusb_release_interface(dev_handle, 1); //release the claimed interface
    if(r!=0) {
        printf("Cannot Release Interface\n");
        return 1;
    }
    printf("Released Interface\n");

    libusb_close(dev_handle); //close the device we opened

		libusb_exit(ctx); //close the session
		return 0;
}

void Printdev(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("failed to get device descriptor\n");
		return;
	}
	printf("Number of possible configurations: %d \n",(int)desc.bNumConfigurations);
	printf("Device Class: %d\n",(int)desc.bDeviceClass);
	printf("VendorID: %d\n",desc.idVendor);
	printf("ProductID: %d\n",desc.idProduct);
	struct libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Interfaces: %d\n",(int)config->bNumInterfaces);
	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;
	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		printf("Number of alternate settings: %d \n",inter->num_altsetting);
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			printf("Interface Number: %u ",(unsigned char)interdesc->bInterfaceNumber);
			printf("Number of endpoints: %d \n",(int)interdesc->bNumEndpoints);
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				printf("	Descriptor Type: %d ",(int)epdesc->bDescriptorType);
				printf("	EP Address: %d\n",(int)epdesc->bEndpointAddress);
			}
		}
	}
	printf("\n");
	libusb_free_config_descriptor(config);
}

//envoi sur 83 et 85 en alternance
void ControlOut8385(uchar *str, uchar lng)
{
	int r, j=0;
	for(char i=0; i<lng; i++)
	{
		j=(i%2)*2;//pour alterner entre 83 et 85

		//requette control_out vers hote
		r = libusb_control_transfer(dev_handle, 0x40, 0x0c, 0x83+j, 0, (str+i), 1, 0);
		printf("Résultat requete: %s\n",libusb_strerror(r));
	}
}

//envoi sur 82
void ControlOutList82(uchar *str, uchar lng)
{
	int r;
	//requette control_out vers hote
	r = libusb_control_transfer(dev_handle, 0x40, 0x04, 0x82, 0, str, lng, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
}

//envoi sur 83
void ControlOutList83(uchar *str, uchar lng)
{
	int r;
	//requette control_out vers hote
	r = libusb_control_transfer(dev_handle, 0x40, 0x04, 0x83, 0, str, lng, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
}

//reception de 84
uchar ControlIn84(void)
{
	int r;
	uchar str[1];
	//requette control_in depuis scan
	r = libusb_control_transfer(dev_handle, 0xc0, 0x0c, 0x84, 0, str, 1, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
	return r;
}

//envoi sur 8c
void ControlOut8c(uchar *str, uchar lng)
{
	int r;
	//requette control_out vers hote
	r = libusb_control_transfer(dev_handle, 0x40, 0x0c, 0x8c, 0x10, str, lng, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
}

uchar IntCheck()
{
	int r;
	uchar data=0;

	//requette control_in vers scan
    r = libusb_control_transfer(dev_handle, 0xc0, 0x0c, 0x8e, 0x20, &data, 1, 0);
	//handle, brequestType, bRequest, wvalue, wIndex, *data, wLengh, timeout
	printf("Résultat requete: %s\n",libusb_strerror(r));
	printf("Message de sortie: 0x%X\n",data);

	return data;
}

//reception d'un paquet du scan
int BulkIn(uchar *str, uint lng)
{
	int r, len=0;
	//requette bulk transfer_out vers scan
    r = libusb_bulk_transfer(dev_handle, 0x81, str, lng, &len, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
	printf("Octets recus: %d\n",len);
	return lng;
}

//envoi d'un paquet du scan
int BulkOut(uchar *str, uint lng)
{
	int r, len=0;
	//requette bulk transfer_out vers scan
    r = libusb_bulk_transfer(dev_handle, 0x2, str, lng, &len, 0);
	printf("Résultat requete: %s\n",libusb_strerror(r));
	printf("Octets envoyés: %d\n",len);
	return lng;
}
